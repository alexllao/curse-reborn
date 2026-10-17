#pragma once

namespace motor {
struct RecursosDibujo {
  void *contexto;
  const unsigned char *(*banco)(void *, unsigned short referencia, unsigned int &longitud);
  unsigned short escena;
};

// EBBD–EC89: ordena y compone las 69 entidades y las superposiciones que las recortan.
// El llamador restaura primero el fondo en la imagen lineal de 320 × 200.
bool dibujar_entidades(unsigned char *datos, const RecursosDibujo &recursos, unsigned char *imagen);
} // namespace motor
