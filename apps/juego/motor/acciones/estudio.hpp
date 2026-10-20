#pragma once
#include "bosque.hpp"
#include "lista_identificadores.hpp"

namespace motor {
// 563A/6141–6153/633F/61A7: recoge175 sobre la mesa, marca53AD y conserva capacidad y modelo.
inline ResultadoMenu ejecutar_interaccion_estudio(unsigned char *d, unsigned short manejador,
                                                  unsigned short id, const ServicioSonido &sonido) {
  if (manejador == 0x563a && id == 175) {
    const auto icono = d[0x524e + id * 2];
    if (icono == 31 || icono == 34)
      guardar(d, 0x35c7, static_cast<Palabra>(palabra(d, 0x35c7) + (icono == 31 ? 1 : 2)));
    else if (!insertar_identificador(d, id)) {
      marcar_accion_rechazada(d);
      return ResultadoMenu::accion;
    }
    d[0x53ad] |= 1;
    guardar(d, 0x1a38, static_cast<Palabra>(palabra(d, 0x2c4e) + 5));
    guardar(d, 0x1a3a, static_cast<Palabra>(palabra(d, 0x2c50) + palabra(d, 0x2c54) + 1));
    const auto lista = palabra(d, 0x53b5 + 70 * 2);
    d[static_cast<Palabra>(lista + 1)] = 0;
    guardar(d, 0x2c5c, 0x1a2e);
    d[0x2c5e] = 0;
    d[0x2c60] = 1;
    guardar(d, 0x0ad7, d[0x358c] == 20 ? 0x9705 : 0x96cf);
    guardar(d, 0x0adc, d[0x358c] == 20 ? 0x970e : 0x96d8);
    unsigned short pasos = 0;
    if (!iniciar_accion(d, pasos))
      return ResultadoMenu::error;
    d[0x2c5f] = static_cast<unsigned char>(pasos + 6);
    return ResultadoMenu::accion;
  }
  if (id == 175) {
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
