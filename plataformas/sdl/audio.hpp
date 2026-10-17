#pragma once

#include <SDL3/SDL.h>
#include "../../motor/sonido/pcm.hpp"
#include "musica_adlib.hpp"
#include <string>

// Salida compartida entre escenas; toda la lógica del juego permanece en el hilo principal.
class Audio {
public:
  // Abre una salida mono; SDL convierte al formato y frecuencia del dispositivo real.
  bool abrir(const std::string &carpeta);
  // Cierra el flujo antes de SDL_Quit, incluso cuando falla la carga de una escena.
  void cerrar();
  // Asocia una sesión nueva, conserva el dispositivo y limpia efectos de la sesión anterior.
  bool conectar(unsigned char *segmento);
  // Deja de atender los efectos de la sesión; el tema continúa en la presentación.
  void desconectar();
  // Produce el tiempo transcurrido, también con menú abierto o durante fundidos.
  bool actualizar();
  // Atiende una orden de 5288 después de llevar el reloj del altavoz hasta el momento actual.
  bool reproducir(unsigned char *segmento, unsigned short entidad, unsigned char identificador);
  // Silencia la salida audible; las secuencias y prioridades originales siguen avanzando.
  bool alternar_silencio();
  // F4 recorre las opciones implementadas; informa de música y efectos activos en la consola.
  bool alternar_dispositivo();

private:
  SDL_AudioStream *flujo = nullptr;
  unsigned char *datos = nullptr;
  motor::AltavozPcm altavoz{};
  MusicaAdlib musica{};
  Uint64 ultimo = 0, resto = 0;
  bool silencio = false;
  bool musica_audible = true;
};

// Comprueba la apertura y alimentación SDL con un WAV generado por las pruebas del motor.
bool comprobar_salida_audio(const char *ruta);
// Genera un tema real, contrasta particiones y reloj y guarda un WAV local para su escucha.
bool comprobar_musica_adlib(const std::string &carpeta, const char *ruta);
