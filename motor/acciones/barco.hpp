#pragma once
#include "menu.hpp"
namespace motor {
// 5DDC–5E54/AC8F–AFBB: tablones, colocación del puente y trapo de HOLD36.
ResultadoMenu ejecutar_interaccion_barco(unsigned char *datos, unsigned short manejador,
                                         unsigned short identificador,
                                         const ServicioSonido &sonido);
// AFBE–B1F5/5E9A: limpieza, barrera, cruce y salida de ELECTRO39 con servicios comunes.
ResultadoMenu ejecutar_interaccion_electro(unsigned char *datos, unsigned short manejador,
                                           unsigned short identificador,
                                           const ServicioSonido &sonido);
} // namespace motor
