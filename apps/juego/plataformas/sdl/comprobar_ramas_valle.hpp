#pragma once
#include "mazmorra.hpp"
#include "comprobar_dinosaurio.hpp"
#include <initializer_list>

// C737 y5D89: recorre POST→REMOTE y prueba la reacción que protege el mando108.
template <typename Avanzar, typename Abrir, typename Elegir, typename Objeto, typename Ir,
          typename Ruta>
bool comprobar_ramas_valle(Mazmorra &p, const std::string &opcion, Avanzar avanzar, Abrir abrir,
                           Elegir elegir, Objeto objeto, Ir ir, Ruta ruta) {
  // Detiene la comparación en la imagen exacta de cada hito original.
  const auto detener = [&](const char *nombre) {
    return opcion == std::string("--comprobar-final-poblacion-borde_") + nombre;
  };
  // Selecciona los bits de movimiento originales sin escribir coordenadas ni banderas.
  const auto andar = [&](unsigned short direccion, unsigned int pasos) {
    for (const auto bit : {1, 2, 4, 8})
      p.tecla_direccion(bit, (direccion & bit) != 0);
    return avanzar(pasos);
  };
  unsigned short x = 0, y = 0, alto = 0;
  if (p.escena_actual() != 35 || !p.posicion_objeto(0, x, y, alto) ||
      !p.mover_a(static_cast<unsigned short>(x + 16), 170) || !avanzar(192) || !andar(8, 24) ||
      !andar(0, 1) || !p.posicion_objeto(0, x, y, alto) ||
      !p.mover_a(static_cast<unsigned short>(x + 16), 145) || !avanzar(128) ||
      !ruta({{250, 145}, {230, 170}, {120, 185}, {40, 199}}, 34))
    return false;
  if (detener("remote_entrada"))
    return true;
  if (!p.mover_a(180, 175) || !avanzar(192))
    return false;
  if (detener("remote_reaccion"))
    return true;
  if (!p.mover_a(110, 165) || !avanzar(128) || !abrir() || !elegir(1) || !objeto(108) ||
      !avanzar(128) || p.conserva_objeto(108))
    return false;
  if (detener("remote_rechazo"))
    return true;
  if (!ruta({{170, 122}}, 33))
    return false;
  if (detener("celeste_entrada"))
    return true;
  if (detener("dinosaurio_desde_celeste"))
    return ruta({{200, 134}}, 32);
  if (!ruta({{100, 198}}, 35))
    return false;
  if (detener("post_regreso_rama"))
    return true;
  if (opcion.rfind("--comprobar-final-poblacion-borde_dinosaurio_", 0) == 0 ||
      detener("pelo_recogido") || opcion.rfind("--comprobar-final-poblacion-borde_valle_", 0) == 0)
    return comprobar_dinosaurio(p, opcion, avanzar, abrir, elegir, objeto, ir, ruta);
  if (!abrir() || !elegir(1) || !objeto(111) || !avanzar(128) || !p.conserva_objeto(111))
    return false;
  return detener("post_objeto_111");
}
