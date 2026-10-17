#include "entidades.hpp"
#include "orden.hpp"
#include "sprites.hpp"
#include "../estado/segmento.hpp"

namespace motor {
// Reproduce EBBD: conserva el orden original y deja que las superposiciones oculten lo que queda
// detrás.
bool dibujar_entidades(unsigned char *datos, const RecursosDibujo &recursos,
                       unsigned char *imagen) {
  if (!recursos.banco || palabra(datos, 0x358a) > 10)
    return false;
  ordenar_entidades(datos);
  for (unsigned int i = 0; i < 69; ++i) {
    const auto entidad = palabra(datos, static_cast<Palabra>(0xbdd4 + i * 4));
    if (datos[entidad] == 0 || (datos[entidad] & 2) ||
        datos[static_cast<Palabra>(entidad + 1)] == 0xff)
      continue;
    unsigned int longitud = 0;
    const auto *banco = recursos.banco(
        recursos.contexto, palabra(datos, static_cast<Palabra>(entidad + 31)), longitud);
    if (!banco ||
        !dibujar_fotograma(banco, longitud, datos[static_cast<Palabra>(entidad + 1)] & 0x7f, imagen,
                           con_signo(palabra(datos, static_cast<Palabra>(entidad + 2))),
                           con_signo(palabra(datos, static_cast<Palabra>(entidad + 4))),
                           (datos[static_cast<Palabra>(entidad + 1)] & 0x80) != 0))
      return false;
  }
  for (unsigned int i = 0; i < palabra(datos, 0x358a); ++i) {
    const auto superposicion = static_cast<Palabra>(0x3510 + i * 12);
    const auto izquierda = palabra(datos, superposicion),
               arriba = palabra(datos, static_cast<Palabra>(superposicion + 2));
    const auto ancho = palabra(datos, static_cast<Palabra>(superposicion + 4)),
               alto = palabra(datos, static_cast<Palabra>(superposicion + 6));
    if (con_signo(izquierda) > 319 || con_signo(static_cast<Palabra>(izquierda + ancho)) <= 0)
      continue;
    const auto derecha = static_cast<Palabra>(izquierda + ancho - 1),
               abajo = static_cast<Palabra>(arriba + alto - 1);
    bool solapada = false;
    for (unsigned int j = 0; j < 69 && !solapada; ++j) {
      const auto entidad = static_cast<Palabra>(0x2c2b + j * 33);
      if (datos[entidad] != 1)
        continue;
      const auto x = palabra(datos, static_cast<Palabra>(entidad + 2)),
                 y = palabra(datos, static_cast<Palabra>(entidad + 4));
      solapada =
          con_signo(derecha) >= con_signo(x) &&
          con_signo(izquierda) < con_signo(static_cast<Palabra>(
                                     x + palabra(datos, static_cast<Palabra>(entidad + 6)))) &&
          con_signo(abajo) >= con_signo(y) &&
          con_signo(arriba) < con_signo(static_cast<Palabra>(
                                  y + palabra(datos, static_cast<Palabra>(entidad + 8))));
    }
    if (!solapada)
      continue;
    unsigned int longitud = 0;
    const auto *banco = recursos.banco(recursos.contexto, recursos.escena, longitud);
    if (!banco || !dibujar_fotograma_por_parrafo(
                      banco, longitud, palabra(datos, static_cast<Palabra>(superposicion + 10)),
                      imagen, con_signo(izquierda), con_signo(arriba), false))
      return false;
  }
  return true;
}
} // namespace motor
