#pragma once
#include "../mazmorra.hpp"

// 84E7/8503/851E: conserva los mismos pasos de entrada y derrumbe que el EXE.
template <typename Avanzar>
bool comprobar_puente_castillo_sdl(Mazmorra &p, const std::string &opcion, Avanzar avanzar) {
  const auto detener = [&](const std::string &hito) {
    return opcion == "--comprobar-final-poblacion-borde_valle_puente_castillo_" + hito;
  };
  if (p.escena_actual() != 68)
    return false;
  if (detener("entrada"))
    return true;
  unsigned int anterior = 0;
  for (const unsigned int paso : {32, 64, 80}) {
    if (!avanzar(paso - anterior))
      return false;
    if (detener(std::to_string(paso)))
      return true;
    anterior = paso;
  }
  return false;
}
