#pragma once
#include <SDL3/SDL.h>
class FiltrosPantalla;

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
  bool panoramico = false;
  float proporcion = 0;
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
  // F1 alterna el marco decorativo sin modificar la habitación ni avanzar la partida.
  void alternar(Uint64 ahora = SDL_GetTicksNS());
  bool activado() const {
    return visible;
  }
  const char *nombre_actual() const {
    return visible ? "Cielo encantado" : "Sin marco";
  }
  // Mantiene el máximo salvo que se elija manualmente alejar el monitor.
  bool preparar_encuadre(SDL_Renderer *dibujante, Uint64 ahora = SDL_GetTicksNS()) const;
  // Devuelve el área física del juego, también dentro de la carcasa del monitor.
  static bool area_juego(SDL_Renderer *dibujante, SDL_FRect &juego);
  // Rellena el espacio sobrante y restaura las coordenadas lógicas usadas por ratón y juego.
  bool dibujar(SDL_Renderer *dibujante, const char *pantalla, Uint64 ahora = SDL_GetTicksNS(),
               const FiltrosPantalla *filtros = nullptr);
  // Permite cerrar la capa antes de SDL_Quit; admite llamadas repetidas.
  void cerrar();

private:
  int filtro_actual = -1;
  bool visible = false;
  float origen_animacion = 0.0f;
  Uint64 inicio_animacion = 0;
  // Interpola desde el nivel actual para que otra pulsación invierta el recorrido sin saltos.
  float nivel_animacion(Uint64 ahora) const;
  SDL_Texture *textura = nullptr;
  const DefinicionMarco *actual = nullptr;
};
