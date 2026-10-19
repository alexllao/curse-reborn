#include "nieve.hpp"
#include "iniciar_accion.hpp"
#include "lista_identificadores.hpp"
#include "../estado/segmento.hpp"

namespace motor {
namespace {
// 6323–633E: ofrece de nuevo al amigo verde solo con cenizas y ambas liberaciones previas.
void actualizar_rescate(unsigned char *d) {
  if (contiene_identificador(d, 131) && !d[0x490f] && d[0x4948] != 19)
    d[0x484e] = 1;
}
// Conserva la planificación y los tres canales de un actor diferido sin limpiar campos vecinos.
void actor(unsigned char *d, Palabra registro, Palabra entidad, Palabra intervalo, Palabra a,
           Palabra b, Palabra c) {
  guardar(d, registro, entidad);
  guardar(d, registro + 2, intervalo);
  guardar(d, registro + 4, a);
  guardar(d, registro + 6, b);
  guardar(d, registro + 8, c);
}
// B432–B576: lanzamiento134, rodeo original, transformación persistente y tres actores.
ResultadoMenu lanzar_bola(unsigned char *d) {
  actualizar_rescate(d);
  retirar_identificador(d, 134);
  guardar(d, 0x358f, 128);
  guardar(d, 0x1a38, static_cast<Palabra>(581 - palabra(d, 0x3591)));
  guardar(d, 0x1a3a, 147);
  if (con_signo(palabra(d, 0x2c2d)) > con_signo(palabra(d, 0x2c4e))) {
    guardar(d, 0x1a36, 0x1a3c);
    guardar(d, 0x1a3e, palabra(d, 0x1a38));
    guardar(d, 0x1a40, 147);
    guardar(d, 0x1a38, static_cast<Palabra>(palabra(d, 0x2c4e) + palabra(d, 0x2c52)));
    guardar(d, 0x1a3a,
            static_cast<Palabra>(
                palabra(d, 0x2c50) + palabra(d, 0x2c54) +
                (con_signo(palabra(d, 0x2c2f)) < con_signo(palabra(d, 0x2c50)) ? -10 : 4)));
  }
  guardar(d, 0x0ad7, 0xa795);
  guardar(d, 0x0adc, 0xa7ae);
  guardar(d, 0x0ae1, 0xa7c3);
  guardar(d, 0x2c56, 2);
  d[0x4948] = 32;
  guardar(d, 0x4949, static_cast<Palabra>(palabra(d, 0x4949) - 215));
  guardar(d, 0x494b, static_cast<Palabra>(palabra(d, 0x494b) + 43));
  guardar(d, 0x2c6f, static_cast<Palabra>(616 - palabra(d, 0x3591)));
  guardar(d, 0x2c71, 108);
  d[0x2c6d] = 3;
  guardar(d, 0x2c77, 200);
  guardar(d, 0x2c90, static_cast<Palabra>(501 - palabra(d, 0x3591)));
  guardar(d, 0x2c92, 140);
  d[0x2c8e] = 3;
  d[0x2c8f] = 13;
  actor(d, 0x009c, 0x2c4c, 0x0213, 0xa7ed, 0xa81a, 0xa838);
  actor(d, 0x00a6, 0x2c6d, 0x010b, 0xa7d8, 0xa7e1, 0xa7e7);
  actor(d, 0x00b0, 0x2c8e, 0x0147, 0xa856, 0xa86c, 0xa880);
  guardar(d, 0x35c7, static_cast<Palabra>(palabra(d, 0x35c7) + 5));
  unsigned short pasos = 0;
  return iniciar_accion(d, pasos) ? ResultadoMenu::accion : ResultadoMenu::error;
}
// B217: coloca125 sobre el agujero de41 y modifica su descriptor para la visita siguiente.
ResultadoMenu colocar_tablon_nieve(unsigned char *d) {
  retirar_identificador(d, 125);
  d[0x48b4] = 52;
  guardar(d, 0x48b5, static_cast<Palabra>(palabra(d, 0x48b5) - 6));
  guardar(d, 0x48b7, static_cast<Palabra>(palabra(d, 0x48b7) + 7));
  guardar(d, 0x48b9, 0xa683);
  d[0x48bc] = 1;
  guardar(d, 0x1a38, static_cast<Palabra>(274 - palabra(d, 0x3591)));
  guardar(d, 0x1a3a, 143);
  guardar(d, 0x0ad7, 0xa639);
  guardar(d, 0x0adc, 0xa63d);
  guardar(d, 0x35c7, static_cast<Palabra>(palabra(d, 0x35c7) + 5));
  unsigned short pasos = 0;
  return iniciar_accion(d, pasos) ? ResultadoMenu::accion : ResultadoMenu::error;
}
// B265–B34D: entrega128 solo perfumado y ante126 quieto; conserva los tres rodeos originales.
ResultadoMenu entregar_pez(unsigned char *d) {
  if (!(d[0x35c9] & 128) || d[0x2c4d] != 17) {
    marcar_accion_rechazada(d);
    return ResultadoMenu::accion;
  }
  guardar(d, 0x1a38, static_cast<Palabra>(palabra(d, 0x2c4e) + 24));
  guardar(d, 0x1a3a, 171);
  auto vertical =
      static_cast<Palabra>(palabra(d, 0x2c50) + palabra(d, 0x2c54) - 8 - palabra(d, 0x2c2f));
  if (con_signo(vertical) >= con_signo(palabra(d, 0x2c33))) {
    const auto derecha = static_cast<Palabra>(palabra(d, 0x2c2d) + palabra(d, 0x2c31));
    const auto diferencia = static_cast<Palabra>(palabra(d, 0x2c2d) - palabra(d, 0x2c4e));
    guardar(d, 0x1a3e, palabra(d, 0x1a38));
    guardar(d, 0x1a40, palabra(d, 0x1a3a));
    guardar(d, 0x1a36, 0x1a3c);
    if (con_signo(derecha) < con_signo(palabra(d, 0x2c4e)) ||
        con_signo(diferencia) >= con_signo(palabra(d, 0x2c52))) {
      guardar(d, 0x1a38, palabra(d, 0x2c2d));
    } else {
      guardar(d, 0x1a44, palabra(d, 0x1a38));
      guardar(d, 0x1a46, palabra(d, 0x1a3a));
      guardar(d, 0x1a3c, 0x1a42);
      guardar(d, 0x1a38, static_cast<Palabra>(palabra(d, 0x2c4e) - 16));
      guardar(d, 0x1a3e, palabra(d, 0x1a38));
      guardar(d, 0x1a3a, static_cast<Palabra>(palabra(d, 0x2c2f) + 47));
    }
  }
  retirar_identificador(d, 128);
  d[0x48aa] = 0;
  d[0x48c5] = 1;
  guardar(d, 0x0ad7, 0xa640);
  guardar(d, 0x0adc, 0xa64b);
  actor(d, 0x009c, 0x2c4c, 0x0205, 0xa653, 0xa669, 0xa676);
  guardar(d, 0x00a6, 0x2caf);
  guardar(d, 0x00a8, 0x0205);
  guardar(d, 0x00aa, 0x1a2c);
  guardar(d, 0x35c7, static_cast<Palabra>(palabra(d, 0x35c7) + 5));
  unsigned short pasos = 0;
  return iniciar_accion(d, pasos) ? ResultadoMenu::accion : ResultadoMenu::error;
}
// B350–B42F: rompe127 y libera al amigo del hielo con los tres actores originales.
ResultadoMenu liberar_hielo(unsigned char *d) {
  retirar_identificador(d, 127);
  guardar(d, 0x1a38, static_cast<Palabra>(550 - palabra(d, 0x3591)));
  guardar(d, 0x1a3a, 166);
  guardar(d, 0x0ad7, 0xa695);
  guardar(d, 0x0adc, 0xa6b3);
  const auto dx = static_cast<Palabra>(palabra(d, 0x2c2d) - palabra(d, 0x2cb1));
  const auto dy =
      static_cast<Palabra>(palabra(d, 0x2c2f) + palabra(d, 0x2c33) - palabra(d, 0x2cb3));
  if (con_signo(dx) < con_signo(palabra(d, 0x2cb5)) &&
      con_signo(dy) < con_signo(palabra(d, 0x2cb7))) {
    guardar(d, 0x1a36, 0x1a3c);
    guardar(d, 0x1a3e, palabra(d, 0x1a38));
    guardar(d, 0x1a40, palabra(d, 0x1a3a));
    if (con_signo(static_cast<Palabra>(palabra(d, 0x2c2d) + palabra(d, 0x2c31))) <
        con_signo(palabra(d, 0x2cb1)))
      guardar(d, 0x1a38, static_cast<Palabra>(palabra(d, 0x1a38) - 40));
    else
      guardar(d, 0x1a3a, static_cast<Palabra>(palabra(d, 0x1a3a) - 15));
  }
  d[0x492a] = 0;
  d[0x4933] = 1;
  d[0x493c] = 1;
  d[0x493d] = 25;
  guardar(d, 0x493e, static_cast<Palabra>(palabra(d, 0x493e) + 8));
  guardar(d, 0x4940, static_cast<Palabra>(palabra(d, 0x4940) - 3));
  actor(d, 0x009c, 0x2caf, 0x0833, 0xa6cb, 0xa6ee, 0xa703);
  actor(d, 0x00a6, 0x2cf1, 0x021b, 0xa718, 0xa77f, 0xa783);
  guardar(d, 0x00b0, 0x2cd0);
  guardar(d, 0x00b2, 0x014b);
  guardar(d, 0x00b4, 0x1a2c);
  guardar(d, 0x35c7, static_cast<Palabra>(palabra(d, 0x35c7) + 5));
  unsigned short pasos = 0;
  return iniciar_accion(d, pasos) ? ResultadoMenu::accion : ResultadoMenu::error;
}
} // namespace
// Despacha solo acciones investigadas de43; conserva pendientes explícitos para los demás objetos.
ResultadoMenu ejecutar_interaccion_nieve(unsigned char *d, unsigned short manejador,
                                         unsigned short id, const ServicioSonido &sonido) {
  const auto escena = palabra(d, 0x8082);
  if (escena < 40 || escena > 43)
    return ResultadoMenu::pendiente;
  // 34D9–3542: pedir ayuda desde la cornisa inicia el rescate, sin fabricar sus condiciones.
  if (manejador == 0x32de && id == 0 && escena == 40 && con_signo(palabra(d, 0x2c2f)) <= 72 &&
      con_signo(static_cast<Palabra>(palabra(d, 0x2c2d) + palabra(d, 0x3591))) <= 240) {
    guardar(d, 0x358d, 0);
    retirar_identificador(d, 101);
    retirar_identificador(d, 70);
    guardar(d, 0x1a38, static_cast<Palabra>(165 - palabra(d, 0x3591)));
    guardar(d, 0x1a3a, 114);
    guardar(d, 0x0ad7, 0x2675);
    guardar(d, 0x0adc, 0x268d);
    guardar(d, 0x0ae1, 0x269c);
    actor(d, 0x009c, 0x2caf, 0x080b, 0x26ac, 0x26d7, 0x26e5);
    unsigned short pasos = 0;
    return iniciar_accion(d, pasos) ? ResultadoMenu::accion : ResultadoMenu::error;
  }
  if (manejador == 0x32de && id == 1 && escena == 41 && d[0x2c4c] == 1 && d[0x2c4d] == 17) {
    auto distancia = static_cast<Palabra>(palabra(d, 0x2c2d) - palabra(d, 0x2c4e));
    if (con_signo(palabra(d, 0x2c2d)) < con_signo(palabra(d, 0x2c4e)))
      distancia = static_cast<Palabra>(-distancia);
    if (distancia <= 64) {
      guardar(d, 0x2ce0, 0x1a2f);
      d[0x2ce2] = 0;
      d[0x2ce3] = 1;
      d[0x2ce4] = 40;
    }
  }
  if (manejador == 0x32de && id == 1 && escena == 42 && d[0x490f]) {
    auto distancia = static_cast<Palabra>(palabra(d, 0x2c2d) - palabra(d, 0x2c4e));
    if (con_signo(palabra(d, 0x2c2d)) < con_signo(palabra(d, 0x2c4e)))
      distancia = static_cast<Palabra>(-distancia);
    if (con_signo(distancia) <= 64) {
      d[0x490f] = d[0x4918] = 0;
      guardar(d, 0x2c5c, 0x26f3);
      d[0x2c5e] = 0;
      d[0x2c5f] = 7;
      d[0x2c60] = 2;
      guardar(d, 0x2c66, 0x2702);
      d[0x2c68] = 0;
      d[0x2c69] = 7;
      d[0x2c6a] = 2;
      actualizar_rescate(d);
    }
  }
  if (manejador == 0x32de && id <= 1)
    return emitir_sonido(sonido, d, 0x2c2b, static_cast<unsigned char>(id)) ? ResultadoMenu::accion
                                                                            : ResultadoMenu::error;
  if (escena == 43 && id == 134 &&
      (manejador == 0x2fca ||
       (manejador == 0x3b53 && palabra(d, 0xb638) == 0xffff && palabra(d, 0xb634) == 5)))
    return lanzar_bola(d);
  const auto verbo = palabra(d, 0xb634), previo = palabra(d, 0xb638);
  if (manejador == 0x3b53 && escena == 41 && previo == 0xffff && verbo == 5 && id == 125)
    return colocar_tablon_nieve(d);
  if (manejador == 0x3b53 && escena == 41 && previo == 128 && verbo == 6 && id == 126)
    return entregar_pez(d);
  if (escena == 42 && manejador == 0x2fca && id == 127)
    return liberar_hielo(d);
  if (manejador == 0x3b53 && verbo == 4 && previo == 0xffff && id == 132) {
    retirar_identificador(d, 132);
    d[0x35c9] |= 128;
    guardar(d, 0x2c3b, 0xa60a);
    guardar(d, 0x2c40, 0xa626);
    guardar(d, 0x35c7, static_cast<Palabra>(palabra(d, 0x35c7) + 5));
    unsigned short pasos = 0;
    return iniciar_accion(d, pasos) ? ResultadoMenu::accion : ResultadoMenu::error;
  }
  if (manejador == 0x563a &&
      ((escena == 43 && id == 134) || (escena == 42 && (id == 131 || id == 132)) ||
       (escena == 41 && (id == 127 || id == 128)) || (escena == 40 && id == 125))) {
    const auto icono = d[0x524e + id * 2];
    if (icono == 0x1f || icono == 0x22)
      guardar(d, 0x35c7, static_cast<Palabra>(palabra(d, 0x35c7) + (icono == 0x1f ? 1 : 2)));
    else if (!insertar_identificador(d, id)) {
      marcar_accion_rechazada(d);
      return ResultadoMenu::accion;
    }
    if (id == 125) {
      d[0x4857] = 0;
      guardar(d, 0x1a38, static_cast<Palabra>(533 - palabra(d, 0x3591)));
      guardar(d, 0x1a3a, 146);
      guardar(d, 0x0ad7, 0x9789);
      guardar(d, 0x0adc, 0x9796);
      guardar(d, 0x0ae1, 0x979c);
      guardar(d, 0x009c, 0x2c6d);
      guardar(d, 0x009e, 0x0207);
      guardar(d, 0x00a0, 0x1a2e);
      actor(d, 0x00a6, 0x2c8e, 0x0209, 0x97e2, 0x97e6, 0x97ea);
      unsigned short pasos = 0;
      return iniciar_accion(d, pasos) ? ResultadoMenu::accion : ResultadoMenu::error;
    }
    if (id == 128 && palabra(d, 0x2c7d)) {
      retirar_identificador(d, 128);
      marcar_accion_rechazada(d);
      return ResultadoMenu::accion;
    }
    if (id == 131)
      actualizar_rescate(d);
    const Palabra entidad = (id == 134 || id == 128) ? 0x2c6d
                            : id == 132              ? 0x2c8e
                            : id == 127              ? 0x2caf
                                                     : 0x2cf1;
    guardar(d, 0x1a38, static_cast<Palabra>(palabra(d, entidad + 2) + (id == 127 ? 15 : 5)));
    guardar(d, 0x1a3a,
            static_cast<Palabra>(palabra(d, entidad + 4) + palabra(d, entidad + 8) -
                                 (id == 127 ? 7 : 2)));
    const auto lista = palabra(d, static_cast<Palabra>(0x53b5 + escena * 2));
    d[static_cast<Palabra>(lista + ((entidad - 0x2c4c) / 33) * 9 + 1)] = 0;
    guardar(d, entidad + 16, 0x1a2e);
    d[entidad + 18] = 0;
    d[entidad + 20] = 1;
    guardar(d, 0x0ad7, d[0x358c] == 20 ? 0x96f6 : 0x96c0);
    guardar(d, 0x0adc, d[0x358c] == 20 ? 0x96ff : 0x96c9);
    unsigned short pasos = 0;
    if (!iniciar_accion(d, pasos))
      return ResultadoMenu::error;
    d[entidad + 19] = static_cast<unsigned char>(pasos + 6);
    return ResultadoMenu::accion;
  }
  if (id != 70 && id != 76 && id != 101 && id != 124 && (id < 125 || id > 134))
    return ResultadoMenu::pendiente;
  if (manejador == 0x384a)
    return ResultadoMenu::actualizado;
  if (manejador == 0x2fca || manejador == 0x3143 || manejador == 0x3b53) {
    marcar_accion_rechazada(d);
    return ResultadoMenu::accion;
  }
  return ResultadoMenu::pendiente;
}
} // namespace motor
