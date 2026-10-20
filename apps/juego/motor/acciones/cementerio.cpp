#include "cementerio.hpp"
#include "iniciar_accion.hpp"
#include "lista_identificadores.hpp"
#include "../estado/segmento.hpp"

namespace motor {
// 563A/60CE/4502–4554: conserva capacidad, consumo ignorado, canales y puntuación envolvente.
ResultadoMenu ejecutar_interaccion_cementerio(unsigned char *d, unsigned short manejador,
                                              unsigned short id, const ServicioSonido &sonido) {
  if (palabra(d, 0x8082) != 64)
    return ResultadoMenu::pendiente;
  if (manejador == 0x32de && id <= 1)
    return emitir_sonido(sonido, d, 0x2c2b, static_cast<unsigned char>(id)) ? ResultadoMenu::accion
                                                                            : ResultadoMenu::error;
  if (manejador == 0x563a && id == 165) {
    const auto icono = d[0x524e + id * 2];
    if (icono == 31 || icono == 34)
      guardar(d, 0x35c7, static_cast<Palabra>(palabra(d, 0x35c7) + (icono == 31 ? 1 : 2)));
    else if (!insertar_identificador(d, id)) {
      marcar_accion_rechazada(d);
      return ResultadoMenu::accion;
    }
    d[0x2c6d] = 0;
    guardar(d, 0x2c3b, 0x9858);
    unsigned short pasos = 0;
    return iniciar_accion(d, pasos) ? ResultadoMenu::accion : ResultadoMenu::error;
  }
  if (manejador == 0x2fca && id == 165) {
    retirar_identificador(d, 165);
    guardar(d, 0x2c3b, 0x2714);
    constexpr Palabra canales[] = {0x2742, 0x2752, 0x2758};
    for (unsigned int i = 0; i < 3; ++i) {
      const auto p = static_cast<Palabra>(0x2c5c + i * 5);
      guardar(d, p, canales[i]);
      d[p + 2] = 0;
      d[p + 3] = 9;
      d[p + 4] = 8;
    }
    guardar(d, 0x35c7, static_cast<Palabra>(palabra(d, 0x35c7) + 5));
    unsigned short pasos = 0;
    return iniciar_accion(d, pasos) ? ResultadoMenu::accion : ResultadoMenu::error;
  }
  if (id != 76 && id != 124 && id != 164 && id != 165)
    return ResultadoMenu::pendiente;
  if (manejador == 0x384a)
    return ResultadoMenu::actualizado;
  if (manejador == 0x3b53 || manejador == 0x2fca || manejador == 0x3143) {
    marcar_accion_rechazada(d);
    return ResultadoMenu::accion;
  }
  return ResultadoMenu::pendiente;
}
} // namespace motor
