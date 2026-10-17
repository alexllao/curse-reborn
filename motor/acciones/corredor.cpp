#include "corredor.hpp"
#include "iniciar_accion.hpp"
#include "lista_identificadores.hpp"
#include "../estado/segmento.hpp"

namespace motor {
namespace {
// 886B: dispone la aproximación y entrega al intérprete las animaciones originales de Brad.
ResultadoMenu comenzar(unsigned char *d) {
  unsigned short pasos = 0;
  return iniciar_accion(d, pasos) ? ResultadoMenu::accion : ResultadoMenu::error;
}
// 623D–625F: desactiva la entrada persistente de la escena, conservando sus otros ocho bytes.
void retirar_descriptor(unsigned char *d, Palabra entidad) {
  const auto escena = palabra(d, 0x8082);
  const auto lista = palabra(d, static_cast<Palabra>(0x53b5 + escena * 2));
  const auto indice = static_cast<Palabra>((entidad - 0x2c0a) / 33 - 2);
  d[static_cast<Palabra>(lista + indice * 9 + 1)] = 0;
}
// 615C–61A6: conserva la fase de recogida hasta que Brad termina de aproximarse al objeto.
ResultadoMenu recoger_suelo(unsigned char *d, Palabra entidad, Palabra cambio_y) {
  guardar(d, 0x1a38, static_cast<Palabra>(palabra(d, entidad + 2) + 5));
  guardar(d, 0x1a3a,
          static_cast<Palabra>(palabra(d, entidad + 4) + palabra(d, entidad + 8) + cambio_y));
  retirar_descriptor(d, entidad);
  guardar(d, entidad + 16, 0x1a2e);
  d[entidad + 18] = 0;
  d[entidad + 20] = 1;
  guardar(d, 0x0ad7, d[0x358c] == 0x14 ? 0x96f6 : 0x96c0);
  guardar(d, 0x0adc, d[0x358c] == 0x14 ? 0x96ff : 0x96c9);
  unsigned short pasos = 0;
  if (!iniciar_accion(d, pasos))
    return ResultadoMenu::error;
  d[entidad + 19] = static_cast<unsigned char>(pasos + 6);
  return ResultadoMenu::accion;
}
} // namespace

// Los identificadores se validan por escena: no se ejecutan acciones de la celda sobre otras
// entidades.
ResultadoMenu ejecutar_interaccion_corredor(unsigned char *d, unsigned short manejador,
                                            unsigned short id, const ServicioSonido &sonido) {
  if (palabra(d, 0x8082) != 1)
    return ResultadoMenu::pendiente;
  if (manejador == 0x32de && id <= 1)
    return emitir_sonido(sonido, d, 0x2c2b, static_cast<unsigned char>(id)) ? ResultadoMenu::accion
                                                                            : ResultadoMenu::error;
  if ((manejador == 0x2fca || manejador == 0x3143) && id <= 14) {
    marcar_accion_rechazada(d);
    return ResultadoMenu::accion;
  }
  if (manejador == 0x384a && id <= 14) {
    if (id != 8)
      return ResultadoMenu::actualizado;
    // 3858–389E: palanca de la puerta de regreso, con activación diferida de sus dos entidades.
    guardar(d, 0x1a38, palabra(d, 0x2c90));
    guardar(d, 0x1a3a, 160);
    guardar(d, 0x0ad7, 0x1aa1);
    guardar(d, 0x0adc, 0x1aac);
    guardar(d, 0x009c, 0x2c8e);
    guardar(d, 0x009e, 0x0209);
    guardar(d, 0x00a0, 0x1be0);
    guardar(d, 0x00a2, 0x1be9);
    guardar(d, 0x00a4, 0x1bf1);
    guardar(d, 0x00a6, 0x2caf);
    guardar(d, 0x00a8, 0x0213);
    guardar(d, 0x00aa, 0x1bd1);
    return comenzar(d);
  }
  if (manejador != 0x563a || (id != 6 && (id < 9 || id > 14)))
    return ResultadoMenu::pendiente;
  // 5652–5679: las monedas suman puntuación; la pecera necesita una casilla de inventario.
  const auto icono = d[0x524e + id * 2];
  if (icono == 0x1f || icono == 0x22)
    guardar(d, 0x35c7, static_cast<Palabra>(palabra(d, 0x35c7) + (icono == 0x1f ? 1 : 2)));
  else if (!insertar_identificador(d, id)) {
    marcar_accion_rechazada(d);
    return ResultadoMenu::accion;
  }
  if (id == 6) {
    // 56DC–574D: guarda también el agua y desactiva los dos descriptores del conjunto de la pecera.
    insertar_identificador(d, 7);
    d[0x3690] = 0;
    guardar(d, 0x2ce5, 0x97a2);
    d[0x2ce7] = d[0x2cec] & 3;
    d[0x2ce8] = d[0x2ce3];
    d[0x2ce9] = d[0x2ce4];
    d[0x366c] = d[0x3675] = 0;
    guardar(d, 0x2c5c, 0x1a2e);
    d[0x2c5e] = 0;
    d[0x2c5f] = 6;
    guardar(d, 0x2c7d, 0x1a2e);
    d[0x2c7f] = 0;
    d[0x2c80] = 6;
    guardar(d, 0x1a38, static_cast<Palabra>(palabra(d, 0x2c4e) + 11));
    guardar(d, 0x1a3a, 171);
    guardar(d, 0x0ad7, 0x96de);
    guardar(d, 0x0adc, 0x96e7);
    return comenzar(d);
  }
  if (id == 9) {
    // 574E–577A: bolsa situada por encima del suelo, con desaparición inmediata.
    d[0x36a2] = d[0x2d12] = 0;
    guardar(d, 0x1a38, palabra(d, 0x2d14));
    guardar(d, 0x1a3a, static_cast<Palabra>(palabra(d, 0x2d16) + 44));
    guardar(d, 0x0ad7, 0x96de);
    guardar(d, 0x0adc, 0x96e7);
    return comenzar(d);
  }
  const auto entidad = static_cast<Palabra>(0x2d33 + (id - 10) * 33);
  if (id == 13) {
    // 57AE–57DF: moneda alta que se retira antes del gesto lateral de recogida.
    retirar_descriptor(d, entidad);
    d[entidad] = 0;
    guardar(d, 0x1a38, static_cast<Palabra>(palabra(d, entidad + 2) + 5));
    guardar(d, 0x1a3a,
            static_cast<Palabra>(palabra(d, entidad + 4) + palabra(d, entidad + 8) + 12));
    guardar(d, 0x0ad7, 0x96cf);
    guardar(d, 0x0adc, 0x96d8);
    return comenzar(d);
  }
  return recoger_suelo(d, entidad, id == 12 ? 0xfff9 : 0xfffe);
}
} // namespace motor
