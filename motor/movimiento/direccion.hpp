#pragma once

namespace motor {

// 0000:0BF8–0F5E. Índice de cuatro bits: izquierda=8, derecha=4, arriba=2, abajo=1.
// DS completo; conserva los retornos anticipados y la modalidad especial.
// Fuera de 0–15 devuelve false sin escribir; ese caso queda fuera del contrato x86.
bool seleccionar_direccion(unsigned char *datos, unsigned short indice);

} // namespace motor
