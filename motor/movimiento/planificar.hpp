#pragma once

namespace motor {

// 0000:0F5F–10EB, incluidas las tablas de dirección. nodo es una dirección en DS.
// pasos devuelve el AX original. false indica una división no representable en x86.
// En ese caso pueden existir escrituras anteriores al punto de la excepción original.
bool planificar_movimiento(unsigned char* datos, unsigned short nodo, unsigned short& pasos);

}
