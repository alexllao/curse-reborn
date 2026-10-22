#pragma once
#include <SDL3/SDL.h>
#include <string>
#include "texto_hud.hpp"

// Texto temporal de presentación: no modifica el reloj, el estado DOS ni el audio.
class AvisosHud {
public:
  // Las comparaciones del juego conservan sus píxeles de referencia sin avisos añadidos.
  explicit AvisosHud(bool habilitar = true) : habilitado(habilitar) {}
  // Sustituye el aviso y prolonga su lectura al repetir una tecla, evitando parpadeos.
  void mostrar(const std::string &mensaje, Uint64 ahora = SDL_GetTicks()) {
    const bool leyendo = activo && ahora >= inicio && ahora - inicio < 1820;
    texto = mensaje;
    inicio = leyendo && ahora >= 120 ? ahora - 120 : ahora;
    activo = true;
  }
  // Dibuja letras blancas en la esquina física y restituye el encuadre y la mezcla del juego.
  bool dibujar(SDL_Renderer *r, Uint64 ahora = SDL_GetTicks()) {
    if (!habilitado || !activo)
      return true;
    const Uint64 transcurrido = ahora >= inicio ? ahora - inicio : 0;
    if (transcurrido >= 1820) {
      activo = false;
      return true;
    }
    const auto alfa =
        static_cast<Uint8>(transcurrido < 120     ? transcurrido * 255 / 120
                           : transcurrido <= 1320 ? 255
                                                  : (1820 - transcurrido) * 255 / 500);
    if (!alfa)
      return true;
    SDL_Rect viewport;
    const bool viewport_fijado = SDL_RenderViewportSet(r);
    float sx = 1, sy = 1;
    int ancho = 0, alto = 0, logico_ancho = 0, logico_alto = 0;
    SDL_RendererLogicalPresentation presentacion;
    SDL_BlendMode mezcla;
    SDL_Color color;
    if (!SDL_GetRenderViewport(r, &viewport) || !SDL_GetRenderScale(r, &sx, &sy) ||
        !SDL_GetRenderOutputSize(r, &ancho, &alto) ||
        !SDL_GetRenderLogicalPresentation(r, &logico_ancho, &logico_alto, &presentacion) ||
        !SDL_GetRenderDrawBlendMode(r, &mezcla) ||
        !SDL_GetRenderDrawColor(r, &color.r, &color.g, &color.b, &color.a))
      return false;
    float escala = alto >= 1440 ? 3.0f : alto >= 720 ? 2.0f : 1.0f;
    while (escala > 1 && texto.size() * SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE * escala > ancho - 24)
      --escala;
    if (!SDL_SetRenderViewport(r, nullptr) ||
        !SDL_SetRenderLogicalPresentation(r, 0, 0, SDL_LOGICAL_PRESENTATION_DISABLED) ||
        !SDL_SetRenderScale(r, escala, escala))
      return false;
    const bool dibujado = SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND) &&
                          SDL_SetRenderDrawColor(r, 255, 255, 255, alfa) &&
                          dibujar_texto_hud(r, 12 / escala, 12 / escala, texto);
    const bool encuadre =
        SDL_SetRenderLogicalPresentation(r, logico_ancho, logico_alto, presentacion) &&
        SDL_SetRenderViewport(r, viewport_fijado ? &viewport : nullptr) &&
        SDL_SetRenderScale(r, sx, sy);
    const bool mezclado = SDL_SetRenderDrawBlendMode(r, mezcla);
    const bool coloreado = SDL_SetRenderDrawColor(r, color.r, color.g, color.b, color.a);
    return dibujado && encuadre && mezclado && coloreado;
  }

private:
  bool habilitado = true, activo = false;
  Uint64 inicio = 0;
  std::string texto;
};
