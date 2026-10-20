#pragma once
#include "../mazmorra.hpp"

// Continúa la partida real por espada, ataque, dinero y sendero sin modificar sus banderas.
template <typename Avanzar, typename Abrir, typename Elegir, typename Objeto>
bool comprobar_recorrido_asaltante(Mazmorra &partida, const std::string &opcion, Avanzar avanzar,
                                   Abrir abrir, Elegir elegir, Objeto objeto) {
  if (partida.escena_actual() != 16)
    return false;
  if (opcion == "--comprobar-final-asaltante-entrada")
    return true;
  if (!avanzar(32))
    return false;
  if (opcion == "--comprobar-final-asaltante-tropiezo")
    return true;
  if (!avanzar(32))
    return false;
  if (opcion == "--comprobar-final-asaltante-espada-suelo")
    return true;
  if (!partida.mover_a(227, 178) || !avanzar(96) || !abrir() || !elegir(1) || !objeto(75) ||
      !avanzar(128) || !partida.conserva_objeto(75))
    return false;
  if (opcion == "--comprobar-final-asaltante-espada")
    return true;
  if (!abrir() || !elegir(5) || !objeto(75))
    return false;
  if (opcion == "--comprobar-final-asaltante-ataque-inicio")
    return true;
  for (unsigned int i = 0; i < 8; ++i) {
    if (!avanzar(32))
      return false;
    if (opcion == "--comprobar-final-asaltante-ataque-" + std::to_string(i))
      return true;
  }
  if (!partida.mover_a(190, 170) || !avanzar(96) || !abrir() || !elegir(1) || !objeto(76) ||
      !avanzar(128) || !partida.conserva_objeto(76) || partida.conserva_objeto(75))
    return false;
  if (opcion == "--comprobar-final-asaltante-dinero")
    return true;
  constexpr unsigned short camino[][2] = {{65, 175}, {65, 140}, {95, 130}, {110, 110}};
  for (const auto &destino : camino)
    if (!partida.mover_a(destino[0], destino[1]) || !avanzar(96))
      return false;
  if (opcion == "--comprobar-final-asaltante-camino")
    return true;
  if (!partida.mover_a(175, 94) || !avanzar(32))
    return false;
  if (opcion == "--comprobar-final-asaltante-sendero")
    return true;
  for (unsigned int i = 0; i < 512 && partida.escena_actual() == 16; ++i)
    if (!avanzar(1))
      return false;
  return partida.escena_actual() == 21 && partida.conserva_objeto(76);
}
