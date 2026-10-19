#include "poza.hpp"
#include "iniciar_accion.hpp"
#include "lista_identificadores.hpp"
#include "../estado/segmento.hpp"

namespace motor {
// 59B5–59C9, 61A7–61F1 y 9DE4–9E20: conserva las escrituras y demoras de la poza original.
ResultadoMenu ejecutar_interaccion_poza(unsigned char *d, unsigned short manejador,
                                        unsigned short id, const ServicioSonido &sonido) {
  if (palabra(d, 0x8082) != 8)
    return ResultadoMenu::pendiente;
  if (manejador == 0x32de && id <= 1)
    return emitir_sonido(sonido, d, 0x2c2b, static_cast<unsigned char>(id)) ? ResultadoMenu::accion
                                                                            : ResultadoMenu::error;
  if (manejador == 0x563a && id == 49) {
    // 5652–5679: las algas siguen la inserción o puntuación que indique su icono original.
    const auto icono = d[0x524e + id * 2];
    if (icono == 0x1f || icono == 0x22)
      guardar(d, 0x35c7, static_cast<Palabra>(palabra(d, 0x35c7) + (icono == 0x1f ? 1 : 2)));
    else if (!insertar_identificador(d, id)) {
      marcar_accion_rechazada(d);
      return ResultadoMenu::accion;
    }
    guardar(d, 0x2c7b, 48);
    guardar(d, 0x1a38, static_cast<Palabra>(palabra(d, 0x2c90) + 5));
    guardar(d, 0x1a3a, static_cast<Palabra>(palabra(d, 0x2c92) + palabra(d, 0x2c96) - 9));
    const auto lista = palabra(d, 0x53b5 + 8 * 2);
    d[static_cast<Palabra>(lista + 2 * 9 + 1)] = 0;
    guardar(d, 0x2c9e, 0x1a2e);
    d[0x2ca0] = 0;
    d[0x2ca2] = 1;
    guardar(d, 0x0ad7, d[0x358c] == 0x14 ? 0x9705 : 0x96cf);
    guardar(d, 0x0adc, d[0x358c] == 0x14 ? 0x970e : 0x96d8);
    unsigned short pasos = 0;
    if (!iniciar_accion(d, pasos))
      return ResultadoMenu::error;
    d[0x2ca1] = static_cast<unsigned char>(pasos + 6);
    return ResultadoMenu::accion;
  }
  if (manejador == 0x9de4) {
    // El modelo seis indica puerta cerrada; se programa su apertura cuando Brad pulse el botón.
    if (d[0x2c4d] != 6) {
      marcar_accion_rechazada(d);
      return ResultadoMenu::accion;
    }
    guardar(d, 0x2c5c, 0x9dc0);
    guardar(d, 0x1a38, static_cast<Palabra>(palabra(d, 0x2c6f) - 15));
    guardar(d, 0x1a3a, static_cast<Palabra>(palabra(d, 0x2c71) + 33));
    guardar(d, 0x0ad7, 0x9d91);
    guardar(d, 0x0adc, 0x9db2);
    unsigned short pasos = 0;
    if (!iniciar_accion(d, pasos))
      return ResultadoMenu::error;
    d[0x2c5f] = static_cast<unsigned char>(pasos + 12);
    guardar(d, 0x35c7, static_cast<Palabra>(palabra(d, 0x35c7) + 5));
    return ResultadoMenu::accion;
  }
  if (id == 3 || (id >= 48 && id <= 50)) {
    // 3B4F y 313E/32D9: examinar retorna sin acción; usar y combatir no tienen una rama propia.
    if (manejador == 0x384a)
      return ResultadoMenu::actualizado;
    if (manejador == 0x2fca || manejador == 0x3143) {
      marcar_accion_rechazada(d);
      return ResultadoMenu::accion;
    }
  }
  return ResultadoMenu::pendiente;
}
} // namespace motor
