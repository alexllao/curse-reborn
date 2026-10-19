#pragma once
#include "mazmorra.hpp"

// 95–102/BAA6: recoge los cuatro útiles restantes con escoba, tiradas y regresos originales.
template <typename Avanzar, typename Abrir, typename Elegir, typename Objeto, typename Ir>
bool comprobar_laterales_palacio_sdl(Mazmorra &p, const std::string &opcion, Avanzar avanzar,
                                     Abrir abrir, Elegir elegir, Objeto objeto, Ir ir) {
  const auto detener = [&](const std::string &hito) {
    return opcion == "--comprobar-final-poblacion-borde_valle_" + hito;
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
  for (unsigned short escena : {52, 53, 54, 55}) {
    unsigned int tiradas = 0;
    while (!p.puerta_lateral_abierta(escena)) {
      if (++tiradas > 32 || !ir(259, 165) || !accion(1, 140) || !avanzar(192) ||
          !p.conserva_objeto(140) || !abrir() || !elegir(2) || !elegir(5) || !objeto(140) ||
          !avanzar(512))
        return false;
    }
    const std::string nombre = escena == 52   ? "arma"
                               : escena == 53 ? "megafono"
                               : escena == 54 ? "hielo"
                                              : "carambano";
    const unsigned short id = escena + 99;
    const unsigned short corredor = escena >= 54 ? 49 : 48;
    if (detener(nombre + "_tirada"))
      return true;
    if (!ruta(160, 145, 47) || !ruta(160, 166, 48))
      return false;
    if (escena >= 54 && !ruta(160, 166, 49))
      return false;
    if (detener(nombre + "_puerta"))
      return true;
    if (escena % 2 == 0) {
      if (!ir(80, 182) || !ir(49, 182) || !ir(70, 190) || !ir(50, 190) || !ruta(40, 182, escena))
        return false;
    } else if (!ir(220, 175) || !ruta(242, 172, escena))
      return false;
    if (detener(nombre + "_entrada"))
      return true;
    if (escena == 52 || escena == 55) {
      if (!accion(5, 141) || !avanzar(512))
        return false;
      if (detener(nombre + "_bajado"))
        return true;
    }
    unsigned short x = 0, y = 0, alto = 0;
    if (!p.posicion_objeto(id, x, y, alto) || !ir(x + 5, y + alto -
                                                             (id == 153   ? 5
                                                              : id == 154 ? 2
                                                                          : 4)))
      return false;
    if (detener(nombre + "_ante_objeto"))
      return true;
    if (!accion(1, id) || !avanzar(192) || !p.conserva_objeto(id))
      return false;
    if (detener(nombre + "_recogido"))
      return true;
    if (!ruta(escena % 2 == 0 ? 280 : 41, 165, corredor))
      return false;
    if (detener(nombre + "_regreso"))
      return true;
    if (escena >= 54 && !ruta(160, 199, 48))
      return false;
    if (!ruta(160, 199, 47) || !ruta(160, 199, 46))
      return false;
    if (detener(nombre + "_dados"))
      return true;
  }
  return detener("laterales_utiles");
}
