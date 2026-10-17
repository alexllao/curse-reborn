#pragma once

#include "../aleatorio/generador.hpp"

namespace motor {

// La carga de recursos aporta dimensiones de un fotograma, sin exponer segmentos DOS.
struct RecursosEntidades {
  void* contexto;
  bool (*dimensiones)(void* contexto, unsigned short indice,
                      unsigned short& ancho, unsigned short& alto);
  unsigned short segmento_original;
};

// 8B6F–8C25. valido=false identifica recursos o entradas fuera del dominio definido.
struct CreacionEntidad { bool valido; bool creada; unsigned short posicion; };
CreacionEntidad crear_entidad(unsigned char* datos, EstadoAleatorio& estado,
                              const RecursosEntidades& recursos, unsigned short descriptor);

// 1119–1785: actualiza un registro, sin avanzar al siguiente ni despachar una escena.
bool actualizar_entidad(unsigned char* datos, EstadoAleatorio& estado,
                       const RecursosEntidades& recursos, unsigned short posicion);

// 10EC–1790: prepara límites y recorre 68 registros; se detiene antes de 1791.
bool actualizar_entidades(unsigned char* datos, EstadoAleatorio& estado,
                         const RecursosEntidades& recursos);

// C4DC–C5DF. Transición concreta activada por el comportamiento 0411h.
void activar_transicion_entidad(unsigned char* datos);

}
