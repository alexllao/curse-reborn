#pragma once
#include "mazmorra.hpp"
#include "comprobar_reina_hielo.hpp"

// BB8D–BCDC/CFD2: prolonga la partida real desde la ventana hasta la apertura automática de la
// reja.
template <typename Avanzar, typename Abrir, typename Elegir, typename Objeto, typename Ir>
bool comprobar_laser_palacio_sdl(Mazmorra &p, const std::string &opcion, Avanzar avanzar,
                                 Abrir abrir, Elegir elegir, Objeto objeto, Ir ir) {
  const auto detener = [&](const char *hito) {
    return opcion == std::string("--comprobar-final-poblacion-borde_valle_") + hito;
  };
  if (!abrir() || !elegir(6) || !objeto(146))
    return false;
  for (unsigned int i = 0; i < 768 && p.escena_actual() == 49; ++i)
    if (!avanzar(1))
      return false;
  if (p.escena_actual() != 56)
    return false;
  if (detener("laser_entrada"))
    return true;
  if (!avanzar(128))
    return false;
  if (detener("laser_control"))
    return true;
  if (!ir(258, 166) || !abrir() || !elegir(2) || !elegir(7) || !objeto(153) || !objeto(155) ||
      !avanzar(192) || p.conserva_objeto(153))
    return false;
  if (detener("laser_pieza_erronea"))
    return true;
  unsigned short x = 0, y = 0, alto = 0;
  if (!p.posicion_objeto(153, x, y, alto) || !ir(x + 5, y + alto + 1) || !abrir() || !elegir(1) ||
      !objeto(153) || !avanzar(192) || !p.conserva_objeto(153))
    return false;
  if (detener("laser_pieza_recuperada"))
    return true;
  struct Reflector {
    unsigned short pieza, soporte, x, y;
    const char *hito;
  };
  constexpr Reflector reflectores[] = {{150, 155, 258, 166, "laser_lupa"},
                                       {153, 156, 74, 156, "laser_cubito"},
                                       {154, 157, 40, 192, "laser_carambano"},
                                       {145, 158, 275, 192, "laser_cristal"}};
  for (const auto &r : reflectores) {
    if (!ir(r.x, r.y) || !abrir() || !elegir(2) || !elegir(7) || !objeto(r.pieza) ||
        !objeto(r.soporte) || !avanzar(192) || p.conserva_objeto(r.pieza))
      return false;
    if (detener(r.hito))
      return true;
  }
  if (!ir(275, 153) || !abrir() || !elegir(2) || !elegir(1) || !objeto(149) || !objeto(159) ||
      !avanzar(32) || p.conserva_objeto(149))
    return false;
  if (detener("laser_reja_32"))
    return true;
  return comprobar_reina_hielo_sdl(p, opcion, avanzar, abrir, elegir, objeto, ir);
}
