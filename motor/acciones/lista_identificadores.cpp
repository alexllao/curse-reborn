#include "lista_identificadores.hpp"
#include "../estado/segmento.hpp"

namespace motor {

bool contiene_identificador(const unsigned char *datos, unsigned short identificador) {
  // 0000:88BC–88D8. La búsqueda no se detiene en FFFFh.
  for (unsigned int i = 0; i < 10; ++i) {
    if (palabra(datos, static_cast<Palabra>(0x5225 + i * 2)) == identificador) {
      return true;
    }
  }
  return false;
}

bool insertar_identificador(unsigned char *datos, unsigned short identificador) {
  // 0000:88D9–88F2. Permite duplicados y conserva el resto de la lista.
  for (unsigned int i = 0; i < 10; ++i) {
    const auto posicion = static_cast<Palabra>(0x5225 + i * 2);
    if (palabra(datos, posicion) == 0xffff) {
      guardar(datos, posicion, identificador);
      return true;
    }
  }
  return false;
}

bool retirar_identificador(unsigned char *datos, unsigned short identificador) {
  // 0000:88F3–891D. Retira solo la primera coincidencia.
  for (unsigned int i = 0; i < 10; ++i) {
    if (palabra(datos, static_cast<Palabra>(0x5225 + i * 2)) != identificador) {
      continue;
    }
    for (auto j = i + 1; j < 10; ++j) {
      guardar(datos, static_cast<Palabra>(0x5225 + (j - 1) * 2),
              palabra(datos, static_cast<Palabra>(0x5225 + j * 2)));
    }
    guardar(datos, 0x5237, 0xffff);
    return true;
  }
  return false;
}

} // namespace motor
