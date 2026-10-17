#include "direccion.hpp"
#include "../estado/segmento.hpp"

namespace motor {
namespace {

void limpiar_secuencias(unsigned char *datos) {
  guardar(datos, 0x2c3b, 0);
  guardar(datos, 0x2c40, 0);
  guardar(datos, 0x2c45, 0);
}

void seleccionar_especial(unsigned char *datos, Palabra indice) {
  const bool izquierda = indice == 8 || indice == 9 || indice == 10;
  const bool derecha = indice == 4 || indice == 5 || indice == 6;
  if (izquierda || derecha) {
    guardar(datos, 0x2c45,
            (indice == 8 || indice == 4) ? 0x0acc
                                         : ((indice == 9 || indice == 5) ? 0x0ac6 : 0x0ac9));
    const Palabra secuencia = izquierda ? 0x0ac4 : 0x0ac2;
    if (palabra(datos, 0x2c40) == secuencia) {
      return;
    }
    guardar(datos, 0x2c40, secuencia);
  } else if (indice == 1 || indice == 2) {
    guardar(datos, 0x2c40, 0);
    const Palabra secuencia = indice == 1 ? 0x0ac6 : 0x0ac9;
    if (palabra(datos, 0x2c45) == secuencia) {
      return;
    }
    guardar(datos, 0x2c45, secuencia);
  } else {
    limpiar_secuencias(datos);
    return;
  }
  datos[0x2c42] = datos[0x2c47] = 0;
  datos[0x2c43] = datos[0x2c44] = datos[0x2c48] = datos[0x2c49] = 2;
}

} // namespace

bool seleccionar_direccion(unsigned char *datos, unsigned short indice) {
  if (indice > 15) {
    return false;
  }
  if (palabra(datos, 0x8082) == 0x24 && datos[0x358c] == 0xfe) {
    seleccionar_especial(datos, indice);
    return true;
  }
  const bool izquierda = indice == 8 || indice == 9 || indice == 10;
  const bool derecha = indice == 4 || indice == 5 || indice == 6;
  const auto anterior = palabra(datos, 0x2c3b);
  const auto imagen = datos[0x2c2c];
  if (izquierda || derecha) {
    guardar(datos, 0x2c45,
            (indice == 8 || indice == 4) ? 0 : ((indice == 9 || indice == 5) ? 0x0ab6 : 0x0ab8));
    if (izquierda) {
      if (anterior == 0x0a97 || anterior == 0x0a7e) {
        return true;
      }
      guardar(datos, 0x2c40, 0x0ab4);
      const bool transicion = imagen <= 8;
      guardar(datos, 0x2c3b, transicion ? 0x0a7e : 0x0a97);
      datos[0x2c43] = transicion ? 2 : 1;
    } else {
      if (anterior == 0x0a8e || anterior == 0x0a82) {
        return true;
      }
      guardar(datos, 0x2c40, 0x0ab2);
      const bool transicion = imagen >= 0x80 && imagen <= 0x88;
      guardar(datos, 0x2c3b, transicion ? 0x0a82 : 0x0a8e);
      datos[0x2c43] = datos[0x2c48] = transicion ? 2 : 1;
    }
  } else if (indice == 2 || indice == 1) {
    const bool arriba = indice == 2;
    if ((arriba && (anterior == 0x0aa9 || anterior == 0x0a86)) ||
        (!arriba && (anterior == 0x0aa0 || anterior == 0x0a8a))) {
      return true;
    }
    guardar(datos, 0x2c40, 0);
    guardar(datos, 0x2c45, arriba ? 0x0ab8 : 0x0ab6);
    const bool transicion =
        arriba ? (imagen >= 9 && imagen <= 0x11) : (imagen >= 0x12 && imagen <= 0x1a);
    guardar(datos, 0x2c3b,
            arriba ? (transicion ? 0x0a86 : 0x0aa9) : (transicion ? 0x0a8a : 0x0aa0));
    datos[0x2c48] = transicion ? 2 : 1;
  } else {
    if (anterior != 0) {
      datos[0x2c2c] = imagen < 9 ? 0 : (imagen < 0x12 ? 9 : (imagen < 0x80 ? 0x12 : 0x80));
      limpiar_secuencias(datos);
    }
    return true;
  }
  if (datos[0x358c] == 0x14) {
    const auto secuencia = palabra(datos, 0x2c3b);
    if (secuencia == 0x0a97 || secuencia == 0x0a7e)
      guardar(datos, 0x2c40, 0x0abc);
    else if (secuencia == 0x0a8e || secuencia == 0x0a82)
      guardar(datos, 0x2c40, 0x0aba);
    else if (secuencia == 0x0aa9 || secuencia == 0x0a86)
      guardar(datos, 0x2c45, 0x0ac0);
    else if (secuencia == 0x0aa0 || secuencia == 0x0a8a)
      guardar(datos, 0x2c45, 0x0abe);
  }
  datos[0x2c3d] = datos[0x2c42] = datos[0x2c47] = 0;
  datos[0x2c3e] = datos[0x2c3f] = datos[0x2c44] = datos[0x2c49] = 1;
  return true;
}

} // namespace motor
