#pragma once

#include <string>
#include <SDL3/SDL.h>
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
  bool dibujar(motor::ImagenIndexada &imagen, unsigned int paso_menu = 0);

  // Abre/cierra el menú original y procesa sus casillas desde entrada SDL en coordenadas lógicas.
  bool entrada_menu(unsigned short x, unsigned short y, bool seleccionar, bool alternar);
  // El clic izquierdo propone un destino; el control y las paredes originales deciden el avance.
  bool mover_a(unsigned short x, unsigned short y);
  // Conserva las flechas pulsadas con los cuatro bits originales de dirección.
  void tecla_direccion(unsigned short bit, bool pulsada);
  // Consulta si las animaciones deben permanecer detenidas mientras se escoge una acción.
  bool menu_abierto() const {
    return menu_visible;
  }
  // Dibuja rectángulos de diagnóstico sin escribir en el estado ni en las imágenes originales.
  bool depurar(SDL_Renderer *dibujante, bool objetos, bool interacciones, bool limites) const;

private:
  unsigned char datos[65536]{};
  motor::ImagenIndexada fondo{};
  motor::EstadoAleatorio aleatorio{};
  std::string carpeta;
  std::vector<unsigned char> escena, personaje, modelo, iconos, raton, geometria;
  unsigned char fondo_menu[10240]{}, panel_menu[64000]{};
  bool menu_visible = false;
  unsigned short direccion = 0xffff;
  unsigned short casilla_menu = 0;

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
