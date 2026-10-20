#include "filtros.hpp"
#include "../registro.hpp"
#include "escaladores_pixel.hpp"
#include <cstring>

namespace {
// Misma transformación de RGBA para la escena y las ilustraciones laterales.
void filtrar_rgba(const unsigned char *rgba, unsigned char *salida, int ancho, int alto,
                  unsigned int modo) {
  if (modo >= 4) {
    if (modo == 4)
      escaladores_pixel::xbrz(rgba, salida, ancho, alto);
    else
      escaladores_pixel::sai(rgba, salida, ancho, alto);
    return;
  }
  for (int y = 0; y < (alto * 2); ++y) {
    const unsigned int linea = (y & 1) ? 178u : 256u;
    const int dy = 2 * y - (alto * 2 - 1);
    for (int x = 0; x < (ancho * 2); ++x) {
      const unsigned int origen = static_cast<unsigned int>(((y / 2) * ancho + x / 2) * 4);
      const unsigned int destino = static_cast<unsigned int>((y * (ancho * 2) + x) * 4);
      const int vecino =
          (x & 1) ? (x / 2 < (ancho - 1) ? x / 2 + 1 : (ancho - 1)) : (x / 2 > 0 ? x / 2 - 1 : 0);
      const unsigned int adyacente = static_cast<unsigned int>(((y / 2) * ancho + vecino) * 4);
      const int dx = 2 * x - (ancho * 2 - 1);
      const unsigned int vineta =
          static_cast<unsigned int>(256 - 20 * dx * dx / ((ancho * 2) * (ancho * 2)) -
                                    16 * dy * dy / ((alto * 2) * (alto * 2)));
      for (unsigned int canal = 0; canal < 3; ++canal) {
        unsigned int color = rgba[origen + canal];
        if (modo == 3) {
          color = (3 * color + rgba[adyacente + canal]) / 4;
          color = color * (static_cast<unsigned int>(x % 3) == canal ? 256u : 220u) / 256u;
          color = color * vineta / 256u;
        }
        salida[destino + canal] = static_cast<unsigned char>(color * linea / 256u);
      }
      salida[destino + 3] = 255;
    }
  }
}
} // namespace

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
  filtrar_rgba(rgba, pixeles.data(), 320, 200, modo);
  return SDL_UpdateTexture(textura, nullptr, pixeles.data(), 640 * 4);
}

// Se prepara solo al cambiar el marco o F11, nunca por fotograma. El arte maestro queda intacto.
SDL_Surface *FiltrosPantalla::filtrar_marco(SDL_Surface *imagen) const {
  if (indice_actual() < 2)
    return SDL_ConvertSurface(imagen, SDL_PIXELFORMAT_RGBA32);
  const int alto = 200;
  const int ancho = std::max(1, imagen->w * alto / imagen->h);
  SDL_Surface *base = SDL_CreateSurface(ancho, alto, SDL_PIXELFORMAT_RGBA32);
  SDL_Surface *doble = SDL_CreateSurface(ancho * 2, alto * 2, SDL_PIXELFORMAT_RGBA32);
  SDL_Surface *resultado = SDL_CreateSurface(imagen->w, imagen->h, SDL_PIXELFORMAT_RGBA32);
  bool ok = base && doble && resultado && SDL_SetSurfaceBlendMode(imagen, SDL_BLENDMODE_NONE) &&
            SDL_BlitSurfaceScaled(imagen, nullptr, base, nullptr, SDL_SCALEMODE_LINEAR);
  if (ok) {
    std::vector<unsigned char> entrada(static_cast<size_t>(ancho) * alto * 4);
    std::vector<unsigned char> salida(static_cast<size_t>(ancho) * alto * 16);
    for (int y = 0; y < alto; ++y)
      std::memcpy(entrada.data() + y * ancho * 4,
                  static_cast<unsigned char *>(base->pixels) + y * base->pitch, ancho * 4);
    filtrar_rgba(entrada.data(), salida.data(), ancho, alto, indice_actual());
    // Los escaladores RGB son opacos; el canal alfa conserva el relieve del marco original.
    for (int y = 0; y < alto * 2; ++y) {
      for (int x = 0; x < ancho * 2; ++x)
        salida[(y * ancho * 2 + x) * 4 + 3] = entrada[((y / 2) * ancho + x / 2) * 4 + 3];
      std::memcpy(static_cast<unsigned char *>(doble->pixels) + y * doble->pitch,
                  salida.data() + y * ancho * 8, ancho * 8);
    }
    ok = SDL_SetSurfaceBlendMode(doble, SDL_BLENDMODE_NONE) &&
         SDL_BlitSurfaceScaled(doble, nullptr, resultado, nullptr,
                               indice_actual() == 3 ? SDL_SCALEMODE_LINEAR : SDL_SCALEMODE_NEAREST);
  }
  SDL_DestroySurface(base);
  SDL_DestroySurface(doble);
  if (!ok) {
    SDL_DestroySurface(resultado);
    return nullptr;
  }
  return resultado;
}
