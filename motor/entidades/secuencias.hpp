#pragma once

#include "../aleatorio/generador.hpp"

namespace motor {

struct RecursosSecuencias {
  void* contexto;
  bool (*dimensiones)(void*, unsigned short referencia, unsigned short imagen,
                      unsigned short& ancho, unsigned short& alto);
  unsigned short personaje;
  unsigned short escena;
  unsigned short modelo;
  // Devuelven true solo cuando el servicio ha terminado; nullptr conserva la parada pendiente.
  bool (*sonido)(void*, unsigned char* datos, unsigned short entidad, unsigned char identificador) = nullptr;
  bool (*cargar_modelo)(void*, const unsigned char* datos, unsigned short nombre,
                        unsigned short capacidad) = nullptr;
};

enum class EstadoSecuencias { correcto, entrada_invalida, sonido_pendiente, recurso_pendiente };

// 83EA–87C6: tres canales de un registro; no incluye la selección del siguiente.
EstadoSecuencias avanzar_secuencias_entidad(unsigned char* datos, EstadoAleatorio& estado,
    const RecursosSecuencias& recursos, unsigned short posicion);

// 87E4–881A: conserva el peculiar orden de comparaciones y limpia las tres referencias.
void retirar_entidad_fuera_de_limites(unsigned char* datos, unsigned short posicion);

// 83DE–8821 con servicios explícitos, sin acceso directo a DOS. Puede devolver una operación pendiente.
EstadoSecuencias avanzar_secuencias(unsigned char* datos, EstadoAleatorio& estado,
    const RecursosSecuencias& recursos);

}
