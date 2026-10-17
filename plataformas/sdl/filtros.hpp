#pragma once
#include <SDL3/SDL.h>
#include <vector>

// Efectos opcionales de presentación; reciben únicamente RGBA y no acceden al estado del juego.
class FiltrosPantalla {
public:
  // Libera la textura auxiliar antes de cerrar el dibujante SDL.
  ~FiltrosPantalla() {
    cerrar();
  }
  // Cierra la salida auxiliar de forma idempotente.
  void cerrar();
  // F11 recorre original, suavizado, líneas de escaneo, CRT suave, xBRZ 2x y 2xSaI.
  bool alternar(SDL_Renderer *dibujante, SDL_Texture *original, const unsigned char *rgba);
  // Actualiza la textura original y, cuando corresponde, la imagen auxiliar de 640 por 400.
  bool actualizar(SDL_Texture *original, const unsigned char *rgba);
  // El modo cero conserva exactamente la textura original y su muestreo por vecino más próximo.
  SDL_Texture *textura_visible(SDL_Texture *original) const {
    return modo < 2 ? original : textura;
  }

private:
  unsigned int modo = 0;
  SDL_Texture *textura = nullptr;
  std::vector<unsigned char> pixeles;
};
