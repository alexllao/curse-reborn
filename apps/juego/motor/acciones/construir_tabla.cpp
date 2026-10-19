#include "construir_tabla.hpp"

namespace motor {
namespace {

using Palabra = unsigned short;
static_assert(sizeof(Palabra) == 2, "Se necesitan palabras de 16 bits.");
static_assert(static_cast<unsigned char>(-1) == 0xff, "Se necesitan bytes de 8 bits.");
static_assert(static_cast<Palabra>(-1) == 0xffff, "Se necesitan palabras de 16 bits.");

Palabra leer_palabra(const unsigned char *datos, Palabra posicion) {
  return static_cast<Palabra>(
      datos[posicion] |
      (static_cast<unsigned int>(datos[static_cast<Palabra>(posicion + 1)]) << 8));
}

void escribir_palabra(unsigned char *datos, Palabra posicion, Palabra valor) {
  datos[posicion] = static_cast<unsigned char>(valor & 0xff);
  datos[static_cast<Palabra>(posicion + 1)] = static_cast<unsigned char>(valor >> 8);
}

} // namespace

void construir_tabla_acciones(unsigned char *datos) {
  bool terminar_lista = false;
  for (Palabra casilla = 0; casilla < 10; ++casilla) {
    Palabra destino = 0xde96;
    if (!terminar_lista) {
      const auto identificador = leer_palabra(datos, static_cast<Palabra>(0xb620 + casilla * 2));
      if (identificador == 0xffff) {
        // También rellena las casillas siguientes, sin volver a leer identificadores.
        terminar_lista = true;
      } else if (identificador < 0x17 || identificador >= 0x8000) {
        // CMP AX,17h / JGE compara con signo. SHL y la dirección vuelven a 16 bits.
        const auto indice = static_cast<Palabra>(identificador << 1);
        destino = leer_palabra(datos, static_cast<Palabra>(0xb686 + indice));
      } else {
        destino = leer_palabra(datos, 0xb636);
      }
    }
    escribir_palabra(datos, static_cast<Palabra>(0xb60c + casilla * 2), destino);
  }
}

} // namespace motor
