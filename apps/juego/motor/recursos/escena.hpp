#pragma once
#include "../video/imagen.hpp"

namespace motor {
// 4EE5–4F25 y 50D2–513A: lee la paleta y monta diez tiras MAP de 32 × 200.
// Conserva los últimos 64 colores de imagen.paleta, como el original. El llamador
// proporciona diez índices de tira, planos de 262.144 bytes y una tira de 6.400 bytes.
// Un fallo puede dejar parte del área de trabajo modificada: no debe presentarse.
bool preparar_fondo_mapa(const unsigned char *mapa, unsigned int longitud,
                         const unsigned char *seleccion, ImagenIndexada &imagen,
                         unsigned char *planos, unsigned char *tira);
} // namespace motor
