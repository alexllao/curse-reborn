#pragma once
#include "../mazmorra.hpp"
#include "comprobar_desenlace.hpp"

// 20CD/4925–4A76: repite las defensas de la ruta DOS sin cambiar fases ni inventario.
template <typename Avanzar, typename Abrir, typename Elegir, typename Objeto>
bool comprobar_bruja_sdl(Mazmorra &p, const std::string &opcion, Avanzar avanzar, Abrir abrir,
                         Elegir elegir, Objeto objeto) {
  const auto detener = [&](const char *hito) {
    return opcion == std::string("--comprobar-final-poblacion-borde_valle_bruja_") + hito;
  };
  if (p.escena_actual() != 72)
    return false;
  if (detener("entrada"))
    return true;
  if (!avanzar(65))
    return false;
  if (detener("fantasma"))
    return true;
  if (!abrir() || !elegir(5) || !objeto(171) || !avanzar(30))
    return false;
  if (detener("aspiradora"))
    return true;
  if (!abrir() || !elegir(2) || !elegir(2) || !elegir(0) || !objeto(164) || !avanzar(28))
    return false;
  if (detener("extintor"))
    return true;
  if (!abrir() || !elegir(5) || !objeto(124) || !avanzar(159))
    return false;
  if (detener("ventilador"))
    return true;
  if (!abrir() || !elegir(2) || !elegir(7) || !objeto(175) || !objeto(177) || !avanzar(32))
    return false;
  if (detener("anillo_32"))
    return true;
  if (!avanzar(32))
    return false;
  if (detener("anillo_64"))
    return true;
  return avanzar(21) && comprobar_desenlace_sdl(p, opcion, avanzar);
}
