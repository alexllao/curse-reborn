#pragma once
#include "../mazmorra.hpp"
#include "comprobar_puente_castillo.hpp"

// 4838: reproduce la llegada117, la aproximación y los canales de apertura sin preparar la verja.
template <typename Avanzar, typename Abrir, typename Elegir, typename Objeto>
bool comprobar_verja_sdl(Mazmorra &p, const std::string &opcion, Avanzar avanzar, Abrir abrir,
                         Elegir elegir, Objeto objeto) {
  const auto detener = [&](const char *hito) {
    return opcion == std::string("--comprobar-final-poblacion-borde_valle_verja_") + hito;
  };
  if (p.escena_actual() != 67)
    return false;
  if (detener("entrada"))
    return true;
  if (!avanzar(128))
    return false;
  if (detener("control"))
    return true;
  if (!p.mover_a(180, 171) || !avanzar(192))
    return false;
  if (detener("cerrada"))
    return true;
  if (!abrir() || !elegir(2) || !elegir(2) || !objeto(174) || !avanzar(32))
    return false;
  if (detener("abriendo"))
    return true;
  if (!avanzar(480))
    return false;
  if (detener("abierta"))
    return true;
  for (const auto &punto :
       {std::pair<unsigned int, unsigned short>{370, 145}, {475, 130}, {480, 118}}) {
    for (unsigned int i = 0; i < 1500; ++i) {
      const int local = static_cast<int>(punto.first) - p.ventana_actual() * 32;
      if (local >= 0 && local <= 319) {
        if (!p.mover_a(static_cast<unsigned short>(local), punto.second))
          return false;
        break;
      }
      const unsigned short bits = local < 0 ? 8 : 4;
      p.tecla_direccion(bits, true);
      const bool correcto = avanzar(1);
      p.tecla_direccion(bits, false);
      if (!correcto)
        return false;
    }
    for (unsigned int i = 0; i < 192 && p.escena_actual() == 67; ++i)
      if (!avanzar(1))
        return false;
    if (p.escena_actual() != 67)
      break;
  }
  return comprobar_puente_castillo_sdl(p, opcion, avanzar, abrir, elegir, objeto);
}
