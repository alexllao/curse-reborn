#include "preparar_resultados.hpp"
#include "seleccionar_coincidencias.hpp"

namespace motor {
namespace {

using Palabra = unsigned short;

Palabra leer(const unsigned char* datos, unsigned int posicion) {
  return static_cast<Palabra>(datos[posicion] | (static_cast<unsigned int>(datos[posicion + 1]) << 8));
}

void escribir(unsigned char* datos, unsigned int posicion, Palabra valor) {
  datos[posicion] = static_cast<unsigned char>(valor & 0xff);
  datos[posicion + 1] = static_cast<unsigned char>(valor >> 8);
}

void retirar_identificador(unsigned char* datos, Palabra identificador) {
  // 88F3 busca la primera coincidencia en las diez posiciones, incluso tras FFFFh.
  for (unsigned int casilla = 0; casilla < 10; ++casilla) {
    if (leer(datos, 0x5225 + casilla * 2) == identificador) {
      for (auto siguiente = casilla + 1; siguiente < 10; ++siguiente) {
        escribir(datos, 0x5225 + (siguiente - 1) * 2, leer(datos, 0x5225 + siguiente * 2));
      }
      escribir(datos, 0x5237, 0xffff);
      return;
    }
  }
}

Palabra siguiente(EstadoAleatorio& estado, Palabra limite) {
  Palabra resultado = 0;
  generar_aleatorio(estado, limite, resultado); // Solo se utiliza con 6 o 100.
  return resultado;
}

}

void preparar_resultados(unsigned char* datos, EstadoAleatorio& estado) {
  retirar_identificador(datos, 0x008c);
  constexpr unsigned short marcas[] = {0x4b50, 0x4b3e, 0x4b2c, 0x4b59, 0x4b47,
    0x4b35, 0x4b88, 0x4b76, 0x4b91, 0x4b7f, 0x4b6d, 0x4bd2, 0x4bdb, 0x4bc9};
  for (auto posicion : marcas) {
    datos[posicion] = 0;
  }
  const auto primero = static_cast<unsigned char>(siguiente(estado, 6));
  datos[0x2c8f] = static_cast<unsigned char>(primero + 1);
  const auto segundo = siguiente(estado, 100) < 50
    ? primero : static_cast<unsigned char>(siguiente(estado, 6));
  datos[0x2cb0] = static_cast<unsigned char>(segundo + 1);
  // La tercera decisión reutiliza el primer resultado, no el segundo.
  const auto tercero = siguiente(estado, 100) < 50
    ? primero : static_cast<unsigned char>(siguiente(estado, 6));
  datos[0x2cd1] = static_cast<unsigned char>(tercero + 1);
  seleccionar_coincidencias(datos, primero, segundo, tercero);

  datos[0x4aa3] = 1;
  datos[0x2c4c] = 3;
  escribir(datos, 0x1a38, 0x00e8);
  escribir(datos, 0x1a3a, 0x009f);
  escribir(datos, 0x0ad7, 0xab1e);
  escribir(datos, 0x0adc, 0xab33);
  escribir(datos, 0x009c, 0x2c4c);
  escribir(datos, 0x009e, 0x0211);
  escribir(datos, 0x00a0, 0xab41);
  escribir(datos, 0x00a2, 0xab4e);
  escribir(datos, 0x00a4, 0xab57);
  constexpr unsigned short registros[] = {0x2c6d, 0x2c8e, 0x2caf, 0x2cd0};
  for (unsigned int indice = 0; indice < 4; ++indice) {
    const auto posicion = 0x00a6 + indice * 10;
    escribir(datos, posicion, registros[indice]);
    escribir(datos, posicion + 2, 0x321f);
    escribir(datos, posicion + 4, 0x1a2f);
    // Los campos +6 y +8 se conservan, igual que en las instrucciones originales.
  }
  escribir(datos, 0x00ce, 0x2d12);
  escribir(datos, 0x00d0, 0x0111);
  escribir(datos, 0x00d2, 0xab60);
  escribir(datos, 0x00d6, 0xab63);
}

}
