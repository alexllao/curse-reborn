#include "palacio.hpp"
#include "iniciar_accion.hpp"
#include "lista_identificadores.hpp"
#include "../estado/segmento.hpp"
namespace motor {
namespace {
// 563A/5F67/5F8E: conserva capacidad, iconos de puntuación y animaciones de dados y escoba.
ResultadoMenu recoger(unsigned char *d, Palabra id) {
  const auto icono = d[0x524e + id * 2];
  if (icono == 0x1f || icono == 0x22)
    guardar(d, 0x35c7, static_cast<Palabra>(palabra(d, 0x35c7) + (icono == 0x1f ? 1 : 2)));
  else if (!insertar_identificador(d, id)) {
    marcar_accion_rechazada(d);
    return ResultadoMenu::accion;
  }
  const bool dados = id == 140;
  const Palabra entidad = dados ? 0x2c4c : 0x2cf1;
  if (dados) {
    guardar(d, 0x009c, 0x2d12);
    guardar(d, 0x009e, 0x0109);
    guardar(d, 0x00a0, 0x9819);
    guardar(d, 0x00a4, 0x981c);
  }
  guardar(d, 0x1a38, static_cast<Palabra>(palabra(d, entidad + 2) + (dados ? 5 : 9)));
  guardar(
      d, 0x1a3a,
      static_cast<Palabra>(palabra(d, entidad + 4) + palabra(d, entidad + 8) + (dados ? 25 : 3)));
  const auto lista = palabra(d, static_cast<Palabra>(0x53b5 + palabra(d, 0x8082) * 2));
  d[static_cast<Palabra>(lista + ((entidad - 0x2c4c) / 33) * 9 + 1)] = 0;
  guardar(d, entidad + 16, 0x1a2e);
  d[entidad + 18] = 0;
  d[entidad + 20] = 1;
  guardar(d, 0x0ad7, dados ? 0x96de : (d[0x358c] == 20 ? 0x9705 : 0x96cf));
  guardar(d, 0x0adc, dados ? 0x96e7 : (d[0x358c] == 20 ? 0x970e : 0x96d8));
  unsigned short pasos = 0;
  if (!iniciar_accion(d, pasos))
    return ResultadoMenu::error;
  d[entidad + 19] = static_cast<unsigned char>(pasos + 6);
  return ResultadoMenu::accion;
}
} // namespace
// Reutiliza B740–B939 y886B ya contrastados; el generador pertenece a la partida llamadora.
ResultadoMenu ejecutar_interaccion_palacio(unsigned char *d, unsigned short manejador,
                                           unsigned short id, const ServicioSonido &sonido,
                                           const ServicioCreacion &creacion) {
  if (palabra(d, 0x8082) != 46)
    return ResultadoMenu::pendiente;
  if (manejador == 0x32de && id <= 1)
    return emitir_sonido(sonido, d, 0x2c2b, static_cast<unsigned char>(id)) ? ResultadoMenu::accion
                                                                            : ResultadoMenu::error;
  if (manejador == 0x563a && (id == 140 || id == 141))
    return recoger(d, id);
  if (manejador == 0x3b53 && palabra(d, 0xb638) == 0xffff && palabra(d, 0xb634) == 5 && id == 140) {
    if (!creacion.azar)
      return ResultadoMenu::pendiente;
    unsigned short ax = 0;
    return ejecutar_accion_resultados(d, *creacion.azar, ax) ? ResultadoMenu::accion
                                                             : ResultadoMenu::error;
  }
  // 49DA/BAA6: ventilador y escoba solo tienen efectos especiales en sus habitaciones posteriores.
  if (id != 76 && id != 124 && id != 140 && id != 141)
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
