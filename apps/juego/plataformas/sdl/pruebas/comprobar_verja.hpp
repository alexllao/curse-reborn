#pragma once
#include "../mazmorra.hpp"

// 4838: reproduce la llegada117, la aproximación y los canales de apertura sin preparar la verja.
template <typename Avanzar, typename Abrir, typename Elegir, typename Objeto>
bool comprobar_verja_sdl(Mazmorra &p, const std::string &opcion, Avanzar avanzar, Abrir abrir,
                         Elegir elegir, Objeto objeto) {
  const auto detener = [&](const char *hito) {
    return opcion == std::string("--comprobar-final-poblacion-borde_valle_verja_") + hito;
  };
  if (p.escena_actual() != 67)
    return false;
  if (detener("entrada"))
    return true;
  if (!avanzar(128))
    return false;
  if (detener("control"))
    return true;
  if (!p.mover_a(180, 171) || !avanzar(192))
    return false;
  if (detener("cerrada"))
    return true;
  if (!abrir() || !elegir(2) || !elegir(2) || !objeto(174) || !avanzar(32))
    return false;
  if (detener("abriendo"))
    return true;
  if (!avanzar(480))
    return false;
  return detener("abierta");
}
