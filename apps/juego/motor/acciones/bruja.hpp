#pragma once
#include "estudio.hpp"

namespace motor {
// 4925–4A76/418A: conserva fases, canales no escritos y puntos sin consumir los cuatro útiles.
inline ResultadoMenu ejecutar_interaccion_bruja(unsigned char *d, unsigned short manejador,
                                                unsigned short id, const ServicioSonido &sonido) {
  const auto fase = d[0x35ca] >> 6;
  bool accion = false;
  if (manejador == 0x2fca && id == 171 && fase == 0) {
    guardar(d, 0x2c3b, 0x2b61);
    guardar(d, 0x2c40, 0x2b65);
    accion = true;
  } else if (manejador == 0x3b53 && palabra(d, 0xb638) == 0xffff && palabra(d, 0xb634) == 2 &&
             id == 164 && fase == 1) {
    d[0x35ca] ^= 0xc0;
    guardar(d, 0x2c3b, 0x2b79);
    guardar(d, 0x2c40, 0x2b85);
    guardar(d, 0x2d64, 0x1a2f);
    d[0x2d66] = 0;
    d[0x2d67] = 3;
    d[0x2d68] = 22;
    constexpr Palabra secuencias[] = {0x2b8c, 0x2b98, 0x2ba0};
    for (unsigned int i = 0; i < 3; ++i) {
      const auto p = static_cast<Palabra>(0x2d85 + i * 5);
      guardar(d, p, secuencias[i]);
      d[p + 2] = 0;
      d[p + 3] = 9;
      d[p + 4] = 2;
    }
    accion = true;
  } else if (manejador == 0x2fca && id == 124 && fase == 2) {
    guardar(d, 0x2c3b, 0x2b67);
    guardar(d, 0x2c40, 0x2b6c);
    guardar(d, 0x2d43, 0x2b6f);
    d[0x2d45] = 0;
    d[0x2d46] = 3;
    d[0x2d47] = 1;
    accion = true;
  } else if (manejador == 0x3b53 && palabra(d, 0xb634) == 7 && palabra(d, 0xb638) == 175 &&
             id == 177) {
    // 418A/4A1D: el EXE admite únicamente este orden y no exige otra comprobación de fase.
    guardar(d, 0x2c3b, 0x2ba8);
    guardar(d, 0x2c40, 0x2bc0);
    guardar(d, 0x2c45, 0x2bd2);
    constexpr Palabra secuencias[] = {0x2be7, 0x2bf4, 0x2bff};
    for (unsigned int i = 0; i < 3; ++i) {
      const auto p = static_cast<Palabra>(0x2c5c + i * 5);
      guardar(d, p, secuencias[i]);
      d[p + 2] = 0;
      d[p + 3] = 17;
      d[p + 4] = 2;
    }
    accion = true;
  }
  if (accion) {
    guardar(d, 0x35c7, static_cast<Palabra>(palabra(d, 0x35c7) + 5));
    unsigned short pasos = 0;
    return iniciar_accion(d, pasos) ? ResultadoMenu::accion : ResultadoMenu::error;
  }
  if (id == 177) {
    if (manejador == 0x384a)
      return ResultadoMenu::actualizado;
    if (manejador == 0x2fca || manejador == 0x3143 || manejador == 0x3b53) {
      marcar_accion_rechazada(d);
      return ResultadoMenu::accion;
    }
    return ResultadoMenu::pendiente;
  }
  return ejecutar_interaccion_estudio(d, manejador, id, sonido);
}
} // namespace motor
