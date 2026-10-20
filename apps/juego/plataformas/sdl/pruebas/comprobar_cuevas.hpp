#pragma once
#include "../mazmorra.hpp"

// Reproduce clics y casillas de la partida persistente; solo consulta estado y nunca lo modifica.
template <typename Avanzar, typename Abrir, typename Elegir, typename Objeto>
bool comprobar_recorrido_cuevas(Mazmorra &partida, const std::string &opcion, Avanzar avanzar,
                                Abrir abrir, Elegir elegir, Objeto objeto) {
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
  // Recoge el identificador original y espera su animación antes de continuar.
  const auto recoger = [&](unsigned short id) {
    unsigned short x = 0, y = 0, alto = 0;
    if (!partida.posicion_objeto(id, x, y, alto))
      return false;
    const int global = static_cast<unsigned short>(x + partida.ventana_actual() * 32 + 5);
    return caminar(global, static_cast<unsigned short>(y + alto - 2)) && abrir() && elegir(1) &&
           objeto(id) && avanzar(96) && partida.conserva_objeto(id);
  };
  // Verifica el cambio real de habitación y consume los mismos 96 pasos de entrada que DOS.
  const auto cruzar = [&](int x, unsigned short y, unsigned short escena) {
    return caminar(x, y) && partida.escena_actual() == escena && avanzar(96);
  };
  // Da la piedra al picapedrero con sus dos selecciones del menú original.
  const auto entregar = [&](unsigned short piedra) {
    return abrir() && elegir(2) && elegir(6) && objeto(piedra) && objeto(54) && avanzar(96) &&
           !partida.conserva_objeto(piedra);
  };
  for (const auto piedra : {27, 28, 29})
    if (!recoger(static_cast<unsigned short>(piedra)))
      return false;
  if (opcion == "--comprobar-cuevas-piedras-uno")
    return true;
  if (!cruzar(160, 199, 5))
    return false;
  for (const auto piedra : {31, 32, 34})
    if (!recoger(static_cast<unsigned short>(piedra)))
      return false;
  if (!cruzar(1048, 90, 10) || !caminar(66, 174))
    return false;
  for (const auto piedra : {27, 28, 29, 31, 32, 34})
    if (!entregar(static_cast<unsigned short>(piedra)))
      return false;
  if (opcion == "--comprobar-cuevas-primera-entrega")
    return true;
  if (!cruzar(150, 199, 5) || !recoger(33) || !recoger(35) || !cruzar(148, 90, 6))
    return false;
  for (const auto piedra : {38, 39, 40, 41})
    if (!recoger(static_cast<unsigned short>(piedra)))
      return false;
  if (opcion == "--comprobar-cuevas-piedras-tres")
    return true;
  if (!cruzar(160, 199, 5) || !cruzar(1048, 90, 10) || !caminar(66, 174))
    return false;
  for (const auto piedra : {33, 35, 38, 39, 40, 41})
    if (!entregar(static_cast<unsigned short>(piedra)))
      return false;
  return partida.conserva_objeto(55);
}
