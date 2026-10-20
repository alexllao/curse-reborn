#pragma once
#include "../interfaz/partidas.hpp"
#include "../../../motor/estado/partidas.hpp"

// Comprueba ranuras reales y restauración sobre la salida de las cuevas, aislando el perfil del
// usuario.
inline bool comprobar_partidas_sdl(PartidasSDL &panel, SDL_Window *ventana, Mazmorra &partida,
                                   const std::string &carpeta,
                                   const motor::ImagenIndexada &imagen) {
  const char *ruta = SDL_getenv("CURSE_REBORN_PARTIDAS");
  if (!ruta || !*ruta || partida.escena_actual() != 15 || !partida.conserva_objeto(70))
    return false;
  std::vector<unsigned char> archivo(motor::bytes_archivo_partidas, 0);
  if (!partida.guardar_partida(archivo.data(), static_cast<unsigned int>(archivo.size()), 0,
                               "Cuevas") ||
      !partida.iniciar_vista(carpeta, imagen, 0) || partida.escena_actual() != 0 ||
      !partida.guardar_partida(archivo.data(), static_cast<unsigned int>(archivo.size()), 1,
                               "Celda") ||
      !partida.cargar_partida(archivo.data(), static_cast<unsigned int>(archivo.size()), 1) ||
      partida.escena_actual() != 0 ||
      !partida.cargar_partida(archivo.data(), static_cast<unsigned int>(archivo.size()), 0) ||
      partida.escena_actual() != 15 || !partida.conserva_objeto(70) || partida.conserva_objeto(74))
    return false;
  // Mantiene el archivo original intacto cuando la ranura solicita una zona todavía pendiente.
  auto incompatible = archivo;
  const auto campo = 1 + motor::caracteres_nombre_partida + 0x3598 - motor::inicio_estado_partida;
  incompatible[campo] = 100;
  incompatible[campo + 1] = 0;
  if (partida.cargar_partida(incompatible.data(), static_cast<unsigned int>(incompatible.size()),
                             0) ||
      partida.escena_actual() != 15)
    return false;
  bool redibujar = false;
  SDL_Event tecla{};
  tecla.type = SDL_EVENT_KEY_DOWN;
  tecla.key.key = SDLK_RETURN;
  // Guardar abre primero el selector y después el teclado de trece caracteres del original.
  panel.abrir(2, ventana);
  if (!panel.evento(tecla, ventana, partida, redibujar) || !panel.abierta())
    return false;
  SDL_Event escribir{};
  escribir.type = SDL_EVENT_TEXT_INPUT;
  escribir.text.text = "CUEVAS";
  if (!panel.evento(escribir, ventana, partida, redibujar) ||
      !panel.evento(tecla, ventana, partida, redibujar) || panel.abierta())
    return false;
  size_t longitud = 0;
  auto *guardado = static_cast<unsigned char *>(SDL_LoadFile(ruta, &longitud));
  const bool valido = guardado && longitud == motor::bytes_archivo_partidas && guardado[0] == 1 &&
                      motor::validar_partidas(guardado, static_cast<unsigned int>(longitud));
  SDL_free(guardado);
  if (!valido)
    return false;
  panel.abrir(1, ventana);
  if (!panel.evento(tecla, ventana, partida, redibujar) || !redibujar || panel.abierta() ||
      partida.escena_actual() != 15)
    return false;
  panel.abrir(3, ventana);
  if (!panel.evento(tecla, ventana, partida, redibujar) || panel.abierta())
    return false;
  guardado = static_cast<unsigned char *>(SDL_LoadFile(ruta, &longitud));
  const bool borrada = guardado && guardado[0] == 0;
  SDL_free(guardado);
  if (!borrada)
    return false;
  const char *importada = SDL_getenv("CURSE_REBORN_IMPORTAR_PRUEBA");
  if (importada && *importada) {
    panel.abrir(2, ventana);
    if (!panel.evento(tecla, ventana, partida, redibujar) || !panel.abierta() ||
        !panel.evento(escribir, ventana, partida, redibujar) ||
        !panel.evento(tecla, ventana, partida, redibujar) || panel.abierta())
      return false;
    panel.abrir(1, ventana);
    SDL_Event arrastrar{};
    arrastrar.type = SDL_EVENT_DROP_FILE;
    arrastrar.drop.data = importada;
    if (!panel.evento(arrastrar, ventana, partida, redibujar))
      return false;
    guardado = static_cast<unsigned char *>(SDL_LoadFile(ruta, &longitud));
    const bool unido = guardado && guardado[0] == 3;
    SDL_free(guardado);
    SDL_Event bajar = tecla;
    bajar.key.key = SDLK_DOWN;
    if (!unido || !panel.evento(bajar, ventana, partida, redibujar) ||
        !panel.evento(tecla, ventana, partida, redibujar) || panel.abierta() ||
        partida.escena_actual() != 5 || !partida.conserva_objeto(37) ||
        !partida.conserva_objeto(64) || !partida.movimiento_habilitado())
      return false;
  }
  // El teclado en pantalla conserva letras, espacio, borrado y aceptación de la tabla BA70.
  panel.abrir(2, ventana);
  SDL_Event bajar{};
  bajar.type = SDL_EVENT_KEY_DOWN;
  bajar.key.key = SDLK_DOWN;
  for (unsigned int i = 0; i < 6; ++i)
    if (!panel.evento(bajar, ventana, partida, redibujar))
      return false;
  tecla.key.key = SDLK_RETURN;
  if (!panel.evento(tecla, ventana, partida, redibujar) || !panel.abierta())
    return false;
  SDL_Event clic{};
  clic.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
  clic.button.button = SDL_BUTTON_LEFT;
  constexpr int teclas[][2] = {{108, 70},  {124, 70},  {124, 120},
                               {140, 120}, {188, 120}, {204, 120}};
  for (const auto &posicion : teclas) {
    clic.button.x = static_cast<float>(posicion[0]);
    clic.button.y = static_cast<float>(posicion[1]);
    if (!panel.evento(clic, ventana, partida, redibujar))
      return false;
  }
  if (panel.abierta())
    return false;
  guardado = static_cast<unsigned char *>(SDL_LoadFile(ruta, &longitud));
  const auto nombre_seis = 1 + 6 * motor::bytes_ranura_partida;
  const bool teclado_correcto = guardado && (guardado[0] & 64) && guardado[nombre_seis] == 'A' &&
                                guardado[nombre_seis + 1] == 'B' &&
                                guardado[nombre_seis + 2] == ' ' &&
                                guardado[nombre_seis + 3] == ' ';
  SDL_free(guardado);
  if (!teclado_correcto)
    return false;
  panel.abrir(3, ventana);
  for (unsigned int i = 0; i < 6; ++i)
    if (!panel.evento(bajar, ventana, partida, redibujar))
      return false;
  if (!panel.evento(tecla, ventana, partida, redibujar) || panel.abierta())
    return false;
  for (unsigned int i = 0; i < 16; ++i)
    if (!partida.avanzar())
      return false;
  panel.abrir(4, ventana);
  tecla.key.key = SDLK_ESCAPE;
  if (!panel.evento(tecla, ventana, partida, redibujar) || panel.abierta())
    return false;
  panel.abrir(2, ventana);
  return partida.guardar_estado_diagnostico(std::string(ruta) + ".panel.ds.bin");
}

