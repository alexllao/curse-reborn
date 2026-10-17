#pragma once
#include "mazmorra.hpp"
// Reproduce el trayecto de la guía con clics y selecciones, sin escribir estado de juego.
template <typename Avanzar, typename Abrir, typename Elegir, typename Objeto>
bool comprobar_recorrido_poblacion(Mazmorra &partida, const std::string &opcion, Avanzar avanzar,
                                   Abrir abrir, Elegir elegir, Objeto objeto) {
  if (partida.escena_actual() != 21)
    return false;
  // Lleva el destino a la ventana visible antes de usar el mismo clic del jugador.
  const auto caminar = [&](int global, unsigned short destino_y) {
    int local = global - partida.ventana_actual() * 32;
    unsigned short x = 0, y = 0, alto = 0;
    if (local < 30 || local > 290) {
      if (!partida.posicion_objeto(0, x, y, alto) ||
          !partida.mover_a(static_cast<unsigned short>(x + 16), 170) || !avanzar(96))
        return false;
    }
    bool visible = false;
    for (unsigned int i = 0; i < 1500; ++i) {
      local = global - partida.ventana_actual() * 32;
      if (local >= 30 && local <= 290) {
        visible = true;
        break;
      }
      const unsigned short bit = local < 30 ? 8 : 4;
      partida.tecla_direccion(bit, true);
      const bool correcto = avanzar(1);
      partida.tecla_direccion(bit, false);
      if (!correcto)
        return false;
    }
    if (!visible || !partida.mover_a(static_cast<unsigned short>(local), destino_y))
      return false;
    const auto escena = partida.escena_actual();
    for (unsigned int i = 0; i < 192 && partida.escena_actual() == escena; ++i)
      if (!avanzar(1))
        return false;
    return true;
  };
  // Cada punto de control permite comparar el mismo instante del recorrido original.
  const auto detener = [&](const char *hito) {
    return opcion == std::string("--comprobar-final-poblacion-") + hito;
  };
  if (detener("entrada"))
    return true;
  if (!avanzar(160))
    return false;
  if (detener("guardias"))
    return true;
  if (!caminar(320, 185) || !caminar(250, 170) || !caminar(245, 150) ||
      partida.escena_actual() != 29 || !avanzar(128))
    return false;
  if (detener("frank"))
    return true;
  if (!partida.mover_a(150, 140) || !avanzar(128) || !partida.mover_a(150, 125) || !avanzar(128) ||
      !abrir() || !elegir(4) || !elegir(0) || !avanzar(96))
    return false;
  if (detener("saludo"))
    return true;
  if (!abrir() || !elegir(2) || !elegir(6) || !objeto(76) || !objeto(83) || !avanzar(192) ||
      !partida.conserva_objeto(84))
    return false;
  if (detener("comida"))
    return true;
  if (!abrir() || !elegir(2) || !elegir(3) || !objeto(84))
    return false;
  if (detener("comer"))
    return true;
  if (!avanzar(192) || partida.conserva_objeto(84))
    return false;
  if (detener("consumida"))
    return true;
  if (!caminar(160, 180) || partida.escena_actual() != 21 || !avanzar(128))
    return false;
  if (detener("regreso"))
    return true;
  if (!caminar(140, 185) || !caminar(110, 160) || !caminar(105, 140) ||
      partida.escena_actual() != 17 || !avanzar(128))
    return false;
  if (detener("fuente"))
    return true;
  if (!caminar(220, 180) || !caminar(235, 120) || !caminar(225, 70) || !caminar(170, 50) ||
      !caminar(170, 5) || partida.escena_actual() != 18 || !avanzar(128))
    return false;
  if (detener("maravillas"))
    return true;
  if (!caminar(115, 170) || !caminar(110, 130) || partida.escena_actual() != 22 || !avanzar(128))
    return false;
  if (detener("mago"))
    return true;
  if (!abrir() || !elegir(2) || !elegir(6) || !objeto(76) || !objeto(77))
    return false;
  if (detener("transporte"))
    return true;
  for (unsigned int i = 0; i < 768 && partida.escena_actual() == 22; ++i)
    if (!avanzar(1))
      return false;
  return partida.escena_actual() == 30;
}
