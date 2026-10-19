#pragma once
#include "mazmorra.hpp"
#include "comprobar_entrada_palacio.hpp"

// 81/CF26/B579/B5D4: conserva rescate, entrada, salto opcional y cenizas de la misma partida.
template <typename Avanzar, typename Abrir, typename Elegir, typename Objeto, typename Ir>
bool comprobar_lago_helado_sdl(Mazmorra &p, const std::string &opcion, Avanzar avanzar, Abrir abrir,
                               Elegir elegir, Objeto objeto, Ir ir) {
  const auto detener = [&](const char *hito) {
    return opcion == std::string("--comprobar-final-poblacion-borde_valle_") + hito;
  };
  for (unsigned int i = 0; i < 1024 && p.escena_actual() == 40; ++i)
    if (!avanzar(1))
      return false;
  if (p.escena_actual() != 44)
    return false;
  if (detener("lago_entrada"))
    return true;
  if (!avanzar(256))
    return false;
  if (detener("lago_control"))
    return true;
  if (!ir(110, 180))
    return false;
  if (detener("lago_ante_cenizas"))
    return true;
  if (detener("lago_salto135"))
    return abrir() && elegir(6) && objeto(135) && avanzar(512) && p.conserva_objeto(131) &&
           p.movimiento_habilitado();
  if (!abrir() || !elegir(2) || !elegir(5) || !objeto(131))
    return false;
  if (detener("lago_cenizas"))
    return true;
  if (!avanzar(64))
    return false;
  return detener("lago_criatura_64") ||
         comprobar_entrada_palacio_sdl(p, opcion, avanzar, abrir, elegir, objeto, ir);
}
