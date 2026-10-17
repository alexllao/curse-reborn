#pragma once
#include "../aleatorio/generador.hpp"
namespace motor {
struct Colision {
  bool valida;
  unsigned short acciones;
  unsigned char parametro;
};
// 504F–50CC: selecciona el bloque BOXDET; devuelve posición y longitud sin copiar datos ajenos a
// él.
bool localizar_geometria(const unsigned char *archivo, unsigned int longitud, unsigned short escena,
                         unsigned int &inicio, unsigned int &cantidad);
// 89DA–8AA5: construye nodos en DS:9978 desde las cajas y rutas originales, o el destino directo.
bool preparar_camino(unsigned char *datos, const unsigned char *geometria, unsigned int longitud);
// 254A–28B1: consulta rectángulos y condiciones originales; conserva los efectos en DS y RNG.
Colision comprobar_colision(unsigned char *datos, EstadoAleatorio &azar,
                            const unsigned char *geometria, unsigned int longitud,
                            unsigned short izquierda, unsigned short arriba, unsigned short derecha,
                            unsigned short abajo);
// 0A6E–0BA0: rectifica el paso contra BOXDET; devuelve transiciones especiales para la plataforma.
Colision resolver_colision_personaje(unsigned char *datos, EstadoAleatorio &azar,
                                     const unsigned char *geometria, unsigned int longitud);
// 0BA6–0BC0: conserva diez posiciones horizontales que consumen las entidades del juego.
void actualizar_historial_personaje(unsigned char *datos);
} // namespace motor
