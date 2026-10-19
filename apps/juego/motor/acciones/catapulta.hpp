#pragma once
#include "menu.hpp"
namespace motor {
// 5B60–5BDD, 5C2E–5C43, A07E–A1D3 y A383–A405: tablón, monitor, imán y alambre.
ResultadoMenu ejecutar_interaccion_catapulta(unsigned char *datos, unsigned short manejador,
                                             unsigned short identificador,
                                             const ServicioSonido &sonido = {});
} // namespace motor
