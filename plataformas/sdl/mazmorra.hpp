#pragma once

#include <string>
#include <vector>
#include "../../motor/aleatorio/generador.hpp"
#include "../../motor/video/imagen.hpp"

// Adaptador de los bancos originales y el estado de escena para las rutinas C++ reconstruidas.
class Mazmorra {
public:
  // Carga solo datos del EXE y los bancos; prepara la entrada y la lista de objetos originales.
  bool iniciar(const std::string &carpeta, const motor::ImagenIndexada &fondo);
  // Avanza las secuencias, comportamientos y lógica particular sin ejecutar instrucciones DOS.
  bool avanzar();
  // Restaura el fondo y compone todas las entidades en el orden reconstruido.
  bool dibujar(motor::ImagenIndexada &imagen);

private:
  unsigned char datos[65536]{};
  motor::ImagenIndexada fondo{};
  motor::EstadoAleatorio aleatorio{};
  std::string carpeta;
  std::vector<unsigned char> escena, personaje, modelo;

  // Lee un banco local sin asumir que sus bytes estén alineados o terminen en cero.
  bool cargar_banco(const std::string &nombre, std::vector<unsigned char> &destino);
  // Resuelve etiquetas de recurso como bancos; nunca las desreferencia como segmentos DOS.
  static const unsigned char *banco(void *contexto, unsigned short referencia,
                                    unsigned int &longitud);
  // Consulta el tamaño del fotograma para las secuencias de cualquiera de los tres bancos.
  static bool dimensiones(void *contexto, unsigned short referencia, unsigned short imagen,
                          unsigned short &ancho, unsigned short &alto);
  // Adapta las dimensiones del banco de escena al cargador y los comportamientos de entidades.
  static bool dimensiones_escena(void *contexto, unsigned short imagen, unsigned short &ancho,
                                 unsigned short &alto);
  // Proporciona la referencia de una superposición leída de la tabla original del banco.
  static bool parrafo_escena(void *contexto, unsigned short imagen, unsigned short &referencia);
  // Consume una orden únicamente bajo la modalidad original sin dispositivo de sonido.
  static bool omitir_sonido(void *, unsigned char *datos, unsigned short, unsigned char);
  // Atiende F8 leyendo el nombre acotado de la tabla original y reemplazando el banco de modelo.
  static bool cargar_modelo(void *contexto, const unsigned char *datos, unsigned short nombre,
                            unsigned short capacidad);
};
