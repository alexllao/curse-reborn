#pragma once

#include "seleccionar.hpp"

namespace motor {
// Estado del primer bloque DMA de 7FB3; la cuenta de hardware representa longitud menos uno.
struct BloqueDma {
  unsigned short direccion, cuenta;
  unsigned char pagina;
  unsigned int restantes;
};
// 7FB3–801C: convierte segmento y posición en una transferencia que no cruza 64 KiB.
BloqueDma preparar_dma(unsigned short segmento, unsigned short posicion, unsigned int longitud);
// 038E y 4EC8/4FD1: obtiene el nombre DOS del banco general o de cualquiera de las 78 escenas.
bool nombre_banco_sonido(const unsigned char *datos, unsigned short escena, char *nombre);

// Sustituye DSP/DMA por PCM mono a 48 kHz; conserva selección y prioridad de 5288.
class DigitalPcm {
public:
  // Sustituye los bancos de la sesión y cancela la muestra anterior antes de cambiar sus
  // referencias.
  void conectar(const unsigned char *general, unsigned int longitud_general,
                const unsigned char *escena, unsigned int longitud_escena);
  // 5288: selecciona una muestra visible y permite interrumpirla únicamente con mayor prioridad.
  bool reproducir(unsigned char *datos, unsigned short entidad, unsigned char identificador);
  // Adapta la orden 7FB3; el byte canal de la orden es la constante temporal del DSP, no un canal
  // MIDI.
  bool iniciar(const OrdenSonido &orden);
  // Convierte PCM sin signo de ocho bits sin redondear la frecuencia 1000000/(256-constante).
  void generar(short *salida, unsigned int cantidad);
  // Equivale a parar DMA y liberar CS:8351; conserva los bancos cargados.
  void detener();
  // Refleja CS:8351 para comprobar prioridades y finalización sin acceder a hardware.
  bool activa() const {
    return estado.muestra_activa != 0;
  }

private:
  EstadoSonido estado{};
  const unsigned char *bancos[2]{};
  unsigned int longitudes[2]{};
  unsigned char muestra[65536]{};
  unsigned int longitud_muestra = 0;
  unsigned long long reloj = 0;
  unsigned int periodo = 0;
  // Lee palabras acotadas de los bancos uno y dos del selector, sin interpretarlos como segmentos
  // DOS.
  static bool leer(void *contexto, unsigned short recurso, unsigned short posicion,
                   unsigned short &valor);
};
} // namespace motor
