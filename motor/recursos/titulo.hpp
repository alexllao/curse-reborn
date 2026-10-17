#pragma once

namespace motor {

struct ImagenTitulo {
  unsigned char indices[64000];
  unsigned char paleta[768];
};

// Recorrido gráfico de 0584–0612, sin apertura DOS, espera de entrada ni selección de página.
// El llamador aporta los archivos y cuatro planos contiguos de 65.536 bytes de trabajo.
bool preparar_titulo(const unsigned char* comprimido, unsigned int longitud,
    const unsigned char* paleta, unsigned int longitud_paleta, ImagenTitulo& imagen,
    unsigned char* planos);

}
