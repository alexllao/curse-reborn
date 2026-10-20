#pragma once
#include "../mazmorra.hpp"
#include "comprobar_morsa.hpp"

// 74/CE35/5F56/B432: entra desde el vestido real y resuelve la bola sin preparar inventario.
template <typename Avanzar, typename Abrir, typename Elegir, typename Objeto, typename Ir,
          typename Ruta>
bool comprobar_nieve_sdl(Mazmorra &p, const std::string &opcion, Avanzar avanzar, Abrir abrir,
                         Elegir elegir, Objeto objeto, Ir ir, Ruta ruta) {
  const auto detener = [&](const char *hito) {
    return opcion == std::string("--comprobar-final-poblacion-borde_valle_") + hito;
  };
  if (!ruta({{160, 160}, {120, 142}}, 43) || !avanzar(128))
    return false;
  if (detener("nieve_control"))
    return true;
  if (!ir(564, 154))
    return false;
  if (detener("nieve_ante_bola"))
    return true;
  if (!abrir() || !elegir(1) || !objeto(134) || !avanzar(192) || !p.conserva_objeto(134))
    return false;
  if (detener("nieve_bola_recogida"))
    return true;
  if (!abrir() || !elegir(2) || !elegir(5) || !objeto(134) || !avanzar(512) ||
      p.conserva_objeto(134))
    return false;
  return detener("nieve_ser_liberado") ||
         comprobar_morsa_sdl(p, opcion, avanzar, abrir, elegir, objeto, ir, ruta);
}
