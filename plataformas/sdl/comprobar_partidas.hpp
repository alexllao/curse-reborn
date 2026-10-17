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
  panel.abrir(4, ventana, partida.puntuacion());
  if (!panel.evento(tecla, ventana, partida, redibujar) || panel.abierta())
    return false;
  panel.abrir(2, ventana);
  return true;
}
