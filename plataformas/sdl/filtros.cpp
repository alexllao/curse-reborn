#include "filtros.hpp"
#include "registro.hpp"
#include "escaladores_pixel.hpp"

// La propiedad de la textura queda en la plataforma y nunca alcanza los recursos originales.
void FiltrosPantalla::cerrar() {
  if (textura)
    SDL_DestroyTexture(textura);
  textura = nullptr;
}

// Cambia el aspecto sin reiniciar música, animaciones, menús ni partida.
bool FiltrosPantalla::alternar(SDL_Renderer *dibujante, SDL_Texture *original,
                               const unsigned char *rgba) {
  const unsigned int siguiente = (modo + 1) % 6;
  if (siguiente >= 2 && !textura) {
    textura =
        SDL_CreateTexture(dibujante, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, 640, 400);
    if (!textura)
      return false;
    pixeles.resize(640u * 400u * 4u);
  }
  if (!SDL_SetTextureScaleMode(original,
                               siguiente == 1 ? SDL_SCALEMODE_LINEAR : SDL_SCALEMODE_NEAREST))
    return false;
  if (siguiente >= 2 && !SDL_SetTextureScaleMode(textura, siguiente == 3 ? SDL_SCALEMODE_LINEAR
                                                                         : SDL_SCALEMODE_NEAREST))
    return false;
  modo = siguiente;
  if (!actualizar(original, rgba))
    return false;
  constexpr const char *nombres[] = {
      "original (sin filtro)", "suavizado", "líneas de escaneo", "CRT suave", "xBRZ 2x", "2xSaI"};
  registrar_accion("Filtro de pantalla: %s", nombres[modo]);
  return true;
}

// Escaneo a doble resolución; el CRT añade mezcla horizontal, máscara RGB y viñeta suaves.
bool FiltrosPantalla::actualizar(SDL_Texture *original, const unsigned char *rgba) {
  if (!SDL_SetTextureScaleMode(original, !suspendidos && modo == 1 ? SDL_SCALEMODE_LINEAR
                                                                   : SDL_SCALEMODE_NEAREST) ||
      !SDL_UpdateTexture(original, nullptr, rgba, 320 * 4))
    return false;
  if (suspendidos || modo < 2)
    return true;
  if (modo >= 4) {
    if (modo == 4)
      escaladores_pixel::xbrz(rgba, pixeles.data(), 320, 200);
    else
      escaladores_pixel::sai(rgba, pixeles.data(), 320, 200);
    return SDL_UpdateTexture(textura, nullptr, pixeles.data(), 640 * 4);
  }
  for (int y = 0; y < 400; ++y) {
    const unsigned int linea = (y & 1) ? 178u : 256u;
    const int dy = 2 * y - 399;
    for (int x = 0; x < 640; ++x) {
      const unsigned int origen = static_cast<unsigned int>(((y / 2) * 320 + x / 2) * 4);
      const unsigned int destino = static_cast<unsigned int>((y * 640 + x) * 4);
      const int vecino = (x & 1) ? (x / 2 < 319 ? x / 2 + 1 : 319) : (x / 2 > 0 ? x / 2 - 1 : 0);
      const unsigned int adyacente = static_cast<unsigned int>(((y / 2) * 320 + vecino) * 4);
      const int dx = 2 * x - 639;
      const unsigned int vineta =
          static_cast<unsigned int>(256 - 20 * dx * dx / (640 * 640) - 16 * dy * dy / (400 * 400));
      for (unsigned int canal = 0; canal < 3; ++canal) {
        unsigned int color = rgba[origen + canal];
        if (modo == 3) {
          color = (3 * color + rgba[adyacente + canal]) / 4;
          color = color * (static_cast<unsigned int>(x % 3) == canal ? 256u : 220u) / 256u;
          color = color * vineta / 256u;
        }
        pixeles[destino + canal] = static_cast<unsigned char>(color * linea / 256u);
      }
      pixeles[destino + 3] = 255;
    }
  }
  return SDL_UpdateTexture(textura, nullptr, pixeles.data(), 640 * 4);
}
