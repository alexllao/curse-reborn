#include "partidas.hpp"
#include "registro.hpp"
#include "../../motor/estado/partidas.hpp"
#include <cstdint>
#include <cstdio>

// Lee el perfil del usuario; la variable de pruebas permite aislarlo dentro de trabajo/.
void PartidasSDL::preparar() {
  if (!ruta.empty())
    return;
  const auto *alternativa = SDL_getenv("CURSE_REBORN_PARTIDAS");
  if (alternativa && *alternativa)
    ruta = alternativa;
  else {
    char *carpeta = SDL_GetPrefPath("AlexLlao", "CurseReborn");
    if (!carpeta) {
      mensaje = "No se puede abrir el perfil";
      return;
    }
    ruta = std::string(carpeta) + "CURSE.SAV";
    SDL_free(carpeta);
  }
  archivo.assign(motor::bytes_archivo_partidas, 0);
  SDL_PathInfo informacion{};
  if (SDL_GetPathInfo(ruta.c_str(), &informacion)) {
    size_t longitud = 0;
    auto *bytes = static_cast<unsigned char *>(SDL_LoadFile(ruta.c_str(), &longitud));
    if (!bytes ||
        (longitud && (longitud > motor::bytes_archivo_partidas ||
                      !motor::validar_partidas(bytes, static_cast<unsigned int>(longitud))))) {
      SDL_free(bytes);
      mensaje = "Archivo de partidas no valido";
      registrar_accion("Partidas: archivo no válido; se conserva sin modificar: %s", ruta.c_str());
      return;
    }
    for (size_t i = 0; i < longitud; ++i)
      archivo[i] = bytes[i];
    SDL_free(bytes);
  }
  disponible = true;
  registrar_accion("Archivo de partidas: %s", ruta.c_str());
}

// La copia anterior permite recuperar el perfil si falla la sustitución del archivo temporal.
bool PartidasSDL::persistir() {
  size_t longitud = 0;
  void *anterior = SDL_LoadFile(ruta.c_str(), &longitud);
  if (anterior) {
    const bool copia = SDL_SaveFile((ruta + ".anterior").c_str(), anterior, longitud);
    SDL_free(anterior);
    if (!copia)
      return false;
  }
  return SDL_SaveFile((ruta + ".temporal").c_str(), archivo.data(), archivo.size()) &&
         SDL_RenamePath((ruta + ".temporal").c_str(), ruta.c_str());
}

// Conserva nombres DOS como bytes; el editor nuevo acepta los caracteres ASCII del juego.
void PartidasSDL::seleccionar(unsigned int ranura) {
  seleccion = ranura;
  confirmacion = false;
  char texto[motor::caracteres_nombre_partida + 1]{};
  nombre = motor::nombre_partida(archivo.data(), static_cast<unsigned int>(archivo.size()),
                                 seleccion, texto)
               ? texto
               : "Partida " + std::to_string(seleccion + 1);
  while (!nombre.empty() && nombre.back() == ' ')
    nombre.pop_back();
  for (auto &c : nombre)
    if (static_cast<unsigned char>(c) < 32 || static_cast<unsigned char>(c) > 126)
      c = '?';
  registrar_accion("Partidas: seleccionada ranura %u", seleccion + 1);
}

// Inicia la edición únicamente al guardar y conserva el juego detenido detrás del panel.
void PartidasSDL::abrir(unsigned int accion, SDL_Window *ventana, unsigned short puntuacion) {
  if (accion != 4)
    preparar();
  puntos = puntuacion;
  modo = accion;
  seleccion = 0;
  confirmacion = false;
  if (disponible) {
    mensaje.clear();
    seleccionar(0);
  }
  if (!evento_importacion)
    evento_importacion = SDL_RegisterEvents(1);
  if (modo == 2)
    SDL_StartTextInput(ventana);
}

// Devuelve la entrada al juego sin que Escape se propague al menú de acciones.
void PartidasSDL::cerrar(SDL_Window *ventana) {
  SDL_StopTextInput(ventana);
  modo = 0;
  confirmacion = false;
}

// Conserva una ruta independiente del selector y evita acceder a objetos desde su hilo de retorno.
void SDLCALL PartidasSDL::recibir_archivo(void *contexto, const char *const *archivos, int) {
  if (!archivos || !archivos[0])
    return;
  SDL_Event evento{};
  evento.type = static_cast<Uint32>(reinterpret_cast<std::uintptr_t>(contexto));
  evento.user.data1 = SDL_strdup(archivos[0]);
  if (!evento.user.data1 || !SDL_PushEvent(&evento))
    SDL_free(evento.user.data1);
}

