#include "mazmorra.hpp"
#include "../estado/segmento.hpp"

namespace motor {
namespace {
// Prepara referencia, fase, contador e intervalo sin alterar el resto del registro.
void configurar_secuencia(unsigned char *datos, Palabra posicion, Palabra referencia,
                          unsigned char contador, unsigned char intervalo) {
  guardar(datos, posicion, referencia);
  datos[static_cast<Palabra>(posicion + 2)] = 0;
  datos[static_cast<Palabra>(posicion + 3)] = contador;
  datos[static_cast<Palabra>(posicion + 4)] = intervalo;
}
} // namespace

// Conserva las condiciones y coordenadas de 183F, incluidas las sumas de palabra con envoltura.
void actualizar_mazmorra(unsigned char *datos) {
  if (datos[0x2cf1] == 0 || palabra(datos, 0x2d01) != 0 || datos[0x3594] == 0)
    return;
  guardar(datos, 0x2cf3, static_cast<Palabra>(palabra(datos, 0x2d14) + 0x79));
  guardar(datos, 0x2cf5, static_cast<Palabra>(palabra(datos, 0x2d16) + 0x19));
  configurar_secuencia(datos, 0x2d01, 0x1211, 1, 2);
  configurar_secuencia(datos, 0x2d06, 0x124b, 1, 2);
  configurar_secuencia(datos, 0x2d0b, 0x1280, 1, 2);
  if (datos[0x2d54] != 1)
    return;
  datos[0x3ac7] = 0;
  datos[0x3af4] = 1;
  guardar(datos, 0x2d01, 0x12b5);
  configurar_secuencia(datos, 0x2da6, 0x1a2c, 0x35, 2);
  configurar_secuencia(datos, 0x2d85, 0x12ee, 0x2f, 2);
  configurar_secuencia(datos, 0x2d8f, 0x12f7, 0x2f, 2);
}
} // namespace motor
