#pragma once

namespace motor {

using Palabra = unsigned short;
static_assert(static_cast<Palabra>(-1) == 0xffff, "Se necesitan palabras de 16 bits.");
static_assert(static_cast<unsigned char>(-1) == 0xff, "Se necesitan bytes de 8 bits.");

inline Palabra palabra(const unsigned char *datos, Palabra posicion) {
  return static_cast<Palabra>(
      datos[posicion] |
      (static_cast<unsigned int>(datos[static_cast<Palabra>(posicion + 1)]) << 8));
}

inline void guardar(unsigned char *datos, Palabra posicion, Palabra valor) {
  datos[posicion] = static_cast<unsigned char>(valor & 0xff);
  datos[static_cast<Palabra>(posicion + 1)] = static_cast<unsigned char>(valor >> 8);
}

inline int con_signo(Palabra valor) {
  return valor < 0x8000 ? static_cast<int>(valor) : static_cast<int>(valor) - 65536;
}

} // namespace motor
