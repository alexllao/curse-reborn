#pragma once
#include "../mazmorra.hpp"
#include "comprobar_estudio_castillo.hpp"

// Entrada119: contrasta las aproximaciones a las dos escaleras sin escribir posición ni progreso.
template <typename Avanzar, typename Abrir, typename Elegir, typename Objeto>
bool comprobar_vestibulo_castillo_sdl(Mazmorra &p, const std::string &opcion, Avanzar avanzar,
                                      Abrir abrir, Elegir elegir, Objeto objeto) {
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
  if (detener(derecha ? "derecha" : "izquierda"))
    return true;
  if (!p.mover_a(48, 126))
    return false;
  for (unsigned int i = 0; i < 192 && p.escena_actual() == 69; ++i)
    if (!avanzar(1))
      return false;
  return comprobar_estudio_castillo_sdl(p, opcion, avanzar, abrir, elegir, objeto);
}
