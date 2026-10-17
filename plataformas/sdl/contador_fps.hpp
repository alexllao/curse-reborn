#pragma once
#include <SDL3/SDL.h>

// Indicador de rendimiento de presentación, independiente del reloj lógico y de los filtros.
class ContadorFps {
public:
  // Las capturas de referencia desactivan el indicador salvo al comprobar expresamente F12.
  explicit ContadorFps(bool mostrar = true) : visible(mostrar) {}
  // Alterna únicamente la visibilidad e informa del cambio por consola.
  void alternar();
  // Mide los fotogramas presentados y dibuja una placa pequeña en píxeles físicos.
  bool dibujar(SDL_Renderer *dibujante);

private:
  bool visible = true;
  Uint64 inicio = 0;
  unsigned int fotogramas = 0;
  char texto[24] = "FPS --";
};
