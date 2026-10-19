#pragma once
#include "mazmorra.hpp"
#include "comprobar_plataforma_valle.hpp"
#include <initializer_list>
#include <utility>

// A952/A982/5DDC/AC8F: abre el barco y monta el puente con los dos tablones originales.
template <typename Avanzar, typename Abrir, typename Elegir, typename Objeto, typename Ir,
          typename Ruta>
bool comprobar_barco_valle_sdl(Mazmorra &p, const std::string &opcion, Avanzar avanzar, Abrir abrir,
                               Elegir elegir, Objeto objeto, Ir ir, Ruta ruta) {
  // Conserva los hitos originales de guardia-barco y puente-barco.
  const auto detener = [&](const char *nombre) {
    return opcion == std::string("--comprobar-final-poblacion-borde_valle_") + nombre;
  };
  // Recorre las mismas posiciones globales y tiempos del comparador nativo.
  const auto caminar = [&](std::initializer_list<std::pair<int, unsigned short>> puntos) {
    for (const auto &punto : puntos)
      if (!ir(punto.first, punto.second))
        return false;
    return true;
  };
  // Usa los menús originales, incluido el inventario que precede a Empujar un tablón.
  const auto accion = [&](std::initializer_list<unsigned short> casillas, unsigned short id) {
    if (!abrir())
      return false;
    for (const auto casilla : casillas)
      if (!elegir(casilla))
        return false;
    return objeto(id);
  };
  if (p.escena_actual() != 32 || !p.conserva_objeto(120) || !p.conserva_objeto(121) ||
      !caminar({{250, 170}, {400, 170}, {450, 115}, {500, 115}}) || !accion({1}, 100) ||
      !avanzar(192) || !p.conserva_objeto(100) || !ruta({{550, 150}, {620, 120}, {639, 110}}, 33) ||
      !caminar({{330, 170}, {500, 180}, {552, 163}}) || !accion({2, 7}, 100) || !objeto(107) ||
      !avanzar(256) || !p.conserva_objeto(104) || p.conserva_objeto(100) ||
      !caminar({{500, 180}, {360, 170}}) || !accion({5}, 104) || !avanzar(768) ||
      p.conserva_objeto(104) || !ruta({{340, 140}}, 36))
    return false;
  if (detener("barco_interior_entrada"))
    return true;
  if (!caminar({{225, 185}, {160, 185}, {85, 180}, {70, 173}}) || !accion({1}, 117) ||
      !avanzar(192) || !p.conserva_objeto(117))
    return false;
  if (detener("barco_primer_tablon"))
    return true;
  if (!caminar({{100, 180}, {170, 180}, {136, 166}}) || !abrir() || !elegir(2) || !elegir(2) ||
      !elegir(0) || !objeto(117) || !avanzar(256) || p.conserva_objeto(117))
    return false;
  if (detener("barco_tablon_horizontal"))
    return true;
  if (!caminar({{170, 180}, {95, 180}, {91, 171}}) || !accion({1}, 118) || !avanzar(192) ||
      !p.conserva_objeto(118) || !caminar({{170, 180}, {175, 166}, {163, 166}}) || !abrir() ||
      !elegir(2) || !elegir(2) || !elegir(0) || !objeto(118) || !avanzar(256) ||
      p.conserva_objeto(118))
    return false;
  if (detener("barco_puente_completo"))
    return true;
  if (!caminar({{170, 165}, {170, 150}, {170, 120}, {230, 105}, {281, 102}}) || !accion({1}, 119) ||
      !avanzar(192) || !p.conserva_objeto(119))
    return false;
  if (detener("barco_trapo_recogido"))
    return true;
  if (!ruta({{170, 120}, {170, 175}, {280, 199}}, 33))
    return false;
  return detener("barco_regreso_celeste") ||
         comprobar_plataforma_valle_sdl(p, opcion, avanzar, abrir, elegir, objeto, ir, ruta);
}
