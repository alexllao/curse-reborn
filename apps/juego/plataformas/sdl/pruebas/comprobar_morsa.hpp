#pragma once
#include "../mazmorra.hpp"
#include "comprobar_lago_helado.hpp"
#include <utility>

// CBD0–CF25/B1F8–B576: conserva el recorrido real por las cuatro zonas nevadas y sus regresos.
template <typename Avanzar, typename Abrir, typename Elegir, typename Objeto, typename Ir,
          typename Ruta>
bool comprobar_morsa_sdl(Mazmorra &p, const std::string &opcion, Avanzar avanzar, Abrir abrir,
                         Elegir elegir, Objeto objeto, Ir ir, Ruta ruta) {
  const auto detener = [&](const char *hito) {
    return opcion == std::string("--comprobar-final-poblacion-borde_valle_") + hito;
  };
  const auto accion = [&](unsigned int menu, unsigned short id) {
    return abrir() && elegir(menu) && objeto(id);
  };
  if (!ruta({{620, 170}, {620, 95}}, 42))
    return false;
  if (detener("morsa_entrada"))
    return true;
  if (!ir(600, 143))
    return false;
  if (detener("morsa_ante_desodorante"))
    return true;
  if (!accion(1, 132) || !avanzar(192) || !p.conserva_objeto(132))
    return false;
  if (detener("morsa_desodorante"))
    return true;
  if (!abrir() || !elegir(2) || !elegir(4) || !objeto(132) || !avanzar(256) ||
      p.conserva_objeto(132))
    return false;
  if (detener("morsa_perfumado"))
    return true;
  if (!ir(1000, 175))
    return false;
  if (detener("morsa_ante_saludo"))
    return true;
  if (!abrir() || !elegir(4) || !elegir(0) || !avanzar(256))
    return false;
  if (detener("morsa_saludada"))
    return true;
  if (!ruta({{620, 175}, {620, 95}}, 41))
    return false;
  if (detener("esquimal_entrada"))
    return true;
  if (!ruta({{620, 175}, {620, 95}}, 40))
    return false;
  if (detener("rescate_entrada"))
    return true;
  if (!ir(533, 148))
    return false;
  if (detener("rescate_ante_tablon"))
    return true;
  if (!accion(1, 125) || !avanzar(256) || !p.conserva_objeto(125))
    return false;
  if (detener("rescate_tablon"))
    return true;
  if (!ruta({{620, 180}, {620, 199}}, 41))
    return false;
  if (detener("esquimal_con_tablon"))
    return true;
  if (!ir(274, 150) || !abrir() || !elegir(2) || !elegir(5) || !objeto(125) || !avanzar(256))
    return false;
  if (detener("esquimal_tablon_colocado"))
    return true;
  if (!accion(1, 128) || !avanzar(192) || !p.conserva_objeto(128))
    return false;
  if (detener("esquimal_pez"))
    return true;
  if (!ir(850, 175) || !abrir() || !elegir(2) || !elegir(6) || !objeto(128) || !objeto(126) ||
      !avanzar(512))
    return false;
  if (detener("esquimal_entrega"))
    return true;
  if (!accion(1, 127) || !avanzar(192) || !p.conserva_objeto(127))
    return false;
  if (detener("esquimal_cana"))
    return true;
  if (!ruta({{620, 180}, {620, 199}}, 42))
    return false;
  if (detener("morsa_con_cana"))
    return true;
  if (!ir(550, 166) || !accion(5, 127) || !avanzar(1024) || p.conserva_objeto(127))
    return false;
  if (detener("morsa_amigo_liberado"))
    return true;
  if (!accion(1, 131) || !avanzar(192) || !p.conserva_objeto(131))
    return false;
  if (detener("morsa_cenizas"))
    return true;
  if (!ruta({{620, 175}, {620, 95}}, 41) || !ruta({{620, 175}, {620, 95}}, 40))
    return false;
  if (detener("rescate_amigos_listos"))
    return true;
  if (!ir(165, 114))
    return false;
  // La ruta bordea la pared con entrada real: bajar8, izquierda4 y subir16 provoca CBD0.
  for (const auto paso : {std::pair<unsigned short, unsigned int>{1, 8}, {8, 4}, {2, 16}}) {
    p.tecla_direccion(paso.first, true);
    const bool correcto = avanzar(paso.second);
    p.tecla_direccion(paso.first, false);
    if (!correcto)
      return false;
  }
  if (!avanzar(128) || !p.movimiento_habilitado())
    return false;
  if (detener("rescate_cornisa"))
    return true;
  if (!abrir() || !elegir(4) || !elegir(1))
    return false;
  return detener("rescate_ayuda") ||
         comprobar_lago_helado_sdl(p, opcion, avanzar, abrir, elegir, objeto, ir);
}
