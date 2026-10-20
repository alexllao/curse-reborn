#pragma once
#include "idioma.hpp"
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

// Dibuja la ficha propia con FONT.SPR y el idioma elegido, conservando la autoría del port.
inline bool componer_creditos_port(const unsigned char *datos, const unsigned char *fuente,
                                   unsigned int longitud, unsigned char *destino) {
  constexpr unsigned int izquierda = 24, arriba = 51, ancho = 272, alto = 98;
  for (unsigned int y = arriba; y < arriba + alto; ++y)
    for (unsigned int x = izquierda; x < izquierda + ancho; ++x)
      destino[y * 320 + x] = 0;
  const bool castellano = idioma_interfaz == IdiomaInterfaz::castellano;
  const std::string lineas[] = {
      "CURSE OF ENCHANTIA: REBORN", castellano ? "PORT POR ÀLEX LLAÓ" : "PORT BY ÀLEX LLAÓ",
      std::string(castellano ? "VERSIÓN " : "VERSION ") + CURSE_REBORN_VERSION,
      std::string(castellano ? "COMPILACIÓN " : "BUILD ") + fecha_compilacion_port()};
  for (unsigned int fila = 0; fila < 4; ++fila)
    if (!dibujar_texto_interfaz(datos, fuente, longitud, destino, lineas[fila],
                                (320 - letras_interfaz(lineas[fila]) * 8) / 2,
                                arriba + 12 + fila * 20))
      return false;
  return true;
}
