#pragma once
#include "menu.hpp"
namespace motor {
// 5F67–62CC/B740–BD70: dados, útiles e interacciones de las salas46–62 del palacio.
ResultadoMenu ejecutar_interaccion_palacio(unsigned char *datos, unsigned short manejador,
                                           unsigned short identificador,
                                           const ServicioSonido &sonido,
                                           const ServicioCreacion &creacion);
} // namespace motor
