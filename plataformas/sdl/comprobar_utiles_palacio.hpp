#pragma once
#include "mazmorra.hpp"

// 91–94/5FC4/6062: obtiene gato y lupa con tiradas reales y conserva los regresos e inventario.
template <typename Avanzar, typename Abrir, typename Elegir, typename Objeto, typename Ir>
bool comprobar_utiles_palacio_sdl(Mazmorra &p, const std::string &opcion, Avanzar avanzar,
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
  if (!ruta(160, 199, 48) || !ruta(160, 199, 47) || !ruta(160, 199, 46))
    return false;
  for (unsigned short escena : {50, 51}) {
    unsigned int tiradas = 0;
    while (!p.puerta_lateral_abierta(escena)) {
      if (++tiradas > 32 || !ir(259, 165) || !accion(1, 140) || !avanzar(192) ||
          !p.conserva_objeto(140) || !abrir() || !elegir(2) || !elegir(5) || !objeto(140) ||
          !avanzar(512))
        return false;
    }
    const std::string nombre = escena == 50 ? "gato" : "lupa";
    const unsigned short id = escena == 50 ? 149 : 150;
    if (detener(nombre + "_tirada"))
      return true;
    if (!ruta(160, 145, 47))
      return false;
    if (detener(nombre + "_puerta"))
      return true;
    if (escena == 50) {
      if (!ir(80, 182) || !ir(49, 182) || !ir(70, 190) || !ir(50, 190) || !ruta(40, 182, escena))
        return false;
    } else if (!ir(220, 175) || !ruta(242, 172, escena))
      return false;
    if (detener(nombre + "_entrada"))
      return true;
    if (!ir(escena == 50 ? 116 : 242, 180))
      return false;
    if (escena == 51 && !ir(242, 165))
      return false;
    if (detener(nombre + "_ante_objeto"))
      return true;
    if (!accion(1, id) || !avanzar(192) || !p.conserva_objeto(id))
      return false;
    if (detener(nombre + "_recogido"))
      return true;
    if (!ruta(escena == 50 ? 280 : 41, 165, 47) || !p.conserva_objeto(id))
      return false;
    if (detener(nombre + "_regreso"))
      return true;
    if (!ruta(160, 199, 46))
      return false;
  }
  return detener("utiles_dados");
}
