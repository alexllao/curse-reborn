#pragma once

#include "../aleatorio/generador.hpp"

namespace motor {

// Traslado integrado de 0000:B740–B939, con la auxiliar 0000:88F3–891D.
// datos representa un DS completo; estado corresponde a las cuatro palabras de CS.
// Se detiene antes del salto común de B93A: no ejecuta esa animación ni sus retornos.
void preparar_resultados(unsigned char *datos, EstadoAleatorio &estado);

} // namespace motor
