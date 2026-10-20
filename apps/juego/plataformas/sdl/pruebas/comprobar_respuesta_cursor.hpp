#pragma once
#include "../video/cursor.hpp"
#include <array>

// Ejercita un rechazo real al usar la llave desde el menú de la mazmorra y su retorno por el
// canal9976.
inline bool comprobar_respuesta_cursor(SDL_Window *ventana, SDL_Renderer *dibujante,
                                       CursorOriginal &cursor, Mazmorra &partida,
                                       const std::string &destino) {
  const auto avanzar = [&](unsigned int cantidad) {
    for (unsigned int i = 0; i < cantidad; ++i)
      if (!partida.avanzar_presentacion(SDL_GetTicksNS() * 70u / 1000000000u + 100u) ||
          !partida.avanzar())
        return false;
    return true;
  };
  const auto abrir = [&]() { return partida.entrada_menu(152, 92, false, true); };
  const auto elegir = [&](unsigned short casilla) {
    return partida.entrada_menu(static_cast<unsigned short>(casilla * 32 + 16), 92, true, false);
  };
  // Pedir ayuda deja la llave al alcance; se recoge y se espera su secuencia original.
  if (!avanzar(1) || !abrir() || !elegir(4) || !elegir(1) || !avanzar(192) || !abrir() ||
      !elegir(1) || !elegir(0) || !avanzar(128) || !partida.conserva_objeto(1))
    return SDL_SetError("No se ha podido preparar la llave para comprobar su rechazo");
  SDL_FPoint centro;
  std::array<unsigned char, 16 * 16 * 4> normal{}, rechazo{}, recuperada{};
  if (partida.fotograma_cursor() != 0 || !partida.preparar_cursor_rgba(normal.data()) ||
      !SDL_RenderCoordinatesToWindow(dibujante, 160, 100, &centro.x, &centro.y) ||
      !cursor.actualizar_puntero(ventana, dibujante, partida, true, false, &centro))
    return false;
  auto *mano_normal = SDL_GetCursor();
  // Usar la llave sola carece de acción válida y activa el rechazo común88A0.
  if (!abrir() || !elegir(5) || !elegir(0) || partida.fotograma_cursor() != 2 ||
      !partida.preparar_cursor_rgba(rechazo.data()) || rechazo == normal ||
      !cursor.actualizar_puntero(ventana, dibujante, partida, true, false, &centro) ||
      SDL_GetCursor() == mano_normal)
    return SDL_SetError("El rechazo original no sustituye la mano del sistema");
  SDL_Surface *imagen =
      SDL_CreateSurfaceFrom(16, 16, SDL_PIXELFORMAT_RGBA32, rechazo.data(), 16 * 4);
  const bool guardada = imagen && SDL_SavePNG(imagen, (destino + ".rechazo.png").c_str());
  SDL_DestroySurface(imagen);
  if (!guardada)
    return false;
  // La composición previa al filtro debe utilizar exactamente la misma mano que el cursor nativo.
  std::vector<unsigned char> fondo(320 * 200 * 4, 19), compuesto = fondo;
  if (!cursor.actualizar_puntero(ventana, dibujante, partida, true, false, &centro, true) ||
      !cursor.componer_rgba(partida, compuesto.data()))
    return false;
  for (unsigned int y = 0; y < 16; ++y)
    for (unsigned int x = 0; x < 16; ++x) {
      const auto origen = (y * 16 + x) * 4;
      const auto pixel = ((100 + y) * 320 + 160 + x) * 4;
      for (unsigned int c = 0; c < 4; ++c)
        if (compuesto[pixel + c] != (rechazo[origen + 3] ? rechazo[origen + c] : 19))
          return SDL_SetError("La mano filtrada no corresponde al fotograma de rechazo");
    }
  unsigned int pasos = 0;
  while (partida.fotograma_cursor() != 0 && pasos < 64) {
    if (!partida.avanzar())
      return false;
    ++pasos;
  }
  auto *mano_rechazo = SDL_GetCursor();
  if (pasos == 0 || pasos == 64 || !partida.preparar_cursor_rgba(recuperada.data()) ||
      recuperada != normal ||
      !cursor.actualizar_puntero(ventana, dibujante, partida, true, false, &centro) ||
      SDL_GetCursor() == mano_rechazo)
    return SDL_SetError("La mano original no vuelve al terminar el rechazo");
  return true;
}
