#include "mazmorra.hpp"
#include "actualizar.hpp"
#include "../acciones/iniciar_accion.hpp"
#include "../estado/segmento.hpp"

namespace motor {
namespace {
// Programa los campos de canal que escribe la bruja; un intervalo negativo conserva el anterior.
void canal_bruja(unsigned char *d, Palabra p, Palabra secuencia, unsigned char cuenta,
                 int intervalo = 2) {
  guardar(d, p, secuencia);
  d[p + 2] = 0;
  d[p + 3] = cuenta;
  if (intervalo >= 0)
    d[p + 4] = static_cast<unsigned char>(intervalo);
}
// Reparte las tres secuencias originales sin modificar actividad, posición ni otros canales.
void triple_bruja(unsigned char *d, Palabra p, Palabra a, Palabra b, Palabra c,
                  unsigned char cuenta) {
  canal_bruja(d, p, a, cuenta);
  canal_bruja(d, p + 5, b, cuenta);
  canal_bruja(d, p + 10, c, cuenta);
}
// 2242/230D/24DF: el fracaso bloquea la acción antes de preparar los tres canales de Brad.
void fracaso_bruja(unsigned char *d) {
  reiniciar_estado_accion(d);
  guardar(d, 0x2c3b, 0x162d);
  guardar(d, 0x2c40, 0x1647);
  guardar(d, 0x2c45, 0x1657);
}
} // namespace

// 20CD–2549: cuatro fases del duelo, fallos y partículas con los mismos consumos de azar.
bool actualizar_bruja(unsigned char *d, EstadoAleatorio &azar, const RecursosEntidades &recursos) {
  const auto fase = d[0x35ca] >> 6;
  if (!fase && d[0x3595] == 1) {
    guardar(d, 0x358d, 0);
    guardar(d, 0x358f, 96);
  }
  if (!fase) {
    if (palabra(d, 0x2c3b) || palabra(d, 0x2c5c))
      return true;
    if (!palabra(d, 0x2d22)) {
      triple_bruja(d, 0x2c5c, 0x1667, 0x1671, 0x1679, 1);
      canal_bruja(d, 0x2c9e, 0x1681, 5);
      canal_bruja(d, 0x2cbf, 0x1681, 5);
      triple_bruja(d, 0x2d22, 0x16a2, 0x16ad, 0x16c0, 21);
    } else if (d[0x358c] == 27) {
      if (con_signo(palabra(d, 0x2d14)) <= 120) {
        d[0x35ca] ^= 0x40;
        canal_bruja(d, 0x2c3b, 0x16d3, 13, -1);
        canal_bruja(d, 0x2c40, 0x16d8, 13, -1);
        triple_bruja(d, 0x2d22, 0x16da, 0x16e2, 0x16e8, 1);
      }
    } else if (palabra(d, 0x2d14) == 85)
      fracaso_bruja(d);
    return true;
  }
  if (fase == 1) {
    if (!palabra(d, 0x2c3b) && !palabra(d, 0x2c5c)) {
      triple_bruja(d, 0x2c5c, 0x16ee, 0x16f8, 0x1700, 1);
      canal_bruja(d, 0x2c9e, 0x1708, 5);
      canal_bruja(d, 0x2cbf, 0x1708, 5);
    }
    if (d[0x2c8e] == 1) {
      Palabra valor = 0;
      if (!generar_aleatorio(azar, 11, valor))
        return false;
      const auto tabla = static_cast<Palabra>(0x17a8 + valor * 4);
      guardar(d, 0x1768, palabra(d, tabla));
      guardar(d, 0x176a, palabra(d, tabla + 2));
      if (!crear_entidad(d, azar, recursos, 0x1765).valido)
        return false;
    }
    if (d[0x2c5f] == 3)
      fracaso_bruja(d);
    return true;
  }
  if (fase == 2) {
    if (palabra(d, 0x2c3b))
      return true;
    if (!palabra(d, 0x2c5c) && !palabra(d, 0x2ce0)) {
      triple_bruja(d, 0x2c5c, 0x17d4, 0x17e1, 0x17ec, 1);
      triple_bruja(d, 0x2ce0, 0x17f7, 0x1801, 0x1808, 11);
      triple_bruja(d, 0x2d01, 0x17f7, 0x1801, 0x1808, 11);
    }
    if (d[0x2d03] != 4)
      return true;
    if (d[0x2d33] == 1) {
      d[0x35ca] ^= 0x40;
      triple_bruja(d, 0x2c5c, 0x1829, 0x183f, 0x1853, 2);
      triple_bruja(d, 0x2ce0, 0x180f, 0x1819, 0x1821, 2);
      triple_bruja(d, 0x2d01, 0x180f, 0x1819, 0x1821, 2);
    } else
      fracaso_bruja(d);
    return true;
  }
  if (d[0x2c6d] && !palabra(d, 0x2c5c)) {
    guardar(d, 0x358f, 999);
    reiniciar_estado_accion(d);
    guardar(d, 0x2c3b, 0x1867);
    guardar(d, 0x2c40, 0x1897);
    guardar(d, 0x2c45, 0x18c3);
    canal_bruja(d, 0x2d43, 0x1a2e, 1);
    canal_bruja(d, 0x2c7d, 0x1a2e, 1);
  }
  return true;
}
} // namespace motor
