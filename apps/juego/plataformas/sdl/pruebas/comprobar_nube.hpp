#pragma once
#include "../mazmorra.hpp"
#include "comprobar_cueva_valle.hpp"
#include <initializer_list>
#include <utility>

// AB60/ABD5/AAA3/A8DF/AC5E/C8F9: une grabación, correo, pase y bolsa sin preparar inventarios.
template <typename Avanzar, typename Abrir, typename Elegir, typename Objeto, typename Ir,
          typename Ruta>
bool comprobar_nube(Mazmorra &p, const std::string &opcion, Avanzar avanzar, Abrir abrir,
                    Elegir elegir, Objeto objeto, Ir ir, Ruta ruta) {
  // Conserva los hitos del comparador original, con un prefijo propio de la continuidad del valle.
  const auto detener = [&](const char *nombre) {
    return opcion == std::string("--comprobar-final-poblacion-borde_valle_") + nombre;
  };
  // Recorre las casillas reales y localiza el objeto en la lista ofrecida.
  const auto accion = [&](std::initializer_list<unsigned short> casillas, unsigned short id) {
    if (!abrir())
      return false;
    for (const auto casilla : casillas)
      if (!elegir(casilla))
        return false;
    return objeto(id);
  };
  // Mantiene los clics globales y el número de pasos de los recorridos nativos.
  const auto caminar = [&](std::initializer_list<std::pair<int, unsigned short>> puntos) {
    for (const auto &punto : puntos)
      if (!ir(punto.first, punto.second))
        return false;
    return true;
  };
  // Espera la petición del guion; la plataforma realiza su fundido antes de avanzar el destino.
  const auto automatico = [&](unsigned short destino, unsigned int limite, unsigned int entrada) {
    for (unsigned int i = 0; i < limite && p.escena_actual() != destino; ++i)
      if (!avanzar(1))
        return false;
    return p.escena_actual() == destino && avanzar(entrada);
  };
  // Las flechas seleccionan el control original de LOST y su balanceo firmado.
  const auto andar = [&](unsigned short direccion, unsigned int pasos) {
    for (const auto bit : {1, 2, 4, 8})
      p.tecla_direccion(bit, (direccion & bit) != 0);
    return avanzar(pasos);
  };
  if (p.escena_actual() != 32 || !p.conserva_objeto(109) || !p.conserva_objeto(101) ||
      !ruta({{400, 185}, {550, 189}}, 34) || !avanzar(128) || !ir(110, 165) || !accion({1}, 108) ||
      !avanzar(128) || !p.conserva_objeto(108) || !ruta({{170, 122}}, 33) ||
      !caminar({{330, 170}, {90, 170}, {74, 145}}) || !accion({1}, 105) || !avanzar(128) ||
      !p.conserva_objeto(105) || !ruta({{100, 198}}, 35) ||
      !caminar({{550, 140}, {620, 145}, {704, 145}, {704, 185}, {760, 185}, {760, 171}}) ||
      !accion({2, 1}, 105) || !objeto(113) || !avanzar(256) || p.conserva_objeto(105) || !abrir() ||
      !elegir(2) || !elegir(2) || !elegir(0) || !objeto(108) || !avanzar(768) ||
      !p.conserva_objeto(106) || p.conserva_objeto(108) ||
      !caminar({{704, 185}, {704, 145}, {620, 145}, {550, 140}, {484, 104}}) || !accion({1}, 111) ||
      !avanzar(128) || !accion({2, 7}, 109) || !elegir(0) || !objeto(111) || !avanzar(128) ||
      !p.conserva_objeto(112) || !caminar({{450, 175}, {435, 175}}) || !accion({2, 1}, 112) ||
      !objeto(114) || !avanzar(256) || p.conserva_objeto(112) ||
      !ruta({{470, 145}, {550, 140}, {620, 145}, {704, 145}, {850, 190}, {850, 199}}, 32) ||
      !ir(150, 165) || !accion({2, 6}, 106) || !objeto(98) || !avanzar(512) ||
      !p.conserva_objeto(99) || p.conserva_objeto(106) || !ruta({{250, 199}}, 35) ||
      !caminar({{850, 145}, {925, 140}}) || !accion({2, 1}, 99) || !objeto(116) ||
      !automatico(38, 256, 128))
    return false;
  if (detener("nube_entrada"))
    return true;
  for (const unsigned short direccion : {8, 4, 8, 4})
    if (!andar(direccion, 64))
      return false;
  if (!andar(0, 32))
    return false;
  if (detener("nube_impulso_alternado"))
    return true;
  if (!andar(8, 32) || !andar(4, 16) || !andar(0, 1) || !accion({1}, 121) || !avanzar(96) ||
      !p.conserva_objeto(121))
    return false;
  if (detener("nube_bolsa_recogida"))
    return true;
  if (!andar(1, 0) || !automatico(35, 96, 256) || !p.conserva_objeto(121))
    return false;
  return detener("nube_regreso_post") ||
         comprobar_cueva_valle_sdl(p, opcion, avanzar, abrir, elegir, objeto, ir, ruta);
}
