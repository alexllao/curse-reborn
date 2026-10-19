#include "banco.hpp"

namespace motor {
// Valida la tabla y la cabecera leídas por el original antes de entregar dimensiones o referencias.
bool consultar_fotograma(const unsigned char *banco, unsigned int longitud, unsigned short indice,
                         unsigned short &parrafo, unsigned short &ancho, unsigned short &alto) {
  if (indice >= 128 || longitud < 2)
    return false;
  const unsigned int tabla = (banco[0] | (static_cast<unsigned int>(banco[1]) << 8)) * 16u;
  const auto entrada = indice * 2u;
  if (entrada + 2 > longitud || entrada + 2 > tabla)
    return false;
  parrafo = static_cast<unsigned short>(banco[entrada] |
                                        (static_cast<unsigned int>(banco[entrada + 1]) << 8));
  const auto posicion = parrafo * 16u;
  if (posicion < tabla || posicion > longitud || longitud - posicion < 10)
    return false;
  ancho = static_cast<unsigned short>(banco[posicion] |
                                      (static_cast<unsigned int>(banco[posicion + 1]) << 8));
  alto = static_cast<unsigned short>(banco[posicion + 2] |
                                     (static_cast<unsigned int>(banco[posicion + 3]) << 8));
  return ancho > 0 && ancho <= 32767 && alto > 0 && alto <= 32767;
}
} // namespace motor
