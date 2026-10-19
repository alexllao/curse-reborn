#pragma once
#include "menu.hpp"
namespace motor {
// B579–B68A: salto del lago y cenizas que llaman a su criatura; solo escena44.
ResultadoMenu ejecutar_interaccion_hielo(unsigned char *datos, unsigned short manejador,
                                         unsigned short identificador,
                                         const ServicioSonido &sonido);
} // namespace motor
