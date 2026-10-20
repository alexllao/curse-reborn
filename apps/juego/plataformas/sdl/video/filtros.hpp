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
  // Nombre breve para el aviso blanco de F11, sin depender de la consola.
  const char *nombre_actual() const {
    static constexpr const char *nombres[] = {"Original",  "Suavizado", "Lineas de escaneo",
                                              "CRT suave", "xBRZ 2x",   "2xSaI"};
    return nombres[modo];
  }
  // El modo original conserva el cursor del sistema; los filtros componen la mano con la escena.
  bool aplicado() const {
    return !suspendidos && modo != 0;
  }
  // Omite los filtros en la habitación sin perder la selección para la vista ampliada.
  void suspender(bool pausa) {
    suspendidos = pausa;
  }
  // Actualiza la textura original y, cuando corresponde, la imagen auxiliar de 640 por 400.
  bool actualizar(SDL_Texture *original, const unsigned char *rgba);
  // El modo cero conserva exactamente la textura original y su muestreo por vecino más próximo.
  SDL_Texture *textura_visible(SDL_Texture *original) const {
    return suspendidos || modo < 2 ? original : textura;
  }

  // Procesa también los marcos con el mismo modo, conservando su tamaño y transparencia.
  SDL_Surface *filtrar_marco(SDL_Surface *imagen) const;
  unsigned int indice_elegido() const {
    return modo;
  }
  unsigned int indice_actual() const {
    return suspendidos ? 0 : modo;
  }

private:
  unsigned int modo = 0;
  bool suspendidos = false;
  SDL_Texture *textura = nullptr;
  std::vector<unsigned char> pixeles;
};
