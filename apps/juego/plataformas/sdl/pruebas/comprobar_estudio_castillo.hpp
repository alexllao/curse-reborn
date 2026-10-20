#pragma once
#include "../mazmorra.hpp"
#include "comprobar_biblioteca_castillo.hpp"

// 6141/633F/61A7: recoge175 por el menú real, regresa121 y comprueba la segunda visita120.
template <typename Avanzar, typename Abrir, typename Elegir, typename Objeto>
bool comprobar_estudio_castillo_sdl(Mazmorra &p, const std::string &opcion, Avanzar avanzar,
                                    Abrir abrir, Elegir elegir, Objeto objeto) {
  const auto detener = [&](const char *hito) {
    return opcion == std::string("--comprobar-final-poblacion-borde_valle_estudio_") + hito;
  };
  if (p.escena_actual() != 70)
    return false;
  if (detener("entrada"))
    return true;
  if (!avanzar(128))
    return false;
  if (detener("control"))
    return true;
  unsigned short x = 0, y = 0, alto = 0;
  if (!p.posicion_objeto(175, x, y, alto) || !p.mover_a(x + 5, y + alto + 1) || !avanzar(192))
    return false;
  if (detener("anillo"))
    return true;
  if (!abrir() || !elegir(1) || !objeto(175) || !avanzar(96) || !p.conserva_objeto(175))
    return false;
  if (detener("recogido"))
    return true;
  if (!p.mover_a(319, 180))
    return false;
  for (unsigned int i = 0; i < 192 && p.escena_actual() == 70; ++i)
    if (!avanzar(1))
      return false;
  if (p.escena_actual() != 69 || !avanzar(128))
    return false;
  if (detener("regreso"))
    return true;
  if (opcion.find("_biblioteca_") != std::string::npos ||
      opcion.find("_bruja_") != std::string::npos)
    return comprobar_biblioteca_castillo_sdl(p, opcion, avanzar, abrir, elegir, objeto);
  if (!p.mover_a(48, 126))
    return false;
  for (unsigned int i = 0; i < 192 && p.escena_actual() == 69; ++i)
    if (!avanzar(1))
      return false;
  if (p.escena_actual() != 70 || !avanzar(128) || !p.conserva_objeto(175))
    return false;
  return detener("segunda_visita");
}
