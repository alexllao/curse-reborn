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
struct DefinicionMonitor {
  const char *archivo;
  int ancho, alto;
  SDL_FRect pantalla;
  const char *fondo;
  int ancho_fondo, alto_fondo;
};

// Los marcos propios rellenan las bandas y admiten un solape mínimo sobre sus bordes.
class MarcosPantalla {
public:
  // Libera la textura antes de cerrar el dibujante SDL.
  ~MarcosPantalla() {
    cerrar();
  }
  // F1 recorre predeterminado, monitor y sin marco sin avanzar la partida.
  void alternar(Uint64 ahora = SDL_GetTicksNS());
  // Conserva el mismo encuadre máximo del juego con cualquiera de los marcos.
  bool preparar_encuadre(SDL_Renderer *dibujante, Uint64 ahora = SDL_GetTicksNS()) const;
  // Devuelve el área física del juego, también dentro de la carcasa del monitor.
  static bool area_juego(SDL_Renderer *dibujante, SDL_FRect &juego);
  // Rellena el espacio sobrante y restaura las coordenadas lógicas usadas por ratón y juego.
  bool dibujar(SDL_Renderer *dibujante, const char *pantalla, Uint64 ahora = SDL_GetTicksNS());
  // Permite cerrar la capa antes de SDL_Quit; admite llamadas repetidas.
  void cerrar();

private:
  bool visible = false;
  unsigned int modo = 0;
  bool monitor_en_salida = false;
  bool monitor_presentado = false;
  SDL_Texture *textura_monitor = nullptr;
  SDL_Texture *textura_habitacion = nullptr;
  float origen_animacion = 0.0f;
  Uint64 inicio_animacion = 0;
  // Interpola desde el nivel actual para que otra pulsación invierta el recorrido sin saltos.
  float nivel_animacion(Uint64 ahora) const;
  SDL_Texture *textura = nullptr;
  const DefinicionMarco *actual = nullptr;
};
