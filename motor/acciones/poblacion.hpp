#pragma once
#include "menu.hpp"
namespace motor {
// Respuestas comunes comprobadas de POST35; las acciones de objetos posteriores quedan pendientes.
ResultadoMenu ejecutar_interaccion_poste(unsigned char *datos, unsigned short manejador,
                                         unsigned short identificador,
                                         const ServicioSonido &sonido);
ResultadoMenu ejecutar_interaccion_poblacion(unsigned char *datos, unsigned short manejador,
                                             unsigned short identificador,
                                             const ServicioSonido &sonido);
} // namespace motor
