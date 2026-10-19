#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>

// Implementaciones propias RGB de las reglas de xBRZ 2x (Zenju) y 2xSaI (Derek Liauw Kie Fa).
// Son efectos de presentación: no interpretan recursos ni modifican el estado reconstruido.
namespace escaladores_pixel {
using Color = std::uint32_t;

// Lee RGB sin depender del orden de bytes de un entero ni de su alineación en memoria.
inline Color leer(const unsigned char *imagen, int ancho, int alto, int x, int y) {
  const auto *p = imagen + (std::clamp(y, 0, alto - 1) * ancho + std::clamp(x, 0, ancho - 1)) * 4;
  return Color(p[0]) | (Color(p[1]) << 8) | (Color(p[2]) << 16);
}

// Extrae un componente de ocho bits del color RGB lógico.
inline unsigned int componente(Color color, int canal) {
  return (color >> (canal * 8)) & 255u;
}

// Guarda un píxel opaco de la imagen escalada, sin modificar la entrada.
inline void escribir(unsigned char *imagen, int ancho, int x, int y, Color color) {
  auto *p = imagen + (y * ancho + x) * 4;
  for (int canal = 0; canal < 3; ++canal)
    p[canal] = static_cast<unsigned char>(componente(color, canal));
  p[3] = 255;
}

// Promedia componentes completos, sin el redondeo a RGB565 de las versiones antiguas.
inline Color promedio(Color a, Color b, Color c, Color d) {
  Color resultado = 0;
  for (int canal = 0; canal < 3; ++canal)
    resultado |= ((componente(a, canal) + componente(b, canal) + componente(c, canal) +
                   componente(d, canal)) /
                  4u)
                 << (canal * 8);
  return resultado;
}

// Decide qué diagonal tiene continuidad en los pares vecinos de una trama 2xSaI.
inline int continuidad(Color a, Color b, Color u, Color v) {
  const int apoyo_a = int(u == a) + int(v == a);
  const int apoyo_b = int(u != a && u == b) + int(v != a && v == b);
  return int(apoyo_a <= 1) - int(apoyo_b <= 1);
}

// 2xSaI reconoce diagonales en una vecindad 4x4 y completa sus tres subpíxeles interpolados.
inline void sai(const unsigned char *entrada, unsigned char *salida, int ancho, int alto) {
  for (int y = 0; y < alto; ++y) {
    for (int x = 0; x < ancho; ++x) {
      // Vecindad: I E F J / G A B K / H C D L / M N O P. Los bordes se prolongan.
      Color vecinos[16];
      for (int fila = 0; fila < 4; ++fila)
        for (int columna = 0; columna < 4; ++columna)
          vecinos[fila * 4 + columna] = leer(entrada, ancho, alto, x + columna - 1, y + fila - 1);
      const Color a = vecinos[5], b = vecinos[6], c = vecinos[9], d = vecinos[10];
      const Color e = vecinos[1], f = vecinos[2], g = vecinos[4], h = vecinos[8];
      const Color i = vecinos[0], j = vecinos[3], k = vecinos[7], l = vecinos[11];
      const Color m = vecinos[12], n = vecinos[13], o = vecinos[14];
      const bool diagonal_a = a == d, diagonal_b = b == c;
      const bool horizontal_a = a == c && a == f && b != e && b == j;
      const bool horizontal_b = b == e && b == d && a != f && a == i;
      const bool vertical_a = a == b && a == h && g != c && c == m;
      const bool vertical_b = c == g && c == d && a != h && a == i;
      Color derecha = promedio(a, a, b, b);
      Color abajo = promedio(a, a, c, c);
      Color esquina = promedio(a, b, c, d);
      if (diagonal_a && diagonal_b) {
        const int voto = continuidad(a, b, g, e) + continuidad(a, b, k, f) +
                         continuidad(a, b, h, n) + continuidad(a, b, l, o);
        if (a == b || voto > 0)
          esquina = a;
        else if (voto < 0)
          esquina = b;
      } else if (diagonal_a) {
        esquina = a;
        if ((a == e && b == l) || horizontal_a)
          derecha = a;
        if ((a == g && c == o) || vertical_a)
          abajo = a;
      } else if (diagonal_b) {
        esquina = b;
        if ((b == f && a == h) || horizontal_b)
          derecha = b;
        if ((c == h && a == f) || vertical_b)
          abajo = c;
      } else {
        if (horizontal_a)
          derecha = a;
        else if (horizontal_b)
          derecha = b;
        if (vertical_a)
          abajo = a;
        else if (vertical_b)
          abajo = c;
      }
      escribir(salida, ancho * 2, x * 2, y * 2, a);
      escribir(salida, ancho * 2, x * 2 + 1, y * 2, derecha);
      escribir(salida, ancho * 2, x * 2, y * 2 + 1, abajo);
      escribir(salida, ancho * 2, x * 2 + 1, y * 2 + 1, esquina);
    }
  }
}

// Mide separación de color en luminancia y crominancia con los coeficientes de xBRZ.
inline float distancia(Color a, Color b) {
  const float rojo = float(componente(a, 0)) - float(componente(b, 0));
  const float verde = float(componente(a, 1)) - float(componente(b, 1));
  const float azul = float(componente(a, 2)) - float(componente(b, 2));
  const float luz = rojo * 0.2627f + verde * 0.6780f + azul * 0.0593f;
  const float croma_azul = (azul - luz) * (0.5f / (1.0f - 0.0593f));
  const float croma_rojo = (rojo - luz) * (0.5f / (1.0f - 0.2627f));
  return std::sqrt(luz * luz + croma_azul * croma_azul + croma_rojo * croma_rojo);
}

// Rota coordenadas alrededor del píxel central para evaluar las cuatro esquinas por igual.
inline void rotar(int &x, int &y, int giro) {
  for (int i = 0; i < giro; ++i) {
    const int anterior = x;
    x = -y;
    y = anterior;
  }
}

// Obtiene un vecino del mosaico 5x5 en la orientación de una esquina.
inline Color vecino(const Color *mosaico, int giro, int x, int y) {
  rotar(x, y, giro);
  return mosaico[(y + 2) * 5 + x + 2];
}

// Determina si una diagonal merece mezcla normal o dominante; cero conserva el centro.
inline int esquina_xbrz(const Color *mosaico, int giro) {
  const Color centro = vecino(mosaico, giro, 0, 0);
  const Color derecha = vecino(mosaico, giro, 1, 0);
  const Color abajo = vecino(mosaico, giro, 0, 1);
  const Color diagonal = vecino(mosaico, giro, 1, 1);
  if ((centro == derecha && abajo == diagonal) || (centro == abajo && derecha == diagonal))
    return 0;
  const float cruz = distancia(vecino(mosaico, giro, -1, 1), centro) +
                     distancia(centro, vecino(mosaico, giro, 1, -1)) +
                     distancia(vecino(mosaico, giro, 0, 2), diagonal) +
                     distancia(diagonal, vecino(mosaico, giro, 2, 0)) +
                     4.0f * distancia(abajo, derecha);
  const float borde = distancia(vecino(mosaico, giro, -1, 0), abajo) +
                      distancia(abajo, vecino(mosaico, giro, 1, 2)) +
                      distancia(vecino(mosaico, giro, 0, -1), derecha) +
                      distancia(derecha, vecino(mosaico, giro, 2, 1)) +
                      4.0f * distancia(centro, diagonal);
  if (cruz >= borde || centro == derecha || centro == abajo)
    return 0;
  return cruz * 3.6f < borde ? 2 : 1;
}

// Mezcla un subpíxel en coordenadas rotadas; mantiene precisión hasta terminar las cuatro esquinas.
inline void mezclar(float colores[4][3], int giro, int x, int y, Color color, float peso) {
  for (int i = 0; i < giro; ++i) {
    const int anterior = x;
    x = 1 - y;
    y = anterior;
  }
  for (int canal = 0; canal < 3; ++canal)
    colores[y * 2 + x][canal] +=
        (float(componente(color, canal)) - colores[y * 2 + x][canal]) * peso;
}

// Reconstruye contornos xBRZ 2x con comparación perceptual, pendientes y esquinas redondeadas.
inline void xbrz(const unsigned char *entrada, unsigned char *salida, int ancho, int alto) {
  for (int y = 0; y < alto; ++y) {
    for (int x = 0; x < ancho; ++x) {
      Color mosaico[25];
      for (int fila = 0; fila < 5; ++fila)
        for (int columna = 0; columna < 5; ++columna)
          mosaico[fila * 5 + columna] = leer(entrada, ancho, alto, x + columna - 2, y + fila - 2);
      const Color centro = mosaico[12];
      float colores[4][3];
      for (auto &color : colores)
        for (int canal = 0; canal < 3; ++canal)
          color[canal] = float(componente(centro, canal));
      int esquinas[4];
      for (int giro = 0; giro < 4; ++giro)
        esquinas[giro] = esquina_xbrz(mosaico, giro);
      for (int giro = 0; giro < 4; ++giro) {
        if (!esquinas[giro])
          continue;
        const Color derecha = vecino(mosaico, giro, 1, 0);
        const Color abajo = vecino(mosaico, giro, 0, 1);
        const Color diagonal = vecino(mosaico, giro, 1, 1);
        const Color inferior = vecino(mosaico, giro, -1, 1);
        const Color superior = vecino(mosaico, giro, 1, -1);
        const bool cadena =
            distancia(inferior, abajo) < 30.0f && distancia(abajo, diagonal) < 30.0f &&
            distancia(diagonal, derecha) < 30.0f && distancia(derecha, superior) < 30.0f &&
            distancia(centro, diagonal) >= 30.0f;
        const bool adyacentes =
            (esquinas[(giro + 3) % 4] && distancia(centro, inferior) >= 30.0f) ||
            (esquinas[(giro + 1) % 4] && distancia(centro, superior) >= 30.0f);
        const bool linea = esquinas[giro] == 2 || (!cadena && !adyacentes);
        const float horizontal = distancia(derecha, inferior);
        const float vertical = distancia(abajo, superior);
        const bool tendida = horizontal * 2.2f <= vertical && centro != inferior &&
                             vecino(mosaico, giro, -1, 0) != inferior;
        const bool empinada = vertical * 2.2f <= horizontal && centro != superior &&
                              vecino(mosaico, giro, 0, -1) != superior;
        const Color mezcla =
            distancia(centro, derecha) <= distancia(centro, abajo) ? derecha : abajo;
        const float peso = !linea                ? 0.2146018366f
                           : tendida && empinada ? 5.0f / 6.0f
                           : tendida || empinada ? 0.75f
                                                 : 0.5f;
        if (linea && empinada)
          mezclar(colores, giro, 1, 0, mezcla, 0.25f);
        mezclar(colores, giro, 1, 1, mezcla, peso);
        if (linea && tendida)
          mezclar(colores, giro, 0, 1, mezcla, 0.25f);
      }
      for (int subpixel = 0; subpixel < 4; ++subpixel) {
        Color color = 0;
        for (int canal = 0; canal < 3; ++canal)
          color |= Color(std::clamp(int(colores[subpixel][canal] + 0.5f), 0, 255)) << (canal * 8);
        escribir(salida, ancho * 2, x * 2 + subpixel % 2, y * 2 + subpixel / 2, color);
      }
    }
  }
}
} // namespace escaladores_pixel
