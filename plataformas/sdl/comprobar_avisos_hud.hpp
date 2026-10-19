#pragma once
#include "avisos_hud.hpp"
#include <cstring>
#include <vector>

// Comprueba píxeles, desaparición y restauración del dibujante sin ventanas ni esperas reales.
inline bool comprobar_avisos_hud(const std::string &prefijo) {
  SDL_Surface *s = SDL_CreateSurface(640, 400, SDL_PIXELFORMAT_RGBA32);
  if (!s)
    return false;
  SDL_Renderer *r = SDL_CreateSoftwareRenderer(s);
  if (!r) {
    SDL_DestroySurface(s);
    return false;
  }
  AvisosHud aviso;
  bool correcto = true;
  std::vector<unsigned char> base, media, plena, final;
  const auto fotograma = [&](Uint64 momento, std::vector<unsigned char> &pixeles) {
    const SDL_Rect viewport{7, 5, 300, 180};
    if (!SDL_SetRenderDrawColor(r, 16, 32, 48, 255) || !SDL_RenderClear(r) ||
        !SDL_SetRenderLogicalPresentation(r, 320, 200, SDL_LOGICAL_PRESENTATION_LETTERBOX) ||
        !SDL_SetRenderViewport(r, &viewport) || !SDL_SetRenderScale(r, 1.25f, 1.5f) ||
        !SDL_SetRenderDrawColor(r, 9, 19, 29, 39) ||
        !SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_ADD) || !aviso.dibujar(r, momento) ||
        !SDL_FlushRenderer(r))
      return false;
    SDL_Rect comprobado;
    float sx = 0, sy = 0;
    SDL_Color color;
    SDL_BlendMode mezcla;
    int ancho = 0, alto = 0;
    SDL_RendererLogicalPresentation presentacion;
    if (!SDL_GetRenderViewport(r, &comprobado) ||
        std::memcmp(&viewport, &comprobado, sizeof(viewport)) || !SDL_RenderViewportSet(r) ||
        !SDL_GetRenderScale(r, &sx, &sy) || sx != 1.25f || sy != 1.5f ||
        !SDL_GetRenderDrawBlendMode(r, &mezcla) || mezcla != SDL_BLENDMODE_ADD ||
        !SDL_GetRenderDrawColor(r, &color.r, &color.g, &color.b, &color.a) || color.r != 9 ||
        color.g != 19 || color.b != 29 || color.a != 39 ||
        !SDL_GetRenderLogicalPresentation(r, &ancho, &alto, &presentacion) || ancho != 320 ||
        alto != 200 || presentacion != SDL_LOGICAL_PRESENTATION_LETTERBOX)
      return false;
    const auto *inicio = static_cast<unsigned char *>(s->pixels);
    pixeles.assign(inicio, inicio + s->pitch * s->h);
    return true;
  };
  aviso.mostrar("Filtro: xBRZ 2x", 1000);
  correcto = fotograma(1000, base) && fotograma(1060, media) && fotograma(1120, plena) &&
             base != media && media != plena &&
             SDL_SavePNG(s, (prefijo + ".visible.png").c_str()) && fotograma(2820, final) &&
             base == final;
  if (correcto) {
    for (int y = 0; y < s->h; ++y)
      for (int x = 0; x < s->w; ++x) {
        const auto p = y * s->pitch + x * 4;
        if ((x < 12 || x >= 140 || y < 12 || y >= 20) && std::memcmp(&base[p], &plena[p], 4))
          correcto = false;
      }
    aviso.mostrar("Volumen: 200%", 3000);
    correcto = correcto && fotograma(3120, plena) && base != plena &&
               SDL_SavePNG(s, (prefijo + ".volumen.png").c_str()) && fotograma(4820, final) &&
               base == final;
  }
  SDL_DestroyRenderer(r);
  SDL_DestroySurface(s);
  return correcto;
}
