#include "menu.hpp"
#include "sprites.hpp"
#include "../estado/segmento.hpp"
namespace motor {
// DF4E: cada icono empieza en (casilla×32+4,4); los objetos consultan DS:524E.
bool dibujar_menu(const unsigned char *d, const unsigned char *fondo, const unsigned char *iconos,
                  unsigned int longitud, unsigned char *destino) {
  for (unsigned int i = 0; i < 64000; ++i)
    destino[i] = i < 10240 ? fondo[i] : 0;
  for (unsigned short i = 0; i < 10; ++i) {
    const auto id = palabra(d, static_cast<Palabra>(0xb620 + i * 2));
    if (id == 0xffff)
      break;
    const auto indice =
        con_signo(id) < 0x1a
            ? id
            : d[static_cast<Palabra>(0x524e + static_cast<Palabra>((id - 0x1a) * 2))];
    if (longitud < 2 || indice * 2u + 2u > longitud || indice * 2u + 2u > palabra(iconos, 0) * 16u)
      return false;
    if (!dibujar_fotograma_por_parrafo(iconos, longitud,
                                       palabra(iconos, static_cast<Palabra>(indice * 2)), destino,
                                       i * 32 + 4, 4, false))
      return false;
    if (con_signo(static_cast<Palabra>(id - 0x1a)) >= 0 &&
        static_cast<Palabra>(id - 0x1a) == palabra(d, 0xb638)) {
      if (!dibujar_fotograma_por_parrafo(d + 0xbc10, 65536 - 0xbc10, 0, destino, i * 32 + 4, 4,
                                         false))
        return false;
    }
  }
  if (palabra(d, 0xb620) == 0 && palabra(d, 0xb622) == 1 && palabra(d, 0x2c3b) != 0) {
    for (unsigned int i = 1; i < 3; ++i)
      if (!dibujar_fotograma_por_parrafo(d + 0xbc10, 65536 - 0xbc10, 0, destino, i * 32 + 4, 4,
                                         false))
        return false;
  }
  return true;
}

// B608=4980h corresponde a y=168 con paso de 112 bytes por plano; el marco mide 26×26.
void componer_menu(const unsigned char *d, const unsigned char *panel, unsigned short casilla,
                   unsigned char *destino) {
  const unsigned int y = palabra(d, 0xb608) == 0 ? 0 : 168;
  for (unsigned int i = 0; i < 10240; ++i)
    destino[y * 320 + i] = panel[i];
  if (casilla >= 10)
    return;
  const auto x = casilla * 32u + 3u;
  for (unsigned int i = 0; i < 26; ++i) {
    destino[(y + 3) * 320 + x + i] = 255;
    destino[(y + 28) * 320 + x + i] = 255;
    destino[(y + 3 + i) * 320 + x] = 255;
    destino[(y + 3 + i) * 320 + x + 25] = 255;
  }
}
} // namespace motor
