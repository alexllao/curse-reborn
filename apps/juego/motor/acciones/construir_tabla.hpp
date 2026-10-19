#pragma once

namespace motor {

// Traslado de 0000:DF15–DF4D. Reproduce las escrituras sobre el segmento DS.
// datos debe apuntar a un segmento completo de 65.536 bytes modificables.
// No reproduce registros, indicadores ni el dibujo posterior a 0000:DF4E.
void construir_tabla_acciones(unsigned char *datos);

} // namespace motor
