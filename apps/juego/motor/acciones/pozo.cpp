#include "pozo.hpp"
#include "iniciar_accion.hpp"
#include "lista_identificadores.hpp"
#include "../estado/segmento.hpp"
namespace motor {
namespace {
// 9F11–9F38: crea fragmentos con coordenadas relativas, profundidad y tipo de las tablas DOS.
bool fragmentos(unsigned char *d, const ServicioCreacion &servicio, Palabra tabla, Palabra entidad,
                unsigned int cantidad, unsigned char demora) {
  if (!servicio.crear)
    return false;
  for (unsigned int i = 0; i < cantidad; ++i) {
    const auto p = static_cast<Palabra>(tabla + i * 8);
    guardar(d, 0x9e41, static_cast<Palabra>(palabra(d, p) + palabra(d, entidad + 2)));
    guardar(d, 0x9e43, static_cast<Palabra>(palabra(d, p + 2) + palabra(d, entidad + 4)));
    const auto descriptor = palabra(d, 0x9e45);
    guardar(d, descriptor + 10, palabra(d, p + 4));
    guardar(d, descriptor + 14, palabra(d, p + 6));
    Palabra creada = 0;
    if (!servicio.crear(servicio.contexto, d, 0x9e3e, creada))
      return false;
    d[creada + 19] = demora;
  }
  return true;
}
// Prepara un canal sin adelantar su demora respecto a la llegada de Brad.
void canal(unsigned char *d, Palabra campo, Palabra secuencia, unsigned char intervalo) {
  guardar(d, campo, secuencia);
  d[campo + 2] = 0;
  d[campo + 4] = intervalo;
}
// 886B: reutiliza el planificador compartido y propaga los errores de recursos.
ResultadoMenu comenzar(unsigned char *d) {
  unsigned short pasos = 0;
  return iniciar_accion(d, pasos) ? ResultadoMenu::accion : ResultadoMenu::error;
}
} // namespace

// Reconstruye las alternativas del pozo, incluidos los premios que no permiten salir de las cuevas.
ResultadoMenu ejecutar_interaccion_pozo(unsigned char *d, unsigned short manejador,
                                        unsigned short id, const ServicioSonido &sonido,
                                        const ServicioCreacion &creacion) {
  if (palabra(d, 0x8082) != 9)
    return ResultadoMenu::pendiente;
  if (manejador == 0x9e21) {
    if (d[0x2c4c]) {
      marcar_accion_rechazada(d);
      return ResultadoMenu::accion;
    }
    retirar_identificador(d, 47);
    d[0x3593] = 1;
    guardar(d, 0x0ad7, 0x9dce);
    guardar(d, 0x0adc, 0x9de5);
    guardar(d, 0x1a38, 118);
    guardar(d, 0x1a3a, 167);
    canal(d, 0x2c7d, 0x9df6, 1);
    canal(d, 0x2c82, 0x9e18, 1);
    canal(d, 0x2c87, 0x9e2b, 1);
    canal(d, 0x2c9e, 0x1a2c, 2);
    canal(d, 0x2cbf, 0x1a2c, 2);
    canal(d, 0x2ce0, 0x1a2c, 2);
    unsigned short pasos = 0;
    if (!iniciar_accion(d, pasos))
      return ResultadoMenu::error;
    unsigned char demora = static_cast<unsigned char>(pasos + 8);
    d[0x2c80] = d[0x2c85] = d[0x2c8a] = demora;
    demora = static_cast<unsigned char>(demora + 17);
    if (!fragmentos(d, creacion, 0x9e6a, 0x2c6d, 7, demora))
      return ResultadoMenu::error;
    demora = static_cast<unsigned char>(demora + 32);
    d[0x2ca1] = demora;
    if (!fragmentos(d, creacion, 0x9ea2, 0x2c8e, 8, demora))
      return ResultadoMenu::error;
    demora = static_cast<unsigned char>(demora + 12);
    d[0x2cc2] = demora;
    if (!fragmentos(d, creacion, 0x9ea2, 0x2caf, 8, demora))
      return ResultadoMenu::error;
    demora = static_cast<unsigned char>(demora + 12);
    d[0x2ce3] = demora;
    if (!fragmentos(d, creacion, 0x9ea2, 0x2cd0, 8, demora))
      return ResultadoMenu::error;
    guardar(d, 0x35c7, static_cast<Palabra>(palabra(d, 0x35c7) + 5));
    return ResultadoMenu::accion;
  }
  if (manejador == 0x563a && (id == 51 || id == 52 || id == 53)) {
    // 5648–5650 omite la inserción para dinero y mujer; el casco sí pasa por 5652.
    if (id == 53 && !insertar_identificador(d, id)) {
      marcar_accion_rechazada(d);
      return ResultadoMenu::accion;
    }
    if (id == 51) {
      if (!fragmentos(d, creacion, 0x9ea2, 0x2c8e, 8, 1))
        return ResultadoMenu::error;
      canal(d, 0x2c9e, 0x97a7, 1);
      canal(d, 0x2ca3, 0x97ae, 1);
      canal(d, 0x2ca8, 0x97b0, 1);
      d[0x2ca1] = d[0x2ca6] = d[0x2cab] = 1;
      guardar(d, 0x2c3b, 0x973d);
      guardar(d, 0x2c40, 0x974b);
      const auto resultado = comenzar(d);
      d[0x2c3e] = d[0x2c43] = 9;
      d[0x2c3f] = d[0x2c44] = 1;
      return resultado;
    }
    if (id == 52) {
      if (!fragmentos(d, creacion, 0x9ea2, 0x2caf, 8, 1) ||
          !fragmentos(d, creacion, 0x9e6a, 0x2c2b, 7, 1))
        return ResultadoMenu::error;
      d[0x2caf] = 0;
      guardar(d, 0x2c3b, 0x974e);
      return comenzar(d);
    }
    if (d[0x2cd1] != 17) {
      d[0x3d4e] = 0;
      guardar(d, 0x1a38, static_cast<Palabra>(palabra(d, 0x2cd2) + 5));
      guardar(d, 0x1a3a, static_cast<Palabra>(palabra(d, 0x2cd4) + palabra(d, 0x2cd8) - 2));
      const auto lista = palabra(d, 0x53c7);
      d[static_cast<Palabra>(lista + 4 * 9 + 1)] = 0;
      canal(d, 0x2ce0, 0x1a2e, 1);
      guardar(d, 0x0ad7, d[0x358c] == 20 ? 0x96f6 : 0x96c0);
      guardar(d, 0x0adc, d[0x358c] == 20 ? 0x96ff : 0x96c9);
      unsigned short pasos = 0;
      if (!iniciar_accion(d, pasos))
        return ResultadoMenu::error;
      d[0x2ce3] = static_cast<unsigned char>(pasos + 6);
      return ResultadoMenu::accion;
    }
    d[0x3593] = 0;
    retirar_identificador(d, 53);
    if (!fragmentos(d, creacion, 0x9e6a, 0x2c6d, 7, 1))
      return ResultadoMenu::error;
    d[0x2c6d] = 0;
    constexpr Palabra ofertas[] = {0x2c8e, 0x2caf};
    for (const auto entidad : ofertas)
      if (d[entidad]) {
        d[entidad] = 0;
        if (!fragmentos(d, creacion, 0x9ea2, static_cast<Palabra>(entidad), 8, 1))
          return ResultadoMenu::error;
      }
    if (!fragmentos(d, creacion, 0x9ea2, 0x2cd0, 8, 1))
      return ResultadoMenu::error;
    canal(d, 0x2ce0, 0x97bb, 1);
    canal(d, 0x2ce5, 0x97c0, 1);
    canal(d, 0x2cea, 0x97c2, 1);
    d[0x2ce3] = d[0x2ce8] = d[0x2ced] = 1;
    d[0x3d4e] = 1;
    d[0x3d4f] = 21;
    guardar(d, 0x3d50, 188);
    guardar(d, 0x3d52, 151);
    return comenzar(d);
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
