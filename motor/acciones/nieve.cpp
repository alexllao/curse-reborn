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
} // namespace
// Despacha solo acciones investigadas de43; conserva pendientes explícitos para los demás objetos.
ResultadoMenu ejecutar_interaccion_nieve(unsigned char *d, unsigned short manejador,
                                         unsigned short id, const ServicioSonido &sonido) {
  if (palabra(d, 0x8082) != 43)
    return ResultadoMenu::pendiente;
  if (manejador == 0x32de && id <= 1)
    return emitir_sonido(sonido, d, 0x2c2b, static_cast<unsigned char>(id)) ? ResultadoMenu::accion
                                                                            : ResultadoMenu::error;
  if (id == 134 && (manejador == 0x2fca || (manejador == 0x3b53 && palabra(d, 0xb638) == 0xffff &&
                                            palabra(d, 0xb634) == 5)))
    return lanzar_bola(d);
  if (manejador == 0x563a && id == 134) {
    const auto icono = d[0x524e + id * 2];
    if (icono == 0x1f || icono == 0x22)
      guardar(d, 0x35c7, static_cast<Palabra>(palabra(d, 0x35c7) + (icono == 0x1f ? 1 : 2)));
    else if (!insertar_identificador(d, id)) {
      marcar_accion_rechazada(d);
      return ResultadoMenu::accion;
    }
    guardar(d, 0x1a38, static_cast<Palabra>(palabra(d, 0x2c6f) + 5));
    guardar(d, 0x1a3a, static_cast<Palabra>(palabra(d, 0x2c71) + palabra(d, 0x2c75) - 2));
    const auto lista = palabra(d, 0x53b5 + 43 * 2);
    d[static_cast<Palabra>(lista + 10)] = 0;
    guardar(d, 0x2c7d, 0x1a2e);
    d[0x2c7f] = 0;
    d[0x2c81] = 1;
    guardar(d, 0x0ad7, d[0x358c] == 20 ? 0x96f6 : 0x96c0);
    guardar(d, 0x0adc, d[0x358c] == 20 ? 0x96ff : 0x96c9);
    unsigned short pasos = 0;
    if (!iniciar_accion(d, pasos))
      return ResultadoMenu::error;
    d[0x2c80] = static_cast<unsigned char>(pasos + 6);
    return ResultadoMenu::accion;
  }
  if (id != 70 && id != 76 && id != 101 && id != 124 && id != 134)
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
