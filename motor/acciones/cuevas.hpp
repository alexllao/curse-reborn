#pragma once
#include "menu.hpp"
namespace motor {
// 5849–59A3 y 9F39–A07B: piedras de los corredores y entregas al picapedrero.
ResultadoMenu ejecutar_interaccion_cuevas(unsigned char *datos, unsigned short manejador,
                                          unsigned short identificador,
                                          const ServicioSonido &sonido = {});
} // namespace motor
