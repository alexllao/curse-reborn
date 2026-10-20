#pragma once
#include "../mazmorra.hpp"
#include <initializer_list>
#include <utility>

// 43B4/60CE/4502: sigue el tercer pago real y excava el ataúd sin añadir inventario.
template <typename Avanzar, typename Abrir, typename Elegir, typename Objeto>
bool comprobar_ataud_sdl(Mazmorra &p, const std::string &opcion, Avanzar avanzar, Abrir abrir,
                         Elegir elegir, Objeto objeto) {
  const auto detener = [&](const std::string &hito) {
    return opcion == "--comprobar-final-poblacion-borde_valle_" + hito;
  };
  const auto ruta = [&](std::initializer_list<std::pair<int, unsigned short>> puntos,
                        unsigned short destino) {
    const auto anterior = p.escena_actual();
    for (const auto &punto : puntos) {
      for (unsigned int i = 0; i < 1500; ++i) {
        const int local = punto.first - p.ventana_actual() * 32;
        if (local >= 0 && local <= 319) {
          if (!p.mover_a(static_cast<unsigned short>(local), punto.second))
            return false;
          break;
        }
        const unsigned short direccion = local < 0 ? 8 : 4;
        p.tecla_direccion(direccion, true);
        const bool correcto = avanzar(1);
        p.tecla_direccion(direccion, false);
        if (!correcto)
          return false;
      }
      for (unsigned int i = 0; i < 192 && p.escena_actual() == anterior; ++i)
        if (!avanzar(1))
          return false;
      if (p.escena_actual() != anterior)
        break;
    }
    return p.escena_actual() == destino && avanzar(128);
  };
  if (p.escena_actual() != 24 || !ruta({{200, 155}, {210, 145}}, 19) ||
      !ruta({{200, 180}, {160, 199}}, 17) ||
      !ruta({{220, 180}, {235, 120}, {225, 70}, {170, 50}, {170, 5}}, 18) ||
      !ruta({{115, 170}, {110, 130}}, 22))
    return false;
  if (detener("ataud_mago"))
    return true;
  if (!abrir() || !elegir(2) || !elegir(6) || !objeto(76) || !objeto(77))
    return false;
  for (unsigned int i = 0; i < 768 && p.escena_actual() == 22; ++i)
    if (!avanzar(1))
      return false;
  if (p.escena_actual() != 64)
    return false;
  if (detener("ataud_entrada"))
    return true;
  if (!avanzar(192))
    return false;
  if (detener("ataud_control"))
    return true;
  if (!abrir() || !elegir(1) || !objeto(165) || !avanzar(96) || !p.conserva_objeto(165))
    return false;
  if (detener("ataud_hueso"))
    return true;
  if (!abrir() || !elegir(5) || !objeto(165) || !avanzar(32))
    return false;
  if (detener("ataud_excavar_32"))
    return true;
  return avanzar(32) && detener("ataud_excavar_64");
}
