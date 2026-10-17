#pragma once

namespace motor {
// F29A–F30E: actualiza las 69 claves y conserva el orden de intercambios del original.
// La tabla DS:BDD2 contiene pares de clave y referencia a entidad.
void ordenar_entidades(unsigned char *datos);
} // namespace motor
