#include "mazmorra.hpp"
#include "../estado/segmento.hpp"
namespace motor {
namespace {
// Dispone un canal sin sobrescribir los otros campos del registro de entidad.
void configurar_secuencia(unsigned char *d, Palabra p, Palabra ref, unsigned char contador,
                          unsigned char intervalo) {
  guardar(d, p, ref);
  d[p + 2] = 0;
  d[p + 3] = contador;
  d[p + 4] = intervalo;
}
} // namespace
// CBD0–CF25: respeta las comparaciones firmadas y las sumas de palabra de cada zona.
bool actualizar_nieve(unsigned char *datos) {
  auto *d = datos;
  const auto escena = palabra(d, 0x8082);
  if (escena == 40) {
    const auto global = static_cast<Palabra>(palabra(d, 0x2c2d) + palabra(d, 0x3591));
    auto distancia = static_cast<Palabra>(global - 165);
    if (con_signo(global) < 165)
      distancia = static_cast<Palabra>(-distancia);
    if (d[0x2c4c] == 1 && d[0x3594] && con_signo(palabra(d, 0x2c2f)) >= 72 && distancia <= 5 &&
        palabra(d, 0x2c2f) <= 100) {
      guardar(d, 0x0dfa, 0);
      d[0x3594] = d[0x3595] = d[0x3596] = 0;
      guardar(d, 0x359a, 0);
      configurar_secuencia(d, 0x2c3b, 0xb2fc, 1, 1);
      guardar(d, 0x2c40, 0);
      configurar_secuencia(d, 0x2c45, 0xb30f, 1, 1);
    }
    return true;
  }
  if (escena == 41) {
    const auto diferencia = static_cast<Palabra>(palabra(d, 0x2c2d) - palabra(d, 0x2c4e));
    auto distancia = static_cast<Palabra>(diferencia - 20);
    if (con_signo(diferencia) < 20)
      distancia = static_cast<Palabra>(-distancia);
    if (!(d[0x35c9] & 128) && d[0x2c4c] && d[0x2c4d] == 17 && con_signo(distancia) < 120) {
      configurar_secuencia(d, 0x2c5c, 0xb32a, 1, 2);
      configurar_secuencia(d, 0x2c61, 0xb338, 1, 2);
      configurar_secuencia(d, 0x2c66, 0xb33c, 1, 2);
    }
    if (d[0x2c6e] == 39 && d[0x2c2c] == 28 && !palabra(d, 0x2c3b)) {
      // CCCА–CD3B conserva los intervalos previos de los dos canales de Brad.
      guardar(d, 0x2c3b, 0xb321);
      d[0x2c3d] = 0;
      d[0x2c3e] = 2;
      guardar(d, 0x2c40, 0xb327);
      d[0x2c42] = 0;
      d[0x2c43] = 2;
      configurar_secuencia(d, 0x2c7d, 0xb344, 2, 2);
      configurar_secuencia(d, 0x2c82, 0xb352, 2, 2);
      configurar_secuencia(d, 0x2c87, 0xb35e, 2, 2);
      configurar_secuencia(d, 0x2c9e, 0x1a2c, 2, 2);
    }
    if (d[0x2c4c] == 1) {
      const auto izquierda = static_cast<Palabra>(palabra(d, 0x2c4e) + palabra(d, 0x3591));
      const bool normal = d[0x2c4d] == 17 && palabra(d, 0x2c5c) != 0xb32a;
      const auto abajo =
          static_cast<Palabra>(palabra(d, 0x2c50) + palabra(d, 0x2c54) - (normal ? 8 : 2));
      guardar(d, 0x18f5, izquierda);
      guardar(d, 0x18f9, static_cast<Palabra>(izquierda + palabra(d, 0x2c52) - 8));
      guardar(d, 0x18fb, abajo);
      guardar(d, 0x18f7, static_cast<Palabra>(abajo - (normal ? 10 : 16)));
    }
    return true;
  }
  if (escena == 42) {
    const auto global = static_cast<Palabra>(palabra(d, 0x2c2d) + palabra(d, 0x3591));
    if (global <= 900) {
      if (d[0x2caf] != 1)
        return true;
      const bool quieto = d[0x2cb0] == 36;
      const auto izquierda = static_cast<Palabra>(palabra(d, 0x2cb1) + palabra(d, 0x3591));
      const auto abajo = static_cast<Palabra>(palabra(d, 0x2cb3) + palabra(d, 0x2cb7));
      guardar(d, 0x18f5, izquierda);
      guardar(d, 0x18f9, static_cast<Palabra>(izquierda + palabra(d, 0x2cb5) - (quieto ? 0 : 4)));
      guardar(d, 0x18fb, abajo);
      guardar(d, 0x18f7,
              quieto ? static_cast<Palabra>(abajo - 6)
                     : static_cast<Palabra>(palabra(d, 0x2cb3) + 6));
    } else if (d[0x2c4c] == 1) {
      const auto izquierda = static_cast<Palabra>(palabra(d, 0x2c4e) + palabra(d, 0x3591));
      guardar(d, 0x18f5, izquierda);
      guardar(d, 0x18f9, static_cast<Palabra>(izquierda + palabra(d, 0x2c52) + palabra(d, 0x2c73)));
      guardar(d, 0x18fb, 165);
      guardar(d, 0x18f7, 142);
    }
    return true;
  }
  if (escena == 43) {
    // CE35–CF25: reacción única al entrar entero y proximidad según postura del primer actor.
    const auto x = palabra(datos, 0x2c4e);
    const auto ancho = palabra(datos, 0x2c52);
    if (!(datos[0x35ca] & 1) && con_signo(x) >= 0 &&
        con_signo(static_cast<Palabra>(x + ancho)) <= 320) {
      datos[0x35ca] |= 1;
      configurar_secuencia(datos, 0x2c5c, 0xb36a, 10, 2);
      configurar_secuencia(datos, 0x2c61, 0xb377, 10, 2);
      configurar_secuencia(datos, 0x2c66, 0xb381, 10, 2);
      configurar_secuencia(datos, 0x2c9e, 0xb38b, 12, 2);
      configurar_secuencia(datos, 0x2ca3, 0xb390, 12, 2);
      configurar_secuencia(datos, 0x2ca8, 0xb393, 12, 2);
    }
    if (datos[0x2c4c] == 1) {
      const auto izquierda = static_cast<Palabra>(x + palabra(datos, 0x3591));
      const auto abajo = static_cast<Palabra>(palabra(datos, 0x2c50) + palabra(datos, 0x2c54));
      const bool postura = datos[0x2c4d] == 32;
      guardar(datos, 0x18f5, izquierda);
      guardar(datos, 0x18f9, static_cast<Palabra>(izquierda + ancho - (postura ? 12 : 8)));
      guardar(datos, 0x18fb, abajo);
      guardar(datos, 0x18f7,
              postura ? static_cast<Palabra>(palabra(datos, 0x2c50) + 16)
                      : static_cast<Palabra>(abajo - 12));
    }
    return true;
  }
  return false;
}
} // namespace motor
