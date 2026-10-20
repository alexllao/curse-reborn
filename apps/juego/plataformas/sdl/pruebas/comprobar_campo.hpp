#pragma once
#include "../mazmorra.hpp"

// Continúa COUNTRY1 desde el cubo real, sin inventar objetos ni escribir banderas de los guiones.
template <typename Avanzar>
bool comprobar_recorrido_campo(Mazmorra &partida, const std::string &opcion, Avanzar avanzar) {
  if (partida.escena_actual() != 15)
    return false;
  unsigned int anterior = 0;
  for (const auto paso : {0u, 32u, 64u, 96u, 160u}) {
    if (!avanzar(paso - anterior))
      return false;
    anterior = paso;
    if (opcion == "--comprobar-final-campo-" + std::to_string(paso))
      return true;
  }
  if (!partida.movimiento_habilitado())
    return false;
  for (const auto x : {120u, 150u}) {
    if (!partida.mover_a(static_cast<unsigned short>(x), x == 120 ? 165 : 130) || !avanzar(96))
      return false;
    if (opcion == "--comprobar-final-campo-camino-" + std::to_string(x))
      return true;
  }
  if (!partida.mover_a(110, 119) || !avanzar(32))
    return false;
  if (opcion == "--comprobar-final-campo-sendero")
    return true;
  for (unsigned int i = 0; i < 512 && partida.escena_actual() == 15; ++i)
    if (!avanzar(1))
      return false;
  return partida.escena_actual() == 16;
}
