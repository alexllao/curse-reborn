#include "fundido.hpp"

namespace motor {
// 780F–781B calcula el decremento entero; 783B–7845 resta antes de enviar cada paleta.
bool calcular_fundido(const unsigned char *paleta, unsigned short colores, unsigned short pasos,
                      unsigned short paso, unsigned char *salida) {
  if (colores == 0 || colores > 256 || pasos == 0 || paso > pasos)
    return false;
  for (unsigned int i = 0; i < colores * 3u; ++i)
    if (paleta[i] > 63)
      return false;
  for (unsigned int i = 0; i < colores * 3u; ++i) {
    const unsigned int acumulador = static_cast<unsigned int>(paleta[i]) << 8;
    const unsigned int decremento = acumulador / pasos;
    salida[i] = static_cast<unsigned char>((acumulador - decremento * paso) >> 8);
  }
  return true;
}
} // namespace motor
