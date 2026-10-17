#include "paso_escena.hpp"
#include "segmento.hpp"
#include "../acciones/menu.hpp"
#include "../acciones/iniciar_accion.hpp"
#include "../movimiento/direccion.hpp"
#include "../entidades/mazmorra.hpp"
namespace motor {
// El despacho específico respeta CS:17A3: la escena cero no llama a 183F, que pertenece a la cinco.
Colision avanzar_escena(unsigned char *d, EstadoAleatorio &azar,
                        const RecursosSecuencias &secuencias, const RecursosEntidades &entidades,
                        const unsigned char *geometria, unsigned int longitud,
                        unsigned short direccion) {
  guardar(d, 0x0ae6, palabra(d, 0x2c2d));
  guardar(d, 0x0ae8, palabra(d, 0x2c2f));
  if (direccion != 0xffff && d[0x3594]) {
    guardar(d, 0x0dfa, 0);
    if (!seleccionar_direccion(d, direccion))
      return {false, 0, 0};
  }
  if (!avanzar_accion_pendiente(d) ||
      avanzar_secuencias(d, azar, secuencias) != EstadoSecuencias::correcto)
    return {false, 0, 0};
  if (d[0x9975] != 0xff) {
    const auto escena = d[0x9975];
    d[0x9975] = 0xff;
    return {true, 16, escena};
  }
  const auto colision = resolver_colision_personaje(d, azar, geometria, longitud);
  if (colision.valida && colision.acciones == 8 && colision.parametro == 0 &&
      palabra(d, 0x8082) == 15) {
    // 0AC8 -> 53B4–53E6: entrar en el sendero prepara las tres secuencias de perspectiva.
    guardar(d, 0x0ad7, 0x93dc);
    guardar(d, 0x0adc, 0x944d);
    guardar(d, 0x0ae1, 0x94b9);
    guardar(d, 0x1a38, 100);
    guardar(d, 0x1a3a, 122);
    unsigned short pasos = 0;
    if (!iniciar_accion(d, pasos))
      return {false, 0, 0};
    d[0x2c0b] = 0;
    guardar(d, 0x2c1a, 0);
    guardar(d, 0x359c, 2);
  } else if (!colision.valida || colision.acciones != 0)
    return colision;
  actualizar_historial_personaje(d);
  if (!actualizar_entidades(d, azar, entidades) || !actualizar_rutina_escena(d, azar, entidades))
    return {false, 0, 0};
  return {true, 0, 0};
}
} // namespace motor
