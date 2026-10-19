#include "contador_fps.hpp"
#include "registro.hpp"
#include <cstdio>
#include <cstring>

// Recorta las esquinas por filas sin superponer transparencias en el fondo del indicador.
static bool dibujar_placa_redondeada(SDL_Renderer *dibujante, const SDL_FRect &placa) {
  constexpr int recortes[] = {3, 1, 1, 0};
  const int alto = static_cast<int>(placa.h);
  for (int fila = 0; fila < alto; ++fila) {
    const int distancia = fila < alto / 2 ? fila : alto - 1 - fila;
    const float recorte = distancia < 4 ? static_cast<float>(recortes[distancia]) : 0.0f;
    const SDL_FRect franja{placa.x + recorte, placa.y + static_cast<float>(fila),
                           placa.w - 2.0f * recorte, 1.0f};
    if (!SDL_RenderFillRect(dibujante, &franja))
      return false;
  }
  return true;
}

// F12 no altera el avance del personaje, las animaciones ni la reproducción del audio.
void ContadorFps::alternar() {
  visible = !visible;
  registrar_accion("Contador FPS: %s", visible ? "visible" : "oculto");
}

// Actualiza la media cada medio segundo y restaura el encuadre y la mezcla después del HUD.
bool ContadorFps::dibujar(SDL_Renderer *dibujante) {
  const auto ahora = SDL_GetTicksNS();
  if (!inicio)
    inicio = ahora;
  else {
    ++fotogramas;
    const auto duracion = ahora - inicio;
    if (duracion >= 500000000u) {
      std::snprintf(texto, sizeof(texto), "FPS %.0f",
                    static_cast<double>(fotogramas) * 1000000000.0 / static_cast<double>(duracion));
      fotogramas = 0;
      inicio = ahora;
    }
  }
  if (!visible)
    return true;
  int ancho = 0, alto = 0, logico_ancho = 0, logico_alto = 0;
  SDL_RendererLogicalPresentation presentacion;
  SDL_BlendMode mezcla;
  SDL_Rect viewport;
  const bool viewport_fijado = SDL_RenderViewportSet(dibujante);
  float sx = 1, sy = 1;
  if (!SDL_GetRenderViewport(dibujante, &viewport) || !SDL_GetRenderScale(dibujante, &sx, &sy))
    return false;
  if (!SDL_GetRenderOutputSize(dibujante, &ancho, &alto) ||
      !SDL_GetRenderLogicalPresentation(dibujante, &logico_ancho, &logico_alto, &presentacion) ||
      !SDL_GetRenderDrawBlendMode(dibujante, &mezcla))
    return false;
  if (!SDL_SetRenderViewport(dibujante, nullptr) || !SDL_SetRenderScale(dibujante, 1, 1) ||
      !SDL_SetRenderLogicalPresentation(dibujante, 0, 0, SDL_LOGICAL_PRESENTATION_DISABLED))
    return false;
  const float ancho_texto =
      static_cast<float>(std::strlen(texto) * SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE);
  const float ancho_placa = ancho_texto + 12.0f;
  const SDL_FRect placa{static_cast<float>(ancho) - ancho_placa - 8.0f, 8.0f, ancho_placa, 16.0f};
  const bool dibujado =
      SDL_SetRenderDrawBlendMode(dibujante, SDL_BLENDMODE_BLEND) &&
      SDL_SetRenderDrawColor(dibujante, 0, 0, 0, 176) &&
      dibujar_placa_redondeada(dibujante, placa) &&
      SDL_SetRenderDrawColor(dibujante, 224, 240, 224, 255) &&
      SDL_RenderDebugText(dibujante, placa.x + (placa.w - ancho_texto) / 2.0f,
                          placa.y + (placa.h - SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE) / 2.0f, texto);
  const bool encuadre =
      SDL_SetRenderLogicalPresentation(dibujante, logico_ancho, logico_alto, presentacion) &&
      SDL_SetRenderViewport(dibujante, viewport_fijado ? &viewport : nullptr) &&
      SDL_SetRenderScale(dibujante, sx, sy);
  const bool mezclado = SDL_SetRenderDrawBlendMode(dibujante, mezcla);
  return dibujado && encuadre && mezclado;
}
