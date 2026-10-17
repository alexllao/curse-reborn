#pragma once
#include "menu.hpp"
namespace motor {
// 3934–3AFC y 5BDE–5C2D: inspección de cuatro agujeros y recogida de ramitas.
ResultadoMenu ejecutar_interaccion_agujeros(unsigned char *datos, unsigned short manejador,
                                            unsigned short identificador,
                                            const ServicioSonido &sonido = {});
} // namespace motor
