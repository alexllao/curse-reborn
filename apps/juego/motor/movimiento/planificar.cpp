#include "planificar.hpp"
#include "direccion.hpp"
#include "../estado/segmento.hpp"

namespace motor {
namespace {

Palabra magnitud(Palabra valor) {
  return con_signo(valor) < 0 ? static_cast<Palabra>(0 - valor) : valor;
}

bool guardar_incrementos(unsigned char *datos, Palabra delta, bool horizontal) {
  const int divisor = con_signo(palabra(datos, 0x359a));
  if (divisor == 0)
    return false;
  // MOV AH,DL / SAR DX,8 construye el numerador de IDIV, equivalente a delta*256.
  const int cociente = (con_signo(delta) * 256) / divisor;
  if (cociente < -32768 || cociente > 32767)
    return false;
  const auto fraccion = static_cast<Palabra>(static_cast<Palabra>(cociente) << 8);
  const auto entero =
      static_cast<Palabra>(cociente < 0 ? -((-cociente + 255) / 256) : cociente / 256);
  if (horizontal) {
    guardar(datos, 0x35a8, entero);
    guardar(datos, 0x35a6, fraccion);
    guardar(datos, 0x35ac, 0);
    guardar(datos, 0x35aa, 0);
  } else {
    guardar(datos, 0x35ac, entero);
    guardar(datos, 0x35aa, fraccion);
    guardar(datos, 0x35a8, 0);
    guardar(datos, 0x35a6, 0);
  }
  guardar(datos, 0x35a0, palabra(datos, 0x2c2d));
  guardar(datos, 0x359e, 0);
  guardar(datos, 0x35a4, palabra(datos, 0x2c2f));
  guardar(datos, 0x35a2, 0);
  return true;
}

} // namespace

bool planificar_movimiento(unsigned char *datos, unsigned short nodo, unsigned short &pasos) {
  guardar(datos, 0x359c, 2);
  auto cx =
      static_cast<Palabra>(palabra(datos, static_cast<Palabra>(nodo + 2)) - palabra(datos, 0x2c2d));
  auto dx = static_cast<Palabra>(palabra(datos, static_cast<Palabra>(nodo + 4)) -
                                 palabra(datos, 0x2c2f) - palabra(datos, 0x2c33) + 1);
  const auto ajuste = static_cast<Palabra>((palabra(datos, 0x2c31) >> 1) - 1);
  cx = static_cast<Palabra>(cx - ajuste);
  auto ax = magnitud(cx);
  auto bx = magnitud(dx);
  const bool especial = palabra(datos, 0x8082) == 0x24 && datos[0x358c] == 0xfe;
  const bool horizontal = con_signo(ax) >= con_signo(bx);
  bool iniciar = true;
  if (especial) {
    if (horizontal) {
      if (ax == 0)
        iniciar = false;
      else
        guardar(datos, 0x359a, ax);
    } else {
      if (bx == 0)
        iniciar = false;
      else
        guardar(datos, 0x359a, static_cast<Palabra>(bx << 1));
    }
  } else if (horizontal) {
    if (datos[0x358c] != 0x14)
      ax = static_cast<Palabra>(ax >> 1);
    ax = static_cast<Palabra>(ax + 1);
    ax = static_cast<Palabra>(ax >> 1);
    if (ax == 0)
      iniciar = false;
    else
      guardar(datos, 0x359a, ax);
  } else {
    if (datos[0x358c] != 0x14)
      bx = static_cast<Palabra>(static_cast<Palabra>(bx + 1) >> 1);
    if (bx == 0)
      iniciar = false;
    else
      guardar(datos, 0x359a, bx);
  }
  if (iniciar) {
    const auto delta_principal = horizontal ? cx : dx;
    const auto delta_secundario = horizontal ? dx : cx;
    const bool negativo = con_signo(delta_principal) < 0;
    if (!especial) {
      const auto imagen = datos[0x2c2c];
      const bool retraso =
          horizontal
              ? (negativo ? imagen <= 8 : (imagen >= 0x80 && imagen <= 0x88))
              : (negativo ? (imagen >= 9 && imagen <= 0x11) : (imagen >= 0x12 && imagen <= 0x1a));
      if (retraso)
        guardar(datos, 0x359a, static_cast<Palabra>(palabra(datos, 0x359a) + 1));
    }
    seleccionar_direccion(datos, horizontal ? (negativo ? 8 : 4) : (negativo ? 2 : 1));
    if (!guardar_incrementos(datos, delta_secundario, !horizontal))
      return false;
  }
  pasos = palabra(datos, 0x359a);
  if (pasos == 0)
    pasos = 1;
  return true;
}

} // namespace motor
