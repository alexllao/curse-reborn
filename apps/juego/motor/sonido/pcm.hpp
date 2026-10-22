#pragma once

#include "altavoz.hpp"

namespace motor {

// Adaptación digital del canal dos del PIT utilizado por 7AC6, independiente de SDL.
class AltavozPcm {
public:
  static constexpr unsigned int frecuencia = 48000;
  // Reinicia fase, prioridad y reloj sin modificar el segmento del juego.
  void reiniciar();
  // 5288 y 7A46: conserva selección, prioridades y secuencias originales.
  bool reproducir(unsigned char *datos, unsigned short entidad, unsigned char identificador,
                  bool *iniciada = nullptr);
  // Genera PCM mono de 16 bits e invoca 7AC6 cada 4250h ciclos, como IRQ0 en 0240.
  bool generar(unsigned char *datos, short *salida, unsigned int cantidad);
  // Adapta las escrituras B6/42/61 del reproductor original al contador de onda cuadrada.
  void escribir(unsigned short puerto, unsigned char valor);
  // Proporciona el valor persistente del puerto 61h para las operaciones de encendido/apagado.
  unsigned char leer(unsigned short puerto) const;
  // 7A61: apaga la secuencia y su compuerta sin alterar las peculiaridades del estado original.
  bool detener(unsigned char *datos);

private:
  EstadoSonido seleccion{};
  unsigned int divisor = 65536, fase = 0, hasta_interrupcion = 0x4250, fraccion = 0;
  unsigned short pendiente = 0;
  unsigned char control = 0;
  bool segundo_byte = false, programado = false;
  // Puente de lectura para la rutina reconstruida; no accede a puertos reales del equipo.
  static unsigned char leer_puerto(void *contexto, unsigned short puerto);
  // Puente de escritura para la rutina reconstruida; no accede a puertos reales del equipo.
  static void escribir_puerto(void *contexto, unsigned short puerto, unsigned char valor);
};

} // namespace motor
