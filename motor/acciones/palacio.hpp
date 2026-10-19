#pragma once
#include "menu.hpp"
namespace motor {
// 5F67/5F8E y B740: dados, escoba y servicios comunes de las salas del palacio.
ResultadoMenu ejecutar_interaccion_palacio(unsigned char *datos, unsigned short manejador,
                                           unsigned short identificador,
                                           const ServicioSonido &sonido,
                                           const ServicioCreacion &creacion);
} // namespace motor
