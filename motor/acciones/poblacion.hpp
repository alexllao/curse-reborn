#pragma once
#include "menu.hpp"
namespace motor {
// Primeras recogidas y respuestas comunes del valle32–35; grabación y puzles posteriores
// pendientes.
ResultadoMenu ejecutar_interaccion_poste(unsigned char *datos, unsigned short manejador,
                                         unsigned short identificador,
                                         const ServicioSonido &sonido);
ResultadoMenu ejecutar_interaccion_poblacion(unsigned char *datos, unsigned short manejador,
                                             unsigned short identificador,
                                             const ServicioSonido &sonido);
} // namespace motor
