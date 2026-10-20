#pragma once
#include "../mazmorra.hpp"

// Entrada125 y canales7403/7415/7424: golpe de bate y retorno al reposo, sin adelantar animación.
template <typename Avanzar>
bool comprobar_desenlace_sdl(Mazmorra &p, const std::string &opcion, Avanzar avanzar) {
  const auto detener = [&](const std::string &hito) {
    return opcion == "--comprobar-final-poblacion-borde_valle_bruja_desenlace_" + hito;
  };
  if (p.escena_actual() != 73)
    return false;
  for (const unsigned short id : {76, 124, 164, 167, 171, 175})
    if (p.conserva_objeto(id))
      return false;
  if (detener("entrada"))
    return true;
  unsigned int anterior = 0;
  for (const unsigned int paso : {16, 32, 48}) {
    if (!avanzar(paso - anterior))
      return false;
    if (detener(std::to_string(paso)))
      return true;
    anterior = paso;
  }
  if (!avanzar(3))
    return false;
  if (detener("reposo"))
    return true;
  return avanzar(128) && detener("final");
}
