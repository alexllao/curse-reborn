#pragma once
#include "../../motor/estado/segmento.hpp"
#include "../../motor/video/sprites.hpp"
#include <string>

inline constexpr unsigned int paginas_informacion = 5;

// Convierte la fecha real de esta compilación a día/mes/año, independientemente del idioma del SO.
inline std::string fecha_compilacion_port() {
  constexpr const char *meses = "JanFebMarAprMayJunJulAugSepOctNovDec";
  const std::string fecha = __DATE__;
  const auto mes = std::string(meses).find(fecha.substr(0, 3)) / 3 + 1;
  return std::string(1, fecha[4] == ' ' ? '0' : fecha[4]) + fecha[5] + "/" + (mes < 10 ? "0" : "") +
         std::to_string(mes) + "/" + fecha.substr(7, 4);
}

// Dibuja la ficha propia con FONT.SPR; conserva las tildes del nombre sobre las letras originales.
inline bool componer_creditos_port(const unsigned char *datos, const unsigned char *fuente,
                                   unsigned int longitud, unsigned char *destino) {
  constexpr unsigned int izquierda = 24, arriba = 51, ancho = 272, alto = 98;
  for (unsigned int y = arriba; y < arriba + alto; ++y)
    for (unsigned int x = izquierda; x < izquierda + ancho; ++x)
      destino[y * 320 + x] = 0;
  const std::string lineas[] = {"CURSE REBORN", "PORT POR ÀLEX LLAÓ",
                                std::string("VERSION ") + CURSE_REBORN_VERSION,
                                "COMPILACION " + fecha_compilacion_port()};
  unsigned char tinta = 255;
  for (unsigned int fila = 0; fila < 4; ++fila) {
    const auto &texto = lineas[fila];
    unsigned int letras = 0;
    for (const unsigned char c : texto)
      if ((c & 0xc0) != 0x80)
        ++letras;
    unsigned int x = (320 - letras * 8) / 2, y = arriba + 12 + fila * 20;
    for (unsigned int i = 0; i < texto.size(); ++i, x += 8) {
      unsigned char letra = static_cast<unsigned char>(texto[i]);
      int acento = 0;
      if (letra == 0xc3 && i + 1 < texto.size()) {
        const auto siguiente = static_cast<unsigned char>(texto[++i]);
        if (siguiente == 0x80) {
          letra = 'A';
          acento = -1;
        } else if (siguiente == 0x93) {
          letra = 'O';
          acento = 1;
        } else
          return false;
      }
      const auto indice = datos[0xb6b4 + letra];
      if (indice < 128 &&
          !motor::dibujar_fotograma(fuente, longitud, indice, destino, static_cast<int>(x),
                                    static_cast<int>(y), false))
        return false;
      for (unsigned int yy = y; yy < y + 8; ++yy)
        for (unsigned int xx = x; xx < x + 8; ++xx)
          if (destino[yy * 320 + xx])
            tinta = destino[yy * 320 + xx];
      // FONT.SPR no ofrece estos separadores: píxeles propios con la tinta de sus letras.
      if (letra == '.')
        destino[(y + 6) * 320 + x + 3] = destino[(y + 6) * 320 + x + 4] = tinta;
      if (letra == '/')
        for (unsigned int j = 0; j < 7; ++j)
          destino[(y + j) * 320 + x + 6 - j] = tinta;
      if (letra == '-')
        for (unsigned int j = 2; j < 6; ++j)
          destino[(y + 3) * 320 + x + j] = tinta;
      if (acento) {
        destino[(y - 3) * 320 + x + (acento < 0 ? 2 : 5)] = tinta;
        destino[(y - 2) * 320 + x + (acento < 0 ? 3 : 4)] = tinta;
      }
    }
  }
  return true;
}
