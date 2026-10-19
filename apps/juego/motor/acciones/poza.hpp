#pragma once
#include "menu.hpp"
namespace motor {
// Escena ocho: 59B5, 61A7 y 9DE4 reconstruyen algas y botón; no despacha puzles de otras cuevas.
ResultadoMenu ejecutar_interaccion_poza(unsigned char *datos, unsigned short manejador,
                                        unsigned short identificador,
                                        const ServicioSonido &sonido = {});
} // namespace motor
