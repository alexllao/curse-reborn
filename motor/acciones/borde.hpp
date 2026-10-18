#pragma once
#include "menu.hpp"
namespace motor {
// 324C, 5CF6–5D2E y A406–A598: primeros contratos del acantilado, todavía sin activación SDL.
ResultadoMenu ejecutar_interaccion_borde(unsigned char *datos, unsigned short manejador,
                                         unsigned short identificador,
                                         const ServicioSonido &sonido);
} // namespace motor
