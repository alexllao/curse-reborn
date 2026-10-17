#pragma once

#include "../aleatorio/generador.hpp"

namespace motor {

void reiniciar_estado_accion(unsigned char *datos); // 8822–886A.
void marcar_accion_rechazada(unsigned char *datos); // 88A0–88BB; acarreo original activo.
// 886B–889F, incluidos reinicio y planificación. AX es el retorno original; CF=1.
bool iniciar_accion(unsigned char *datos, unsigned short &ax);
// Acción B740 completa hasta RET, incluyendo su salto común a 886B.
bool ejecutar_accion_resultados(unsigned char *datos, EstadoAleatorio &estado, unsigned short &ax);

} // namespace motor
