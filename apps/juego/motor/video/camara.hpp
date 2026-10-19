#pragma once

namespace motor {
// EAFE–EB1D, EC8B–EC94 y EF80–EF85: -1 izquierda, 1 derecha o cero sin desplazamiento.
int decidir_camara(const unsigned char *datos);
// EC99/ED41–ED92 y EF8A/F05C–F0AD: traslada 32 píxeles, destinos y coordenadas de escena.
// Requiere dirección -1/1 disponible y hasta diez superposiciones; no altera inventario ni guiones.
bool desplazar_camara(unsigned char *datos, int direccion);
// 4E98–4EC7: dispone la tabla de tiras y acota la ventana de las escenas 0–4 y ocho (POOL).
bool preparar_vista_escena(unsigned char *datos);
// EC8A–EF79 y EF7F–F294: ventana de 320 píxeles sobre las páginas antes/después del salto de 32.
// Ocho retrazos desplazan cuatro píxeles cada uno; entradas y salida contienen 64.000 índices.
bool componer_camara(const unsigned char *anterior, const unsigned char *destino, int direccion,
                     unsigned int paso, unsigned char *salida);
} // namespace motor
