#include "mascara.hpp"
#include "iniciar_accion.hpp"
#include "lista_identificadores.hpp"
#include "../estado/segmento.hpp"
namespace motor {
// A1D4–A382: conserva las seis recetas, sus condiciones de modelo/control y la máscara final.
ResultadoMenu ejecutar_interaccion_mascara(unsigned char *d, unsigned short manejador) {
  if (manejador == 0xa2d8) {
    if (d[0x358c] != 14) {
      marcar_accion_rechazada(d);
      return ResultadoMenu::accion;
    }
    constexpr Palabra objetos[] = {74, 1,  27, 34, 39, 41, 43, 46, 28, 30,
                                   32, 35, 40, 45, 29, 31, 33, 38, 42, 44};
    for (const auto objeto : objetos)
      retirar_identificador(d, objeto);
    guardar(d, 0x2c3b, 0x9ff8);
    guardar(d, 0x2c45, 0xa00a);
    guardar(d, 0x2c66, 0xa016);
    d[0x2c68] = 0;
    d[0x2c69] = 21;
    d[0x2c6a] = 2;
  } else {
    constexpr Palabra recetas[][4] = {{0xa1d4, 37, 64, 71}, {0xa1fa, 37, 49, 72},
                                      {0xa220, 64, 49, 73}, {0xa245, 49, 71, 74},
                                      {0xa26a, 64, 72, 74}, {0xa28f, 37, 73, 74}};
    const Palabra *receta = nullptr;
    for (const auto &entrada : recetas)
      if (entrada[0] == manejador)
        receta = entrada;
    if (!receta)
      return ResultadoMenu::pendiente;
    if (d[0x358c] != 14 && !d[0x3594]) {
      marcar_accion_rechazada(d);
      return ResultadoMenu::accion;
    }
    retirar_identificador(d, receta[1]);
    retirar_identificador(d, receta[2]);
    insertar_identificador(d, receta[3]);
    guardar(d, 0x2c3b, d[0x358c] == 255 ? 0x9fd6 : 0x9fed);
    if (d[0x358c] == 255)
      guardar(d, 0x2c40, 0x9fe3);
  }
  guardar(d, 0x35c7, static_cast<Palabra>(palabra(d, 0x35c7) + 5));
  unsigned short pasos = 0;
  return iniciar_accion(d, pasos) ? ResultadoMenu::accion : ResultadoMenu::error;
}
} // namespace motor
