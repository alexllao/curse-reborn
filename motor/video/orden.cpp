#include "orden.hpp"
#include "../estado/segmento.hpp"

namespace motor {
// Traslada la ordenación por altura, tamaño y profundidad, incluidas sus igualdades y envolturas.
void ordenar_entidades(unsigned char *datos) {
  for (unsigned int i = 0; i < 69; ++i) {
    const auto entrada = static_cast<Palabra>(0xbdd2 + i * 4);
    const auto entidad = palabra(datos, static_cast<Palabra>(entrada + 2));
    guardar(datos, entrada,
            static_cast<Palabra>(palabra(datos, static_cast<Palabra>(entidad + 4)) +
                                 palabra(datos, static_cast<Palabra>(entidad + 8)) +
                                 palabra(datos, static_cast<Palabra>(entidad + 10))));
  }
  unsigned int salto = 69;
  bool intercambiado = false;
  do {
    if (salto > 1) {
      salto = salto * 3 / 4;
      if (salto == 9 || salto == 10)
        salto = 11;
    }
    intercambiado = false;
    for (unsigned int i = 0; i + salto < 69; ++i) {
      const auto a = static_cast<Palabra>(0xbdd2 + i * 4);
      const auto b = static_cast<Palabra>(a + salto * 4);
      if (con_signo(palabra(datos, a)) <= con_signo(palabra(datos, b)))
        continue;
      const auto clave = palabra(datos, a), entidad = palabra(datos, static_cast<Palabra>(a + 2));
      guardar(datos, a, palabra(datos, b));
      guardar(datos, static_cast<Palabra>(a + 2), palabra(datos, static_cast<Palabra>(b + 2)));
      guardar(datos, b, clave);
      guardar(datos, static_cast<Palabra>(b + 2), entidad);
      intercambiado = true;
    }
  } while (salto > 1 || intercambiado);
}
} // namespace motor
