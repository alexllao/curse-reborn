#include "objetos_cercanos.hpp"
#include "../estado/segmento.hpp"

namespace motor {
// Traduce las comparaciones con signo de 891E y los bits de visibilidad y afinidad de cada objeto.
ObjetosCercanos buscar_objetos_cercanos(unsigned char *datos, unsigned char mascara,
                                        unsigned short destino, unsigned short capacidad) {
  if (capacidad > 68)
    return {false, 0};
  const auto izquierda = static_cast<Palabra>(palabra(datos, 0x2c2d) - 5);
  const auto derecha =
      static_cast<Palabra>(palabra(datos, 0x2c2d) + palabra(datos, 0x2c31) - 1 + 5);
  const auto pies = static_cast<Palabra>(palabra(datos, 0x2c2f) + palabra(datos, 0x2c33) - 1);
  const auto arriba = static_cast<Palabra>(pies - 3 - 4), abajo = static_cast<Palabra>(pies + 4);
  Palabra cantidad = 0;
  for (unsigned int i = 0; i < 68; ++i) {
    const auto entidad = static_cast<Palabra>(0x2c4c + i * 33);
    const auto estado = datos[entidad];
    if (estado == 0 || ((estado & 4) == 0 && (estado & 2)))
      continue;
    const auto tipo = palabra(datos, static_cast<Palabra>(entidad + 14));
    if (tipo == 0xffff || con_signo(tipo) >= 0x400)
      continue;
    if ((mascara & datos[static_cast<Palabra>(0x524d + static_cast<Palabra>(tipo * 2))]) == 0)
      continue;
    if (datos[0x3593] == 0) {
      const auto x = palabra(datos, static_cast<Palabra>(entidad + 2));
      const auto y = static_cast<Palabra>(palabra(datos, static_cast<Palabra>(entidad + 4)) -
                                          palabra(datos, static_cast<Palabra>(entidad + 12)));
      if (con_signo(x) > con_signo(derecha) ||
          con_signo(static_cast<Palabra>(x + palabra(datos, static_cast<Palabra>(entidad + 6)))) <=
              con_signo(izquierda) ||
          con_signo(y) > con_signo(abajo) ||
          con_signo(static_cast<Palabra>(y + palabra(datos, static_cast<Palabra>(entidad + 8)) +
                                         palabra(datos, static_cast<Palabra>(entidad + 12)) * 2)) <=
              con_signo(arriba))
        continue;
    }
    if (cantidad == capacidad)
      return {false, cantidad};
    guardar(datos, static_cast<Palabra>(destino + cantidad * 2), static_cast<Palabra>(tipo + 0x1a));
    ++cantidad;
  }
  for (Palabra i = cantidad; i < capacidad; ++i)
    guardar(datos, static_cast<Palabra>(destino + i * 2), 0xffff);
  return {true, cantidad};
}
} // namespace motor
