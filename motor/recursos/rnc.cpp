#include "rnc.hpp"

namespace motor {
namespace {
using Palabra = unsigned short;
struct Entrada { Palabra mascara, codigo; unsigned char simbolo, bits; };
struct Tabla { Entrada entradas[16]; unsigned int cantidad = 0; };

// 66CE–672D. Dos palabras de ventana y los bits disponibles de la palabra adelantada.
struct Bits {
  const unsigned char* datos;
  unsigned int longitud;
  unsigned long long posicion = 18;
  Palabra bajo = 0, alto = 0;
  unsigned int disponibles = 0;
  bool valido = true;

  Palabra palabra(unsigned long long p) {
    if (p > longitud && p - longitud > 2u) { valido = false; return 0; }
    const auto a = p < longitud ? datos[p] : 0;
    const auto b = p + 1 < longitud ? datos[p + 1] : 0;
    return static_cast<Palabra>(a | (b << 8));
  }
  void desplazar(unsigned int n) {
    if (n == 0) return;
    const unsigned int mascara = (1u << n) - 1;
    bajo = static_cast<Palabra>((static_cast<unsigned int>(bajo) >> n) |
                               ((alto & mascara) << (16 - n)));
    alto = static_cast<Palabra>(static_cast<unsigned int>(alto) >> n);
  }
  Palabra tomar(unsigned int n) {
    if (n > 16) { valido = false; return 0; }
    const auto resultado = static_cast<Palabra>(bajo & ((1u << n) - 1));
    if (n > disponibles) {
      const auto resto = n - disponibles;
      desplazar(disponibles);
      posicion += 2; alto = palabra(posicion);
      desplazar(resto); disponibles = 16 - resto;
    } else { disponibles -= n; desplazar(n); }
    return resultado;
  }
  // 65F4–661A: los literales comparten el flujo con las palabras de bits adelantadas.
  void despues_de_literales(unsigned int cantidad) {
    posicion += cantidad;
    const auto a = palabra(posicion), b = palabra(posicion + 2);
    const auto mascara = (1u << disponibles) - 1;
    const auto rotada = disponibles == 0 ? a :
      static_cast<Palabra>((static_cast<unsigned int>(a) << disponibles) | (a >> (16 - disponibles)));
    bajo = static_cast<Palabra>((bajo & mascara) | (static_cast<unsigned int>(a) << disponibles));
    alto = static_cast<Palabra>((static_cast<unsigned int>(b) << disponibles) | (rotada & mascara));
  }
};

bool construir(Bits& bits, Tabla& tabla) {
  const auto cantidad = bits.tomar(5);
  if (cantidad == 0) return bits.valido; // El original conserva la tabla anterior.
  if (cantidad > 16) return false;
  unsigned char longitudes[16];
  for (unsigned int i = 0; i < cantidad; ++i) longitudes[i] = static_cast<unsigned char>(bits.tomar(4));
  tabla.cantidad = 0;
  unsigned int codigo = 0;
  for (unsigned int longitud = 1; longitud <= 16; ++longitud) {
    for (unsigned int simbolo = 0; simbolo < cantidad; ++simbolo) {
      if (longitudes[simbolo] != longitud) continue;
      unsigned int valor = codigo >> (16 - longitud), invertido = 0;
      for (unsigned int i = 0; i < longitud; ++i) { invertido = (invertido << 1) | (valor & 1); valor >>= 1; }
      tabla.entradas[tabla.cantidad++] = {static_cast<Palabra>((1u << longitud) - 1),
        static_cast<Palabra>(invertido), static_cast<unsigned char>(simbolo), static_cast<unsigned char>(longitud)};
      codigo += 1u << (16 - longitud);
      if (codigo > 65536u) return false;
    }
  }
  return bits.valido && tabla.cantidad != 0;
}

bool valor(Bits& bits, const Tabla& tabla, unsigned int& resultado) {
  for (unsigned int i = 0; i < tabla.cantidad; ++i) {
    const auto& entrada = tabla.entradas[i];
    if ((bits.bajo & entrada.mascara) != entrada.codigo) continue;
    bits.tomar(entrada.bits);
    resultado = entrada.simbolo;
    if (resultado >= 2) {
      const auto extras = resultado - 1;
      resultado = bits.tomar(extras) | (1u << extras);
    }
    return bits.valido;
  }
  return false;
}

unsigned int grande(const unsigned char* datos) {
  return (static_cast<unsigned int>(datos[0]) << 24) | (static_cast<unsigned int>(datos[1]) << 16) |
         (static_cast<unsigned int>(datos[2]) << 8) | datos[3];
}
}

ResultadoRnc descomprimir_rnc1(const unsigned char* origen, unsigned int longitud,
                             unsigned char* destino, unsigned int capacidad) {
  if (longitud < 4 || origen[0] != 'R' || origen[1] != 'N' || origen[2] != 'C') return {EstadoRnc::sin_compresion, 0};
  if (origen[3] != 1) return {EstadoRnc::metodo_no_admitido, 0};
  if (longitud < 18) return {EstadoRnc::entrada_invalida, 0};
  const auto esperados = grande(origen + 4), empaquetados = grande(origen + 8);
  if (empaquetados > longitud - 18 || empaquetados < 2 || esperados > capacidad || origen[17] == 0) {
    return {EstadoRnc::entrada_invalida, 0};
  }
  Bits bits{origen, empaquetados + 18};
  bits.bajo = bits.palabra(18); bits.tomar(2);
  Tabla literales, distancias, repeticiones;
  unsigned int escritos = 0;
  for (unsigned int bloque = 0; bloque < origen[17]; ++bloque) {
    if (!construir(bits, literales) || !construir(bits, distancias) || !construir(bits, repeticiones)) {
      return {EstadoRnc::entrada_invalida, escritos};
    }
    const auto grupos = bits.tomar(16);
    if (grupos == 0) return {EstadoRnc::entrada_invalida, escritos};
    for (unsigned int grupo = 0; grupo < grupos; ++grupo) {
      if (grupo != 0) {
        unsigned int distancia = 0, cantidad = 0;
        if (!valor(bits, distancias, distancia) || !valor(bits, repeticiones, cantidad)) return {EstadoRnc::entrada_invalida, escritos};
        ++distancia; cantidad += 2;
        if (distancia > escritos || cantidad > esperados - escritos) return {EstadoRnc::entrada_invalida, escritos};
        for (unsigned int i = 0; i < cantidad; ++i) { destino[escritos] = destino[escritos - distancia]; ++escritos; }
      }
      unsigned int cantidad = 0;
      if (!valor(bits, literales, cantidad) || cantidad > esperados - escritos ||
          bits.posicion > bits.longitud || cantidad > bits.longitud - bits.posicion) return {EstadoRnc::entrada_invalida, escritos};
      for (unsigned int i = 0; i < cantidad; ++i) destino[escritos++] = origen[bits.posicion + i];
      if (cantidad != 0) bits.despues_de_literales(cantidad);
    }
  }
  return {bits.valido && escritos == esperados ? EstadoRnc::correcto : EstadoRnc::entrada_invalida, escritos};
}

}
