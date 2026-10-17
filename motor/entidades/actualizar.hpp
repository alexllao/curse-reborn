#pragma once

#include "../aleatorio/generador.hpp"
#include "../sonido/servicio.hpp"

namespace motor {

// La carga de recursos aporta dimensiones de un fotograma, sin exponer segmentos DOS.
struct RecursosEntidades {
  void *contexto;
  bool (*dimensiones)(void *contexto, unsigned short indice, unsigned short &ancho,
                      unsigned short &alto);
  unsigned short segmento_original;
  // Solo la carga de superposiciones necesita la referencia de párrafos del fotograma.
  bool (*parrafo)(void *contexto, unsigned short indice, unsigned short &referencia) = nullptr;
  ServicioSonido sonido{};
};

// 8B6F–8C25. valido=false identifica recursos o entradas fuera del dominio definido.
struct CreacionEntidad {
  bool valido;
  bool creada;
  unsigned short posicion;
};
CreacionEntidad crear_entidad(unsigned char *datos, EstadoAleatorio &estado,
                              const RecursosEntidades &recursos, unsigned short descriptor);

// 8AA6–8B6E: limpia las 68 entidades de escena y carga su lista de descriptores.
// Conserva cursor y personaje. Admite hasta 68 entidades y diez superposiciones de fondo.
bool cargar_entidades_escena(unsigned char *datos, EstadoAleatorio &estado,
                             const RecursosEntidades &recursos);

// 1119–1785: actualiza un registro, sin avanzar al siguiente ni despachar una escena.
bool actualizar_entidad(unsigned char *datos, EstadoAleatorio &estado,
                        const RecursosEntidades &recursos, unsigned short posicion);

// 10EC–1790: prepara límites y recorre 68 registros; se detiene antes de 1791.
bool actualizar_entidades(unsigned char *datos, EstadoAleatorio &estado,
                          const RecursosEntidades &recursos);

// C4DC–C5DF. Transición concreta activada por el comportamiento 0411h.
void activar_transicion_entidad(unsigned char *datos);

} // namespace motor
