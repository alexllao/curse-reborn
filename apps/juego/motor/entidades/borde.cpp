#include "mazmorra.hpp"
#include "actualizar.hpp"
#include "../acciones/lista_identificadores.hpp"
#include "../estado/segmento.hpp"

namespace motor {
namespace {
// Escribe el canal original sin tocar los demás campos del registro.
void canal(unsigned char *d, Palabra p, Palabra secuencia, unsigned char espera,
           unsigned char intervalo) {
  guardar(d, p, secuencia);
  d[p + 2] = 0;
  d[p + 3] = espera;
  d[p + 4] = intervalo;
}
// Conserva las dos comparaciones con signo de C0E8/C202/C466, incluida la resta envolvente.
bool cerca(Palabra objeto, Palabra personaje, int margen) {
  return con_signo(objeto) >= con_signo(personaje) &&
         con_signo(static_cast<Palabra>(objeto - personaje)) <= margen;
}
// C251/C4B5: el contacto del obstáculo se comprueba después de su animación inicial.
bool contacto(const unsigned char *d, Palabra objeto) {
  const auto y = palabra(d, objeto + 4), brad_y = palabra(d, 0x2c2f);
  if (con_signo(y) < con_signo(brad_y) ||
      con_signo(static_cast<Palabra>(y - brad_y)) > con_signo(palabra(d, 0x2c33)))
    return false;
  const auto x =
      static_cast<Palabra>(palabra(d, objeto + 2) - palabra(d, 0x2c2d) - palabra(d, 0x2c31));
  return con_signo(x) + 4 <= 0 && d[0x3594] != 0;
}
// C210–C24A y C474–C4AE comparten los mismos tres canales de reacción.
void reaccion(unsigned char *d, Palabra registro) {
  canal(d, registro + 16, 0xb0a8, 1, 1);
  canal(d, registro + 21, 0xb0d0, 1, 1);
  canal(d, registro + 26, 0xb0d7, 1, 1);
}
} // namespace

// C093–C4DB: rescate, obstáculos, caída de rocas y proximidad, con servicios comunes.
bool actualizar_borde(unsigned char *d, EstadoAleatorio &azar, const RecursosEntidades &recursos) {
  if (d[0x358c] == 20 && d[0x2c2c] == 90 && palabra(d, 0x2c5c) != 0xb0ed) {
    canal(d, 0x2c5c, 0xb0ed, 2, 2);
    canal(d, 0x2c61, 0xb0f6, 2, 2);
    canal(d, 0x2c66, 0xb0fd, 2, 2);
  }
  if (cerca(palabra(d, 0x2cb1), palabra(d, 0x2c2d), 10) && d[0x3594]) {
    canal(d, 0x2cbf, 0xb0e5, 1, 1);
    canal(d, 0x2cc4, 0xb0e8, 1, 1);
    activar_transicion_entidad(d);
    return true;
  }
  if (!(d[0x2db7] & 2)) {
    if (con_signo(palabra(d, 0x2db9)) + con_signo(palabra(d, 0x2dbd)) <= 0) {
      d[0x2db7] |= 2;
    } else {
      const auto y = palabra(d, 0x2dbb);
      if (y != 50)
        guardar(d, 0x2dbb, static_cast<Palabra>(y + (con_signo(y) > 50 ? -4 : 4)));
      if (palabra(d, 0x2c3b) != 0xb02d && d[0x3594] != 1) {
        const auto tabla =
            static_cast<Palabra>(0xb036 + static_cast<unsigned char>(d[0x2db8] - 22) * 4);
        guardar(d, 0x2c2d, static_cast<Palabra>(palabra(d, tabla) + palabra(d, 0x2db9)));
        guardar(d, 0x2c2f, static_cast<Palabra>(palabra(d, tabla + 2) + palabra(d, 0x2dbb)));
        if (static_cast<Palabra>(palabra(d, 0x2c2d) + palabra(d, 0x3591)) <= 175) {
          guardar(d, 0x2c2d, static_cast<Palabra>(175 - palabra(d, 0x3591)));
          guardar(d, 0x2c2f, 79);
          canal(d, 0x2c3b, 0xb02d, 1, 2);
          canal(d, 0x2c45, 0xb032, 1, 2);
          return true;
        }
      }
    }
  }
  if (d[0x2d12] & 2) {
    guardar(d, 0x18f5, 714);
    guardar(d, 0x18f9, 825);
    guardar(d, 0x18fb, 199);
  }
  if (d[0x2dd8]) {
    if (palabra(d, 0x2de8) != 0xb0a8) {
      if (cerca(palabra(d, 0x2dda), palabra(d, 0x2c2d), 16))
        reaccion(d, 0x2dd8);
    } else if (contacto(d, 0x2dd8)) {
      activar_transicion_entidad(d);
      return true;
    }
  }
  --d[0xadce];
  if (d[0xadce] >= 180) {
    d[0xadce] = 180;
    d[0x0def] = 0;
  }
  Palabra valor = 0;
  if (d[0xadce] <= 50) {
    if (d[0x3597] >= 21 && d[0x3597] <= 40) {
      if (!generar_aleatorio(azar, 3, valor))
        return false;
      if (!valor && !emitir_sonido(recursos.sonido, d, 0x2c2b, 6))
        return false;
      if (!d[0x0def])
        d[0x0def] = 1;
    }
    if (d[0xadce] & 1) {
      for (unsigned int i = 0; i < 3; ++i) {
        if (!generar_aleatorio(azar, 2, valor))
          return false;
        const auto descriptor = static_cast<Palabra>(0xaf37 + valor * 9);
        if (!generar_aleatorio(azar, 260, valor))
          return false;
        guardar(d, descriptor + 3, static_cast<Palabra>(valor + 1000 - palabra(d, 0x3591)));
        if (!generar_aleatorio(azar, 30, valor))
          return false;
        guardar(d, descriptor + 5, static_cast<Palabra>(valor - 15));
        const auto propiedades = palabra(d, descriptor + 7);
        if (!generar_aleatorio(azar, 2, valor))
          return false;
        guardar(d, propiedades + 10, static_cast<Palabra>(valor * 2 + 0xafe8));
        if (!crear_entidad(d, azar, recursos, descriptor).valido)
          return false;
      }
    }
  }
  if (d[0x2df9] && palabra(d, 0x2e09) != 0xb104 && palabra(d, 0x2e09) != 0xb11c) {
    auto distancia = static_cast<Palabra>(palabra(d, 0x2dfb) - palabra(d, 0x2c2d) - 4);
    if (con_signo(distancia) < 0)
      distancia = static_cast<Palabra>(0 - distancia);
    if (con_signo(distancia) <= 3) {
      guardar(d, 0x2dfd, static_cast<Palabra>(palabra(d, 0x2c2f) - 89));
      canal(d, 0x2e09, 0xb104, 1, 1);
      canal(d, 0x2e13, 0xb10f, 1, 1);
      if (palabra(d, 0x2c9c) == 0xffff && d[0x2c8e] == 1) {
        insertar_identificador(d, 94);
        canal(d, 0x2c9e, 0x1a2e, 12, 1);
        guardar(d, 0x2e09, 0xb11c);
        guardar(d, 0x2e13, 0xb120);
        canal(d, 0x2c3b, 0xafec, 8, 2);
        canal(d, 0x2c40, 0xaff2, 8, 2);
      }
    }
  }
  if (!palabra(d, 0x18f5) && palabra(d, 0x2cd4) == 57) {
    guardar(d, 0x18f5, 1386);
    guardar(d, 0x18f9, 1443);
    guardar(d, 0x18fb, 199);
  }
  if (!generar_aleatorio(azar, 10, valor))
    return false;
  if (!valor) {
    if (!generar_aleatorio(azar, 2, valor))
      return false;
    const auto descriptor = static_cast<Palabra>(0xb125 + valor * 9);
    if (!generar_aleatorio(azar, 150, valor))
      return false;
    guardar(d, descriptor + 5, static_cast<Palabra>(valor + 25));
    const auto propiedades = palabra(d, descriptor + 7);
    if (!generar_aleatorio(azar, 4, valor))
      return false;
    guardar(d, propiedades + 10, static_cast<Palabra>(palabra(d, propiedades + 6) + valor * 2 + 8));
    if (!crear_entidad(d, azar, recursos, descriptor).valido)
      return false;
  }
  const auto x = static_cast<Palabra>(palabra(d, 0x2c2d) + palabra(d, 0x2c31) - 10);
  if (d[0x2c4c] && con_signo(x) >= con_signo(palabra(d, 0x2c4e)) && d[0x3594]) {
    activar_transicion_entidad(d);
    return true;
  }
  if (d[0x2ee0]) {
    if (palabra(d, 0x2ef0) != 0xb0a8) {
      if (cerca(palabra(d, 0x2ee2), palabra(d, 0x2c2d), 14))
        reaccion(d, 0x2ee0);
    } else if (contacto(d, 0x2ee0)) {
      activar_transicion_entidad(d);
    }
  }
  return true;
}
} // namespace motor
