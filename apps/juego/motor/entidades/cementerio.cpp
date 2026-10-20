#include "mazmorra.hpp"
#include "actualizar.hpp"
#include "../acciones/iniciar_accion.hpp"
#include "../acciones/lista_identificadores.hpp"
#include "../estado/segmento.hpp"

namespace motor {
namespace {
// Escribe fase y contador preservando el intervalo cuando el original no lo modifica.
void canal_cementerio(unsigned char *d, Palabra p, Palabra secuencia, unsigned char cuenta,
                      int intervalo = -1) {
  guardar(d, p, secuencia);
  d[p + 2] = 0;
  d[p + 3] = cuenta;
  if (intervalo >= 0)
    d[p + 4] = static_cast<unsigned char>(intervalo);
}
// Los vampiros comparten tres canales; las transiciones originales no fuerzan su actividad.
void vampiro(unsigned char *d, Palabra a, Palabra b, Palabra c) {
  canal_cementerio(d, 0x2c5c, a, 1);
  canal_cementerio(d, 0x2c61, b, 1);
  canal_cementerio(d, 0x2c66, c, 1);
}
// 1E78/1F0C/1F91: bloquea el paso a partir del vampiro sin inventar colisiones nuevas.
void limite_vampiro(unsigned char *d) {
  guardar(d, 0x18f5, static_cast<Palabra>(palabra(d, 0x2c4e) + palabra(d, 0x3591)));
  guardar(d, 0x18f9, 320);
  guardar(d, 0x18fb, 199);
}
} // namespace

// 1BA3–20CC: azar de murciélagos, tumbas y seis fases de vampiros en65/77.
bool actualizar_cementerio(unsigned char *d, EstadoAleatorio &azar,
                           const RecursosEntidades &recursos) {
  Palabra valor = 0;
  if (!generar_aleatorio(azar, 20, valor))
    return false;
  if (valor == 0) {
    if (!generar_aleatorio(azar, 150, valor))
      return false;
    guardar(d, 0x15a2, static_cast<Palabra>(valor - 5));
    if (!generar_aleatorio(azar, 2, valor))
      return false;
    const bool derecha = valor != 0;
    guardar(d, 0x15a0, derecha ? 328 : 0xfff8);
    d[0x159f] = derecha ? 0xe5 : 0x65;
    guardar(d, 0x15ac, derecha ? 0x15ca : 0x15bd);
    if (!generar_aleatorio(azar, 4, valor))
      return false;
    guardar(d, 0x15b0, static_cast<Palabra>((derecha ? 0x15cf : 0x15c2) + valor * 2));
    if (!crear_entidad(d, azar, recursos, 0x159d).valido)
      return false;
  }
  const bool primera = palabra(d, 0x8082) == 65;
  Palabra entidad = 0, x = 0, y = 0;
  if (primera) {
    if (d[0x2d12]) {
      entidad = 0x2d12;
      x = 374;
      y = 105;
    } else if (d[0x2d33]) {
      entidad = 0x2d33;
      x = 500;
      y = 71;
    }
  } else {
    if (d[0x2cf1]) {
      entidad = 0x2cf1;
      x = 26;
      y = 140;
    } else if (d[0x2d12]) {
      entidad = 0x2d12;
      x = 496;
      y = 97;
    }
  }
  if (entidad) {
    x = static_cast<Palabra>(x - palabra(d, 0x3591));
    guardar(d, 0x15da, x);
    const int diferencia = con_signo(x) - con_signo(palabra(d, 0x2c2d));
    auto distancia = static_cast<Palabra>(x - palabra(d, 0x2c2d));
    if (diferencia < 0)
      distancia = static_cast<Palabra>(0 - distancia);
    if (distancia <= 4) {
      d[entidad] = 0;
      guardar(d, 0x15dc, y);
      for (unsigned int i = 0; i < 6; ++i) {
        if (!generar_aleatorio(azar, 30, valor))
          return false;
        d[0x15e8] = d[0x15ec] = d[0x15f0] = static_cast<unsigned char>(valor);
        if (!crear_entidad(d, azar, recursos, 0x15d7).valido)
          return false;
      }
    }
  }
  auto fase = static_cast<unsigned char>((d[0x35ca] >> 3) & 7);
  if (fase == 0) {
    guardar(d, 0x18f5, 270);
    guardar(d, 0x18f9, 400);
    guardar(d, 0x18fb, 199);
    if (d[0x2c4c] != 1 || palabra(d, 0x2c5c) == 0x14c4)
      return true;
    if (palabra(d, 0x2c5c) == 0x14e9) {
      const int diferencia =
          con_signo(static_cast<Palabra>(palabra(d, 0x2c4e) - palabra(d, 0x2c2d))) -
          con_signo(palabra(d, 0x2c31));
      if (diferencia > 0 || !d[0x3594])
        return true;
      vampiro(d, 0x14f7, 0x14fc, 0x1501);
      reiniciar_estado_accion(d);
      guardar(d, 0x2c3b, 0x14b8);
      guardar(d, 0x2c40, 0x14c1);
      guardar(d, 0x2c45, 0);
      return true;
    }
    if (palabra(d, 0x2c5c) == 0) {
      const auto global = static_cast<Palabra>(palabra(d, 0x2c2d) + palabra(d, 0x3591));
      const auto pie = static_cast<Palabra>(palabra(d, 0x2c50) + palabra(d, 0x2c54));
      const int diferencia = con_signo(pie) - 169;
      auto distancia = static_cast<Palabra>(pie - 169);
      if (diferencia < 0)
        distancia = static_cast<Palabra>(0 - distancia);
      if (con_signo(global) <= 200 && distancia <= 4) {
        d[0x35ca] |= 8;
        guardar(d, 0x4e77, 0x54e1);
        vampiro(d, 0x14c4, 0x14d5, 0x14df);
        canal_cementerio(d, 0x2c7d, 0x1a2f, 7, 2);
      } else
        vampiro(d, 0x14e9, 0x14ef, 0x14f3);
      return true;
    }
    if (static_cast<signed char>(d[0x2c4d]) >= 39 && static_cast<signed char>(d[0x2c4d]) <= 41 &&
        d[0x2c5f] != 1) {
      const auto pie = static_cast<Palabra>(palabra(d, 0x2c50) + palabra(d, 0x2c54));
      const auto resta = static_cast<Palabra>(pie - palabra(d, 0x2c2f));
      const int diferencia = con_signo(resta) - con_signo(palabra(d, 0x2c33));
      guardar(d, 0x2c50, static_cast<Palabra>(palabra(d, 0x2c50) + (diferencia < 0 ? 2 : -2)));
    }
    return true;
  }
  if (fase == 1) {
    if (d[0x2c4c] != 1) {
      if (con_signo(static_cast<Palabra>(palabra(d, 0x2c2d) + palabra(d, 0x3591))) < 270)
        return true;
      guardar(d, 0x2c4e, static_cast<Palabra>(322 - palabra(d, 0x3591)));
      guardar(d, 0x2c50, 127);
      d[0x2c4d] = 32;
      guardar(d, 0x2c56, 200);
      vampiro(d, 0x1506, 0x150c, 0x150f);
    }
    limite_vampiro(d);
    return true;
  }
  if (fase == 2) {
    if (!primera || d[0x2c4c] == 1)
      return true;
    x = static_cast<Palabra>(447 - palabra(d, 0x3591));
    guardar(d, 0x2c4e, x);
    guardar(d, 0x2c50, 101);
    d[0x2c4d] = 31;
    guardar(d, 0x2c56, 0);
    vampiro(d, 0x1515, 0x151a, 0x151f);
    // 1EF4 reutiliza AL tras calcular447−cámara, sin restaurar la fase original.
    fase = static_cast<unsigned char>(x);
  }
  if (fase == 3) {
    if (!primera && d[0x2c4c] == 1)
      limite_vampiro(d);
    return true;
  }
  if (fase == 4 || fase == 5) {
    if (primera)
      return true;
    if (d[0x2c4c] != 1) {
      if (con_signo(static_cast<Palabra>(palabra(d, 0x2c2d) + palabra(d, 0x3591))) <
          (fase == 4 ? 120 : 350))
        return true;
      guardar(d, 0x2c4e, static_cast<Palabra>((fase == 4 ? 203 : 273) - palabra(d, 0x3591)));
      guardar(d, 0x2c50, fase == 4 ? 109 : 93);
      d[0x2c4d] = 31;
      if (fase == 4)
        vampiro(d, 0x1515, 0x151a, 0x151f);
      else
        vampiro(d, 0x1524, 0x152c, 0x1532);
    }
    if (fase == 4)
      limite_vampiro(d);
    else {
      guardar(d, 0x18f5, 0);
      guardar(d, 0x18f9,
              static_cast<Palabra>(palabra(d, 0x2c4e) + palabra(d, 0x2c52) + palabra(d, 0x3591)));
      guardar(d, 0x18fb, 199);
    }
    return true;
  }
  if (fase == 6 && !primera && d[0x2c4c] != 1 && static_cast<signed char>(d[0x3597]) >= 8 &&
      contiene_identificador(d, 171)) {
    guardar(d, 0x2c4e, static_cast<Palabra>(458 - palabra(d, 0x3591)));
    guardar(d, 0x2c50, 92);
    d[0x2c4d] = 31;
    vampiro(d, 0x154d, 0x156b, 0x1581);
    canal_cementerio(d, 0x2ce0, 0x1597, 5, 2);
    canal_cementerio(d, 0x2cea, 0x159b, 5, 2);
  }
  return true;
}
} // namespace motor
