#pragma once
#include "../mazmorra.hpp"

// Entrada119: contrasta las aproximaciones a las dos escaleras sin escribir posición ni progreso.
template <typename Avanzar>
bool comprobar_vestibulo_castillo_sdl(Mazmorra &p, const std::string &opcion, Avanzar avanzar) {
  const auto detener = [&](const char *hito) {
    return opcion == std::string("--comprobar-final-poblacion-borde_valle_vestibulo_") + hito;
  };
  if (p.escena_actual() != 69)
    return false;
  if (detener("entrada"))
    return true;
  if (!avanzar(128))
    return false;
  if (detener("control"))
    return true;
  const bool derecha = detener("derecha");
  if (!p.mover_a(derecha ? 270 : 90, derecha ? 190 : 160) || !avanzar(192))
    return false;
  return detener(derecha ? "derecha" : "izquierda");
}
