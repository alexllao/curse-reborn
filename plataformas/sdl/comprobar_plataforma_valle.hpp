#pragma once
#include "mazmorra.hpp"
#include "comprobar_regreso_benn.hpp"
#include <initializer_list>
#include <utility>

// AFBE–B16D/CAE1: limpia, cruza, recoge124 y vuelve por69; comprueba las dos caídas originales.
template <typename Avanzar, typename Abrir, typename Elegir, typename Objeto, typename Ir,
          typename Ruta>
bool comprobar_plataforma_valle_sdl(Mazmorra &p, const std::string &opcion, Avanzar avanzar,
                                    Abrir abrir, Elegir elegir, Objeto objeto, Ir ir, Ruta ruta) {
  // Cada parada coincide con una imagen contrastada de comparar:plataforma-valle.
  const auto detener = [&](const char *nombre) {
    return opcion == std::string("--comprobar-final-poblacion-borde_valle_") + nombre;
  };
  // Mantiene las aproximaciones globales sin cambiar el estado directamente.
  const auto caminar = [&](std::initializer_list<std::pair<int, unsigned short>> puntos) {
    for (const auto &punto : puntos)
      if (!ir(punto.first, punto.second))
        return false;
    return true;
  };
  // Selecciona verbos y objetos ofrecidos por los menús originales.
  const auto accion = [&](std::initializer_list<unsigned short> casillas, unsigned short id) {
    if (!abrir())
      return false;
    for (const auto casilla : casillas)
      if (!elegir(casilla))
        return false;
    return objeto(id);
  };
  if (p.escena_actual() != 33 || !p.conserva_objeto(119) || !p.conserva_objeto(120) ||
      !p.conserva_objeto(121) || !ruta({{330, 170}, {100, 198}}, 35) ||
      !caminar({{550, 140}, {450, 145}, {300, 160}, {140, 175}, {94, 165}}) || !accion({1}, 115) ||
      !avanzar(192) || !p.conserva_objeto(115))
    return false;
  if (detener("plataforma_bandeja_recogida"))
    return true;
  if (!ruta({{140, 175},
             {300, 160},
             {450, 145},
             {550, 140},
             {620, 145},
             {704, 145},
             {850, 190},
             {850, 199}},
            32))
    return false;
  if (detener("plataforma_puerta_dino"))
    return true;
  if (!ruta({{250, 170}, {340, 170}, {320, 140}, {295, 130}}, 39))
    return false;
  if (detener("plataforma_entrada"))
    return true;
  const auto puntos = p.puntuacion();
  if (!accion({2, 5}, 120) || !avanzar(256) || p.conserva_objeto(120))
    return false;
  if (detener("plataforma_detergente"))
    return true;
  if (!abrir() || !elegir(2) || !elegir(2) || !elegir(0) || !objeto(119) || !avanzar(256) ||
      p.conserva_objeto(119))
    return false;
  if (detener("plataforma_boton_descubierto"))
    return true;
  if (!accion({2, 2}, 122) || !avanzar(512))
    return false;
  if (detener("plataforma_barrera_abierta"))
    return true;
  if (detener("plataforma_caida_izquierda"))
    return ir(100, 160) && avanzar(128) && p.escena_actual() == 39 && p.movimiento_habilitado();
  if (!accion({2, 5}, 121) || !avanzar(256) || p.conserva_objeto(121))
    return false;
  if (detener("plataforma_canicas"))
    return true;
  if (!accion({2, 5}, 115) || !avanzar(1024) || p.conserva_objeto(115))
    return false;
  if (detener("plataforma_cruzada"))
    return true;
  if (detener("plataforma_caida_derecha"))
    return ir(180, 160) && avanzar(128) && p.escena_actual() == 39 && p.movimiento_habilitado();
  if (!caminar({{245, 180}, {264, 176}}) || !accion({1}, 124) || !avanzar(192) ||
      !p.conserva_objeto(124))
    return false;
  if (detener("plataforma_ventilador"))
    return true;
  if (!abrir() || !elegir(2) || !elegir(0) || !elegir(0) || !objeto(3) || !objeto(123))
    return false;
  for (unsigned int i = 0; i < 256 && p.escena_actual() == 39; ++i)
    if (!avanzar(1))
      return false;
  if (p.escena_actual() != 16 || !avanzar(128) || !p.conserva_objeto(124) || p.conserva_objeto(3) ||
      p.conserva_objeto(99) || p.puntuacion() != static_cast<unsigned short>(puntos + 30))
    return false;
  return detener("plataforma_regreso_campo") ||
         comprobar_regreso_benn_sdl(p, opcion, avanzar, abrir, elegir, objeto, ir, ruta);
}
