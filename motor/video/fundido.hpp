#pragma once

namespace motor {
// 77E4–7879: obtiene un paso del fundido de salida con acumuladores de ocho bits fraccionarios.
// El primer paso visible es uno; paso cero conserva la paleta. No inventa un fundido de entrada.
bool calcular_fundido(const unsigned char *paleta, unsigned short colores, unsigned short pasos,
                      unsigned short paso, unsigned char *salida);
} // namespace motor
