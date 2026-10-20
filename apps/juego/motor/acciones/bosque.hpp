#pragma once
#include "menu.hpp"
#include "iniciar_accion.hpp"
#include "../estado/segmento.hpp"

namespace motor {
// 313E/32D9/3B4F/3545/37FF: COPSE/CASTHALL no añaden acciones a los cinco objetos del recorrido.
inline ResultadoMenu ejecutar_interaccion_bosque(unsigned char *d, unsigned short manejador,
                                                 unsigned short id, const ServicioSonido &sonido) {
  if (manejador == 0x32de && id <= 1)
    return emitir_sonido(sonido, d, 0x2c2b, static_cast<unsigned char>(id)) ? ResultadoMenu::accion
                                                                            : ResultadoMenu::error;
  if (id != 76 && id != 124 && id != 164 && id != 167 && id != 171)
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
