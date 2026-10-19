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
// Los actores diferidos conservan los canales que la rutina original no escribe.
void actor_electro(unsigned char *d, Palabra registro, Palabra entidad, Palabra intervalo,
                   Palabra a = 0xffff, Palabra b = 0xffff, Palabra c = 0xffff) {
  guardar(d, registro, entidad);
  guardar(d, registro + 2, intervalo);
  if (a != 0xffff)
    guardar(d, registro + 4, a);
  if (b != 0xffff)
    guardar(d, registro + 6, b);
  if (c != 0xffff)
    guardar(d, registro + 8, c);
}
// AFBE–B1F5: las acciones aceptadas suman cinco puntos y usan886B sin otro consumo de azar.
ResultadoMenu comenzar_electro(unsigned char *d) {
  guardar(d, 0x35c7, static_cast<Palabra>(palabra(d, 0x35c7) + 5));
  unsigned short pasos = 0;
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
// AFBE–B1F5/5E9A–5EA8: conserva acciones por fase y los campos diferidos no escritos.
ResultadoMenu ejecutar_interaccion_electro(unsigned char *d, unsigned short manejador,
                                           unsigned short id, const ServicioSonido &sonido) {
  if (palabra(d, 0x8082) != 39)
    return ResultadoMenu::pendiente;
  if (manejador == 0x32de && id <= 1)
    return emitir_sonido(sonido, d, 0x2c2b, static_cast<unsigned char>(id)) ? ResultadoMenu::accion
                                                                            : ResultadoMenu::error;
  const auto verbo = palabra(d, 0xb634), previo = palabra(d, 0xb638);
  if (manejador == 0x3b53 && previo == 0xffff && verbo == 5 && id == 120) {
    retirar_identificador(d, 120);
    guardar(d, 0x1a38, 39);
    guardar(d, 0x1a3a, 155);
    guardar(d, 0x0ad7, 0xa438);
    guardar(d, 0x0adc, 0xa441);
    actor_electro(d, 0x009c, 0x2cd0, 0x0107, 0x1a2c);
    return comenzar_electro(d);
  }
  if (manejador == 0x3b53 && previo == 0xffff && verbo == 2 && id == 119 && d[0x2cd0] == 1) {
    retirar_identificador(d, 119);
    guardar(d, 0x1a38, 39);
    guardar(d, 0x1a3a, 155);
    guardar(d, 0x0ad7, 0xa447);
    guardar(d, 0x0adc, 0xa450);
    actor_electro(d, 0x009c, 0x2cd0, 0x0107, 0x1a2e);
    actor_electro(d, 0x00a6, 0x2caf, 0x0107, 0x1a2c);
    return comenzar_electro(d);
  }
  if (manejador == 0x3b53 && previo == 0xffff && verbo == 2 && id == 122) {
    guardar(d, 0x1a38, 22);
    guardar(d, 0x1a3a, 155);
    guardar(d, 0x0ad7, 0xa456);
    guardar(d, 0x0adc, 0xa467);
    actor_electro(d, 0x009c, 0x2c6d, 0x010d, 0xffff, 0xffff, 0xa565);
    actor_electro(d, 0x00a6, 0x2c4c, 0x010d, 0xa4ec);
    return comenzar_electro(d);
  }
  if (manejador == 0x3b53 && previo == 0xffff && verbo == 5 && id == 121 && palabra(d, 0x2c71)) {
    retirar_identificador(d, 121);
    guardar(d, 0x1a38, 103);
    guardar(d, 0x1a3a, 177);
    guardar(d, 0x0ad7, 0xa46d);
    guardar(d, 0x0adc, 0xa47a);
    actor_electro(d, 0x009c, 0x2cf1, 0x0207, 0xa5b6, 0xa5c1, 0xa5ca);
    return comenzar_electro(d);
  }
  if (manejador == 0x3b53 && previo == 0xffff && verbo == 5 && id == 115 && d[0x2cf1] == 1) {
    retirar_identificador(d, 115);
    guardar(d, 0x1a38, 112);
    guardar(d, 0x1a3a, 171);
    guardar(d, 0x0ad7, 0xa482);
    guardar(d, 0x0adc, 0xa498);
    guardar(d, 0x0ae1, 0xa4b9);
    actor_electro(d, 0x009c, 0x2d12, 0x0205, 0xa5e5, 0xa5ed);
    actor_electro(d, 0x00a6, 0x2cf1, 0x010f, 0xa5d3, 0xa5d6);
    return comenzar_electro(d);
  }
  if (manejador == 0x3b53 && verbo == 0 &&
      ((id == 123 && previo == 3) || (id == 3 && previo == 123))) {
    retirar_identificador(d, 3);
    retirar_identificador(d, 99);
    d[0x35c9] |= 2;
    d[0x41ec] = d[0x41f5] = 3;
    static constexpr Palabra retirados[] = {0x402e, 0x4025, 0x4066, 0x4083, 0x40d6,
                                            0x418e, 0x41e1, 0x41fe, 0x4236, 0x42dc};
    for (const auto campo : retirados)
      d[campo] = 0;
    guardar(d, 0x1a38, 264);
    guardar(d, 0x1a3a, 166);
    guardar(d, 0x0ad7, 0xa4ca);
    guardar(d, 0x0adc, 0xa4df);
    actor_electro(d, 0x009c, 0x2c8e, 0x0115, 0xa5f0, 0xa5f4, 0xa5ff);
    return comenzar_electro(d);
  }
  if (manejador == 0x563a && id == 124) {
    const auto icono = d[0x5346];
    if (icono == 0x1f || icono == 0x22)
      guardar(d, 0x35c7, static_cast<Palabra>(palabra(d, 0x35c7) + (icono == 0x1f ? 1 : 2)));
    else if (!insertar_identificador(d, 124)) {
      marcar_accion_rechazada(d);
      return ResultadoMenu::accion;
    }
    guardar(d, 0x1a38, static_cast<Palabra>(palabra(d, 0x2d35) + 5));
    guardar(d, 0x1a3a, static_cast<Palabra>(palabra(d, 0x2d37) + palabra(d, 0x2d3b) + 1));
    const auto lista = palabra(d, 0x53b5 + 39 * 2);
    d[static_cast<Palabra>(lista + 64)] = 0;
    guardar(d, 0x2d43, 0x1a2e);
    d[0x2d45] = 0;
    d[0x2d47] = 1;
    guardar(d, 0x0ad7, d[0x358c] == 20 ? 0x9705 : 0x96cf);
    guardar(d, 0x0adc, d[0x358c] == 20 ? 0x970e : 0x96d8);
    unsigned short pasos = 0;
    if (!iniciar_accion(d, pasos))
      return ResultadoMenu::error;
    d[0x2d46] = static_cast<unsigned char>(pasos + 6);
    return ResultadoMenu::accion;
  }
  if (id != 3 && id != 70 && id != 76 && id != 99 && id != 101 && (id < 115 || id > 124))
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
