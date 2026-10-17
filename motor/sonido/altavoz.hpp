#pragma once

#include "seleccionar.hpp"

namespace motor {

struct PuertosAltavoz {
  void* contexto;
  unsigned char (*leer)(void*, unsigned short puerto);
  void (*escribir)(void*, unsigned short puerto, unsigned char valor);
};

// 7A46–7A60: programa la secuencia, sin emitir todavía una nota.
void iniciar_altavoz(unsigned char* datos, unsigned short secuencia);
// 7A61–7A76: limpia la referencia y apaga la salida; conserva el indicador 9924.
bool detener_altavoz(unsigned char* datos, const PuertosAltavoz& puertos);
// 7AC6–7B0C: una llamada del temporizador; duración y divisor son palabras.
bool avanzar_altavoz(unsigned char* datos, const PuertosAltavoz& puertos);
// Atiende una orden nula o una entrada a 7A46; otras modalidades siguen pendientes.
bool atender_orden_altavoz(unsigned char* datos, const OrdenSonido& orden);

}
