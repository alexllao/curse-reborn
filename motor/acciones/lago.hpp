#pragma once
#include "menu.hpp"
namespace motor {
// 2FD3–300A, 38A4–38D2, 57F1–5848 y 9994–9BB7: acciones submarinas limitadas a la escena tres.
ResultadoMenu ejecutar_interaccion_lago(unsigned char *datos, unsigned short manejador,
                                        unsigned short identificador,
                                        const ServicioSonido &sonido = {});
} // namespace motor
