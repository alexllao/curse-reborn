#pragma once
#include <SDL3/SDL.h>

struct PanelMarco {
  float x, y, ancho, alto, margen;
};
struct DefinicionMarco {
  const char *id;
  const char *nombre;
  const char *archivo;
  PanelMarco izquierda, derecha;
  float solape;
  bool cubrir, reflejar_derecha;
};
struct AsignacionMarco {
  const char *pantalla;
  const char *marco;
};

// Los marcos propios rellenan las bandas y admiten un solape mínimo sobre sus bordes.
class MarcosPantalla {
public:
  // Libera la textura antes de cerrar el dibujante SDL.
  ~MarcosPantalla() {
    cerrar();
  }
  // F1 muestra u oculta los marcos sin avanzar las introducciones ni alterar la partida.
  void alternar();
  // Rellena el espacio sobrante y restaura las coordenadas lógicas usadas por ratón y juego.
  bool dibujar(SDL_Renderer *dibujante, const char *pantalla);
  // Permite cerrar la capa antes de SDL_Quit; admite llamadas repetidas.
  void cerrar();

private:
  bool visible = false;
  SDL_Texture *textura = nullptr;
  const DefinicionMarco *actual = nullptr;
};
