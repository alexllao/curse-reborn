#pragma once
#include "../mazmorra.hpp"

// Continúa la partida real por agujeros, barro, pozo y salida de las cuevas.
template <typename Avanzar, typename Abrir, typename Elegir, typename Objeto>
bool comprobar_recorrido_final_cuevas(Mazmorra &partida, const std::string &opcion, Avanzar avanzar,
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
  // Recoge el objeto real a partir de su tamaño, sin escribir coordenadas ni inventario.
  const auto recoger = [&](unsigned short id) {
    unsigned short x = 0, y = 0, alto = 0;
    return partida.posicion_objeto(id, x, y, alto) &&
           caminar(static_cast<unsigned short>(x + partida.ventana_actual() * 32 + 5),
                   static_cast<unsigned short>(y + alto - 2)) &&
           abrir() && elegir(1) && objeto(id) && avanzar(96) && partida.conserva_objeto(id);
  };
  if (!cruzar(160, 199, 7) || !recoger(47))
    return false;
  if (opcion == "--comprobar-final-moneda")
    return true;
  if (!caminar(548, 130) || !cruzar(548, 80, 14))
    return false;
  for (const auto id : {65, 66, 67, 68}) {
    unsigned short x = 0, y = 0, alto = 0;
    if ((id == 68 && !caminar(245, 185)) ||
        !partida.posicion_objeto(static_cast<unsigned short>(id), x, y, alto) ||
        !caminar(x + (id == 68 ? 4 : 19), static_cast<unsigned short>(y + 42)) || !abrir() ||
        !elegir(3) || !objeto(static_cast<unsigned short>(id)) || !avanzar(192))
      return false;
    if (opcion == "--comprobar-final-agujero-" + std::to_string(id))
      return true;
  }
  if (!abrir() || !elegir(1) || !objeto(64) || !avanzar(192))
    return false;
  if (opcion == "--comprobar-final-ramitas")
    return partida.conserva_objeto(64);
  if (!cruzar(160, 199, 7) || !caminar(348, 130) || !cruzar(348, 80, 6) || !cruzar(160, 199, 5) ||
      !caminar(860, 179) || !abrir() || !elegir(2) || !elegir(7) || !objeto(63) || !objeto(36) ||
      !avanzar(256))
    return false;
  if (opcion == "--comprobar-final-trampa")
    return true;
  if (!recoger(37))
    return false;
  if (opcion == "--comprobar-final-barro")
    return true;
  if (!cruzar(598, 90, 4) || !caminar(348, 130) || !cruzar(348, 80, 9) || !abrir() || !elegir(2) ||
      !elegir(5) || !objeto(47) || !avanzar(192))
    return false;
  if (opcion == "--comprobar-final-ofertas-pozo")
    return true;
  if (!abrir() || !elegir(1) || !objeto(53) || !avanzar(192))
    return false;
  if (opcion == "--comprobar-final-casco-suelo")
    return true;
  if (!recoger(53))
    return false;
  if (opcion == "--comprobar-final-casco")
    return true;
  if (!cruzar(150, 199, 4) || !cruzar(160, 199, 5) || !cruzar(148, 90, 6) || !caminar(820, 160) ||
      !abrir() || !elegir(2) || !elegir(4) || !objeto(53) || !avanzar(256))
    return false;
  if (opcion == "--comprobar-final-desprendimiento")
    return true;
  if (!caminar(1048, 130) || !cruzar(1048, 80, 12))
    return false;
  if (opcion == "--comprobar-final-cubo")
    return true;
  if (!abrir() || !elegir(6) || !objeto(60) || !avanzar(192))
    return false;
  if (opcion == "--comprobar-final-dentro-cubo")
    return true;
  if (!abrir() || !elegir(2) || !elegir(7) || !objeto(37) || !elegir(0) || !objeto(49) ||
      !avanzar(96) || !abrir() || !elegir(2) || !elegir(7) || !objeto(72) || !elegir(0) ||
      !objeto(64) || !avanzar(96))
    return false;
  if (opcion == "--comprobar-final-mascara")
    return partida.conserva_objeto(74);
  if (!abrir() || !elegir(2) || !elegir(4) || !objeto(74))
    return false;
  for (unsigned int i = 0; i < 512 && partida.escena_actual() == 12; ++i)
    if (!avanzar(1))
      return false;
  return partida.escena_actual() == 15;
}
