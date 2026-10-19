#pragma once
#include "menu.hpp"
namespace motor {
// 5F56/615C y B432–B576: bola de nieve y reacción del ser azul en SNOWAST4/43.
ResultadoMenu ejecutar_interaccion_nieve(unsigned char *datos, unsigned short manejador,
                                         unsigned short identificador,
                                         const ServicioSonido &sonido);
} // namespace motor
