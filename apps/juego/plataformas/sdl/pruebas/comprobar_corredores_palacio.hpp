#pragma once
#include "../mazmorra.hpp"
#include "comprobar_utiles_palacio.hpp"

// 84–89/B93D/B9CE: recorre los corredores reales y conserva cerrada la ventana sin sus útiles.
template <typename Avanzar, typename Abrir, typename Elegir, typename Objeto, typename Ir>
bool comprobar_corredores_palacio_sdl(Mazmorra &p, const std::string &opcion, Avanzar avanzar,
                                      Abrir abrir, Elegir elegir, Objeto objeto, Ir ir) {
  const auto detener = [&](const char *hito) {
    return opcion == std::string("--comprobar-final-poblacion-borde_valle_") + hito;
  };
  const auto ruta = [&](int x, int y, unsigned short salida, unsigned short llegada) {
    const auto anterior = p.escena_actual();
    if (!p.mover_a(x, y))
      return false;
    for (unsigned int i = 0; i < 192 && p.escena_actual() == anterior; ++i)
      if (!avanzar(1))
        return false;
    (void)salida;
    return p.escena_actual() == llegada && avanzar(128);
  };
  const auto accion = [&](unsigned int menu, unsigned short id) {
    return abrir() && elegir(menu) && objeto(id);
  };
  if (!ruta(160, 145, 84, 47))
    return false;
  if (detener("poleas_entrada"))
    return true;
  if (!ir(231, 182))
    return false;
  if (detener("poleas_ante_inspeccion"))
    return true;
  if (!accion(3, 143) || !avanzar(256))
    return false;
  if (detener("poleas_inspeccion"))
    return true;
  if (!ruta(160, 199, 85, 46))
    return false;
  if (detener("poleas_regreso_dados"))
    return true;
  if (!ruta(160, 145, 84, 47))
    return false;
  if (detener("poleas_regreso"))
    return true;
  if (!ruta(160, 166, 86, 48))
    return false;
  if (detener("hielo_pasillo2"))
    return true;
  if (!ruta(160, 199, 87, 47))
    return false;
  if (detener("hielo_vuelta1"))
    return true;
  if (!ruta(160, 166, 86, 48) || !ruta(160, 166, 88, 49))
    return false;
  if (detener("hielo_pasillo3"))
    return true;
  if (!ir(159, 164))
    return false;
  if (detener("hielo_ante_ventana"))
    return true;
  if (!accion(3, 146) || !avanzar(256))
    return false;
  if (detener("hielo_ventana"))
    return true;
  if (!accion(6, 146) || !avanzar(64))
    return false;
  if (detener("hielo_ventana_cerrada"))
    return true;
  if (!ruta(160, 199, 89, 48))
    return false;
  if (detener("hielo_vuelta2"))
    return true;
  if (!ruta(160, 166, 88, 49))
    return false;
  if (detener("hielo_fin_corredores"))
    return true;
  return comprobar_utiles_palacio_sdl(p, opcion, avanzar, abrir, elegir, objeto, ir);
}
