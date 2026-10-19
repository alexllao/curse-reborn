#include "escena.hpp"
#include "rnc.hpp"
#include "../video/planos.hpp"

namespace motor {
namespace {
// La matriz de planos representa la selección VGA sin escribir en puertos físicos.
void seleccionar_plano(void *, unsigned short, unsigned short, unsigned char) {}
} // namespace

// Reconstruye el fondo visible: índices de la tabla original, párrafos DOS y subida por planos.
bool preparar_fondo_mapa(const unsigned char *mapa, unsigned int longitud,
                         const unsigned char *seleccion, ImagenIndexada &imagen,
                         unsigned char *planos, unsigned char *tira) {
  if (longitud < 580)
    return false;
  for (unsigned int i = 0; i < 576; ++i)
    if (mapa[i] > 63)
      return false;
  const PuertosVideo puertos{nullptr, nullptr, seleccionar_plano};
  for (int columna = 9; columna >= 0; --columna) {
    const auto indice = seleccion[columna];
    if (indice == 0xff) {
      for (unsigned int i = 0; i < 6400; ++i)
        tira[i] = 0;
    } else {
      // 50F7 usa CBW: los índices admitidos por esta ruta son 0–127, además de FF.
      if (indice >= 128)
        return false;
      const unsigned int entrada = 578u + indice * 2u;
      if (entrada + 2 > longitud)
        return false;
      const unsigned int posicion = 578u + (static_cast<unsigned int>(mapa[entrada]) |
                                            (static_cast<unsigned int>(mapa[entrada + 1]) << 8)) *
                                               16u;
      if (posicion > longitud)
        return false;
      const auto resultado = descomprimir_rnc2(mapa + posicion, longitud - posicion, tira, 6400);
      if (resultado.estado != EstadoRnc::correcto || resultado.escritos != 6400)
        return false;
    }
    if (!subir_imagen(tira, planos, puertos, 32, 200, 80, static_cast<unsigned short>(columna * 8)))
      return false;
  }
  if (!recuperar_imagen(planos, imagen.indices, puertos, 320, 200, 80, 0))
    return false;
  for (unsigned int i = 0; i < 576; ++i)
    imagen.paleta[i] = mapa[i];
  return true;
}
} // namespace motor
