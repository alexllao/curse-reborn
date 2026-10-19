#pragma once

namespace motor {
struct ObjetosCercanos {
  bool valido;
  unsigned short cantidad;
};

// 891E–89D8: lista identificadores compatibles con la acción y la proximidad a los pies.
// Escribe en DS:destino y rellena las casillas sobrantes con FFFF. Capacidad máxima: 68.
// Un desbordamiento devuelve valido=false sin escribir fuera de la capacidad, pero puede haber
// resultados parciales.
ObjetosCercanos buscar_objetos_cercanos(unsigned char *datos, unsigned char mascara,
                                        unsigned short destino, unsigned short capacidad);
} // namespace motor
