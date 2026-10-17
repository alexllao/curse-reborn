#include "asaltante.hpp"
#include "iniciar_accion.hpp"
#include "lista_identificadores.hpp"
#include "../estado/segmento.hpp"
namespace motor {
namespace {
// Prepara un canal sin cambiar la demora hasta conocer los pasos calculados por 886B.
void canal(unsigned char *d, Palabra posicion, Palabra secuencia, unsigned char intervalo) {
  guardar(d, posicion, secuencia);
  d[posicion + 2] = 0;
  d[posicion + 4] = intervalo;
}
// 623D–625F: desactiva la oferta persistente asociada al registro de entidad que se recoge.
void retirar_oferta(unsigned char *d, Palabra entidad) {
  const auto indice = static_cast<Palabra>((entidad - 0x2c0a) / 33 - 2);
  const auto lista = palabra(d, static_cast<Palabra>(0x53b5 + palabra(d, 0x8082) * 2));
  d[static_cast<Palabra>(lista + indice * 9 + 1)] = 0;
}
} // namespace

// COUNTRY2 conserva los mismos identificadores 75 (espada) y 76 (dinero) del EXE.
ResultadoMenu ejecutar_interaccion_asaltante(unsigned char *d, unsigned short manejador,
                                             unsigned short id, const ServicioSonido &sonido,
                                             const ServicioCreacion &creacion) {
  if (palabra(d, 0x8082) != 16)
    return ResultadoMenu::pendiente;
  if (manejador == 0x563a && (id == 75 || id == 76)) {
    // 563A–5679: la falta de sitio rechaza la recogida antes de tocar actor y oferta.
    const auto icono = d[0x524e + id * 2];
    if (icono == 0x1f || icono == 0x22)
      guardar(d, 0x35c7, static_cast<Palabra>(palabra(d, 0x35c7) + (icono == 0x1f ? 1 : 2)));
    else if (!insertar_identificador(d, id)) {
      marcar_accion_rechazada(d);
      return ResultadoMenu::accion;
    }
    unsigned short pasos = 0;
    if (id == 75) {
      // 5C44–5C8C: Brad coge la espada mientras el asaltante conserva su pose caída.
      guardar(d, 0x2c56, 0xffe7);
      guardar(d, 0x2c77, 0);
      canal(d, 0x2c7d, 0x1a2e, 1);
      guardar(d, 0x1a38, static_cast<Palabra>(palabra(d, 0x2c6f) + 4));
      guardar(d, 0x1a3a, static_cast<Palabra>(palabra(d, 0x2c71) + palabra(d, 0x2c75)));
      guardar(d, 0x0ad7, 0x9771);
      guardar(d, 0x0adc, 0x977f);
      if (!iniciar_accion(d, pasos))
        return ResultadoMenu::error;
      d[0x2c80] = static_cast<unsigned char>(pasos + 6);
    } else {
      // 5C8F–5CAC y 61A7–61F0: desactiva el dinero tanto en la escena como en su lista.
      d[0x4132] = 0;
      guardar(d, 0x1a38, static_cast<Palabra>(palabra(d, 0x2c90) + 5));
      guardar(d, 0x1a3a, static_cast<Palabra>(palabra(d, 0x2c92) + palabra(d, 0x2c96) + 1));
      retirar_oferta(d, 0x2c8e);
      canal(d, 0x2c9e, 0x1a2e, 1);
      guardar(d, 0x0ad7, d[0x358c] == 20 ? 0x9705 : 0x96cf);
      guardar(d, 0x0adc, d[0x358c] == 20 ? 0x970e : 0x96d8);
      if (!iniciar_accion(d, pasos))
        return ResultadoMenu::error;
      d[0x2ca1] = static_cast<unsigned char>(pasos + 6);
    }
    return ResultadoMenu::accion;
  }
  if (manejador == 0x2fca && id == 75) {
    if (!creacion.crear || !creacion.azar)
      return ResultadoMenu::error;
    // 300B–3099: usar la espada inicia el ataque y consume su objeto de inventario.
    d[0x3fc0] = d[0x3fc9] = 0;
    retirar_identificador(d, 75);
    guardar(d, 0x2c90, static_cast<Palabra>(palabra(d, 0x2c4e) + 18));
    guardar(d, 0x2c92, static_cast<Palabra>(palabra(d, 0x2c50) + 4));
    d[0x2c8e] &= 0xfd;
    guardar(d, 0x1a38, static_cast<Palabra>(palabra(d, 0x2c4e) + 74));
    guardar(d, 0x1a3a, static_cast<Palabra>(palabra(d, 0x2c50) + 25));
    guardar(d, 0x0ad7, 0x1afe);
    guardar(d, 0x0adc, 0x1b13);
    guardar(d, 0x0ae1, 0x1b21);
    canal(d, 0x2c5c, 0x1db4, 2);
    canal(d, 0x2c61, 0x1db8, 2);
    canal(d, 0x2c66, 0x1dbc, 2);
    unsigned short pasos = 0;
    if (!iniciar_accion(d, pasos))
      return ResultadoMenu::error;
    const auto demora = static_cast<unsigned char>(pasos + 18);
    d[0x2c5f] = d[0x2c64] = d[0x2c69] = demora;
    // 309B–30CD: cuarenta fragmentos con tres consumos explícitos de azar por fragmento.
    const auto secuencia = palabra(d, 0x1dc7);
    for (unsigned int i = 0; i < 40; ++i) {
      Palabra valor = 0;
      if (!generar_aleatorio(*creacion.azar, 30, valor))
        return ResultadoMenu::error;
      guardar(d, 0x1dc3, static_cast<Palabra>(valor + 200));
      if (!generar_aleatorio(*creacion.azar, 10, valor))
        return ResultadoMenu::error;
      guardar(d, 0x1dc5, static_cast<Palabra>(valor + 160));
      if (!generar_aleatorio(*creacion.azar, 10, valor))
        return ResultadoMenu::error;
      const auto espera = static_cast<unsigned char>(valor + demora);
      d[static_cast<Palabra>(secuencia + 16)] = espera;
      d[static_cast<Palabra>(secuencia + 8)] = espera;
      Palabra entidad = 0;
      if (!creacion.crear(creacion.contexto, d, 0x1dc0, entidad))
        return ResultadoMenu::error;
    }
    guardar(d, 0x35c7, static_cast<Palabra>(palabra(d, 0x35c7) + 5));
    return ResultadoMenu::accion;
  }
  if (manejador == 0x32de && id <= 1)
    return emitir_sonido(sonido, d, 0x2c2b, static_cast<unsigned char>(id)) ? ResultadoMenu::accion
                                                                            : ResultadoMenu::error;
  if (id == 3 || id == 70 || id == 75 || id == 76) {
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
