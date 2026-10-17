#include "cuevas.hpp"
#include "iniciar_accion.hpp"
#include "lista_identificadores.hpp"
#include "../estado/segmento.hpp"

namespace motor {
namespace {
// 3E89–3F25: clasifica por los identificadores originales, no por el aspecto del icono.
int grupo_piedra(Palabra id) {
  constexpr Palabra grupos[3][6] = {
      {27, 34, 39, 41, 43, 46}, {28, 30, 32, 35, 40, 45}, {29, 31, 33, 38, 42, 44}};
  for (int grupo = 0; grupo < 3; ++grupo)
    for (const auto piedra : grupos[grupo])
      if (piedra == id)
        return grupo;
  return -1;
}
// 5652–5679: la recogida respeta el icono y la capacidad de la lista de inventario original.
bool incorporar(unsigned char *d, Palabra id) {
  const auto icono = d[0x524e + id * 2];
  if (icono == 0x1f || icono == 0x22) {
    guardar(d, 0x35c7, static_cast<Palabra>(palabra(d, 0x35c7) + (icono == 0x1f ? 1 : 2)));
    return true;
  }
  return insertar_identificador(d, id);
}
} // namespace

// 5849–59A3 y 9F39–A07B: conserva aritmética de 16 bits y contadores con signo de ocho bits.
ResultadoMenu ejecutar_interaccion_cuevas(unsigned char *d, unsigned short manejador,
                                          unsigned short id, const ServicioSonido &sonido) {
  const auto escena = palabra(d, 0x8082);
  if ((escena < 4 || escena > 7) && escena != 10)
    return ResultadoMenu::pendiente;
  if (manejador == 0x563a && grupo_piedra(id) >= 0) {
    const Palabra origen = id <= 30 ? 27 : id <= 35 ? 31 : id <= 42 ? 38 : 43;
    const Palabra esperada = id <= 30 ? 4 : id <= 35 ? 5 : id <= 42 ? 6 : 7;
    if (escena != esperada)
      return ResultadoMenu::pendiente;
    if (!incorporar(d, id)) {
      marcar_accion_rechazada(d);
      return ResultadoMenu::accion;
    }
    const auto entidad = static_cast<Palabra>(0x2c4c + (id - origen) * 33);
    guardar(d, 0x1a38, static_cast<Palabra>(palabra(d, entidad + 2) + 5));
    guardar(d, 0x1a3a, static_cast<Palabra>(palabra(d, entidad + 4) + palabra(d, entidad + 8) - 2));
    const auto lista = palabra(d, static_cast<Palabra>(0x53b5 + escena * 2));
    d[static_cast<Palabra>(lista + (id - origen) * 9 + 1)] = 0;
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
  if (manejador == 0x9f39 && escena == 10 && grupo_piedra(id) >= 0) {
    const auto grupo = grupo_piedra(id);
    const auto campo = static_cast<Palabra>(0x35cb + grupo);
    const auto anterior = static_cast<signed char>(d[campo]);
    retirar_identificador(d, id);
    --d[campo];
    // DEC/JGE conserva OF: 80h pasa a 7Fh, pero esa entrega tampoco retira un punto del cartel.
    const bool descontada = anterior > 0;
    if (descontada) {
      constexpr Palabra descriptores[] = {0x3d8e, 0x3db2, 0x3dd6};
      constexpr Palabra entidades[] = {0x2c6d, 0x2cf1, 0x2d75};
      d[static_cast<Palabra>(descriptores[grupo] + d[campo] * 9 + 1)] = 0;
      d[static_cast<Palabra>(entidades[grupo] + d[campo] * 33)] = 0;
    }
    const bool completa = descontada && static_cast<signed char>(d[0x35cb]) <= 0 &&
                          static_cast<signed char>(d[0x35cc]) <= 0 &&
                          static_cast<signed char>(d[0x35cd]) <= 0;
    if (completa)
      insertar_identificador(d, 55);
    guardar(d, 0x1a38, static_cast<Palabra>(palabra(d, 0x2c4e) - 10));
    guardar(d, 0x1a3a, static_cast<Palabra>(palabra(d, 0x2c50) + 34));
    guardar(d, 0x0ad7, completa ? 0x9efd : 0x9ee2);
    guardar(d, 0x0adc, completa ? 0x9f0a : 0x9eef);
    guardar(d, 0x009c, 0x2c4c);
    guardar(d, 0x009e, 0x0203);
    guardar(d, 0x00a0, completa ? 0x9f10 : 0x9ef5);
    guardar(d, 0x35c7, static_cast<Palabra>(palabra(d, 0x35c7) + 5));
    unsigned short pasos = 0;
    return iniciar_accion(d, pasos) ? ResultadoMenu::accion : ResultadoMenu::error;
  }
  if (manejador == 0x32de && id <= 1)
    return emitir_sonido(sonido, d, 0x2c2b, static_cast<unsigned char>(id)) ? ResultadoMenu::accion
                                                                            : ResultadoMenu::error;
  if (id == 3 || (id >= 27 && id <= 55)) {
    if (manejador == 0x384a && id != 8)
      return ResultadoMenu::actualizado;
    if (manejador == 0x2fca || manejador == 0x3143) {
      marcar_accion_rechazada(d);
      return ResultadoMenu::accion;
    }
  }
  return ResultadoMenu::pendiente;
}
} // namespace motor
