#pragma once

namespace motor {
// 183F–1904: reacción particular de la escena cinco, después de actualizar las entidades.
void actualizar_escena_cinco(unsigned char *datos);
// 1791–17A2: despacha solo las escenas con rutina integrada; la escena cero no tiene manejador.
bool actualizar_rutina_escena(unsigned char *datos);
} // namespace motor
