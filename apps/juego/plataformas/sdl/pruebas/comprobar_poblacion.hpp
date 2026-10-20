#pragma once
#include "../mazmorra.hpp"
#include "comprobar_borde.hpp"
#include "../../../motor/estado/partidas.hpp"
#include <memory>
#include <cstdio>

// Contrasta guardar y cargar los nuevos índices sin tocar las partidas del usuario.
inline bool comprobar_partida_tienda(Mazmorra &partida, const std::string &carpeta) {
  std::vector<unsigned char> archivo(motor::bytes_archivo_partidas, 0);
  auto imagenes = std::make_unique<motor::ImagenIndexada[]>(2);
  if (!partida.dibujar(imagenes[0]) ||
      !partida.guardar_partida(archivo.data(), static_cast<unsigned int>(archivo.size()), 0,
                               "TIENDA"))
    return false;
  auto restaurada = std::make_unique<Mazmorra>();
  // Los 64 colores comunes proceden de la presentación, igual que al iniciar una sesión real.
  if (!restaurada->iniciar_vista(carpeta, imagenes[0], 0) ||
      !restaurada->cargar_partida(archivo.data(), static_cast<unsigned int>(archivo.size()), 0) ||
      restaurada->escena_actual() != partida.escena_actual() ||
      restaurada->ventana_actual() != partida.ventana_actual() || !restaurada->dibujar(imagenes[1]))
    return false;
  for (unsigned int i = 0; i < 64000; ++i)
    if (imagenes[0].indices[i] != imagenes[1].indices[i]) {
      std::fprintf(stderr, "Restauración de escena %u: píxel %u distinto (%u/%u).\n",
                   partida.escena_actual(), i, imagenes[0].indices[i], imagenes[1].indices[i]);
      return false;
    }
  for (unsigned int i = 0; i < 768; ++i)
    if (imagenes[0].paleta[i] != imagenes[1].paleta[i]) {
      std::fprintf(stderr, "Restauración de escena %u: color %u distinto (%u/%u).\n",
                   partida.escena_actual(), i, imagenes[0].paleta[i], imagenes[1].paleta[i]);
      return false;
    }
  return true;
}

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
  // Los umbrales del borde permiten clics en todo el ancho, sin exigir cámara adicional.
  const auto ir = [&](int global, unsigned short y) {
    for (unsigned int paso = 0; paso < 1500; ++paso) {
      const int local = global - partida.ventana_actual() * 32;
      if (local >= 0 && local <= 319) {
        if (!partida.mover_a(static_cast<unsigned short>(local), y))
          return false;
        const auto anterior = partida.escena_actual();
        for (unsigned int i = 0; i < 192 && partida.escena_actual() == anterior; ++i)
          if (!avanzar(1))
            return false;
        return true;
      }
      const unsigned short bit = local < 0 ? 8 : 4;
      partida.tecla_direccion(bit, true);
      const bool correcto = avanzar(1);
      partida.tecla_direccion(bit, false);
      if (!correcto)
        return false;
    }
    return false;
  };
  // Las tiendas se alcanzan desde la calle real y regresan conservando la misma partida.
  const auto visitar = [&](unsigned short interior, unsigned short salida,
                           std::initializer_list<std::pair<int, unsigned short>> puntos,
                           std::initializer_list<std::pair<int, unsigned short>> vuelta,
                           unsigned short exterior) {
    for (const auto &p : puntos) {
      if (!ir(p.first, p.second))
        return false;
      if (partida.escena_actual() == interior)
        break;
    }
    if (partida.escena_actual() != interior || !avanzar(128))
      return false;
    if (detener(("tienda_" + std::to_string(salida)).c_str()))
      return true;
    if (interior == 24 && (detener("probador_entrada") || detener("probador_regreso"))) {
      for (const auto &p : {std::pair<int, unsigned short>{140, 160}, {100, 155}, {77, 135}}) {
        if (!ir(p.first, p.second))
          return false;
        if (partida.escena_actual() == 25)
          break;
      }
      if (partida.escena_actual() != 25 || !avanzar(128))
        return false;
      if (detener("probador_entrada"))
        return true;
      if (!ir(160, 160) || !ir(200, 145) || partida.escena_actual() != 24)
        return false;
      return avanzar(128);
    }
    if (interior == 26) {
      if (!ir(120, 178) || !ir(200, 178) || !abrir() || !elegir(4) || !elegir(0) || !avanzar(96))
        return false;
      if (detener("adivina_saludo"))
        return true;
      if (!abrir() || !elegir(2) || !elegir(6) || !objeto(76) || !objeto(81) || !avanzar(192))
        return false;
      if (detener("adivina_respuesta"))
        return true;
    }
    for (const auto &p : vuelta) {
      if (!ir(p.first, p.second))
        return false;
      if (partida.escena_actual() == exterior)
        break;
    }
    return partida.escena_actual() == exterior && avanzar(128);
  };
  // Cada captura se detiene en el mismo paso persistente que el comparador del EXE.
  const auto rama = [&](unsigned short destino,
                        std::initializer_list<std::pair<int, unsigned short>> puntos,
                        const char *nombre,
                        std::initializer_list<std::pair<int, unsigned short>> vuelta,
                        unsigned short origen) {
    const auto anterior = partida.escena_actual();
    for (const auto &p : puntos) {
      if (!ir(p.first, p.second))
        return false;
      if (partida.escena_actual() != anterior)
        break;
    }
    if (partida.escena_actual() == anterior && !ir(puntos.end()[-1].first, puntos.end()[-1].second))
      return false;
    if (partida.escena_actual() != destino || !avanzar(128))
      return false;
    if (detener(nombre))
      return true;
    if (destino == 19 && (detener("tienda_35") || detener("regreso_tienda_35") ||
                          detener("probador_entrada") || detener("probador_regreso")))
      return visitar(24, 35, {{300, 170}, {380, 150}, {380, 120}}, {{200, 155}, {210, 145}}, 19);
    if (destino == 20 && (detener("tienda_38") || detener("regreso_tienda_38") ||
                          detener("adivina_saludo") || detener("adivina_respuesta")))
      return visitar(26, 38, {{420, 170}, {300, 170}, {115, 150}, {108, 127}},
                     {{120, 170}, {85, 145}}, 20);
    if (destino == 20 && (detener("tienda_39") || detener("regreso_tienda_39")))
      return visitar(27, 39, {{420, 170}, {300, 155}, {270, 120}}, {{160, 175}}, 20);
    if (destino == 28) {
      if (!partida.mover_a(90, 165) || !avanzar(8))
        return false;
      if (detener("posadero"))
        return true;
      for (unsigned int i = 0; i < 192 && partida.escena_actual() == destino; ++i)
        if (!avanzar(1))
          return false;
      return partida.escena_actual() == origen && avanzar(128);
    }
    for (const auto &p : vuelta) {
      if (!ir(p.first, p.second))
        return false;
      if (partida.escena_actual() != destino)
        break;
    }
    return partida.escena_actual() == origen && avanzar(128);
  };
  if (detener("entrada"))
    return true;
  if (!avanzar(160))
    return false;
  if (detener("guardias"))
    return true;
  if (detener("calle_41") || detener("regreso_41") || detener("posadero"))
    return rama(28, {{320, 185}, {192, 175}, {192, 145}}, "calle_41", {{30, 195}, {20, 199}}, 21);
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
  if (detener("calle_29") || detener("regreso_29") || detener("tienda_35") ||
      detener("regreso_tienda_35") || detener("probador_entrada") || detener("probador_regreso"))
    return rama(19, {{260, 180}, {310, 100}, {319, 70}}, "calle_29", {{160, 195}, {160, 199}}, 17);
  if (detener("calle_30") || detener("regreso_30") || detener("tienda_38") ||
      detener("regreso_tienda_38") || detener("tienda_39") || detener("regreso_tienda_39") ||
      detener("adivina_saludo") || detener("adivina_respuesta"))
    return rama(20, {{60, 180}, {8, 100}, {0, 70}}, "calle_30", {{470, 185}, {479, 180}}, 17);
  if (!caminar(220, 180) || !caminar(235, 120) || !caminar(225, 70) || !caminar(170, 50) ||
      !caminar(170, 5) || partida.escena_actual() != 18 || !avanzar(128))
    return false;
  if (detener("maravillas"))
    return true;
  if (detener("calle_33") || detener("regreso_33"))
    return rama(23, {{275, 180}, {285, 145}, {284, 136}}, "calle_33", {{104, 148}}, 18);
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
  if (partida.escena_actual() != 30)
    return false;
  return opcion.rfind("--comprobar-final-poblacion-borde_", 0) != 0 ||
         comprobar_recorrido_borde(partida, opcion, avanzar, abrir, elegir, objeto);
}
