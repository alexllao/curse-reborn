#pragma once
#include <SDL3/SDL.h>
#include <string>

// Estadística del perfil, independiente de DS, las partidas DOS y los pasos del motor.
class TiempoJuego {
public:
  // Guarda únicamente el tiempo ya medido antes de liberar el perfil.
  ~TiempoJuego();
  // Usa el perfil SDL o una ruta explícita para las pruebas; conserva archivos no reconocidos.
  void iniciar(const std::string &archivo = "");
  // Acumula reloj monotónico entre muestras activas, descartando interrupciones de más de 5 s.
  void actualizar(bool activo, Uint64 ahora = SDL_GetTicksNS());
  // Persiste mediante un temporal del mismo directorio; no toca CURSE.SAV.
  bool guardar();
  // Guarda y registra la sesión una sola vez; permite cerrar antes de SDL_Quit.
  void cerrar();
  // Incluye todas las sesiones conocidas de este perfil, sin asignarlas a una ranura DOS.
  Uint64 milisegundos_totales() const;
  // Expone el tiempo de esta ejecución para los futuros metadatos de guardado.
  Uint64 milisegundos_sesion() const;
  // Informa de un perfil ilegible sin impedir que continúe el juego.
  bool admite_guardado() const;
  // Comparte la definición de actividad entre el bucle SDL y las pruebas deterministas.
  static bool debe_contar(bool jugable, SDL_WindowFlags ventana, bool panel, bool apagado);
  // Forma horas, minutos y segundos sin limitar las horas a veinticuatro.
  static std::string formatear(Uint64 milisegundos);

private:
  std::string ruta;
  Uint64 total_ns = 0, sesion_ns = 0, muestra_anterior = 0, ultimo_guardado = 0;
  bool iniciado = false, persistente = false, primera_muestra = true;
  bool contando = false, pendiente = false, aviso_guardado = false;
};
