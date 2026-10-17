#pragma once

namespace motor {

// Traslado de 0000:B7D1–B8A2; el original recibe los valores en DL, DH y AL.
// Compara las escrituras en un segmento DS completo. Los resultados válidos son 0–5.
// Devuelve falso sin escribir cuando recibe valores fuera del dominio comprobado.
bool seleccionar_coincidencias(unsigned char *datos, unsigned char primero, unsigned char segundo,
                               unsigned char tercero);

} // namespace motor
