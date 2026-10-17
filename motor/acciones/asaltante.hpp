#pragma once
#include "menu.hpp"
namespace motor {
// 300B–30D4 y 5C44–5CAE: espada, asaltante y dinero de COUNTRY2, con los fragmentos originales.
ResultadoMenu ejecutar_interaccion_asaltante(unsigned char *datos, unsigned short manejador,
                                             unsigned short identificador,
                                             const ServicioSonido &sonido,
                                             const ServicioCreacion &creacion);
} // namespace motor
