#pragma once
#include "menu.hpp"
namespace motor {
// 9E21–9F38 y 59CC–5B5F: moneda, aparición de ofertas y elección de la recompensa del pozo.
ResultadoMenu ejecutar_interaccion_pozo(unsigned char *datos, unsigned short manejador,
                                        unsigned short identificador, const ServicioSonido &sonido,
                                        const ServicioCreacion &creacion);
} // namespace motor