// Une ranuras compatibles sin sobrescribir partidas existentes ni modificar el archivo importado.
void PartidasSDL::importar(const char *origen) {
  size_t longitud = 0;
  auto *bytes = static_cast<unsigned char *>(SDL_LoadFile(origen, &longitud));
  if (!bytes || longitud > motor::bytes_archivo_partidas ||
      !motor::validar_partidas(bytes, static_cast<unsigned int>(longitud))) {
    SDL_free(bytes);
    mensaje = "Archivo DOS no valido";
    return;
  }
  unsigned int entrantes = 0, libres = 0;
  for (unsigned int i = 0; i < 7; ++i) {
    entrantes += bool(bytes[0] & (1u << i));
    libres += !(archivo[0] & (1u << i));
  }
  if (!entrantes || entrantes > libres) {
    mensaje = entrantes ? "Faltan ranuras vacias" : "Archivo sin partidas";
    SDL_free(bytes);
    return;
  }
  auto anterior = archivo;
  for (unsigned int i = 0, destino = 0; i < 7; ++i) {
    if (!(bytes[0] & (1u << i)))
      continue;
    while (archivo[0] & (1u << destino))
      ++destino;
    for (unsigned int j = 0; j < motor::bytes_ranura_partida; ++j)
      archivo[1 + destino * motor::bytes_ranura_partida + j] =
          bytes[1 + i * motor::bytes_ranura_partida + j];
    archivo[0] |= static_cast<unsigned char>(1u << destino);
  }
  SDL_free(bytes);
  if (!persistir()) {
    archivo.swap(anterior);
    mensaje = "No se ha podido importar";
    return;
  }
  seleccionar(0);
  mensaje = "Partidas DOS importadas";
  registrar_accion("Partidas DOS importadas: %u; origen conservado: %s", entrantes, origen);
}

// La confirmación se exige para las dos operaciones que sustituyen una ranura existente.
bool PartidasSDL::aceptar(SDL_Window *ventana, Mazmorra &partida, bool &redibujar) {
  if (!disponible)
    return true;
  const bool ocupada = archivo[0] & (1u << seleccion);
  if (modo == 1) {
    if (!ocupada) {
      mensaje = "Ranura vacia";
      return true;
    }
    if (!partida.cargar_partida(archivo.data(), static_cast<unsigned int>(archivo.size()),
                                seleccion)) {
      mensaje = "Partida no valida o zona pendiente";
      return true;
    }
    redibujar = true;
    cerrar(ventana);
    return true;
  }
  if ((modo == 3 || ocupada) && !confirmacion) {
    if (!ocupada) {
      mensaje = "Ranura vacia";
      return true;
    }
    confirmacion = true;
    mensaje = modo == 3 ? "Confirmar borrado: Intro" : "Confirmar reemplazo: Intro";
    return true;
  }
  auto anterior = archivo;
  const bool hecho =
      modo == 3 ? motor::borrar_partida(archivo.data(), static_cast<unsigned int>(archivo.size()),
                                        seleccion)
                : partida.guardar_partida(archivo.data(), static_cast<unsigned int>(archivo.size()),
                                          seleccion, nombre.c_str());
  if (!hecho || !persistir()) {
    archivo.swap(anterior);
    mensaje = "No se ha podido guardar el cambio";
    return true;
  }
  registrar_accion("Partida %s: ranura %u", modo == 3 ? "borrada" : "guardada", seleccion + 1);
  if (modo == 3) {
    seleccionar(seleccion);
    mensaje = "Partida borrada";
  } else
    cerrar(ventana);
  return true;
}

