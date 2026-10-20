#pragma once
#include "../mazmorra.hpp"

// CS17A3[66]=0: atraviesa116 y el sendero de COPSE sin escribir coordenadas ni banderas.
template <typename Avanzar>
bool comprobar_bosque_sdl(Mazmorra &p, const std::string &opcion, Avanzar avanzar) {
  const auto detener = [&](const char *hito) {
    return opcion == std::string("--comprobar-final-poblacion-borde_valle_bosque_") + hito;
  };
  if (p.escena_actual() != 66)
    return false;
  if (detener("entrada"))
    return true;
  if (!avanzar(128))
    return false;
  if (detener("control"))
    return true;
  if (!p.mover_a(90, 120) || !avanzar(192))
    return false;
  return detener("sendero");
}
