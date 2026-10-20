#pragma once
#include "../mazmorra.hpp"
#include "comprobar_guardia_palacio.hpp"

// 27BA–27F1/60B2: sigue los desvíos reales sin cambiar azar, recoge161/162 y vuelve al paso del
// guardia.
template <typename Avanzar, typename Abrir, typename Elegir, typename Objeto, typename Ir>
bool comprobar_laberinto_palacio_sdl(Mazmorra &p, const std::string &opcion, Avanzar avanzar,
                                     Abrir abrir, Elegir elegir, Objeto objeto, Ir ir) {
  const auto detener = [&](const std::string &hito) {
    return opcion == "--comprobar-final-poblacion-borde_valle_" + hito;
  };
  const auto cruzar = [&](int x) {
    const auto origen = p.escena_actual();
    if (!p.mover_a(x, 140))
      return false;
    for (unsigned int paso = 0; paso < 192 && p.escena_actual() == origen; ++paso)
      if (!avanzar(1))
        return false;
    return p.escena_actual() != origen;
  };
  if (!cruzar(160) || p.escena_actual() != 59)
    return false;
  if (detener("laberinto_59_entrada"))
    return true;
  if (!avanzar(16))
    return false;
  if (detener("laberinto_59_control"))
    return true;
  bool vistos[63]{};
  vistos[59] = true;
  bool cerillas = false;
  for (unsigned int intento = 0; intento < 64; ++intento) {
    const auto sala = p.escena_actual();
    if (sala == 62 && !cerillas) {
      unsigned short x = 0, y = 0, alto = 0;
      if (!p.posicion_objeto(161, x, y, alto) || !ir(x + 5, y + alto - 2))
        return false;
      if (detener("laberinto_62_ante_cerillas"))
        return true;
      if (!abrir() || !elegir(1) || !objeto(161) || !avanzar(192) || !p.conserva_objeto(161) ||
          !p.conserva_objeto(162))
        return false;
      cerillas = true;
      if (detener("laberinto_62_cerillas"))
        return true;
    }
    if (sala == 59 && cerillas && detener("laberinto_59_con_cerillas"))
      return true;
    if (!cruzar(sala == 58 || sala == 61 ? 160 : sala == 62 ? 204 : 115))
      return false;
    const auto nueva = p.escena_actual();
    if (nueva == 63)
      return comprobar_guardia_palacio_sdl(p, opcion, avanzar, abrir, elegir, objeto, ir);
    if (nueva < 58 || nueva > 62)
      return false;
    const auto base = "laberinto_" + std::to_string(nueva);
    if (!vistos[nueva] && detener(base + "_entrada"))
      return true;
    if (!avanzar(16))
      return false;
    if (!vistos[nueva] && detener(base + "_control"))
      return true;
    vistos[nueva] = true;
  }
  return false;
}
