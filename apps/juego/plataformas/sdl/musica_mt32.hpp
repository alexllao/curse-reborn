#pragma once
#include <SDL3/SDL.h>
#include <string>
#include <vector>
#define MT32EMU_API_TYPE 1
#define MT32EMU_RUNTIME_VERSION_CHECK 0
#include <mt32emu/mt32emu.h>
#include "../../motor/sonido/musica_rs.hpp"

// Reproductor común de los cinco recursos RS, con síntesis MT-32 de las ROM locales.
class MusicaMt32 {
public:
  // Valida las ROM mediante Munt y precarga recursos antes de iniciar el hilo de sonido.
  bool iniciar(const std::string &carpeta, const unsigned char *ejecutable, unsigned int longitud,
               unsigned short escena_inicial = 0xffff);
  // Libera el contexto antes de descargar la biblioteca compartida.
  void cerrar();
  // FFFF es presentación; las demás escenas usan exclusivamente la tabla musical original.
  bool cambiar_escena(unsigned short escena);
  // Atiende el reloj MPU de 120 pulsos por negra y produce PCM estéreo de 48 kHz; salida requiere
  // dos muestras por fotograma.
  bool generar(short *salida, unsigned int cantidad);
  // Consulta disponibilidad tras validar ambas ROM y abrir el sintetizador.
  bool disponible() const {
    return contexto != nullptr;
  }
  // Expone el banco seleccionado para la comprobación de transiciones y reutilización.
  unsigned char banco() const {
    return banco_actual;
  }

private:
  SDL_SharedObject *biblioteca = nullptr;
  mt32emu_context contexto = nullptr;
  decltype(&mt32emu_create_context) crear = nullptr;
  decltype(&mt32emu_free_context) liberar = nullptr;
  decltype(&mt32emu_add_rom_file) agregar_rom = nullptr;
  decltype(&mt32emu_get_rom_info) consultar_rom = nullptr;
  decltype(&mt32emu_set_analog_output_mode) fijar_modo_analogico = nullptr;
  decltype(&mt32emu_set_stereo_output_samplerate) fijar_frecuencia = nullptr;
  decltype(&mt32emu_open_synth) abrir_sintesis = nullptr;
  decltype(&mt32emu_play_msg) enviar = nullptr;
  decltype(&mt32emu_render_bit16s) sintetizar = nullptr;
  unsigned char datos[65536]{};
  std::vector<unsigned char> bancos[5];
  unsigned char banco_actual = 255, tempo = 0, estado = 0;
  unsigned short posicion = 0, espera = 0;
  unsigned int fraccion = 0;
  motor::EventoMusicaRS evento{};
  // Consume mensajes con demora cero y conserva el siguiente evento temporizado.
  bool preparar_evento();
};
