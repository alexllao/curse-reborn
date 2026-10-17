#include "titulo.hpp"
#include "rnc.hpp"
#include "../video/planos.hpp"

namespace motor {
namespace {
void seleccionar_plano(void*, unsigned short, unsigned short, unsigned char) {}
}

bool preparar_titulo(const unsigned char* comprimido, unsigned int longitud,
    const unsigned char* paleta, unsigned int longitud_paleta, ImagenTitulo& imagen,
    unsigned char* planos) {
  if (longitud_paleta != 768) return false;
  for (unsigned int i = 0; i < 768; ++i) if (paleta[i] > 63) return false;
  const auto resultado = descomprimir_rnc1(comprimido, longitud, imagen.indices, 64000);
  if (resultado.estado != EstadoRnc::correcto || resultado.escritos != 64000) return false;
  // La selección de planos queda representada en las matrices, sin acceso al hardware.
  const PuertosVideo puertos{nullptr, nullptr, seleccionar_plano};
  if (!subir_imagen(imagen.indices, planos, puertos, 320, 200, 112, 0) ||
      !recuperar_imagen(planos, imagen.indices, puertos, 320, 200, 112, 0)) return false;
  for (unsigned int i = 0; i < 768; ++i) imagen.paleta[i] = paleta[i];
  return true;
}

}
