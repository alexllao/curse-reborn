#include "barco.hpp"
#include "iniciar_accion.hpp"
#include "lista_identificadores.hpp"
#include "../estado/segmento.hpp"

namespace motor {
namespace {
// AC8F–AFBB: usa el tablón que se lleva, conserva los dos emplazamientos y la caída fuera de ellos.
ResultadoMenu colocar_tablon(unsigned char *d, Palabra solicitado) {
  Palabra id = solicitado;
  if (!contiene_identificador(d, id))
    id = id == 117 ? 118 : 117;
  if (!retirar_identificador(d, id)) {
    marcar_accion_rechazada(d);
    return ResultadoMenu::accion;
  }
  const bool primero = id == 117;
  const Palabra entidad = primero ? 0x2c4c : 0x2c6d;
  const Palabra descriptor = primero ? 0x46e8 : 0x46f1;
  const Palabra bandera = primero ? 0x5337 : 0x5339;
  const bool horizontal = (d[primero ? 0x5339 : 0x5337] & 2) != 0;
  const int x = con_signo(palabra(d, 0x2c2d)), y = con_signo(palabra(d, 0x2c2f));
  const bool encaja =
      y >= 90 && y <= 100 && (horizontal ? (x >= 119 && x <= 139) : (x >= 146 && x <= 166));
  unsigned short pasos = 0;
  if (encaja) {
    d[bandera] &= 0xfd;
    d[descriptor] = horizontal ? 9 : 10;
    guardar(d, descriptor + 1, horizontal ? 131 : 160);
    guardar(d, descriptor + 3, horizontal ? 156 : 132);
    if (horizontal)
      guardar(d, descriptor + 5, 0xa3fe);
    guardar(d, 0x1a38, static_cast<Palabra>((palabra(d, 0x2c31) >> 1) + (horizontal ? 122 : 155)));
    guardar(d, 0x1a3a, static_cast<Palabra>(palabra(d, 0x2c33) + (horizontal ? 93 : 90)));
    guardar(d, 0x0ad7, horizontal ? 0xa3c7 : 0xa3d3);
    guardar(d, 0x0adc, horizontal ? 0xa3cd : 0xa3d9);
    guardar(d, 0x0ae1, horizontal ? 0xa3d0 : 0xa3dc);
    if (horizontal)
      guardar(d, entidad + 10, 0xfe70);
    guardar(d, entidad + 2, horizontal ? 130 : 161);
    guardar(d, entidad + 4, horizontal ? 113 : 92);
    guardar(d, 0x009c, entidad);
    guardar(d, 0x009e, 0x0201);
    guardar(d, 0x00a0, horizontal ? 0xa3df : 0xa3ed);
    guardar(d, 0x00a2, horizontal ? 0xa3e5 : 0xa3f4);
    guardar(d, 0x00a4, horizontal ? 0xa3e9 : 0xa3f9);
  } else {
    guardar(d, 0x2c3b, 0xa422);
    guardar(d, 0x2c40, 0xa42a);
    guardar(d, 0x2c45, 0xa42f);
    const auto suelo_x = static_cast<Palabra>(palabra(d, 0x2c2d) - 29);
    const auto suelo_y = static_cast<Palabra>(palabra(d, 0x2c2f) + 68);
    guardar(d, entidad + 2, suelo_x);
    guardar(d, descriptor + 1, suelo_x);
    guardar(d, entidad + 4, suelo_y);
    guardar(d, descriptor + 3, suelo_y);
    guardar(d, entidad + 16, 0xa434);
    d[entidad + 18] = 0;
    d[entidad + 19] = 3;
    d[entidad + 20] = 2;
  }
  return iniciar_accion(d, pasos) ? ResultadoMenu::accion : ResultadoMenu::error;
}
} // namespace

// 563A/5DDC–5E54 y AC8F–AFBB: conserva capacidad, exclusión de los tablones y oferta persistente.
ResultadoMenu ejecutar_interaccion_barco(unsigned char *d, unsigned short manejador,
                                         unsigned short id, const ServicioSonido &sonido) {
  if (palabra(d, 0x8082) != 36)
    return ResultadoMenu::pendiente;
  if (manejador == 0x32de && id <= 1)
    return emitir_sonido(sonido, d, 0x2c2b, static_cast<unsigned char>(id)) ? ResultadoMenu::accion
                                                                            : ResultadoMenu::error;
  if (manejador == 0x563a && (id == 117 || id == 118 || id == 119)) {
    const auto icono = d[0x524e + id * 2];
    if (icono == 0x1f || icono == 0x22)
      guardar(d, 0x35c7, static_cast<Palabra>(palabra(d, 0x35c7) + (icono == 0x1f ? 1 : 2)));
    else if (!insertar_identificador(d, id)) {
      marcar_accion_rechazada(d);
      return ResultadoMenu::accion;
    }
    const Palabra entidad = id == 117 ? 0x2c4c : id == 118 ? 0x2c6d : 0x2c8e;
    if (id != 119 && contiene_identificador(d, id == 117 ? 118 : 117)) {
      retirar_identificador(d, id);
      marcar_accion_rechazada(d);
      return ResultadoMenu::accion;
    }
    guardar(d, 0x1a38, static_cast<Palabra>(palabra(d, entidad + 2) + (id == 119 ? 5 : 35)));
    guardar(d, 0x1a3a,
            static_cast<Palabra>(palabra(d, entidad + 4) +
                                 (id == 119 ? palabra(d, entidad + 8) - 2 : 2)));
    if (id == 119) {
      const auto lista = palabra(d, 0x53b5 + 36 * 2);
      d[static_cast<Palabra>(lista + 19)] = 0;
    }
    guardar(d, 0x0ad7, id == 119 ? (d[0x358c] == 20 ? 0x96f6 : 0x96c0) : 0x97ee);
    guardar(d, 0x0adc, id == 119 ? (d[0x358c] == 20 ? 0x96ff : 0x96c9) : 0x97fa);
    if (id != 119)
      guardar(d, 0x0ae1, 0x9801);
    guardar(d, entidad + 16, id == 119 ? 0x1a2e : 0x1a34);
    d[entidad + 18] = 0;
    d[entidad + 20] = id == 119 ? 1 : 2;
    unsigned short pasos = 0;
    if (!iniciar_accion(d, pasos))
      return ResultadoMenu::error;
    d[entidad + 19] = static_cast<unsigned char>(pasos + (id == 119 ? 6 : 8));
    return ResultadoMenu::accion;
  }
  const auto verbo = palabra(d, 0xb634);
  if (manejador == 0x3b53 && palabra(d, 0xb638) == 0xffff && (verbo == 2 || verbo == 5) &&
      (id == 117 || id == 118))
    return colocar_tablon(d, id);
  if (id != 3 && id != 70 && id != 76 && id != 99 && id != 101 && id != 117 && id != 118 &&
      id != 119 && id != 120 && id != 121)
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
