#include "partidas.hpp"
#include "registro.hpp"
#include "../../motor/estado/partidas.hpp"
#include <algorithm>

// El panel se destruye antes que Presentacion y su dibujante.
PartidasSDL::~PartidasSDL() {
  SDL_DestroyTexture(textura_informacion);
}

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

// E3F1/E41E: conserva los trece bytes de la descripción y retira solo sus espacios finales.
void PartidasSDL::seleccionar(unsigned int ranura) {
  seleccion = ranura;
  char texto[motor::caracteres_nombre_partida + 1]{};
  nombre = motor::nombre_partida(archivo.data(), static_cast<unsigned int>(archivo.size()),
                                 seleccion, texto)
               ? texto
               : "";
  while (!nombre.empty() && nombre.back() == ' ')
    nombre.pop_back();
  for (auto &letra : nombre)
    if (letra >= 'a' && letra <= 'z')
      letra = static_cast<char>(letra - 'a' + 'A');
  cursor_y = 70 + static_cast<int>(seleccion) * 10;
  registrar_accion("Partidas: seleccionada ranura %u", seleccion + 1);
}
// E1F5/E376/E7D9: inicia el selector en su primera fila, sin abrir aún el editor de nombre.
void PartidasSDL::abrir(unsigned int accion, SDL_Window *ventana) {
  if (accion != 4)
    preparar();
  SDL_DestroyTexture(textura_informacion);
  textura_informacion = nullptr;
  pagina_informacion = 0;
  pagina_preparada = paginas_informacion;
  modo = accion;
  if (modo != 4)
    SDL_HideCursor();
  seleccion = 0;
  editando = false;
  cursor_x = 108;
  cursor_y = 70;
  SDL_StopTextInput(ventana);
  if (disponible && modo != 4) {
    mensaje.clear();
    seleccionar(0);
  }
  if (modo == 4)
    registrar_accion("Información: página 1 de %u; clic o Intro avanza, Escape vuelve",
                     paginas_informacion);
}
// Devuelve la entrada al juego sin propagar la pulsación al personaje.
void PartidasSDL::cerrar(SDL_Window *ventana) {
  if (modo == 4)
    registrar_accion("Información: volver al juego");
  SDL_DestroyTexture(textura_informacion);
  textura_informacion = nullptr;
  pagina_preparada = paginas_informacion;
  SDL_StopTextInput(ventana);
  SDL_ShowCursor();
  modo = 0;
  editando = false;
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
  for (unsigned int i = 0; i < motor::ranuras_partida; ++i) {
    entrantes += bool(bytes[0] & (1u << i));
    libres += !(archivo[0] & (1u << i));
  }
  if (!entrantes || entrantes > libres) {
    mensaje = entrantes ? "Faltan ranuras vacias" : "Archivo sin partidas";
    SDL_free(bytes);
    return;
  }
  auto anterior = archivo;
  for (unsigned int i = 0, destino = 0; i < motor::ranuras_partida; ++i) {
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

// E23B/E3BC/E81F: seleccionar guardar abre el teclado; cargar y borrar actúan al elegir la fila.
bool PartidasSDL::aceptar(SDL_Window *ventana, Mazmorra &partida, bool &redibujar) {
  if (!disponible) {
    registrar_accion("Partidas: %s", mensaje.c_str());
    return true;
  }
  const bool ocupada = archivo[0] & (1u << seleccion);
  if (modo == 1) {
    if (ocupada) {
      if (!partida.cargar_partida(archivo.data(), static_cast<unsigned int>(archivo.size()),
                                  seleccion)) {
        registrar_accion("No se puede cargar la partida: archivo o zona pendiente");
        return true;
      }
      redibujar = true;
    }
    cerrar(ventana);
    return true;
  }
  if (modo == 2 && !editando) {
    editando = true;
    cursor_x = 108;
    cursor_y = 70;
    SDL_StartTextInput(ventana);
    return true;
  }
  if (modo == 2 && nombre.empty())
    return true;
  if (modo == 3 && !ocupada) {
    cerrar(ventana);
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
    registrar_accion("No se ha podido escribir el archivo de partidas");
    return true;
  }
  registrar_accion("Partida %s: ranura %u", modo == 3 ? "borrada" : "guardada", seleccion + 1);
  cerrar(ventana);
  return true;
}
// E595–E6F6: cero añade espacio, uno borra, dos guarda; el resto escribe la letra del recurso.
bool PartidasSDL::pulsar_tecla(SDL_Window *ventana, Mazmorra &partida, bool &redibujar) {
  const auto letra = partida.tecla_nombre_partida(static_cast<unsigned int>(cursor_x - 96),
                                                  static_cast<unsigned int>(cursor_y - 50));
  if (letra == 0x20)
    return true; // E5B7: los espacios entre teclas no seleccionan un carácter.
  if (letra == 2)
    return aceptar(ventana, partida, redibujar);
  if (letra == 1) {
    if (!nombre.empty())
      nombre.pop_back();
  } else if (nombre.size() < 13 && letra != 0xff)
    nombre += letra ? static_cast<char>(letra) : ' ';
  return true;
}

// E89D/E4A3: adapta las entradas SDL a las filas y las teclas originales, sin botones añadidos.
bool PartidasSDL::evento(const SDL_Event &entrada, SDL_Window *ventana, Mazmorra &partida,
                         bool &redibujar) {
  // Integración de escritorio: importar por arrastre no añade controles al panel del juego.
  if (entrada.type == SDL_EVENT_DROP_FILE && entrada.drop.data && (modo == 0 || modo == 1)) {
    const std::string fichero = entrada.drop.data;
    if (fichero.size() >= 4) {
      auto extension = fichero.substr(fichero.size() - 4);
      for (auto &letra : extension)
        if (letra >= 'A' && letra <= 'Z')
          letra = static_cast<char>(letra - 'A' + 'a');
      if (extension == ".sav") {
        preparar();
        if (disponible)
          importar(entrada.drop.data);
        return true;
      }
    }
  }
  if (!abierta())
    return false;
  if (modo == 4) {
    if (entrada.type == SDL_EVENT_KEY_DOWN && entrada.key.repeat)
      return true;
    const bool teclado = entrada.type == SDL_EVENT_KEY_DOWN;
    const bool raton = entrada.type == SDL_EVENT_MOUSE_BUTTON_DOWN;
    const bool avanzar =
        (teclado && (entrada.key.key == SDLK_RETURN || entrada.key.key == SDLK_KP_ENTER ||
                     entrada.key.key == SDLK_SPACE)) ||
        (raton && entrada.button.button == SDL_BUTTON_LEFT);
    if (avanzar) {
      if (++pagina_informacion >= paginas_informacion)
        cerrar(ventana);
      else
        registrar_accion("Información: página %u de %u", pagina_informacion + 1,
                         paginas_informacion);
    } else if (teclado || (raton && entrada.button.button == SDL_BUTTON_RIGHT))
      cerrar(ventana);
    return true;
  }
  if (entrada.type == SDL_EVENT_MOUSE_BUTTON_DOWN && entrada.button.button == SDL_BUTTON_RIGHT) {
    cerrar(ventana);
    return true;
  }
  if (entrada.type == SDL_EVENT_TEXT_INPUT && modo == 2 && editando && disponible) {
    for (const unsigned char *p = reinterpret_cast<const unsigned char *>(entrada.text.text);
         *p && nombre.size() < 13; ++p) {
      auto letra = *p;
      if (letra >= 'a' && letra <= 'z')
        letra = static_cast<unsigned char>(letra - 'a' + 'A');
      if (partida.admite_letra_partida(letra))
        nombre += static_cast<char>(letra);
    }
    return true;
  }
  if (entrada.type == SDL_EVENT_KEY_DOWN) {
    if (entrada.key.repeat)
      return true;
    const auto tecla = entrada.key.key;
    if (tecla == SDLK_ESCAPE) {
      if (!editando)
        cerrar(ventana);
    } else if (tecla == SDLK_RETURN || tecla == SDLK_KP_ENTER)
      aceptar(ventana, partida, redibujar);
    else if (editando) {
      if (tecla == SDLK_BACKSPACE) {
        if (!nombre.empty())
          nombre.pop_back();
      } else if (tecla == SDLK_INSERT)
        pulsar_tecla(ventana, partida, redibujar);
      else {
        if (tecla == SDLK_LEFT)
          cursor_x = std::max(108, cursor_x - 16);
        if (tecla == SDLK_RIGHT)
          cursor_x = std::min(204, cursor_x + 16);
        if (tecla == SDLK_UP)
          cursor_y = std::max(70, cursor_y - 10);
        if (tecla == SDLK_DOWN)
          cursor_y = std::min(120, cursor_y + 10);
      }
    } else if (tecla == SDLK_UP || tecla == SDLK_DOWN) {
      if (tecla == SDLK_UP && seleccion > 0)
        seleccionar(seleccion - 1);
      if (tecla == SDLK_DOWN && seleccion + 1 < motor::ranuras_partida)
        seleccionar(seleccion + 1);
    }
  }
  const bool clic =
      entrada.type == SDL_EVENT_MOUSE_BUTTON_DOWN && entrada.button.button == SDL_BUTTON_LEFT;
  if (entrada.type == SDL_EVENT_MOUSE_MOTION || clic) {
    const auto x = clic ? entrada.button.x : entrada.motion.x;
    const auto y = clic ? entrada.button.y : entrada.motion.y;
    if (editando) {
      cursor_x = std::clamp(static_cast<int>(x), 104, 207);
      cursor_y = std::clamp(static_cast<int>(y), 65, 124);
      if (clic)
        pulsar_tecla(ventana, partida, redibujar);
    } else {
      const auto fila = static_cast<unsigned int>(std::clamp(static_cast<int>(y - 65) / 10, 0, 7));
      if (fila != seleccion)
        seleccionar(fila);
      cursor_x = std::clamp(static_cast<int>(x), 104, 207);
      cursor_y = std::clamp(static_cast<int>(y), 65, 144);
      if (clic)
        aceptar(ventana, partida, redibujar);
    }
  }
  return true;
}

// Compone las fichas originales o el panel de ranuras; solo renueva la textura al cambiar de ficha.
bool PartidasSDL::dibujar(SDL_Renderer *dibujante, const Mazmorra &partida) {
  if (!abierta())
    return true;
  if (modo == 4) {
    if (pagina_preparada != pagina_informacion) {
      motor::ImagenIndexada imagen{};
      if (!partida.dibujar_informacion(pagina_informacion, imagen, placa_informacion))
        return false;
      std::vector<unsigned char> rgba(320 * 200 * 4);
      for (unsigned int i = 0; i < 64000; ++i) {
        for (unsigned int c = 0; c < 3; ++c)
          rgba[i * 4 + c] =
              static_cast<unsigned char>(imagen.paleta[imagen.indices[i] * 3 + c] * 255u / 63u);
        rgba[i * 4 + 3] = 255;
      }
      SDL_DestroyTexture(textura_informacion);
      textura_informacion =
          SDL_CreateTexture(dibujante, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, 320, 200);
      if (!textura_informacion ||
          !SDL_SetTextureScaleMode(textura_informacion, SDL_SCALEMODE_NEAREST) ||
          !SDL_UpdateTexture(textura_informacion, nullptr, rgba.data(), 320 * 4))
        return false;
      pagina_preparada = pagina_informacion;
    }
    return SDL_RenderTexture(dibujante, textura_informacion, &placa_informacion,
                             &placa_informacion);
  }
  motor::ImagenIndexada negro{}, blanco{};
  if (!partida.dibujar_panel_partidas(modo, archivo, editando, nombre, cursor_x, cursor_y, 0,
                                      negro) ||
      !partida.dibujar_panel_partidas(modo, archivo, editando, nombre, cursor_x, cursor_y, 255,
                                      blanco))
    return false;
  std::vector<unsigned char> rgba(320 * 200 * 4);
  for (unsigned int i = 0; i < 64000; ++i) {
    for (unsigned int c = 0; c < 3; ++c)
      rgba[i * 4 + c] =
          static_cast<unsigned char>(negro.paleta[negro.indices[i] * 3 + c] * 255u / 63u);
    // Dos fondos centinela distinguen la transparencia del negro opaco del cursor original.
    rgba[i * 4 + 3] = negro.indices[i] == blanco.indices[i] ? 255 : 0;
  }
  SDL_DestroyTexture(textura_informacion);
  textura_informacion =
      SDL_CreateTexture(dibujante, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, 320, 200);
  return textura_informacion && SDL_SetTextureBlendMode(textura_informacion, SDL_BLENDMODE_BLEND) &&
         SDL_SetTextureScaleMode(textura_informacion, SDL_SCALEMODE_NEAREST) &&
         SDL_UpdateTexture(textura_informacion, nullptr, rgba.data(), 320 * 4) &&
         SDL_RenderTexture(dibujante, textura_informacion, nullptr, nullptr);
}

// Usa los mismos dibujantes DOS que la textura de paneles, para filtrar también su cursor.
bool PartidasSDL::componer_rgba(const Mazmorra &partida, unsigned char *rgba) const {
  if (!abierta())
    return true;
  motor::ImagenIndexada negro{}, blanco{};
  SDL_FRect placa{};
  if (modo == 4) {
    if (!partida.dibujar_informacion(pagina_informacion, negro, placa))
      return false;
  } else if (!partida.dibujar_panel_partidas(modo, archivo, editando, nombre, cursor_x, cursor_y, 0,
                                             negro) ||
             !partida.dibujar_panel_partidas(modo, archivo, editando, nombre, cursor_x, cursor_y,
                                             255, blanco))
    return false;
  for (unsigned int y = 0; y < 200; ++y)
    for (unsigned int x = 0; x < 320; ++x) {
      const auto i = y * 320 + x;
      if (modo == 4 ? x < placa.x || x >= placa.x + placa.w || y < placa.y || y >= placa.y + placa.h
                    : negro.indices[i] != blanco.indices[i])
        continue;
      for (unsigned int c = 0; c < 3; ++c)
        rgba[i * 4 + c] =
            static_cast<unsigned char>(negro.paleta[negro.indices[i] * 3 + c] * 255u / 63u);
      rgba[i * 4 + 3] = 255;
    }
  return true;
}
