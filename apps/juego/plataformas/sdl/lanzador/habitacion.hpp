#pragma once
#include <SDL3/SDL.h>
#include <utility>

struct VarianteHabitacion {
  const char *archivo;
  int ancho, alto;
  SDL_FRect pantalla, boton_pc, boton_monitor, piloto_pc, piloto_monitor;
  bool boton_pc_auxiliar;
  SDL_FPoint cristal[16];
};
struct TemaHabitacion {
  const char *id, *nombre;
  unsigned int inicio, cantidad;
};

// Presentación opcional de plataforma, independiente de marcos, escenas y partidas DOS.
class Habitacion {
public:
  // Devuelve solo los diseños presentes en el catálogo compilado de esta entrega.
  static unsigned int cantidad_temas();
  // Libera sus imágenes antes de destruir el dibujante.
  ~Habitacion() {
    cerrar();
  }
  // Todo arranque normal empieza en la habitación con el ordenador apagado.
  void iniciar();
  // Programa solo el primer encendido; los botones posteriores conservan control manual.
  void programar_encendido(Uint64 ahora) {
    encendido_programado = ahora;
  }
  // Entrega solo pulsaciones aceptadas de la torre o del monitor, nunca clics del juego.
  unsigned int tomar_sonido() {
    return std::exchange(sonido_pendiente, 0u);
  }
  // Deja el arranque DOS original de las comparaciones fuera de esta presentación.
  bool activa() const {
    return habilitada;
  }
  // Suspende lógica y audio mientras el ordenador está apagado o arrancando.
  bool reiniciando() const {
    return habilitada && estado != Estado::encendido;
  }
  // Cubre el juego durante los apagados y entrega una sola solicitud de inicio tras BIOS/DOS.
  bool tomar_reinicio(Uint64 ahora = SDL_GetTicksNS());
  // Consume botones, zoom, cambio de habitación y clics que caen fuera de la pantalla del PC.
  bool evento(SDL_Window *ventana, SDL_Renderer *dibujante, const SDL_Event &evento,
              Uint64 ahora = SDL_GetTicksNS());
  // Comparte con el ratón la transformación física de la pantalla y de todos sus controles.
  bool geometria(SDL_Renderer *dibujante, SDL_FRect &imagen, SDL_FRect &pantalla, SDL_FRect &juego,
                 Uint64 ahora = SDL_GetTicksNS()) const;
  // Configura el área jugable sin deformarla durante el acercamiento o al redimensionar.
  bool preparar_encuadre(SDL_Renderer *dibujante, Uint64 ahora = SDL_GetTicksNS()) const;
  // Dibuja el exterior, los pilotos y la presentación de BIOS/DOS sin modificar el juego.
  bool dibujar(SDL_Renderer *dibujante, Uint64 ahora = SDL_GetTicksNS());
  // Expone los mismos rectángulos que usa el clic para las comprobaciones de integración.
  bool controles(SDL_Renderer *dibujante, SDL_FRect &pc, SDL_FRect &monitor,
                 Uint64 ahora = SDL_GetTicksNS()) const;
  // La habitación deja espacio a los marcos únicamente cuando el juego está ya ampliado.
  bool ampliada(Uint64 ahora = SDL_GetTicksNS()) const;
  // Conserva la línea CRT al apagar; el resto del tiempo cubre o muestra el juego completo.
  SDL_FRect destino_juego(Uint64 ahora = SDL_GetTicksNS()) const;
  // Libera texturas y permite repetir el cierre durante cambios de variante.
  void cerrar();

private:
  enum class Estado { apagado, arrancando, encendido, apagando };
  Estado estado = Estado::encendido;
  bool habilitada = false;
  unsigned int sonido_pendiente = 0;
  bool imagen_al_apagar = false;
  unsigned int tema = 0;
  Uint64 inicio_estado = 0, inicio_zoom = 0, encendido_programado = 0;
  // Comparte el mismo arranque y sonido entre el botón y el plazo automático.
  void encender(Uint64 ahora);
  float zoom_origen = 0, zoom_destino = 0;
  SDL_Texture *textura = nullptr;
  SDL_Texture *pilotos_rojos[2] = {nullptr, nullptr};
  mutable SDL_Texture *textura_bios = nullptr;
  const VarianteHabitacion *cargada = nullptr;
  // Elige la proporción más próxima en escala logarítmica, con desempate estable.
  const VarianteHabitacion &variante(int ancho, int alto) const;
  // Interpola el mismo encuadre al invertir el acercamiento.
  float nivel_zoom(Uint64 ahora) const;
  // Solicita acercarse o volver a la habitación sin reiniciar la sesión.
  void ajustar_zoom(float destino, Uint64 ahora);
  // Presentación de época compartida con los siete segundos del arranque.
  bool dibujar_arranque(SDL_Renderer *dibujante, const SDL_FRect &juego, Uint64 ahora) const;
};
