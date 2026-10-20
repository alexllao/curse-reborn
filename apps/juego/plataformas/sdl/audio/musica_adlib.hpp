#pragma once

#include "../../../terceros/nuked_opl3/opl3.h"
#include "opl2.hpp"

// Síntesis OPL de las órdenes reconstruidas, sin ejecutar instrucciones del EXE.
class MusicaAdlib {
public:
  // Lee el segmento CS del MZ como datos e inicia el tema uno, elegido por 0487–048A.
  bool iniciar(const unsigned char *ejecutable, unsigned int longitud, unsigned short tema = 1);
  // Produce OPL3 y, si se solicita, OPL2 mono de 48 kHz; ambos conservan el mismo reloj PIT.
  bool generar(short *salida, unsigned int cantidad, short *salida_opl2 = nullptr);
  // Permite comprobar que los saltos de vista y cambios de habitación conservan la música.
  unsigned long long pulsos() const {
    return interrupciones;
  }

private:
  unsigned char codigo[65536]{};
  opl3_chip chip{};
  SintesisOpl2 opl2;
  unsigned int fraccion = 0, hasta_interrupcion = 0x4250;
  unsigned long long interrupciones = 0;
  bool preparada = false;
  // Conserva el orden de escrituras, usando la cola temporal del emulador OPL.
  static void escribir(void *contexto, unsigned short registro, unsigned char valor);
};
