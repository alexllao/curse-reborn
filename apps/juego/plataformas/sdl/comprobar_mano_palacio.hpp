#pragma once
#include "mazmorra.hpp"
#include "comprobar_laberinto_palacio.hpp"

// D36C/BD00: recibe la expulsión del trono, camina hasta la mano y se libera con el aceite real.
template <typename Avanzar, typename Abrir, typename Elegir, typename Objeto, typename Ir>
bool comprobar_mano_palacio_sdl(Mazmorra &p, const std::string &opcion, Avanzar avanzar,
                                Abrir abrir, Elegir elegir, Objeto objeto, Ir ir) {
  const auto detener = [&](const char *hito) {
    return opcion == std::string("--comprobar-final-poblacion-borde_valle_") + hito;
  };
  for (unsigned int i = 0; i < 192 && p.escena_actual() == 57; ++i)
    if (!avanzar(1))
      return false;
  if (p.escena_actual() != 58 || !p.conserva_objeto(144))
    return false;
  if (detener("mano_entrada"))
    return true;
  if (!avanzar(8))
    return false;
  if (detener("mano_control"))
    return true;
  if (!p.mover_a(160, 125) || !avanzar(26))
    return false;
  if (detener("mano_atrapado"))
    return true;
  if (!abrir() || !elegir(2) || !elegir(4) || !objeto(144) || !avanzar(8))
    return false;
  if (detener("mano_libera_8"))
    return true;
  if (!avanzar(16))
    return false;
  if (detener("mano_libera_24"))
    return true;
  if (!avanzar(16))
    return false;
  if (detener("mano_libera_40"))
    return true;
  if (!avanzar(24) || p.conserva_objeto(144))
    return false;
  if (detener("mano_liberado"))
    return true;
  return comprobar_laberinto_palacio_sdl(p, opcion, avanzar, abrir, elegir, objeto, ir);
}
