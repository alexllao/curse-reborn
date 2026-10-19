#pragma once
#include "mazmorra.hpp"

// D114/BCDF: continúa la reja real, sufre un golpe y esquiva dos proyectiles hasta el trono.
template <typename Avanzar, typename Abrir, typename Elegir, typename Objeto>
bool comprobar_reina_hielo_sdl(Mazmorra &p, const std::string &opcion, Avanzar avanzar, Abrir abrir,
                               Elegir elegir, Objeto objeto) {
  const auto detener = [&](const char *hito) {
    return opcion == std::string("--comprobar-final-poblacion-borde_valle_") + hito;
  };
  for (unsigned int i = 0; i < 192 && p.escena_actual() == 56; ++i)
    if (!avanzar(1))
      return false;
  if (p.escena_actual() != 57)
    return false;
  if (detener("reina_entrada"))
    return true;
  if (!avanzar(8))
    return false;
  if (detener("reina_control"))
    return true;
  // Usa las mismas teclas y la selección original de Saltar160 que el recorrido contrastado.
  const auto caminar = [&](unsigned int pasos) {
    p.tecla_direccion(4, true);
    const bool bien = avanzar(pasos);
    p.tecla_direccion(4, false);
    return bien;
  };
  const auto saltar = [&]() { return abrir() && elegir(6) && objeto(160); };
  if (!saltar() || !caminar(8))
    return false;
  if (detener("reina_salto_temprano"))
    return true;
  if (!caminar(10))
    return false;
  if (detener("reina_golpe"))
    return true;
  if (!caminar(20))
    return false;
  if (detener("reina_recuperada"))
    return true;
  if (!caminar(8))
    return false;
  if (detener("reina_preparada"))
    return true;
  if (!saltar() || !caminar(8))
    return false;
  if (detener("reina_salto_primero"))
    return true;
  if (!caminar(17))
    return false;
  if (detener("reina_entre_saltos"))
    return true;
  if (!saltar() || !caminar(8))
    return false;
  if (detener("reina_salto_segundo"))
    return true;
  if (!caminar(20))
    return false;
  if (detener("reina_trono"))
    return true;
  if (!caminar(16))
    return false;
  return detener("reina_expulsion");
}
