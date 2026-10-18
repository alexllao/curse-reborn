#include "panel_partidas.hpp"
#include "sprites.hpp"
#include "../estado/segmento.hpp"
#include "../estado/partidas.hpp"
namespace motor {
// D87A: avanza ocho píxeles por carácter y diez por FF; FE termina el texto original.
static bool texto(const unsigned char *datos, const unsigned char *fuente, unsigned int longitud,
                  const unsigned char *cadena, unsigned int cantidad, unsigned char *destino) {
  int x = 100, y = 55;
  for (unsigned int i = 0; i < cantidad; ++i) {
    const auto letra = cadena[i];
    if (letra == 0xfe)
      return true;
    if (letra == 0xff) {
      x = 100;
      y += 10;
      continue;
    }
    const int caracter = letra < 128 ? letra : static_cast<int>(letra) - 256;
    const auto indice = datos[0xb6b4 + caracter];
    if (indice < 128 && !dibujar_fotograma(fuente, longitud, indice, destino, x, y, false))
      return false;
    x += 8;
  }
  return false;
}
// E997 recorre ocho bits; los textos se leen de DS y los nombres del CURSE.SAV del usuario.
bool componer_panel_partidas(const unsigned char *d, const unsigned char *fuente,
                             unsigned int longitud, const unsigned char *archivo,
                             unsigned int longitud_archivo, unsigned int modo, bool editando,
                             const unsigned char *nombre, unsigned int letras,
                             unsigned char *destino) {
  if (!d || !fuente || longitud < 2 || !destino || modo < 1 || modo > 3 || letras > 13 ||
      (letras && !nombre) || !validar_partidas(archivo, longitud_archivo))
    return false;
  unsigned char cadena[136]{};
  const Palabra inicio = editando ? 0xba62 : modo == 1 ? 0xb949 : modo == 2 ? 0xb9cf : 0xbae9;
  for (unsigned int i = 0; i < 136; ++i)
    cadena[i] = d[inicio + i];
  if (editando) {
    // E41E–E447: el nombre ocupa BADA–BAE6; los caracteres libres se muestran como guiones.
    for (unsigned int i = 0; i < 13; ++i) {
      auto letra = i < letras ? nombre[i] : static_cast<unsigned char>('-');
      if (letra >= 'a' && letra <= 'z')
        letra = static_cast<unsigned char>(letra - 'a' + 'A');
      const int caracter = letra < 128 ? letra : static_cast<int>(letra) - 256;
      const auto glifo = d[0xb6b4 + caracter];
      if (glifo < 128 && glifo * 2u + 2u > palabra(fuente, 0) * 16u)
        letra = '?';
      cadena[0xbada - 0xba62 + i] = letra;
    }
  } else {
    // E997–E9D3: nombres de trece bytes, prefijo de un espacio y salto de línea conservados.
    for (unsigned int ranura = 0; ranura < ranuras_partida; ++ranura) {
      for (unsigned int i = 0; i < 13; ++i) {
        auto letra = (archivo[0] & (1u << ranura)) ? archivo[1 + ranura * bytes_ranura_partida + i]
                                                   : d[0xbb7b + i];
        // El antiguo panel SDL permitía minúsculas que el teclado DOS no produce.
        // Se adaptan solo al dibujar; sus bytes originales se conservan en CURSE.SAV.
        if (letra >= 'a' && letra <= 'z')
          letra = static_cast<unsigned char>(letra - 'a' + 'A');
        const int caracter = letra < 128 ? letra : static_cast<int>(letra) - 256;
        const auto glifo = d[0xb6b4 + caracter];
        if (glifo < 128 && glifo * 2u + 2u > palabra(fuente, 0) * 16u)
          letra = '?';
        cadena[(modo == 3 ? 16 : 14) + ranura * 15 + i] = letra;
      }
    }
  }
  // E89D/E46D: DI15F8 con paso112 corresponde a x96,y50; se rellena de negro.
  for (unsigned int y = 50; y < 150; ++y)
    for (unsigned int x = 96; x < 224; ++x)
      destino[y * 320 + x] = 0;
  return texto(d, fuente, longitud, cadena, 136, destino);
}
} // namespace motor
