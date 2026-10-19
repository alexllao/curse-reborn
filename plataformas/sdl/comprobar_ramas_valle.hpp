#pragma once
#include "mazmorra.hpp"
#include <initializer_list>

// C737 y5D89: recorre POST→REMOTE y prueba la reacción que protege el mando108.
template <typename Avanzar, typename Abrir, typename Elegir, typename Objeto, typename Ruta>
bool comprobar_ramas_valle(Mazmorra &p, const std::string &opcion, Avanzar avanzar, Abrir abrir,
                           Elegir elegir, Objeto objeto, Ruta ruta) {
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
  return detener("remote_rechazo");
}
