#include "avanzar.hpp"
#include "../estado/segmento.hpp"

namespace motor {

bool avanzar_movimiento(unsigned char* datos) {
  const auto restantes = palabra(datos, 0x359a);
  if (restantes == 0) return false;
  const auto fraccion_x = static_cast<unsigned int>(palabra(datos, 0x359e)) + palabra(datos, 0x35a6);
  guardar(datos, 0x359e, static_cast<Palabra>(fraccion_x));
  guardar(datos, 0x2c2d, static_cast<Palabra>(palabra(datos, 0x2c2d) +
    static_cast<unsigned int>(palabra(datos, 0x35a8)) + (fraccion_x > 0xffff ? 1 : 0)));
  const auto fraccion_y = static_cast<unsigned int>(palabra(datos, 0x35a2)) + palabra(datos, 0x35aa);
  guardar(datos, 0x35a2, static_cast<Palabra>(fraccion_y));
  guardar(datos, 0x2c2f, static_cast<Palabra>(palabra(datos, 0x2c2f) +
    static_cast<unsigned int>(palabra(datos, 0x35ac)) + (fraccion_y > 0xffff ? 1 : 0)));
  const auto siguiente = static_cast<Palabra>(restantes - 1);
  guardar(datos, 0x359a, siguiente);
  return siguiente != 0;
}

}
