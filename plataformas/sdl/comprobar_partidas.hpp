#pragma once
#include "partidas.hpp"
#include "../../motor/estado/partidas.hpp"

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
  panel.abrir(2, ventana);
  if (!panel.evento(tecla, ventana, partida, redibujar) || panel.abierta())
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
  if (!panel.evento(tecla, ventana, partida, redibujar) || !panel.abierta())
    return false;
  guardado = static_cast<unsigned char *>(SDL_LoadFile(ruta, &longitud));
  const bool sin_confirmar = guardado && guardado[0] == 1;
  SDL_free(guardado);
  if (!sin_confirmar || !panel.evento(tecla, ventana, partida, redibujar))
    return false;
  guardado = static_cast<unsigned char *>(SDL_LoadFile(ruta, &longitud));
  const bool borrada = guardado && guardado[0] == 0;
  SDL_free(guardado);
  if (!borrada)
    return false;
  const char *importada = SDL_getenv("CURSE_REBORN_IMPORTAR_PRUEBA");
  if (importada && *importada) {
    panel.abrir(2, ventana);
    if (!panel.evento(tecla, ventana, partida, redibujar) || panel.abierta())
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
  for (unsigned int i = 0; i < 16; ++i)
    if (!partida.avanzar())
      return false;
  panel.abrir(4, ventana);
  tecla.key.key = SDLK_ESCAPE;
  if (!panel.evento(tecla, ventana, partida, redibujar) || panel.abierta())
    return false;
  panel.abrir(2, ventana);
  return true;
}

// Abre el décimo icono real, recorre las fichas y comprueba que no altera la partida ni el perfil.
inline bool comprobar_informacion_sdl(PartidasSDL &panel, SDL_Window *ventana,
                                      SDL_Renderer *dibujante, Mazmorra &partida,
                                      unsigned int pagina_captura) {
  if (pagina_captura >= 4 || !partida.menu_abierto())
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
  for (unsigned int pagina = 0; pagina < 4; ++pagina)
    if (!panel.dibujar(dibujante, partida) || !panel.evento(tecla, ventana, partida, redibujar) ||
        panel.abierta() != (pagina < 3))
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