// Comprueba ambos idiomas, el encuadre y la conservación de nombres, teclado y formato de partidas.
inline bool comprobar_textos_bilingues(Mazmorra &partida) {
  const auto idioma_anterior = idioma_interfaz;
  struct RestaurarIdioma {
    IdiomaInterfaz anterior;
    ~RestaurarIdioma() {
      idioma_interfaz = anterior;
    }
  } restaurar{idioma_anterior};
  std::vector<unsigned char> archivo(motor::bytes_archivo_partidas, 0);
  if (!partida.guardar_partida(archivo.data(), static_cast<unsigned int>(archivo.size()), 0,
                               "PRUEBA"))
    return SDL_SetError("Comprobación bilingüe: condición %u", __LINE__);
  const auto antes = archivo;
  for (unsigned int pagina = 0; pagina < paginas_informacion; ++pagina) {
    motor::ImagenIndexada ingles{}, castellano{};
    SDL_FRect placa{}, traducida{};
    idioma_interfaz = IdiomaInterfaz::ingles;
    if (!partida.dibujar_informacion(pagina, ingles, placa))
      return SDL_SetError("Comprobación bilingüe: condición %u", __LINE__);
    idioma_interfaz = IdiomaInterfaz::castellano;
    if (!partida.dibujar_informacion(pagina, castellano, traducida) || placa.x != traducida.x ||
        placa.y != traducida.y || placa.w != traducida.w || placa.h != traducida.h)
      return SDL_SetError("Comprobación bilingüe: condición %u", __LINE__);
    unsigned int diferencias = 0;
    for (unsigned int y = 0; y < 200; ++y)
      for (unsigned int x = 0; x < 320; ++x)
        if (ingles.indices[y * 320 + x] != castellano.indices[y * 320 + x]) {
          if (x < placa.x || y < placa.y || x >= placa.x + placa.w || y >= placa.y + placa.h)
            return SDL_SetError("Comprobación bilingüe: condición %u", __LINE__);
          ++diferencias;
        }
    if (!diferencias)
      return SDL_SetError("Comprobación bilingüe: condición %u", __LINE__);
  }
  for (unsigned int modo = 1; modo <= 3; ++modo)
    for (const bool editando : {false, true}) {
      motor::ImagenIndexada ingles{}, castellano{};
      idioma_interfaz = IdiomaInterfaz::ingles;
      if (!partida.dibujar_panel_partidas(modo, archivo, editando, "PRUEBA", 310, 190, 255, ingles))
        return SDL_SetError("Comprobación bilingüe: condición %u", __LINE__);
      idioma_interfaz = IdiomaInterfaz::castellano;
      if (!partida.dibujar_panel_partidas(modo, archivo, editando, "PRUEBA", 310, 190, 255,
                                          castellano))
        return SDL_SetError("Comprobación bilingüe: condición %u", __LINE__);
      unsigned int diferencias = 0;
      for (unsigned int y = 0; y < 200; ++y)
        for (unsigned int x = 0; x < 320; ++x)
          if (ingles.indices[y * 320 + x] != castellano.indices[y * 320 + x]) {
            // Fuera de la cabecera solo se traducen las siete ranuras vacías, nunca PRUEBA.
            if (x < 100 || x >= 212 || y < 55 || y >= (editando ? 63u : 143u) ||
                (y >= 63 && y < 75))
              return SDL_SetError("Comprobación bilingüe: condición %u", __LINE__);
            ++diferencias;
          }
      if (!diferencias || archivo != antes)
        return SDL_SetError("Comprobación bilingüe: condición %u", __LINE__);
    }
  std::fprintf(stdout,
               "Idiomas: cinco fichas y seis paneles; encuadre, nombres y teclado conservados.\n");
  return true;
}

