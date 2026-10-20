#pragma once
#include "menu.hpp"
namespace motor {
// 60CE/4502: recoge el hueso y excava el ataúd con los contratos originales.
ResultadoMenu ejecutar_interaccion_cementerio(unsigned char *datos, unsigned short manejador,
                                              unsigned short identificador,
                                              const ServicioSonido &sonido,
                                              const ServicioCreacion &creacion);
} // namespace motor
