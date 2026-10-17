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
// 7970–79B7 con BX=10 y CX=3: alinea el decimal y conserva el truncado de palabras DOS.
static void decimal_informacion(unsigned char *destino, Palabra valor) {
  const bool negativo = con_signo(valor) < 0;
  unsigned int magnitud = negativo ? static_cast<Palabra>(-valor) : valor;
  unsigned int restantes = negativo ? 2 : 3;
  int posicion = 2;
  do {
    destino[posicion--] = static_cast<unsigned char>('0' + magnitud % 10);
    magnitud /= 10;
    --restantes;
  } while (magnitud && restantes);
  if (magnitud)
    return;
  if (negativo)
    destino[posicion--] = '-';
  while (posicion >= 0)
    destino[posicion--] = ' ';
}

// E160–E19E: tabla DS:B74E, divisoria 478 y textos terminados en FE; FF cambia de línea.
bool componer_informacion(const unsigned char *datos, const unsigned char *fuente,
                          unsigned int longitud, unsigned int pagina, unsigned char *destino,
                          unsigned int &x, unsigned int &y, unsigned int &ancho,
                          unsigned int &alto) {
  if (!datos || !fuente || !destino || pagina >= 4)
    return false;
  const unsigned int inicio = palabra(datos, static_cast<Palabra>(0xb74e + pagina * 2));
  if (inicio + 4 >= 65536)
    return false;
  ancho = palabra(datos, static_cast<Palabra>(inicio));
  alto = palabra(datos, static_cast<Palabra>(inicio + 2));
  if (!ancho || ancho > 320 || !alto || alto > 200)
    return false;
  x = (320 - ancho) / 2;
  y = (200 - alto) / 2;
  for (unsigned int fila = y; fila < y + alto; ++fila)
    for (unsigned int columna = x; columna < x + ancho; ++columna)
      destino[fila * 320 + columna] = 0;
  unsigned char puntos[3]{}, porcentaje[3]{};
  const auto puntuacion = palabra(datos, 0x35c7);
  decimal_informacion(puntos, puntuacion);
  decimal_informacion(porcentaje, static_cast<Palabra>(puntuacion * 100u / 478u));
  int columna = static_cast<int>(x) + 8, fila = static_cast<int>(y) + 5;
  for (unsigned int posicion = inicio + 4; posicion < 65536; ++posicion) {
    auto letra = datos[posicion];
    if (posicion >= 0xb772 && posicion < 0xb775)
      letra = puntos[posicion - 0xb772];
    if (posicion >= 0xb78c && posicion < 0xb78f)
      letra = porcentaje[posicion - 0xb78c];
    if (letra == 0xfe)
      return true;
    if (letra == 0xff) {
      columna = static_cast<int>(x) + 8;
      fila += 10;
      continue;
    }
    // D897 interpreta el carácter con signo antes de consultar su índice en DS:B6B4.
    const int caracter = letra < 128 ? letra : static_cast<int>(letra) - 256;
    const auto indice = datos[0xb6b4 + caracter];
    if (indice < 128 && !dibujar_fotograma(fuente, longitud, indice, destino, columna, fila, false))
      return false;
    columna += 8;
  }
  return false;
}
} // namespace motor
