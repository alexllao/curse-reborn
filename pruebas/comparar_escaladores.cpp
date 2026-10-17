#include "../plataformas/sdl/escaladores_pixel.hpp"
#include <array>
#include <chrono>
#include <cstdio>
#include <vector>

using Escalador = void (*)(const unsigned char *, unsigned char *, int, int);

// Comprueba extremos de dimensiones, prolongación de bordes, opacidad y escritura acotada.
bool comprobar_planos(Escalador escalar) {
  for (const auto dimensiones : {std::array<int, 2>{1, 1}, {1, 7}, {7, 1}, {7, 7}}) {
    const int ancho = dimensiones[0], alto = dimensiones[1];
    for (const auto color : {0u, 0xffffffu, 0xf102e7u, 0x35ed7bu}) {
      std::vector<unsigned char> entrada(ancho * alto * 4);
      for (int y = 0; y < alto; ++y)
        for (int x = 0; x < ancho; ++x)
          escaladores_pixel::escribir(entrada.data(), ancho, x, y, color);
      const auto copia = entrada;
      std::vector<unsigned char> salida(ancho * alto * 16 + 32, 0x97);
      escalar(entrada.data(), salida.data() + 16, ancho, alto);
      if (entrada != copia)
        return false;
      for (int i = 0; i < 16; ++i)
        if (salida[i] != 0x97 || salida[salida.size() - 1 - i] != 0x97)
          return false;
      for (int y = 0; y < alto * 2; ++y)
        for (int x = 0; x < ancho * 2; ++x)
          if (escaladores_pixel::leer(salida.data() + 16, ancho * 2, alto * 2, x, y) != color ||
              salida[16 + (y * ancho * 2 + x) * 4 + 3] != 255)
            return false;
    }
  }
  return true;
}

// Verifica una esquina conocida y que 2xSaI conserve la muestra superior izquierda de cada bloque.
bool comprobar_contornos() {
  std::vector<unsigned char> entrada(5 * 5 * 4), sai(10 * 10 * 4), xbrz(sai.size());
  for (int y = 0; y < 5; ++y)
    for (int x = 0; x < 5; ++x)
      escaladores_pixel::escribir(entrada.data(), 5, x, y, x >= 2 && y >= 2 ? 0xffffff : 0);
  escaladores_pixel::sai(entrada.data(), sai.data(), 5, 5);
  escaladores_pixel::xbrz(entrada.data(), xbrz.data(), 5, 5);
  // El redondeo de un cuadrante blanco cubre 1 - pi/4 con negro: 255*pi/4 redondea a 200.
  if (escaladores_pixel::leer(xbrz.data(), 10, 10, 4, 4) != 0xc8c8c8 ||
      escaladores_pixel::leer(sai.data(), 10, 10, 3, 3) != 0 ||
      escaladores_pixel::leer(sai.data(), 10, 10, 3, 5) != 0x7f7f7f || sai == xbrz)
    return false;
  for (int y = 0; y < 5; ++y)
    for (int x = 0; x < 5; ++x)
      if (escaladores_pixel::leer(sai.data(), 10, 10, 2 * x, 2 * y) !=
          escaladores_pixel::leer(entrada.data(), 5, 5, x, y))
        return false;
  return true;
}

// Ejecuta las comprobaciones sin SDL y mide una trama de diagonales a la resolución del juego.
int main() {
  if (!comprobar_planos(escaladores_pixel::sai) || !comprobar_planos(escaladores_pixel::xbrz) ||
      !comprobar_contornos()) {
    std::fprintf(stderr, "No coinciden los contratos de los escaladores de píxeles.\n");
    return 1;
  }
  std::vector<unsigned char> entrada(320 * 200 * 4), salida(640 * 400 * 4);
  for (int y = 0; y < 200; ++y)
    for (int x = 0; x < 320; ++x)
      escaladores_pixel::escribir(entrada.data(), 320, x, y, ((x + y) / 5) % 2 ? 0xffffff : 0);
  for (const auto escalar : {escaladores_pixel::xbrz, escaladores_pixel::sai}) {
    const auto inicio = std::chrono::steady_clock::now();
    for (int i = 0; i < 10; ++i)
      escalar(entrada.data(), salida.data(), 320, 200);
    const double milisegundos =
        std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - inicio)
            .count() /
        10.0;
    std::printf("%s: %.2f ms por imagen de 320 x 200.\n",
                escalar == escaladores_pixel::xbrz ? "xBRZ 2x" : "2xSaI", milisegundos);
  }
  std::puts("Coinciden 32 planos, bordes, opacidad, memoria de entrada y contornos conocidos.");
  return 0;
}
