#pragma once
#include "../mazmorra.hpp"
#include <utility>

// 48B0: llega con el anillo del estudio, regresa123 y abre el pasadizo mediante el menú original.
template <typename Avanzar, typename Abrir, typename Elegir, typename Objeto>
bool comprobar_biblioteca_castillo_sdl(Mazmorra &p, const std::string &opcion, Avanzar avanzar,
                                       Abrir abrir, Elegir elegir, Objeto objeto) {
  const auto detener = [&](const char *hito) {
    return opcion == std::string("--comprobar-final-poblacion-borde_valle_biblioteca_") + hito;
  };
  const auto caminar = [&](unsigned short x, unsigned short y) {
    const auto escena = p.escena_actual();
    if (!p.mover_a(x, y))
      return false;
    for (unsigned int i = 0; i < 192 && p.escena_actual() == escena; ++i)
      if (!avanzar(1))
        return false;
    return true;
  };
  if (p.escena_actual() != 69 || !p.conserva_objeto(175))
    return false;
  for (const auto &punto : {std::pair<unsigned short, unsigned short>{80, 170},
                            {140, 190},
                            {270, 190},
                            {280, 170},
                            {300, 150}}) {
    if (!caminar(punto.first, punto.second))
      return false;
    if (p.escena_actual() != 69)
      break;
  }
  if (p.escena_actual() != 71)
    return false;
  if (detener("entrada"))
    return true;
  if (!avanzar(128))
    return false;
  if (detener("control"))
    return true;
  if (!caminar(24, 164) || p.escena_actual() != 69 || !avanzar(128))
    return false;
  if (detener("regreso"))
    return true;
  for (const auto &punto :
       {std::pair<unsigned short, unsigned short>{270, 190}, {280, 170}, {300, 150}}) {
    if (!caminar(punto.first, punto.second))
      return false;
    if (p.escena_actual() != 69)
      break;
  }
  if (p.escena_actual() != 71 || !avanzar(128))
    return false;
  if (detener("segunda_visita"))
    return true;
  for (const auto &punto :
       {std::pair<unsigned short, unsigned short>{80, 190}, {140, 190}, {142, 159}})
    if (!caminar(punto.first, punto.second))
      return false;
  if (detener("libro"))
    return true;
  // 3B4F deja el menú abierto; el jugador lo cierra con clic derecho antes de continuar.
  if (!abrir() || !elegir(3) || !objeto(176) || !p.entrada_menu(152, 92, false, true) ||
      !avanzar(32))
    return false;
  if (detener("examinada"))
    return true;
  if (!abrir() || !elegir(2) || !elegir(2) || !objeto(176) || !avanzar(32))
    return false;
  if (detener("abriendo"))
    return true;
  if (!avanzar(224))
    return false;
  return detener("abierta");
}
