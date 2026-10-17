#pragma once
#include "mazmorra.hpp"

// Recorre monitor, tablón, cornisa y agujero desde el cordel obtenido en la misma partida.
template <typename Avanzar, typename Abrir, typename Elegir, typename Objeto>
bool comprobar_recorrido_catapulta(Mazmorra &partida, const std::string &opcion, Avanzar avanzar,
                                   Abrir abrir, Elegir elegir, Objeto objeto) {
  // Usa el clic y las flechas del jugador, incluyendo las ventanas originales de cámara.
  const auto caminar = [&](int global, unsigned short destino_y) {
    int local = global - partida.ventana_actual() * 32;
    unsigned short x = 0, y = 0, alto = 0;
    if (local < 30 || local > 290)
      if (!partida.posicion_objeto(0, x, y, alto) ||
          !partida.mover_a(static_cast<unsigned short>(x + 16), 170) || !avanzar(96))
        return false;
    for (unsigned int i = 0; i < 1500; ++i) {
      local = global - partida.ventana_actual() * 32;
      if (local >= 30 && local <= 290)
        break;
      const unsigned short bit = local < 30 ? 8 : 4;
      partida.tecla_direccion(bit, true);
      const bool correcto = avanzar(1);
      partida.tecla_direccion(bit, false);
      if (!correcto)
        return false;
    }
    if (local < 30 || local > 290 ||
        !partida.mover_a(static_cast<unsigned short>(local), destino_y))
      return false;
    const auto escena = partida.escena_actual();
    for (unsigned int i = 0; i < 192 && partida.escena_actual() == escena; ++i)
      if (!avanzar(1))
        return false;
    return true;
  };
  // Comprueba la salida real antes de consumir la entrada a la siguiente escena.
  const auto cruzar = [&](int x, unsigned short y, unsigned short escena) {
    return caminar(x, y) && partida.escena_actual() == escena && avanzar(96);
  };
  if (!cruzar(150, 199, 5) || !cruzar(148, 90, 6) || !cruzar(670, 199, 7))
    return false;
  if (opcion == "--comprobar-catapulta-corredor")
    return true;
  if (!cruzar(160, 199, 13) || !caminar(255, 180) || !abrir() || !elegir(1) || !objeto(61) ||
      !avanzar(192))
    return false;
  if (opcion == "--comprobar-catapulta-monitor")
    return partida.conserva_objeto(61);
  if (!cruzar(160, 199, 7) || !caminar(348, 130) || !cruzar(348, 80, 6) || !cruzar(148, 90, 11) ||
      !caminar(75, 162) || !abrir() || !elegir(1) || !objeto(56) || !avanzar(192) || !abrir() ||
      !elegir(2) || !elegir(7) || !objeto(56) || !objeto(58) || !avanzar(192))
    return false;
  if (opcion == "--comprobar-catapulta-tablon")
    return true;
  if (!caminar(123, 160) || !abrir() || !elegir(2) || !elegir(5) || !objeto(61) || !avanzar(256))
    return false;
  if (opcion == "--comprobar-catapulta-cornisa")
    return true;
  if (!abrir() || !elegir(1) || !objeto(57) || !avanzar(192) || !abrir() || !elegir(6) ||
      !objeto(59) || !avanzar(192) || !caminar(200, 180))
    return false;
  if (opcion == "--comprobar-catapulta-iman")
    return partida.conserva_objeto(57);
  if (!cruzar(160, 199, 6) || !cruzar(670, 199, 7) || !cruzar(160, 199, 13) || !abrir() ||
      !elegir(2) || !elegir(7) || !objeto(55) || !elegir(0) || !objeto(57) || !avanzar(96) ||
      !abrir() || !elegir(2) || !elegir(5) || !objeto(70) || !avanzar(192))
    return false;
  return partida.conserva_objeto(63);
}