// Abre el décimo icono real, recorre las fichas y comprueba que no altera la partida ni el perfil.
inline bool comprobar_informacion_sdl(PartidasSDL &panel, SDL_Window *ventana,
                                      SDL_Renderer *dibujante, Mazmorra &partida,
                                      unsigned int pagina_captura) {
  if (pagina_captura >= paginas_informacion || !partida.menu_abierto())
    return false;
  if (pagina_captura == 0 && !comprobar_textos_bilingues(partida))
    return false;
  std::vector<unsigned char> antes(motor::bytes_archivo_partidas, 0), despues(antes);
  if (!partida.entrada_menu(304, 92, true, false) || partida.menu_abierto() ||
      partida.tomar_solicitud_partidas() != 4 || partida.tomar_solicitud_partidas() != 0 ||
      !partida.guardar_partida(antes.data(), static_cast<unsigned int>(antes.size()), 0, "Info"))
    return false;
  panel.abrir(4, ventana);
  bool redibujar = false;
  SDL_Event tecla{};
  tecla.type = SDL_EVENT_KEY_DOWN;
  tecla.key.key = SDLK_RETURN;
  // Una pulsación mantenida no debe saltarse varias páginas.
  tecla.key.repeat = true;
  if (!panel.evento(tecla, ventana, partida, redibujar) || !panel.abierta())
    return false;
  tecla.key.repeat = false;
  for (unsigned int pagina = 0; pagina < paginas_informacion; ++pagina)
    if (!panel.dibujar(dibujante, partida) || !panel.evento(tecla, ventana, partida, redibujar) ||
        panel.abierta() != (pagina + 1 < paginas_informacion))
      return false;
  panel.abrir(4, ventana);
  tecla.key.key = SDLK_ESCAPE;
  if (!panel.evento(tecla, ventana, partida, redibujar) || panel.abierta())
    return false;
  panel.abrir(4, ventana);
  SDL_Event clic{};
  clic.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
  clic.button.button = SDL_BUTTON_RIGHT;
  if (!panel.evento(clic, ventana, partida, redibujar) || panel.abierta())
    return false;
  panel.abrir(4, ventana);
  clic.button.button = SDL_BUTTON_LEFT;
  for (unsigned int pagina = 0; pagina < pagina_captura; ++pagina)
    if (!panel.evento(clic, ventana, partida, redibujar) || !panel.abierta())
      return false;
  return !redibujar &&
         partida.guardar_partida(despues.data(), static_cast<unsigned int>(despues.size()), 0,
                                 "Info") &&
         antes == despues;
}
