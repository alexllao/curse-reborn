#include "iniciar_accion.hpp"
#include "preparar_resultados.hpp"
#include "../movimiento/planificar.hpp"
#include "../estado/segmento.hpp"

namespace motor {
namespace {

void marcar_cursor(unsigned char *datos, unsigned char estado) {
  datos[0x2c0b] = estado;
  guardar(datos, 0x2c1a, 0x9976);
  datos[0x2c1c] = 0;
  datos[0x2c1d] = 0x0f;
  datos[0x2c1e] = 1;
}

} // namespace

void reiniciar_estado_accion(unsigned char *datos) {
  datos[0x3595] = datos[0x3594] = datos[0x3596] = 0;
  guardar(datos, 0x359a, 0);
  guardar(datos, 0x0dfa, 0);
  constexpr unsigned short bases[] = {0x2c3d, 0x2c42, 0x2c47};
  for (auto base : bases) {
    datos[base] = 0;
    datos[base + 1] = 1;
    datos[base + 2] = 2;
  }
}

void marcar_accion_rechazada(unsigned char *datos) {
  marcar_cursor(datos, 2);
}

bool iniciar_accion(unsigned char *datos, unsigned short &ax) {
  reiniciar_estado_accion(datos);
  ax = 1;
  if (palabra(datos, 0x0ad7) != 0) {
    guardar(datos, 0x0dfa, 0x1a36);
    if (!planificar_movimiento(datos, 0x1a36, ax))
      return false;
  }
  marcar_cursor(datos, 1);
  return true;
}

bool ejecutar_accion_resultados(unsigned char *datos, EstadoAleatorio &estado, unsigned short &ax) {
  preparar_resultados(datos, estado);
  return iniciar_accion(datos, ax);
}

} // namespace motor
