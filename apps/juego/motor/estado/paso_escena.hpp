#pragma once
#include "../movimiento/geometria.hpp"
#include "../entidades/secuencias.hpp"
#include "../entidades/actualizar.hpp"
namespace motor {
// 06E8 y 083D–0BC1: aplica entrada, movimiento, guiones, colisiones, historial y rutina de escena.
// direccion=FFFF conserva el control por nodos. Acciones distintas de cero requieren servicio de
// plataforma.
Colision avanzar_escena(unsigned char *datos, EstadoAleatorio &azar,
                        const RecursosSecuencias &secuencias, const RecursosEntidades &entidades,
                        const unsigned char *geometria, unsigned int longitud,
                        unsigned short direccion = 0xffff);
} // namespace motor
