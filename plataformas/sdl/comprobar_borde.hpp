#pragma once
#include "mazmorra.hpp"
#include <initializer_list>

// Reproduce la ruta contrastada de EDGE con flechas y menús, conservando los ciclos originales.
template <typename Avanzar, typename Abrir, typename Elegir, typename Objeto>
bool comprobar_recorrido_borde(Mazmorra &p, const std::string &opcion, Avanzar avanzar, Abrir abrir,
                               Elegir elegir, Objeto objeto) {
  // Las capturas se detienen en el mismo paso lógico que el comparador del EXE.
  const auto detener = [&](const char *nombre) {
    return opcion == std::string("--comprobar-final-poblacion-") + nombre;
  };
  // Selecciona los cuatro bits a través de la misma entrada que usa el jugador.
  const auto andar = [&](unsigned short direccion, unsigned int pasos) {
    for (const auto bit : {1, 2, 4, 8})
      p.tecla_direccion(bit, (direccion & bit) != 0);
    return avanzar(pasos);
  };
  // Recorre el menú visible antes de localizar el objeto ofrecido por el original.
  const auto accion = [&](std::initializer_list<unsigned short> casillas, unsigned short id) {
    if (!abrir())
      return false;
    for (const auto casilla : casillas)
      if (!elegir(casilla))
        return false;
    return objeto(id);
  };
  // La espera consulta ADCE y nunca modifica el temporizador ni las entidades de las rocas.
  const auto esperar = [&]() {
    for (unsigned int i = 0; i < 182; ++i) {
      if (p.ciclo_rocas_iniciado())
        return true;
      if (!andar(0, 1))
        return false;
    }
    return false;
  };
  if (p.escena_actual() != 30 || !avanzar(256))
    return false;
  for (const auto direccion : {4, 4, 4, 5, 9, 5, 4, 4, 4})
    if (!andar(direccion, 8))
      return false;
  if (!andar(4, 104) || !andar(6, 8) || !andar(4, 8) || !andar(5, 8) || !andar(0, 1) ||
      !accion({2, 2}, 92) || !avanzar(192))
    return false;
  if (detener("borde_roca_empujada"))
    return true;
  if (!accion({6}, 92) || !avanzar(192))
    return false;
  if (detener("borde_roca_saltada"))
    return true;
  if (!andar(4, 17) || !andar(0, 32) || !andar(4, 24) || !andar(6, 8) || !andar(0, 1) ||
      !accion({1}, 86) || !avanzar(128) || !p.conserva_objeto(86))
    return false;
  if (detener("borde_guantes"))
    return true;
  if (!andar(4, 8) || !andar(0, 1))
    return false;
  for (const auto id : {88, 90, 91, 90, 89})
    if (!accion({2, 2}, id) || !avanzar(192))
      return false;
  if (detener("borde_puente"))
    return true;
  if (!andar(4, 100) || !andar(0, 1) || !accion({1}, 95) || !avanzar(128) || !p.conserva_objeto(95))
    return false;
  if (detener("borde_cuerda"))
    return true;
  if (!esperar() || !andar(4, 42) || !andar(0, 1) || !accion({5}, 86) || !avanzar(48) ||
      p.conserva_objeto(86) || !p.movimiento_habilitado())
    return false;
  if (detener("borde_electrico"))
    return true;
  if (!andar(8, 8) || !esperar() || !andar(4, 56) || !andar(6, 16) || !andar(0, 1) ||
      !accion({1}, 87) || !avanzar(96) || !p.conserva_objeto(87))
    return false;
  if (detener("borde_caramelo"))
    return true;
  if (!esperar() || !andar(4, 130) || !andar(0, 1) || !andar(4, 30) || !andar(0, 1) ||
      !accion({2, 4}, 87) || !avanzar(128) || !p.conserva_objeto(94))
    return false;
  if (detener("borde_caramelo_puesto"))
    return true;
  if (!accion({2, 5}, 94) || !avanzar(192))
    return false;
  if (detener("borde_roca_grande"))
    return true;
  if (!andar(4, 100) || !andar(0, 1) || !accion({2, 5}, 95) || !avanzar(128))
    return false;
  if (detener("borde_cuerda_lanzada"))
    return true;
  if (!accion({6}, 96) || !avanzar(192))
    return false;
  if (detener("borde_abismo_saltado"))
    return true;
  for (unsigned int i = 0; i < 100; ++i) {
    unsigned short x = 0, y = 0, alto = 0;
    if (!p.posicion_objeto(0, x, y, alto))
      return false;
    if (x + p.ventana_actual() * 32 >= 1888)
      break;
    if (!andar(4, 1))
      return false;
  }
  if (!andar(0, 32) || !andar(4, 100) || !andar(0, 1) || !accion({3}, 97) || !avanzar(128))
    return false;
  if (detener("borde_inscripcion"))
    return true;
  if (!abrir() || !elegir(4) || !elegir(2) || !avanzar(128))
    return false;
  if (detener("borde_puerta_abierta"))
    return true;
  if (!andar(8, 32))
    return false;
  for (unsigned int i = 0; i < 16 && p.escena_actual() == 30; ++i)
    if (!andar(2, 1))
      return false;
  if (p.escena_actual() != 31)
    return false;
  if (detener("borde_loro_cargado"))
    return true;
  if (!avanzar(128))
    return false;
  if (detener("borde_loro_entrada"))
    return true;
  if (!abrir() || !elegir(4) || !elegir(0))
    return false;
  for (unsigned int i = 0; i < 1024 && p.escena_actual() == 31; ++i)
    if (!avanzar(1))
      return false;
  return p.escena_actual() == 27 && avanzar(256) && detener("borde_loro_regreso");
}
