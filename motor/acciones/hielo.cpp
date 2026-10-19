#include "hielo.hpp"
#include "iniciar_accion.hpp"
#include "lista_identificadores.hpp"
#include "../estado/segmento.hpp"
namespace motor {
namespace {
// Conserva los canales diferidos que el manejador original omite.
void actor(unsigned char *d, Palabra p, Palabra entidad, Palabra intervalo, Palabra a,
           Palabra b = 0xffff, Palabra c = 0xffff) {
  guardar(d, p, entidad);
  guardar(d, p + 2, intervalo);
  guardar(d, p + 4, a);
  if (b != 0xffff)
    guardar(d, p + 6, b);
  if (c != 0xffff)
    guardar(d, p + 8, c);
}
} // namespace
// B579/B5D4: recoge exactamente los destinos, actores y puntuación de las dos acciones del lago.
ResultadoMenu ejecutar_interaccion_hielo(unsigned char *d, unsigned short manejador,
                                         unsigned short id, const ServicioSonido &sonido) {
  if (palabra(d, 0x8082) != 44)
    return ResultadoMenu::pendiente;
  if (manejador == 0x32de && id <= 1)
    return emitir_sonido(sonido, d, 0x2c2b, static_cast<unsigned char>(id)) ? ResultadoMenu::accion
                                                                            : ResultadoMenu::error;
  const bool salto = manejador == 0x3143 && id == 135;
  const bool cenizas =
      manejador == 0x3b53 && id == 131 && palabra(d, 0xb634) == 5 && palabra(d, 0xb638) == 0xffff;
  if (salto || cenizas) {
    if (salto) {
      guardar(d, 0x1a38, static_cast<Palabra>(91 - palabra(d, 0x3591)));
      guardar(d, 0x1a3a, 180);
      guardar(d, 0x0ad7, 0xa912);
      guardar(d, 0x0adc, 0xa943);
      guardar(d, 0x0ae1, 0xa96b);
      actor(d, 0x009c, 0x2c4c, 0x020f, 0xa8ab, 0xa8c6, 0xa8dd);
      actor(d, 0x00a6, 0x2c6d, 0x0215, 0xa8f4, 0xffff, 0xa906);
    } else {
      retirar_identificador(d, 131);
      guardar(d, 0x358d, 0);
      guardar(d, 0x358f, 500);
      guardar(d, 0x1a38, static_cast<Palabra>(102 - palabra(d, 0x3591)));
      guardar(d, 0x1a3a, 177);
      guardar(d, 0x0ad7, 0xa993);
      guardar(d, 0x0adc, 0xa9b5);
      guardar(d, 0x0ae1, 0xa9d7);
      actor(d, 0x009c, 0x2c8e, 0x022b, 0xa9f7, 0xaa29, 0xaa41);
      actor(d, 0x00a6, 0x2caf, 0x0215, 0xa8f4, 0xffff, 0xa906);
      actor(d, 0x00b0, 0x2cd0, 0x0231, 0xaa59, 0xaa6c, 0xaa83);
      actor(d, 0x00ba, 0x2cf1, 0x021b, 0xa8f4, 0xffff, 0xa906);
      actor(d, 0x00c4, 0x2d12, 0x2e33, 0x1a32);
    }
    guardar(d, 0x35c7, static_cast<Palabra>(palabra(d, 0x35c7) + (salto ? 2 : 5)));
    unsigned short pasos = 0;
    return iniciar_accion(d, pasos) ? ResultadoMenu::accion : ResultadoMenu::error;
  }
  if (id != 76 && id != 124 && id != 131 && id != 135)
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
