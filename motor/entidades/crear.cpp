#include "actualizar.hpp"
#include "../estado/segmento.hpp"

namespace motor {

CreacionEntidad crear_entidad(unsigned char* datos, EstadoAleatorio& estado,
                              const RecursosEntidades& recursos, unsigned short descriptor) {
  const auto b = [datos, descriptor](Palabra desplazamiento) {
    return datos[static_cast<Palabra>(descriptor + desplazamiento)];
  };
  const auto p = [datos, descriptor](Palabra desplazamiento) {
    return palabra(datos, static_cast<Palabra>(descriptor + desplazamiento));
  };
  if (datos[0x0ad6] == 0 && (b(0) & 0x80) != 0) return {true, false, 0};
  const auto reservadas = palabra(datos, 0x3588);
  if (reservadas >= 70) return {false, false, 0};
  Palabra destino = static_cast<Palabra>(0x2c0a + datos[0x3588] * 33);
  bool encontrada = false;
  for (unsigned int i = reservadas; i < 70; ++i) {
    if (datos[destino] == 0) { encontrada = true; break; }
    destino = static_cast<Palabra>(destino + 33);
  }
  if (!encontrada) return {true, false, 0};
  Palabra ancho = 0, alto = 0;
  const auto indice = static_cast<Palabra>(b(2) & 0x7f);
  if (!recursos.dimensiones || !recursos.dimensiones(recursos.contexto, indice, ancho, alto)) {
    return {false, false, destino};
  }
  Palabra di = destino;
  const auto escribir_byte = [datos, &di](unsigned char valor) {
    datos[di] = valor; di = static_cast<Palabra>(di + 1);
  };
  const auto escribir_palabra = [datos, &di](Palabra valor) {
    guardar(datos, di, valor); di = static_cast<Palabra>(di + 2);
  };
  escribir_byte(b(1)); escribir_byte(b(2));
  escribir_palabra(static_cast<Palabra>(p(3) - palabra(datos, 0x99b8))); escribir_palabra(p(5));
  escribir_palabra(ancho); escribir_palabra(alto);
  Palabra origen = p(7);
  const auto copiar = [&]() {
    const auto valor = palabra(datos, origen);
    escribir_palabra(valor); origen = static_cast<Palabra>(origen + 2);
  };
  for (int i = 0; i < 4; ++i) copiar();
  escribir_byte(0); copiar(); copiar();
  escribir_byte(0); copiar(); copiar();
  escribir_byte(0); copiar();
  escribir_palabra(recursos.segmento_original);
  Palabra ax = recursos.segmento_original;
  if ((b(0) & 2) != 0) {
    Palabra secuencia = palabra(datos, static_cast<Palabra>(destino + 0x10));
    unsigned int cantidad = 0;
    bool terminador = false;
    while (cantidad < 65535) {
      ++cantidad;
      const auto valor = datos[secuencia];
      secuencia = static_cast<Palabra>(secuencia + 1);
      if (valor == 0x7f) { terminador = true; break; }
    }
    if (!terminador || !generar_aleatorio(estado, static_cast<Palabra>(cantidad), ax)) {
      return {false, true, destino};
    }
    datos[static_cast<Palabra>(destino + 0x12)] = static_cast<unsigned char>(ax);
    datos[static_cast<Palabra>(destino + 0x17)] = static_cast<unsigned char>(ax);
    datos[static_cast<Palabra>(destino + 0x1c)] = static_cast<unsigned char>(ax);
  }
  if ((b(0) & 4) != 0) {
    // MOV AL,[DI+14h] conserva AH: el divisor original no es necesariamente un escribir_byte.
    ax = static_cast<Palabra>((ax & 0xff00) | datos[static_cast<Palabra>(destino + 0x14)]);
    if (!generar_aleatorio(estado, ax, ax)) return {false, true, destino};
    datos[static_cast<Palabra>(destino + 0x13)] = static_cast<unsigned char>(ax);
    datos[static_cast<Palabra>(destino + 0x18)] = static_cast<unsigned char>(ax);
    datos[static_cast<Palabra>(destino + 0x1d)] = static_cast<unsigned char>(ax);
  }
  return {true, true, destino};
}

}
