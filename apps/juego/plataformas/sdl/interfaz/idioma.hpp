#pragma once
#include "../../../motor/estado/segmento.hpp"
#include "../../../motor/video/sprites.hpp"
#include <algorithm>
#include <array>
#include <string>
#include <vector>

enum class IdiomaInterfaz { ingles, castellano };
inline IdiomaInterfaz idioma_interfaz = IdiomaInterfaz::ingles;

// Cuenta caracteres UTF8 para centrar las letras de ocho píxeles, incluidas las tildes propias.
inline unsigned int letras_interfaz(const std::string &texto) {
  unsigned int cantidad = 0;
  for (const unsigned char c : texto)
    if ((c & 0xc0) != 0x80)
      ++cantidad;
  return cantidad;
}

// D87A sirve de base; las vocales acentuadas y Ñ se dibujan sobre FONT.SPR sin editar el recurso.
inline bool dibujar_texto_interfaz(const unsigned char *datos, const unsigned char *fuente,
                                   unsigned int longitud, unsigned char *destino,
                                   const std::string &texto, unsigned int x, unsigned int y) {
  if (y < 3 || y + 8 > 200 || x + letras_interfaz(texto) * 8 > 320)
    return false;
  unsigned char tinta = 255;
  for (unsigned int i = 0; i < texto.size(); ++i, x += 8) {
    unsigned char letra = static_cast<unsigned char>(texto[i]);
    int acento = 0;
    if (letra == 0xc3 && i + 1 < texto.size()) {
      const auto c = static_cast<unsigned char>(texto[++i]);
      if (c == 0x80) {
        letra = 'A';
        acento = -1;
      } else if (c == 0x81) {
        letra = 'A';
        acento = 1;
      } else if (c == 0x89) {
        letra = 'E';
        acento = 1;
      } else if (c == 0x8d) {
        letra = 'I';
        acento = 1;
      } else if (c == 0x93) {
        letra = 'O';
        acento = 1;
      } else if (c == 0x9a) {
        letra = 'U';
        acento = 1;
      } else if (c == 0x91) {
        letra = 'N';
        acento = 2;
      } else
        return false;
    }
    if (letra >= 128)
      return false;
    const auto indice = datos[0xb6b4 + letra];
    if (indice < 128 && !motor::dibujar_fotograma(fuente, longitud, indice, destino,
                                                  static_cast<int>(x), static_cast<int>(y), false))
      return false;
    for (unsigned int yy = y; yy < y + 8; ++yy)
      for (unsigned int xx = x; xx < x + 8; ++xx)
        if (destino[yy * 320 + xx])
          tinta = destino[yy * 320 + xx];
    if (letra == '.')
      destino[(y + 6) * 320 + x + 3] = destino[(y + 6) * 320 + x + 4] = tinta;
    if (letra == ':')
      for (const unsigned int yy : {2u, 5u})
        destino[(y + yy) * 320 + x + 3] = destino[(y + yy) * 320 + x + 4] = tinta;
    if (letra == '/')
      for (unsigned int j = 0; j < 7; ++j)
        destino[(y + j) * 320 + x + 6 - j] = tinta;
    if (letra == '-')
      for (unsigned int j = 2; j < 6; ++j)
        destino[(y + 3) * 320 + x + j] = tinta;
    if (acento == 2) {
      for (unsigned int j = 2; j < 6; ++j)
        destino[(y - (j < 4 ? 3 : 2)) * 320 + x + j] = tinta;
    } else if (acento) {
      destino[(y - 3) * 320 + x + (acento < 0 ? 2 : 5)] = tinta;
      destino[(y - 2) * 320 + x + (acento < 0 ? 3 : 4)] = tinta;
    }
  }
  return true;
}

// Traduce las cuatro fichas E160; preserva nombres, puntuación y límites de la placa original.
inline bool componer_informacion_castellano(const unsigned char *d, const unsigned char *fuente,
                                            unsigned int longitud, unsigned int pagina,
                                            unsigned char *destino, unsigned int x, unsigned int y,
                                            unsigned int ancho, unsigned int alto) {
  const auto decimal = [](unsigned int numero) {
    const auto texto = std::to_string(numero);
    return numero > 999 ? std::string(3, ' ') : std::string(3 - texto.size(), ' ') + texto;
  };
  const auto puntos = motor::palabra(d, 0x35c7);
  const std::vector<std::string> fichas[] = {
      {"TU PUNTUACIÓN ES      " + decimal(puntos),
       "PORCENTAJE COMPLETADO " + decimal(static_cast<unsigned short>(puntos * 100u / 478u)),
       "-------------------------", "CRÉDITOS DE ENCHANTIA:", "-------------------",
       "CÓD. 8086 - IAN SABINE", "CÓD. 68000 - ROBERT TOONE"},
      {"GRÁFICOS - BILLY ALLISON", "         - STUART ATKINSON", "-------------------------",
       "FONDOS - ROLF MOHR", "-------------------------", "SONIDO - MARTIN IVESON",
       "       - DAVID PRIDMORE"},
      {"DISEÑO DEL JUEGO Y", "CREACIÓN DE PUZLES:", "", "  ROB TOONE", "   IAN SABINE",
       "    CHRIS LONG"},
      {"GRACIAS A TODOS", "POR VUESTRA AYUDA", "Y POR VUESTRAS", "CRÍTICAS."}};
  if (pagina >= 4 || fichas[pagina].size() * 10 + 5 > alto)
    return false;
  for (const auto &linea : fichas[pagina])
    if (letras_interfaz(linea) * 8 + 8 > ancho)
      return false;
  for (unsigned int yy = y; yy < y + alto; ++yy)
    std::fill(destino + yy * 320 + x, destino + yy * 320 + x + ancho, 0);
  for (const auto &linea : fichas[pagina]) {
    if (!dibujar_texto_interfaz(d, fuente, longitud, destino, linea, x + 8, y + 5))
      return false;
    y += 10;
  }
  return true;
}

// Traduce solo rótulos de E997/E46D en una copia de dibujo; no toca DS vivo ni nombres de usuario.
inline void traducir_panel_partidas(std::array<unsigned char, 65536> &d) {
  const auto sustituir = [&](unsigned int posicion, const std::string &texto) {
    std::copy(texto.begin(), texto.end(), d.begin() + posicion);
  };
  sustituir(0xb949, "CARGAR    :-");
  sustituir(0xb9cf, "GUARDAR   :-");
  sustituir(0xba62, " GUARDAR   :-");
  sustituir(0xbae9, "BORRAR      :-");
  sustituir(0xbb7b, "[ SIN USAR ] ");
}
