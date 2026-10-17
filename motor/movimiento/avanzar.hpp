#pragma once

namespace motor {

// 0000:096D–0999. Suma componentes de 16 bits con acarreo y reduce el contador.
// true corresponde al salto a 0A59 (quedan pasos); false a la continuación 099A.
bool avanzar_movimiento(unsigned char *datos);

} // namespace motor
