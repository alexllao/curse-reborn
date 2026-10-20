#pragma once
#include "bosque.hpp"

namespace motor {
// 4838–48AF: empujar174 prepara cuatro auxiliares conservando sus canales no escritos; +5/886B.
inline ResultadoMenu ejecutar_interaccion_verja(unsigned char *d, unsigned short manejador,
                                                unsigned short id, const ServicioSonido &sonido) {
  if (manejador == 0x3b53 && palabra(d, 0xb638) == 0xffff && palabra(d, 0xb634) == 2 && id == 174) {
    guardar(d, 0x1a38, static_cast<Palabra>(180 - palabra(d, 0x3591)));
    guardar(d, 0x1a3a, 171);
    guardar(d, 0x0ad7, 0x2a65);
    guardar(d, 0x0adc, 0x2a9b);
    guardar(d, 0x0ae1, 0x2acb);
    guardar(d, 0x009c, 0x2c4c);
    guardar(d, 0x009e, 0x0207);
    guardar(d, 0x00a0, 0x2afb);
    guardar(d, 0x00a4, 0x2b01);
    guardar(d, 0x00a6, 0x2c6d);
    guardar(d, 0x00a8, 0x0253);
    guardar(d, 0x00ae, 0x2b1a);
    guardar(d, 0x00b0, 0x2c8e);
    guardar(d, 0x00b2, 0x0243);
    guardar(d, 0x00b4, 0x2b05);
    guardar(d, 0x00b8, 0x2b10);
    guardar(d, 0x00ba, 0x2caf);
    guardar(d, 0x00bc, 0x024b);
    guardar(d, 0x00c2, 0x2b1a);
    guardar(d, 0x35c7, static_cast<Palabra>(palabra(d, 0x35c7) + 5));
    unsigned short pasos = 0;
    return iniciar_accion(d, pasos) ? ResultadoMenu::accion : ResultadoMenu::error;
  }
  if (id == 174) {
    if (manejador == 0x384a)
      return ResultadoMenu::actualizado;
    if (manejador == 0x2fca || manejador == 0x3143 || manejador == 0x3b53) {
      marcar_accion_rechazada(d);
      return ResultadoMenu::accion;
    }
    return ResultadoMenu::pendiente;
  }
  return ejecutar_interaccion_bosque(d, manejador, id, sonido);
}
} // namespace motor
