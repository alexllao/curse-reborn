#pragma once
#include "../mazmorra.hpp"
#include "comprobar_barco_valle.hpp"

// 337D–34D6 y5E57: pide ayuda, descubre el detergente120 y regresa a DINO en la misma partida.
template <typename Avanzar, typename Abrir, typename Elegir, typename Objeto, typename Ir,
          typename Ruta>
bool comprobar_cueva_valle_sdl(Mazmorra &p, const std::string &opcion, Avanzar avanzar, Abrir abrir,
                               Elegir elegir, Objeto objeto, Ir ir, Ruta ruta) {
  // Cada parada corresponde a una composición contrastada de comparar:cueva-valle.
  const auto detener = [&](const char *nombre) {
    return opcion == std::string("--comprobar-final-poblacion-borde_valle_") + nombre;
  };
  if (p.escena_actual() != 35 || !p.conserva_objeto(121) || !ruta({{850, 190}, {850, 199}}, 32) ||
      !ruta({{250, 170}, {150, 165}, {133, 148}}, 37))
    return false;
  if (detener("cueva_entrada"))
    return true;
  if (!abrir() || !elegir(4) || !elegir(1) || !avanzar(512))
    return false;
  if (detener("cueva_derrumbe"))
    return true;
  if (!ir(171, 145) || !abrir() || !elegir(1) || !objeto(120) || !avanzar(192) ||
      !p.conserva_objeto(120))
    return false;
  if (detener("cueva_detergente"))
    return true;
  if (!ruta({{150, 199}}, 32))
    return false;
  return detener("cueva_regreso_dino") ||
         comprobar_barco_valle_sdl(p, opcion, avanzar, abrir, elegir, objeto, ir, ruta);
}
