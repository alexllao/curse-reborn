#pragma once
#include "../mazmorra.hpp"
#include <initializer_list>
#include <utility>

// 1BA3/4555–4837: recorre ambos cementerios desde la excavación real y conserva sus colisiones.
template <typename Avanzar, typename Abrir, typename Elegir, typename Objeto>
bool comprobar_cementerio_sdl(Mazmorra &p, const std::string &opcion, Avanzar avanzar, Abrir abrir,
                              Elegir elegir, Objeto objeto) {
  const auto detener = [&](const std::string &hito) {
    return opcion == "--comprobar-final-poblacion-borde_valle_cementerio_" + hito;
  };
  const auto direccion = [&](unsigned short bits, unsigned int pasos) {
    p.tecla_direccion(bits, true);
    const bool correcto = avanzar(pasos);
    p.tecla_direccion(bits, false);
    return correcto;
  };
  const auto ir = [&](unsigned int x, unsigned short y) {
    for (unsigned int i = 0; i < 1500; ++i) {
      const int local = static_cast<int>(x) - p.ventana_actual() * 32;
      if (local >= 0 && local <= 319)
        return p.mover_a(static_cast<unsigned short>(local), y) && avanzar(192);
      if (!direccion(local < 0 ? 8 : 4, 1))
        return false;
    }
    return false;
  };
  const auto ruta = [&](std::initializer_list<std::pair<unsigned int, unsigned short>> puntos,
                        unsigned short destino) {
    const auto anterior = p.escena_actual();
    for (const auto &punto : puntos) {
      for (unsigned int i = 0; i < 1500; ++i) {
        const int local = static_cast<int>(punto.first) - p.ventana_actual() * 32;
        if (local >= 0 && local <= 319) {
          if (!p.mover_a(static_cast<unsigned short>(local), punto.second))
            return false;
          break;
        }
        if (!direccion(local < 0 ? 8 : 4, 1))
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
  const auto recoger = [&](unsigned short id, unsigned int pasos) {
    return abrir() && elegir(1) && objeto(id) && avanzar(pasos) && p.conserva_objeto(id);
  };
  if (p.escena_actual() != 65)
    return false;
  if (detener("entrada"))
    return true;
  if (!avanzar(49))
    return false;
  if (detener("control"))
    return true;
  if (!direccion(8, 28) || !avanzar(1) || !recoger(168, 12))
    return false;
  if (detener("pala"))
    return true;
  if (!direccion(10, 16) || !avanzar(2))
    return false;
  if (detener("vampiro_tumba"))
    return true;
  if (!avanzar(64))
    return false;
  for (const unsigned short id : {169, 170, 173}) {
    unsigned short x = 0, y = 0, alto = 0;
    if (!p.posicion_objeto(id, x, y, alto))
      return false;
    const unsigned int global = x + p.ventana_actual() * 32u + 5;
    const auto pie = static_cast<unsigned short>(y + alto - 2);
    if (!ir(global, id == 169 ? 195 : 178))
      return false;
    if (id == 170 && (!ir(global + 32, 178) || !ir(global + 32, pie)))
      return false;
    if (!ir(global, pie) || !recoger(id, 96))
      return false;
    if (detener("util_" + std::to_string(id)))
      return true;
    if (id == 169) {
      if (!abrir() || !elegir(5) || !objeto(168) || !avanzar(192))
        return false;
      if (detener("pala_vampiro"))
        return true;
    }
  }
  if (!ir(440, 159) || !abrir() || !elegir(2) || !elegir(2) || !objeto(172) || !avanzar(192))
    return false;
  if (detener("piedra"))
    return true;
  if (!ruta({{550, 178}, {606, 195}}, 77))
    return false;
  if (detener("segundo"))
    return true;
  if (!abrir() || !elegir(2) || !elegir(3) || !objeto(170) || !avanzar(512))
    return false;
  if (detener("ajo"))
    return true;
  if (!abrir() || !elegir(5) || !objeto(173) || !avanzar(512))
    return false;
  if (detener("cruz"))
    return true;
  if (!abrir() || !elegir(5) || !objeto(169) || !avanzar(512))
    return false;
  if (detener("platillos"))
    return true;
  unsigned short x = 0, y = 0, alto = 0;
  if (!p.posicion_objeto(171, x, y, alto) ||
      !ir(x + p.ventana_actual() * 32u + 5, static_cast<unsigned short>(y + alto - 2)) ||
      !recoger(171, 128))
    return false;
  if (detener("util_171"))
    return true;
  if (!ruta({{200, 195}, {0, 195}}, 65))
    return false;
  if (detener("regreso"))
    return true;
  if (!ir(550, 195))
    return false;
  if (!p.posicion_objeto(167, x, y, alto) ||
      !ir(static_cast<unsigned short>(x + p.ventana_actual() * 32u + 5),
          static_cast<unsigned short>(y + alto - 2)) ||
      !recoger(167, 96))
    return false;
  if (detener("util_167"))
    return true;
  if (!ruta({{550, 178}, {606, 195}}, 77) || !ir(110, 170) || !ir(250, 178) || !ir(250, 190) ||
      !ir(450, 190) || !ir(500, 170) || !avanzar(512))
    return false;
  return detener("puerta");
}
