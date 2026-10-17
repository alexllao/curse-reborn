#include "sprites.hpp"

namespace motor {
namespace {
// Lee una palabra del recurso sin imponer alineación de memoria al equipo anfitrión.
unsigned int palabra(const unsigned char *datos) {
  return datos[0] | (static_cast<unsigned int>(datos[1]) << 8);
}
} // namespace

// Traslada las carreras de 6F61–726A a píxeles lineales, conservando los huecos transparentes.
bool dibujar_fotograma(const unsigned char *banco, unsigned int longitud, unsigned int indice,
                       unsigned char *destino, int x, int y, bool invertido) {
  if (indice >= 128 || longitud < 2 || indice * 2u + 2u > longitud)
    return false;
  const unsigned int inicio = palabra(banco + indice * 2u) * 16u;
  if (inicio > longitud || longitud - inicio < 10)
    return false;
  const auto *cabecera = banco + inicio;
  const unsigned int ancho = palabra(cabecera), alto = palabra(cabecera + 2);
  if (ancho == 0 || alto == 0 || ancho > 32767 || alto > 32767 || x < -32768 || x > 32767 ||
      y < -32768 || y > 32767)
    return false;
  if (x >= 320 || y >= 200 || x + static_cast<int>(ancho) <= 0 || y + static_cast<int>(alto) <= 0)
    return true;
  const unsigned int base = inicio + 10;
  // 6F31–6F3A apila los punteros y los consume en orden inverso: 0, campo 8, 6, 4.
  const unsigned int posiciones[4] = {base, base + palabra(cabecera + 8),
                                      base + palabra(cabecera + 6), base + palabra(cabecera + 4)};
  for (unsigned int plano = 0; plano < 4; ++plano) {
    unsigned int posicion = posiciones[plano];
    for (unsigned int fila = 0; fila < alto; ++fila) {
      unsigned int columna = plano;
      while (true) {
        if (posicion >= longitud)
          return false;
        const auto codigo = banco[posicion++];
        if (codigo == 0)
          break;
        if (codigo >= 128) {
          columna += (256u - codigo) * 4u;
          if (columna > ancho + 3u)
            return false;
        } else {
          if (codigo > longitud - posicion)
            return false;
          for (unsigned int i = 0; i < codigo; ++i) {
            if (columna >= ancho)
              return false;
            const int destino_x = x + (invertido ? static_cast<int>(ancho - 1u - columna)
                                                 : static_cast<int>(columna));
            const int destino_y = y + static_cast<int>(fila);
            if (destino_x >= 0 && destino_x < 320 && destino_y >= 0 && destino_y < 200)
              destino[destino_y * 320 + destino_x] = banco[posicion];
            ++posicion;
            columna += 4;
          }
        }
      }
    }
  }
  return true;
}
} // namespace motor
