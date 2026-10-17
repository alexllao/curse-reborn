#pragma once

#include <SDL3/SDL.h>
#include "../../motor/sonido/pcm.hpp"

// Salida compartida entre escenas; toda la lógica del juego permanece en el hilo principal.
class Audio {
public:
  // Abre una salida mono; SDL convierte al formato y frecuencia del dispositivo real.
  bool abrir();
  // Cierra el flujo antes de SDL_Quit, incluso cuando falla la carga de una escena.
  void cerrar();
  // Asocia una sesión nueva, conserva el dispositivo y limpia efectos de la sesión anterior.
  bool conectar(unsigned char *segmento);
  // Deja de atender la sesión al volver a las pantallas de presentación.
  void desconectar();
  // Produce el tiempo transcurrido, también con menú abierto o durante fundidos.
  bool actualizar();
  // Atiende una orden de 5288 después de llevar el reloj del altavoz hasta el momento actual.
  bool reproducir(unsigned char *segmento, unsigned short entidad, unsigned char identificador);
  // Silencia la salida audible; las secuencias y prioridades originales siguen avanzando.
  bool alternar_silencio();

private:
  SDL_AudioStream *flujo = nullptr;
  unsigned char *datos = nullptr;
  motor::AltavozPcm altavoz{};
  Uint64 ultimo = 0, resto = 0;
  bool silencio = false;
};

// Comprueba la apertura y alimentación SDL con un WAV generado por las pruebas del motor.
bool comprobar_salida_audio(const char *ruta);
