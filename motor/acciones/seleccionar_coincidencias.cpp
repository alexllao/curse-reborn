#include "seleccionar_coincidencias.hpp"

namespace motor {
namespace {

void marcar_resultado(unsigned char* datos, unsigned char resultado) {
  constexpr unsigned short posiciones[6][3] = {
    {0x4b50, 0x4b88, 0x4bd2},
    {0x4b3e, 0x4b76, 0},
    {0x4b2c, 0, 0},
    {0x4b59, 0x4b91, 0x4bdb},
    {0x4b47, 0x4b7f, 0x4bc9},
    {0x4b35, 0x4b6d, 0},
  };
  for (auto posicion : posiciones[resultado]) {
    if (posicion != 0) {
      datos[posicion] = 1;
    }
  }
}

}

bool seleccionar_coincidencias(unsigned char* datos, unsigned char primero,
                              unsigned char segundo, unsigned char tercero) {
  if (primero >= 6 || segundo >= 6 || tercero >= 6) {
    return false;
  }
  if (primero == segundo && segundo == tercero) {
    // B857–B89E activa los grupos pares o impares según el bit bajo de AL.
    for (unsigned char resultado = tercero & 1; resultado < 6; resultado += 2) {
      marcar_resultado(datos, resultado);
    }
  } else if (tercero == primero || tercero == segundo) {
    marcar_resultado(datos, tercero);
  } else if (primero == segundo) {
    marcar_resultado(datos, primero);
  }
  // Con tres valores distintos el original no escribe en este tramo.
  return true;
}

}
