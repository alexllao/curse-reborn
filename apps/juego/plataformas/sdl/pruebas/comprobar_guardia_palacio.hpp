#pragma once
#include "../mazmorra.hpp"
#include "comprobar_ataud.hpp"

// D41C/BD73/BDD8/62CF: reproduce la partida contrastada hasta el regreso111 a Benn.
template <typename Avanzar, typename Abrir, typename Elegir, typename Objeto, typename Ir>
bool comprobar_guardia_palacio_sdl(Mazmorra &p, const std::string &opcion, Avanzar avanzar,
                                   Abrir abrir, Elegir elegir, Objeto objeto, Ir ir) {
  const auto detener = [&](const std::string &hito) {
    return opcion == "--comprobar-final-poblacion-borde_valle_" + hito;
  };
  if (detener("guardia_entrada"))
    return true;
  if (!avanzar(32))
    return false;
  if (detener("guardia_control"))
    return true;
  if (!ir(60, 169))
    return false;
  if (detener("guardia_columna"))
    return true;
  if (!avanzar(128))
    return false;
  if (detener("guardia_dormido"))
    return true;
  if (!p.mover_a(242, 163) || !avanzar(60) || !abrir() || !elegir(2) || !elegir(1) ||
      !objeto(162) || !objeto(163) || !avanzar(96) || p.conserva_objeto(162))
    return false;
  if (detener("guardia_cerilla"))
    return true;
  if (!ir(160, 180) || !ir(60, 180) || !ir(60, 169) || !avanzar(128) || !p.mover_a(242, 163) ||
      !avanzar(60) || !abrir() || !elegir(5) || !objeto(161))
    return false;
  for (unsigned int paso = 0; paso < 192; ++paso) {
    if (!avanzar(1))
      return false;
    if ((paso == 47 || paso == 95 || paso == 191) &&
        detener("guardia_fuego_" + std::to_string(paso + 1)))
      return true;
  }
  unsigned short x = 0, y = 0, alto = 0;
  if (p.conserva_objeto(161) || !p.posicion_objeto(164, x, y, alto) || !ir(x + 2, y + alto) ||
      !abrir() || !elegir(1) || !objeto(164))
    return false;
  for (unsigned int paso = 0; paso < 768 && p.escena_actual() == 63; ++paso)
    if (!avanzar(1))
      return false;
  if (p.escena_actual() != 24 || !p.conserva_objeto(164))
    return false;
  if (detener("guardia_regreso_benn"))
    return true;
  if (!avanzar(192))
    return false;
  if (detener("guardia_benn_control"))
    return true;
  return comprobar_ataud_sdl(p, opcion, avanzar, abrir, elegir, objeto);
}
