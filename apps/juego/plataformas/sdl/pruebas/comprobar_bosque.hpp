#pragma once
#include "../mazmorra.hpp"
#include "comprobar_verja.hpp"

// CS17A3[66]=0: atraviesa116 y el sendero de COPSE sin escribir coordenadas ni banderas.
template <typename Avanzar, typename Abrir, typename Elegir, typename Objeto>
bool comprobar_bosque_sdl(Mazmorra &p, const std::string &opcion, Avanzar avanzar, Abrir abrir,
                          Elegir elegir, Objeto objeto) {
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
  if (detener("sendero"))
    return true;
  if (!p.mover_a(90, 78))
    return false;
  for (unsigned int i = 0; i < 192 && p.escena_actual() == 66; ++i)
    if (!avanzar(1))
      return false;
  return comprobar_verja_sdl(p, opcion, avanzar, abrir, elegir, objeto);
}
