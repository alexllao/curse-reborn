#pragma once
#include "mazmorra.hpp"
#include "comprobar_corredores_palacio.hpp"

// 83/CFA9/5F67/B740: recoge escoba y dados, abre la puerta y repite una tirada sin reiniciar el
// azar.
template <typename Avanzar, typename Abrir, typename Elegir, typename Objeto, typename Ir>
bool comprobar_dados_palacio_sdl(Mazmorra &p, const std::string &opcion, Avanzar avanzar,
                                 Abrir abrir, Elegir elegir, Objeto objeto, Ir ir) {
  const auto detener = [&](const char *hito) {
    return opcion == std::string("--comprobar-final-poblacion-borde_valle_") + hito;
  };
  const auto accion = [&](unsigned int menu, unsigned short id) {
    return abrir() && elegir(menu) && objeto(id);
  };
  if (!p.mover_a(145, 100))
    return false;
  for (unsigned int i = 0; i < 192 && p.escena_actual() == 45; ++i)
    if (!avanzar(1))
      return false;
  if (p.escena_actual() != 46)
    return false;
  if (detener("dados_entrada"))
    return true;
  if (!avanzar(256))
    return false;
  if (detener("dados_control"))
    return true;
  if (!ir(48, 146))
    return false;
  if (detener("dados_ante_escoba"))
    return true;
  if (!accion(1, 141) || !avanzar(192) || !p.conserva_objeto(141))
    return false;
  if (detener("dados_escoba"))
    return true;
  if (!ir(259, 165))
    return false;
  if (detener("dados_ante_mesa"))
    return true;
  if (!accion(1, 140) || !avanzar(192) || !p.conserva_objeto(140))
    return false;
  if (detener("dados_recogidos"))
    return true;
  if (!abrir() || !elegir(2) || !elegir(5) || !objeto(140) || !avanzar(512) ||
      p.conserva_objeto(140))
    return false;
  if (detener("dados_tirada"))
    return true;
  if (detener("dados_retirada") || detener("dados_segunda_tirada")) {
    if (!ir(259, 165) || !accion(1, 140) || !avanzar(192))
      return false;
    if (detener("dados_retirada"))
      return true;
    return abrir() && elegir(2) && elegir(5) && objeto(140) && avanzar(512);
  }
  return comprobar_corredores_palacio_sdl(p, opcion, avanzar, abrir, elegir, objeto, ir);
}
