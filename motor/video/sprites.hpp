#pragma once

namespace motor {
// EBE8–EC02: localiza un fotograma por su desplazamiento en párrafos dentro del banco.
// 6D8C–7274: dibuja sus cuatro flujos de carreras, con transparencia y espejo horizontal.
// Destino lineal de 320 × 200; sustituye la selección de planos y omite las listas de daños VGA.
// El índice corresponde a los siete bits bajos del campo de imagen de la entidad.
bool dibujar_fotograma(const unsigned char *banco, unsigned int longitud, unsigned int indice,
                       unsigned char *destino, int x, int y, bool invertido);

// 8B5B y EC67: una superposición conserva la referencia del fotograma en párrafos.
bool dibujar_fotograma_por_parrafo(const unsigned char *banco, unsigned int longitud,
                                   unsigned int parrafo, unsigned char *destino, int x, int y,
                                   bool invertido);
} // namespace motor
