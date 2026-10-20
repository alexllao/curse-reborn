#pragma once
#include "../mazmorra.hpp"
#include "comprobar_laser_palacio.hpp"

// B968/B9F4/BA1F: resuelve las poleas y rompe la ventana usando solo los útiles recogidos.
template <typename Avanzar, typename Abrir, typename Elegir, typename Objeto, typename Ir>
bool comprobar_resolucion_palacio_sdl(Mazmorra &p, const std::string &opcion, Avanzar avanzar,
                                      Abrir abrir, Elegir elegir, Objeto objeto, Ir ir) {
  const auto detener = [&](const char *hito) {
    return opcion == std::string("--comprobar-final-poblacion-borde_valle_") + hito;
  };
  const auto ruta = [&](int x, int y, unsigned short llegada) {
    const auto anterior = p.escena_actual();
    if (!p.mover_a(x, y))
      return false;
    for (unsigned int i = 0; i < 192 && p.escena_actual() == anterior; ++i)
      if (!avanzar(1))
        return false;
    return p.escena_actual() == llegada && avanzar(128);
  };
  const auto accion = [&](unsigned int menu, unsigned short id) {
    return abrir() && elegir(menu) && objeto(id);
  };
  if (!ruta(160, 145, 47) || !ir(89, 180))
    return false;
  if (detener("poleas_con_arma"))
    return true;
  if (!abrir() || !elegir(2) || !elegir(1) || !objeto(151) || !objeto(142) || !avanzar(512) ||
      p.conserva_objeto(151))
    return false;
  if (detener("poleas_bajadas"))
    return true;
  if (!ir(238, 190) || !ir(238, 150))
    return false;
  if (detener("poleas_ante_utiles"))
    return true;
  if (!accion(1, 144) || !avanzar(192) || !p.conserva_objeto(144))
    return false;
  if (detener("poleas_aceite"))
    return true;
  if (!ir(238, 150) || !accion(1, 145) || !avanzar(192) || !p.conserva_objeto(145) ||
      p.conserva_objeto(141))
    return false;
  if (detener("poleas_cristal"))
    return true;
  if (!ruta(160, 166, 48) || !ruta(160, 166, 49))
    return false;
  if (detener("ventana_con_utiles"))
    return true;
  if (!ir(261, 188))
    return false;
  if (detener("ventana_ante_silbato"))
    return true;
  if (!accion(1, 147) || !avanzar(192) || !p.conserva_objeto(147))
    return false;
  if (detener("ventana_silbato"))
    return true;
  if (!abrir() || !elegir(2) || !elegir(7) || !objeto(147) || !elegir(0) || !objeto(152) ||
      !avanzar(192) || !p.conserva_objeto(148))
    return false;
  if (detener("ventana_megafono"))
    return true;
  if (!accion(5, 148) || !avanzar(512))
    return false;
  if (detener("ventana_abierta"))
    return true;
  return comprobar_laser_palacio_sdl(p, opcion, avanzar, abrir, elegir, objeto, ir);
}
