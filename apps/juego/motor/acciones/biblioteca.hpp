#pragma once
#include "estudio.hpp"

namespace motor {
// 48B0–4924: empujar176 exige libro en187 global; rodea las estanterías si Y>=132 con signo.
inline ResultadoMenu ejecutar_interaccion_biblioteca(unsigned char *d, unsigned short manejador,
                                                     unsigned short id,
                                                     const ServicioSonido &sonido) {
  if (manejador == 0x3b53 && palabra(d, 0xb638) == 0xffff && palabra(d, 0xb634) == 2 && id == 176) {
    if (static_cast<Palabra>(palabra(d, 0x2c6f) + palabra(d, 0x3591)) != 187) {
      marcar_accion_rechazada(d);
      return ResultadoMenu::accion;
    }
    guardar(d, 0x1a38, static_cast<Palabra>(142 - palabra(d, 0x3591)));
    guardar(d, 0x1a3a, 159);
    if (con_signo(palabra(d, 0x2c2f)) >= 132) {
      guardar(d, 0x1a3e, palabra(d, 0x1a38));
      guardar(d, 0x1a40, palabra(d, 0x1a3a));
      guardar(d, 0x1a3a, 179);
      guardar(d, 0x1a36, 0x1a3c);
    }
    guardar(d, 0x0ad7, 0x2b24);
    guardar(d, 0x0adc, 0x2b34);
    guardar(d, 0x009c, 0x2c4c);
    guardar(d, 0x009e, 0x010b);
    guardar(d, 0x00a0, 0x1a2c);
    guardar(d, 0x00a6, 0x2c6d);
    guardar(d, 0x00a8, 0x020f);
    guardar(d, 0x00aa, 0x2b3d);
    guardar(d, 0x00ac, 0x2b40);
    unsigned short pasos = 0;
    return iniciar_accion(d, pasos) ? ResultadoMenu::accion : ResultadoMenu::error;
  }
  if (id == 176) {
    if (manejador == 0x384a)
      return ResultadoMenu::actualizado;
    if (manejador == 0x2fca || manejador == 0x3143 || manejador == 0x3b53) {
      marcar_accion_rechazada(d);
      return ResultadoMenu::accion;
    }
    return ResultadoMenu::pendiente;
  }
  return ejecutar_interaccion_estudio(d, manejador, id, sonido);
}
} // namespace motor
