#pragma once
#include "menu.hpp"
namespace motor {
// B1F8–B576/5EAB–5F64: inventario, perfume, hielo, conversaciones y rescate de40–43.
ResultadoMenu ejecutar_interaccion_nieve(unsigned char *datos, unsigned short manejador,
                                         unsigned short identificador,
                                         const ServicioSonido &sonido);
} // namespace motor