// Atiende ratón, flechas, Intro, Escape y texto sin dejar que esas entradas alcancen al personaje.
bool PartidasSDL::evento(const SDL_Event &entrada, SDL_Window *ventana, Mazmorra &partida,
                         bool &redibujar) {
  if (evento_importacion && entrada.type == evento_importacion) {
    if (abierta() && disponible && entrada.user.data1)
      importar(static_cast<const char *>(entrada.user.data1));
    SDL_free(entrada.user.data1);
    return true;
  }
  if (!abierta())
    return false;
  if (modo == 4) {
    if ((entrada.type == SDL_EVENT_KEY_DOWN &&
         (entrada.key.key == SDLK_ESCAPE || entrada.key.key == SDLK_RETURN)) ||
        entrada.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
      cerrar(ventana);
    return true;
  }
  if (entrada.type == SDL_EVENT_DROP_FILE && modo == 1 && disponible && entrada.drop.data) {
    importar(entrada.drop.data);
    return true;
  }
  if (entrada.type == SDL_EVENT_TEXT_INPUT && modo == 2 && disponible) {
    confirmacion = false;
    mensaje.clear();
    for (const char *p = entrada.text.text; *p && nombre.size() < 13; ++p)
      if (static_cast<unsigned char>(*p) >= 32 && static_cast<unsigned char>(*p) <= 126)
        nombre += *p;
  }
  if (entrada.type == SDL_EVENT_KEY_DOWN) {
    if (entrada.key.repeat && entrada.key.key == SDLK_RETURN)
      return true;
    if (entrada.key.key == SDLK_ESCAPE) {
      if (confirmacion) {
        confirmacion = false;
        mensaje.clear();
      } else
        cerrar(ventana);
    } else if (entrada.key.key == SDLK_UP || entrada.key.key == SDLK_DOWN) {
      seleccionar((seleccion + (entrada.key.key == SDLK_UP ? 6 : 1)) % 7);
      mensaje.clear();
    } else if (entrada.key.key == SDLK_BACKSPACE && modo == 2 && !nombre.empty()) {
      nombre.pop_back();
      confirmacion = false;
    } else if (entrada.key.key == SDLK_A && (entrada.key.mod & SDL_KMOD_CTRL) && modo == 2) {
      nombre.clear();
      confirmacion = false;
    } else if (entrada.key.key == SDLK_RETURN)
      aceptar(ventana, partida, redibujar);
  }
  if (entrada.type == SDL_EVENT_MOUSE_BUTTON_DOWN && entrada.button.button == SDL_BUTTON_LEFT) {
    const auto x = entrada.button.x, y = entrada.button.y;
    if (disponible && x >= 32 && x < 288 && y >= 40 && y < 138) {
      seleccionar(static_cast<unsigned int>(y - 40) / 14);
      mensaje.clear();
    } else if (y >= 174 && y < 190) {
      if (x >= 32 && x < 104)
        aceptar(ventana, partida, redibujar);
      else if (x >= 216 && x < 288)
        cerrar(ventana);
      else if (disponible && modo == 1 && x >= 110 && x < 210 && evento_importacion) {
        static const SDL_DialogFileFilter filtro{"Partidas DOS", "sav"};
        SDL_ShowOpenFileDialog(
            recibir_archivo,
            reinterpret_cast<void *>(static_cast<std::uintptr_t>(evento_importacion)), ventana,
            &filtro, 1, nullptr, false);
      }
    }
  }
  return true;
}

// Dibuja un panel metálico discreto sobre el juego; la interfaz no modifica sus píxeles originales.
bool PartidasSDL::dibujar(SDL_Renderer *dibujante) const {
  if (!abierta())
    return true;
  if (modo == 4) {
    const SDL_FRect placa{24, 12, 272, 180};
    char informacion[256]{};
    std::snprintf(informacion, sizeof(informacion),
                  "CURSE OF ENCHANTIA\n\nCore Design (1992)\n\nCurse Reborn\nReconstruccion para "
                  "SDL\n\nPuntuacion: %u\nProgreso: %u%%\n\nIntro o Escape para volver",
                  puntos, static_cast<unsigned int>(puntos) * 100u / 478u);
    return SDL_SetRenderDrawColor(dibujante, 36, 40, 45, 255) &&
           SDL_RenderFillRect(dibujante, &placa) &&
           SDL_SetRenderDrawColor(dibujante, 224, 224, 216, 255) &&
           SDL_RenderRect(dibujante, &placa) && SDL_RenderDebugText(dibujante, 36, 22, informacion);
  }
  const SDL_FRect fondo{24, 12, 272, 180}, borde{26, 14, 268, 176};
  bool correcto =
      SDL_SetRenderDrawColor(dibujante, 36, 40, 45, 255) && SDL_RenderFillRect(dibujante, &fondo) &&
      SDL_SetRenderDrawColor(dibujante, 176, 180, 184, 255) && SDL_RenderRect(dibujante, &borde) &&
      SDL_SetRenderDrawColor(dibujante, 240, 228, 200, 255) &&
      SDL_RenderDebugText(dibujante, 36, 22,
                          modo == 1   ? "Cargar partida"
                          : modo == 2 ? "Guardar partida"
                                      : "Borrar partida");
  for (unsigned int i = 0; i < 7; ++i) {
    const SDL_FRect fila{32, 40.0f + i * 14, 256, 13};
    char etiqueta[40]{}, texto[14]{};
    const bool ocupada =
        disponible &&
        motor::nombre_partida(archivo.data(), static_cast<unsigned int>(archivo.size()), i, texto);
    for (auto &c : texto)
      if (static_cast<unsigned char>(c) > 126 || (c && c < 32))
        c = '?';
    std::snprintf(etiqueta, sizeof(etiqueta), "%u  %s", i + 1, ocupada ? texto : "Vacia");
    correcto = correcto &&
               SDL_SetRenderDrawColor(dibujante, i == seleccion ? 100 : 48,
                                      i == seleccion ? 42 : 52, i == seleccion ? 40 : 58, 255) &&
               SDL_RenderFillRect(dibujante, &fila) &&
               SDL_SetRenderDrawColor(dibujante, 232, 232, 224, 255) &&
               SDL_RenderDebugText(dibujante, 36, fila.y + 2, etiqueta);
  }
  const std::string entrada = modo == 2 ? "Nombre: " + nombre : "";
  correcto = correcto && SDL_SetRenderDrawColor(dibujante, 240, 228, 200, 255) &&
             SDL_RenderDebugText(dibujante, 32, 144, entrada.c_str()) &&
             SDL_RenderDebugText(dibujante, 32, 158, mensaje.c_str()) &&
             SDL_RenderDebugText(dibujante, 32, 178, "Aceptar") &&
             SDL_RenderDebugText(dibujante, 216, 178, "Cancelar");
  return correcto && (modo != 1 || SDL_RenderDebugText(dibujante, 110, 178, "Importar DOS"));
}
