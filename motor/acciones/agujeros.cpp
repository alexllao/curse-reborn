#include "agujeros.hpp"
#include "iniciar_accion.hpp"
#include "lista_identificadores.hpp"
#include "../estado/segmento.hpp"
namespace motor {
namespace {
// 886B: conserva el recorrido y los bloqueos del planificador original.
ResultadoMenu comenzar(unsigned char *d) {
  unsigned short pasos = 0;
  return iniciar_accion(d, pasos) ? ResultadoMenu::accion : ResultadoMenu::error;
}
// 6260–6284: repone las cuatro ofertas del pozo cuando se conserva otro material de la máscara.
void reponer_ofertas(unsigned char *d) {
  const auto lista = palabra(d, 0x53c7);
  d[static_cast<Palabra>(lista + 1)] = 0;
  for (unsigned int i = 1; i < 5; ++i)
    d[static_cast<Palabra>(lista + i * 9 + 1)] = 3;
}
} // namespace

// Mantiene las respuestas, animaciones y condiciones de cada agujero, no una secuencia inventada.
ResultadoMenu ejecutar_interaccion_agujeros(unsigned char *d, unsigned short manejador,
                                            unsigned short id, const ServicioSonido &sonido) {
  if (palabra(d, 0x8082) != 14)
    return ResultadoMenu::pendiente;
  if (manejador == 0x384a && id >= 65 && id <= 68) {
    guardar(d, 0x2c4e, palabra(d, 0x2c2d));
    guardar(d, 0x2c50, static_cast<Palabra>(palabra(d, 0x2c2f) - 30));
    if (id != 68)
      guardar(d, 0x2c5c, 0x1da8);
    d[0x2c5e] = 0;
    d[0x2c5f] = 1;
    d[0x2c60] = 10;
    const auto agujero = static_cast<Palabra>(0x2cf1 + (id - 65) * 33);
    guardar(d, 0x1a38, static_cast<Palabra>(palabra(d, agujero + 2) + (id == 68 ? 4 : 19)));
    guardar(d, 0x1a3a, static_cast<Palabra>(palabra(d, agujero + 4) + 42));
    guardar(d, 0x0ad7, id == 65 ? 0x1acf : id == 68 ? 0x1ae1 : 0xa018);
    guardar(d, 0x0adc, id == 65 || id == 68 ? 0x1af3 : 0xa02a);
    if (id == 65) {
      if (d[0x2c8e] & 2) {
        guardar(d, 0x2c9e, 0x1cf6);
        guardar(d, 0x2ca3, 0x1d67);
        guardar(d, 0x2ca8, 0x1cb5);
      }
      return comenzar(d);
    }
    const auto resultado = comenzar(d);
    d[0x3596] = 1;
    if (id == 68) {
      if (!(d[0x2c6d] & 2) && !(d[0x2c8e] & 2) && !(d[0x2caf] & 2) && (d[0x2cd0] & 1)) {
        guardar(d, 0x2c5c, 0x1dac);
        d[0x524d + 64 * 2] |= 1;
        d[0x2cd0] |= 4;
      } else
        guardar(d, 0x2c5c, 0x1da8);
    } else {
      const auto criatura = static_cast<Palabra>(id == 66 ? 0x2c6d : 0x2caf);
      if (d[criatura] & 2) {
        guardar(d, criatura + 16, id == 66 ? 0x1c03 : 0x1cf6);
        guardar(d, criatura + 21, id == 66 ? 0x1c74 : 0x1d67);
        guardar(d, criatura + 26, 0x1cb5);
      }
    }
    return resultado;
  }
  if (manejador == 0x563a && id == 64) {
    if (!insertar_identificador(d, id)) {
      marcar_accion_rechazada(d);
      return ResultadoMenu::accion;
    }
    if (contiene_identificador(d, 37) || contiene_identificador(d, 72))
      reponer_ofertas(d);
    d[0x3f11] = d[0x2cd0] = 0;
    guardar(d, 0x1a38, static_cast<Palabra>(palabra(d, 0x2d56) + 4));
    guardar(d, 0x1a3a, static_cast<Palabra>(palabra(d, 0x2d58) + 42));
    guardar(d, 0x0ad7, 0x1ae1);
    guardar(d, 0x0adc, 0x1af3);
    const auto resultado = comenzar(d);
    d[0x3596] = 1;
    return resultado;
  }
  if (manejador == 0x32de && id <= 1)
    return emitir_sonido(sonido, d, 0x2c2b, static_cast<unsigned char>(id)) ? ResultadoMenu::accion
                                                                            : ResultadoMenu::error;
  if (id == 3 || (id >= 27 && id <= 74)) {
    if (manejador == 0x384a)
      return ResultadoMenu::actualizado;
    if (manejador == 0x2fca || manejador == 0x3143) {
      marcar_accion_rechazada(d);
      return ResultadoMenu::accion;
    }
  }
  return ResultadoMenu::pendiente;
}
} // namespace motor
