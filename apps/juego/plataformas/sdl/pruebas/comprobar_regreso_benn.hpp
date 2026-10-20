#pragma once
#include "../mazmorra.hpp"
#include "comprobar_nieve.hpp"

// 44AB/5CAF/430F/1A74: regresa con el ventilador, paga a Benn y viste79 en el probador real.
template <typename Avanzar, typename Abrir, typename Elegir, typename Objeto, typename Ir,
          typename Ruta>
bool comprobar_regreso_benn_sdl(Mazmorra &p, const std::string &opcion, Avanzar avanzar,
                                Abrir abrir, Elegir elegir, Objeto objeto, Ir ir, Ruta ruta) {
  // Cada hito detiene la misma partida; la continuación74 se comprueba en el bloque de nieve.
  const auto detener = [&](const char *nombre) {
    return opcion == std::string("--comprobar-final-poblacion-borde_valle_") + nombre;
  };
  if (p.escena_actual() != 16 || !p.conserva_objeto(124) ||
      !ruta({{65, 175}, {65, 140}, {95, 130}, {110, 110}, {175, 94}, {220, 60}}, 21) ||
      !ruta({{140, 185}, {110, 160}, {105, 140}}, 17) ||
      !ruta({{260, 180}, {310, 100}, {319, 70}}, 19) ||
      !ruta({{300, 170}, {380, 150}, {380, 120}}, 24))
    return false;
  if (detener("benn_regreso"))
    return true;
  if (!ir(128, 145))
    return false;
  if (detener("benn_frente_vendedor"))
    return true;
  if (!abrir() || !elegir(2) || !elegir(6) || !objeto(76) || !objeto(78) || !avanzar(256))
    return false;
  if (detener("benn_pago_real"))
    return true;
  if (!abrir() || !elegir(1) || !objeto(79) || !avanzar(192) || !p.conserva_objeto(79))
    return false;
  if (detener("benn_vestido_real"))
    return true;
  if (!ruta({{140, 160}, {100, 155}, {77, 135}}, 25))
    return false;
  if (detener("probador_regreso_valle"))
    return true;
  if (!abrir() || !elegir(2) || !elegir(4) || !objeto(79) || !avanzar(512) ||
      p.conserva_objeto(79) || !p.conserva_objeto(124))
    return false;
  return detener("probador_vestido_real") ||
         comprobar_nieve_sdl(p, opcion, avanzar, abrir, elegir, objeto, ir, ruta);
}
