#pragma once
#include "../mazmorra.hpp"
#include "comprobar_nube.hpp"

// A81A y5D2F–5DD9: conserva la partida real al obtener pelo, sello y bolígrafo y volver a POST.
template <typename Avanzar, typename Abrir, typename Elegir, typename Objeto, typename Ir,
          typename Ruta>
bool comprobar_dinosaurio(Mazmorra &p, const std::string &opcion, Avanzar avanzar, Abrir abrir,
                          Elegir elegir, Objeto objeto, Ir ir, Ruta ruta) {
  // Cada imagen usa el mismo número de pasos y acciones que comparar:dinosaurio.
  const auto detener = [&](const char *nombre) {
    return opcion == std::string("--comprobar-final-poblacion-borde_") + nombre;
  };
  if (p.escena_actual() != 35)
    return false;
  for (const unsigned short y : {100, 140}) {
    const auto local = static_cast<unsigned short>(550 - p.ventana_actual() * 32);
    if (!p.mover_a(local, y) || !avanzar(128))
      return false;
  }
  if (!abrir() || !elegir(1) || !objeto(110) || !avanzar(128) || !p.conserva_objeto(110))
    return false;
  if (detener("pelo_recogido"))
    return true;
  if (!ruta({{550, 140}, {620, 145}, {704, 145}, {850, 190}, {850, 199}}, 32))
    return false;
  if (detener("dinosaurio_desde_post"))
    return true;
  if (!ir(250, 170) || !ir(340, 140) || !abrir() || !elegir(2) || !elegir(1) || !objeto(110) ||
      !objeto(102) || !avanzar(256) || p.conserva_objeto(110))
    return false;
  if (detener("dinosaurio_estornudo"))
    return true;
  for (const unsigned short id : {109, 101})
    if (!abrir() || !elegir(1) || !objeto(id) || !avanzar(128) || !p.conserva_objeto(id))
      return false;
  if (detener("dinosaurio_sello_boligrafo"))
    return true;
  if (opcion.rfind("--comprobar-final-poblacion-borde_valle_", 0) == 0)
    return comprobar_nube(p, opcion, avanzar, abrir, elegir, objeto, ir, ruta);
  return ruta({{400, 185}, {250, 197}, {250, 199}}, 35) && detener("dinosaurio_regreso_post");
}
