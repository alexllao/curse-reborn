#pragma once
#include "menu.hpp"
namespace motor {
// 563A–57F0 y 384A–389E: acciones propias de los objetos seis a catorce de CORRIDOR.
ResultadoMenu ejecutar_interaccion_corredor(unsigned char *datos, unsigned short manejador,
                                            unsigned short identificador,
                                            const ServicioSonido &sonido);
} // namespace motor
