#pragma once
#include "mazmorra.hpp"
#include "comprobar_ramas_valle.hpp"
#include <initializer_list>
#include <utility>

// Continúa la partida real tras el loro hasta el segundo pago al mago y el guardia del valle.
template <typename Avanzar, typename Abrir, typename Elegir, typename Objeto>
bool comprobar_regreso_loro(Mazmorra &p, const std::string &opcion, Avanzar avanzar, Abrir abrir,
                            Elegir elegir, Objeto objeto) {
  // Las capturas de desarrollo no escriben posición, inventario ni banderas de la aventura.
  const auto detener = [&](const char *hito) {
    return opcion == std::string("--comprobar-final-poblacion-borde_") + hito;
  };
  // Recorre la misma planificación del clic, acercando antes la cámara con las flechas.
  const auto ir = [&](int global, unsigned short y) {
    for (unsigned int i = 0; i < 1500; ++i) {
      const int local = global - p.ventana_actual() * 32;
      if (local >= 0 && local <= 319)
        return p.mover_a(static_cast<unsigned short>(local), y) && avanzar(192);
      const unsigned short direccion = local < 0 ? 8 : 4;
      p.tecla_direccion(direccion, true);
      const bool correcto = avanzar(1);
      p.tecla_direccion(direccion, false);
      if (!correcto)
        return false;
    }
    return false;
  };
  // Detiene cada ruta al detectar el mismo cambio original de escena y espera su entrada.
  const auto ruta = [&](std::initializer_list<std::pair<int, unsigned short>> puntos,
                        unsigned short destino) {
    const auto anterior = p.escena_actual();
    for (const auto &punto : puntos) {
      // Un clic que atraviesa la salida no consume pasos del destino en el comparador del EXE.
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
  if (p.escena_actual() != 27 || !ruta({{160, 175}}, 20) ||
      !ruta({{420, 185}, {470, 185}, {479, 180}}, 17) ||
      !ruta({{220, 180}, {235, 120}, {225, 70}, {170, 50}, {170, 5}}, 18) ||
      !ruta({{115, 170}, {110, 130}}, 22))
    return false;
  if (detener("regreso_mago"))
    return true;
  if (!abrir() || !elegir(2) || !elegir(6) || !objeto(76) || !objeto(77) || !avanzar(192))
    return false;
  if (detener("regreso_mago_pagado"))
    return true;
  if (!avanzar(256) || !ruta({{150, 185}, {150, 199}}, 18) ||
      !ruta({{120, 170}, {100, 195}, {100, 199}}, 17) || !ruta({{60, 180}, {8, 100}, {0, 70}}, 20))
    return false;
  for (const auto &punto : {std::pair<int, unsigned short>{420, 185},
                            {210, 180},
                            {200, 145},
                            {100, 145},
                            {80, 165},
                            {20, 165}})
    if (!ir(punto.first, punto.second))
      return false;
  if (detener("regreso_guardia"))
    return true;
  if (!ruta({{0, 165}}, 35))
    return false;
  return detener("valle_post_entrada") ||
         comprobar_ramas_valle(p, opcion, avanzar, abrir, elegir, objeto, ir, ruta);
}
