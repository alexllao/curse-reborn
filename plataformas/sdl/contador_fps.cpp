#include "contador_fps.hpp"
#include "registro.hpp"
#include <cstdio>

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
  if (!SDL_GetRenderOutputSize(dibujante, &ancho, &alto) ||
      !SDL_GetRenderLogicalPresentation(dibujante, &logico_ancho, &logico_alto, &presentacion) ||
      !SDL_GetRenderDrawBlendMode(dibujante, &mezcla))
    return false;
  if (!SDL_SetRenderLogicalPresentation(dibujante, 0, 0, SDL_LOGICAL_PRESENTATION_DISABLED))
    return false;
  const SDL_FRect placa{static_cast<float>(ancho - 88), 8.0f, 80.0f, 18.0f};
  const bool dibujado = SDL_SetRenderDrawBlendMode(dibujante, SDL_BLENDMODE_BLEND) &&
                        SDL_SetRenderDrawColor(dibujante, 0, 0, 0, 176) &&
                        SDL_RenderFillRect(dibujante, &placa) &&
                        SDL_SetRenderDrawColor(dibujante, 224, 240, 224, 255) &&
                        SDL_RenderDebugText(dibujante, placa.x + 4, placa.y + 5, texto);
  const bool encuadre =
      SDL_SetRenderLogicalPresentation(dibujante, logico_ancho, logico_alto, presentacion);
  const bool mezclado = SDL_SetRenderDrawBlendMode(dibujante, mezcla);
  return dibujado && encuadre && mezclado;
}
