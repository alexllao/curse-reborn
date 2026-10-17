#pragma once
#include "menu.hpp"
namespace motor {
ResultadoMenu ejecutar_interaccion_poblacion(unsigned char *datos, unsigned short manejador,
                                             unsigned short identificador,
                                             const ServicioSonido &sonido);
}
