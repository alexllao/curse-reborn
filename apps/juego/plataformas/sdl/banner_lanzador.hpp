#pragma once
#include <SDL3/SDL.h>
#include <algorithm>
#include <cmath>
#include <string>

struct VarianteBanner {
  const char *archivo;
  int ancho, alto;
};
#include "catalogo_presentacion.hpp"

// Presentación silenciosa del lanzador; no avanza el motor ni modifica la habitación.
class BannerLanzador {
public:
  // Libera la imagen antes del cierre del dibujante.
  ~BannerLanzador() {
    cerrar();
  }
  // Inicia un único pase con reloj de plataforma, independiente del juego.
  void iniciar(Uint64 ahora) {
    inicio = ahora;
    habilitado = true;
  }
  // Mantiene un negro final antes de entregar la habitación.
  bool activo(Uint64 ahora) const {
    return habilitado && (ahora < inicio || ahora - inicio < duracion());
  }
  // Expone el plazo total para programar el encendido dos segundos después.
  static constexpr Uint64 duracion() {
    return (banner_entrada_ms + banner_visible_ms + banner_salida_ms) * 1000000ull;
  }
  // Elige por proporción con desempate estable; cubre la ventana sin deformar la imagen.
  static const VarianteBanner &variante(int ancho, int alto) {
    const VarianteBanner *mejor = &variantes_banner[0];
    float distancia = 1000;
    for (const auto &v : variantes_banner) {
      const float d = std::abs(std::log((float(ancho) / alto) / (float(v.ancho) / v.alto)));
      if (d < distancia) {
        mejor = &v;
        distancia = d;
      }
    }
    return *mejor;
  }
  // Fundidos suaves sobre negro; el banner no recibe los filtros del juego.
  Uint8 opacidad(Uint64 ahora) const {
    const float ms = float(ahora >= inicio ? ahora - inicio : 0) / 1000000;
    float t = ms < banner_entrada_ms
                  ? ms / banner_entrada_ms
                  : 1 - (ms - banner_entrada_ms - banner_visible_ms) / banner_salida_ms;
    t = std::clamp(t, 0.0f, 1.0f);
    return Uint8(std::round(255 * t * t * (3 - 2 * t)));
  }
  // Dibuja en píxeles físicos, también tras redimensionar o entrar en pantalla completa.
  bool dibujar(SDL_Renderer *r, Uint64 ahora) {
    int w = 0, h = 0;
    if (!SDL_GetRenderOutputSize(r, &w, &h) || w <= 0 || h <= 0)
      return false;
    const auto &v = variante(w, h);
    if (cargada != &v) {
      cerrar();
      SDL_Surface *s = SDL_LoadPNG((std::string(SDL_GetBasePath()) + v.archivo).c_str());
      if (!s)
        return false;
      textura = SDL_CreateTextureFromSurface(r, s);
      SDL_DestroySurface(s);
      if (!textura || !SDL_SetTextureScaleMode(textura, SDL_SCALEMODE_LINEAR) ||
          !SDL_SetTextureBlendMode(textura, SDL_BLENDMODE_BLEND))
        return false;
      cargada = &v;
    }
    const float e = std::max(float(w) / v.ancho, float(h) / v.alto);
    const SDL_FRect destino{(w - v.ancho * e) / 2, (h - v.alto * e) / 2, v.ancho * e, v.alto * e};
    return SDL_SetRenderViewport(r, nullptr) && SDL_SetRenderScale(r, 1, 1) &&
           SDL_SetRenderLogicalPresentation(r, 0, 0, SDL_LOGICAL_PRESENTATION_DISABLED) &&
           SDL_SetRenderDrawColor(r, 0, 0, 0, 255) && SDL_RenderClear(r) &&
           SDL_SetTextureAlphaMod(textura, opacidad(ahora)) &&
           SDL_RenderTexture(r, textura, nullptr, &destino);
  }
  // Permite repetir el cierre sin perder el reloj de la presentación.
  void cerrar() {
    if (textura)
      SDL_DestroyTexture(textura);
    textura = nullptr;
    cargada = nullptr;
  }

private:
  Uint64 inicio = 0;
  bool habilitado = false;
  SDL_Texture *textura = nullptr;
  const VarianteBanner *cargada = nullptr;
};
