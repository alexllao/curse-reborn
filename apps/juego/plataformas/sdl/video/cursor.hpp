#pragma once
#include <SDL3/SDL.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <vector>
#include "../mazmorra.hpp"
#include "marcos.hpp"

// La mano de MOUSE.SPR sustituye al puntero del sistema durante el juego y sus menús.
class CursorOriginal {
public:
  // Restaura el puntero del escritorio antes de liberar el cursor propio.
  ~CursorOriginal() {
    cerrar();
  }
  // Permite cerrar antes de SDL_Quit y repetir el cierre sin tocar memoria liberada.
  void cerrar() {
    if (cursor) {
      SDL_SetCursor(SDL_GetDefaultCursor());
      SDL_DestroyCursor(cursor);
    }
    cursor = nullptr;
    ancho_actual = 0;
    mano_software = false;
  }
  // Adapta la escala a la imagen jugable; los paneles DOS mantienen su cursor ya incorporado.
  bool actualizar(SDL_Window *ventana, SDL_Renderer *dibujante, const Mazmorra &partida,
                  bool jugando, bool cursor_del_panel, bool por_software = false) {
    SDL_FPoint puntero;
    const bool sobre_ventana = SDL_GetMouseFocus() == ventana;
    if (sobre_ventana)
      SDL_GetMouseState(&puntero.x, &puntero.y);
    return actualizar_puntero(ventana, dibujante, partida, jugando, cursor_del_panel,
                              sobre_ventana ? &puntero : nullptr, por_software);
  }
  // Adapta el cursor a una posición de ventana; null indica que el ratón ha salido de ella.
  bool actualizar_puntero(SDL_Window *ventana, SDL_Renderer *dibujante, const Mazmorra &partida,
                          bool jugando, bool cursor_del_panel, const SDL_FPoint *puntero,
                          bool por_software = false) {
    mano_software = false;
    int ancho = 0, alto = 0, ventana_ancho = 0, ventana_alto = 0;
    SDL_FRect juego{};
    bool dentro = false;
    if (jugando && puntero) {
      if (!SDL_GetRenderOutputSize(dibujante, &ancho, &alto) ||
          !SDL_GetWindowSize(ventana, &ventana_ancho, &ventana_alto) ||
          !MarcosPantalla::area_juego(dibujante, juego) || ancho <= 0 || alto <= 0 ||
          ventana_ancho <= 0 || ventana_alto <= 0)
        return false;
      const float x = puntero->x * ancho / ventana_ancho;
      const float y = puntero->y * alto / ventana_alto;
      dentro = x >= juego.x && x < juego.x + juego.w && y >= juego.y && y < juego.y + juego.h;
    }
    if (!dentro) {
      if (!SDL_SetCursor(SDL_GetDefaultCursor()))
        return false;
      return SDL_CursorVisible() ? true : SDL_ShowCursor();
    }
    if (cursor_del_panel)
      return SDL_CursorVisible() ? SDL_HideCursor() : true;
    if (por_software) {
      float x = 0, y = 0;
      if (!SDL_RenderCoordinatesFromWindow(dibujante, puntero->x, puntero->y, &x, &y))
        return false;
      cursor_x = std::clamp(static_cast<int>(std::floor(x)), 0, 319);
      cursor_y = std::clamp(static_cast<int>(std::floor(y)), 0, 199);
      mano_software = true;
      return SDL_CursorVisible() ? SDL_HideCursor() : true;
    }
    const int deseado =
        std::clamp(static_cast<int>(std::lround(juego.w * ventana_ancho / ancho / 20.0f)), 1, 256);
    if (!cursor || ancho_actual != deseado) {
      std::vector<unsigned char> rgba(16u * 16u * 4u);
      if (!partida.preparar_cursor_rgba(rgba.data()))
        return false;
      SDL_Surface *original =
          SDL_CreateSurfaceFrom(16, 16, SDL_PIXELFORMAT_RGBA32, rgba.data(), 16 * 4);
      if (!original)
        return false;
      SDL_Surface *ampliado = SDL_ScaleSurface(original, deseado, deseado, SDL_SCALEMODE_NEAREST);
      SDL_DestroySurface(original);
      if (!ampliado)
        return false;
      SDL_Cursor *siguiente = SDL_CreateColorCursor(ampliado, 0, 0);
      SDL_DestroySurface(ampliado);
      if (!siguiente)
        return false;
      cerrar();
      cursor = siguiente;
      ancho_actual = deseado;
    }
    if (SDL_GetCursor() != cursor && !SDL_SetCursor(cursor))
      return false;
    return SDL_CursorVisible() ? true : SDL_ShowCursor();
  }

  // Compone la mano transparente sobre una copia nueva del fotograma antes del filtro F11.
  bool componer_rgba(const Mazmorra &partida, unsigned char *rgba) const {
    if (!mano_software)
      return true;
    std::array<unsigned char, 16 * 16 * 4> mano{};
    if (!partida.preparar_cursor_rgba(mano.data()))
      return false;
    for (int y = 0; y < 16 && cursor_y + y < 200; ++y)
      for (int x = 0; x < 16 && cursor_x + x < 320; ++x) {
        const auto origen = (y * 16 + x) * 4;
        if (mano[origen + 3])
          std::copy_n(mano.data() + origen, 4, rgba + ((cursor_y + y) * 320 + cursor_x + x) * 4);
      }
    return true;
  }

private:
  bool mano_software = false;
  int cursor_x = 0, cursor_y = 0;
  SDL_Cursor *cursor = nullptr;
  int ancho_actual = 0;
};
