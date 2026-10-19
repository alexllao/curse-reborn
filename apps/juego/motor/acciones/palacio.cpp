#include "palacio.hpp"
#include "iniciar_accion.hpp"
#include "lista_identificadores.hpp"
#include "../estado/segmento.hpp"
namespace motor {
namespace {
// 6285–62C8: ofrece147 al reunir los siete útiles y retira la escoba ya innecesaria.
void actualizar_oferta_silbato(unsigned char *d) {
  static constexpr Palabra requeridos[] = {153, 154, 152, 149, 150, 144, 145};
  for (const auto id : requeridos)
    if (!contiene_identificador(d, id))
      return;
  retirar_identificador(d, 141);
  d[0x4bae] = 1;
}
// 886B: mantiene el contrato común de planificación y el resultado de la acción.
ResultadoMenu comenzar(unsigned char *d) {
  unsigned short pasos = 0;
  return iniciar_accion(d, pasos) ? ResultadoMenu::accion : ResultadoMenu::error;
}
// 563A/5F67–60B1: conserva capacidad, puntuación y altura propia de cada recogida del palacio.
ResultadoMenu recoger(unsigned char *d, Palabra id) {
  const auto icono = d[0x524e + id * 2];
  if (icono == 0x1f || icono == 0x22)
    guardar(d, 0x35c7, static_cast<Palabra>(palabra(d, 0x35c7) + (icono == 0x1f ? 1 : 2)));
  else if (!insertar_identificador(d, id)) {
    marcar_accion_rechazada(d);
    return ResultadoMenu::accion;
  }
  const bool dados = id == 140;
  // 5FFD–6060: retirar un reflector conserva el soporte elegido, no la habitación de origen.
  const bool reflector =
      palabra(d, 0x8082) == 56 && (id == 150 || id == 153 || id == 154 || id == 145);
  const bool sobre_mesa = dados || id == 144 || id == 145;
  const bool baja = !reflector && (id == 147 || (id >= 149 && id <= 154));
  const Palabra entidad = reflector                           ? (palabra(d, 0x2cde) == id   ? 0x2cd0
                                                                 : palabra(d, 0x2cff) == id ? 0x2cf1
                                                                 : palabra(d, 0x2d41) == id ? 0x2d33
                                                                                            : 0x2d12)
                          : dados || (id >= 149 && id <= 154) ? 0x2c4c
                          : id == 141                         ? 0x2cf1
                          : id == 144                         ? 0x2caf
                                                              : 0x2c8e;
  if (!reflector && (id == 144 || id == 145 || id == 149 || id == 150 || (id >= 152 && id <= 154)))
    actualizar_oferta_silbato(d);
  if (dados) {
    guardar(d, 0x009c, 0x2d12);
    guardar(d, 0x009e, 0x0109);
    guardar(d, 0x00a0, 0x9819);
    guardar(d, 0x00a4, 0x981c);
  }
  guardar(d, 0x1a38, static_cast<Palabra>(palabra(d, entidad + 2) + (id == 141 ? 9 : 5)));
  guardar(d, 0x1a3a,
          static_cast<Palabra>(palabra(d, entidad + 4) + palabra(d, entidad + 8) +
                               (reflector   ? 1
                                : dados     ? 25
                                : id == 141 ? 3
                                : id == 149 ? -16
                                : id == 153 ? -5
                                : id == 154 ? -2
                                : baja      ? -4
                                            : 33)));
  const auto lista = palabra(d, static_cast<Palabra>(0x53b5 + palabra(d, 0x8082) * 2));
  d[static_cast<Palabra>(lista + ((entidad - 0x2c4c) / 33) * 9 + 1)] = 0;
  guardar(d, entidad + 16, 0x1a2e);
  d[entidad + 18] = 0;
  d[entidad + 20] = 1;
  guardar(d, 0x0ad7,
          sobre_mesa && !reflector ? 0x96de
          : baja                   ? (d[0x358c] == 20 ? 0x96f6 : 0x96c0)
                                   : (d[0x358c] == 20 ? 0x9705 : 0x96cf));
  guardar(d, 0x0adc,
          sobre_mesa && !reflector ? 0x96e7
          : baja                   ? (d[0x358c] == 20 ? 0x96ff : 0x96c9)
                                   : (d[0x358c] == 20 ? 0x970e : 0x96d8));
  unsigned short pasos = 0;
  if (!iniciar_accion(d, pasos))
    return ResultadoMenu::error;
  d[entidad + 19] = static_cast<unsigned char>(pasos + 6);
  return ResultadoMenu::accion;
}
} // namespace
// Reutiliza B740–B939 y886B ya contrastados; el generador pertenece a la partida llamadora.
ResultadoMenu ejecutar_interaccion_palacio(unsigned char *d, unsigned short manejador,
                                           unsigned short id, const ServicioSonido &sonido,
                                           const ServicioCreacion &creacion) {
  const auto escena = palabra(d, 0x8082);
  if (escena < 46 || escena > 56)
    return ResultadoMenu::pendiente;
  if (manejador == 0x32de && id <= 1)
    return emitir_sonido(sonido, d, 0x2c2b, static_cast<unsigned char>(id)) ? ResultadoMenu::accion
                                                                            : ResultadoMenu::error;
  if (manejador == 0x563a &&
      ((escena == 46 && (id == 140 || id == 141)) || (escena == 47 && (id == 144 || id == 145)) ||
       (escena == 49 && id == 147) || (escena == 50 && id == 149) || (escena == 51 && id == 150) ||
       (escena >= 52 && escena <= 55 && id == escena + 99) ||
       (escena == 56 && (id == 150 || id == 153 || id == 154 || id == 145))))
    return recoger(d, id);
  if (manejador == 0x3b53 && palabra(d, 0xb638) == 0xffff && palabra(d, 0xb634) == 5 && id == 140) {
    if (!creacion.azar)
      return ResultadoMenu::pendiente;
    unsigned short ax = 0;
    return ejecutar_accion_resultados(d, *creacion.azar, ax) ? ResultadoMenu::accion
                                                             : ResultadoMenu::error;
  }
  // B93D/B9CE: inspecciones de poleas y ventana, con los canales que escribe cada una.
  if (manejador == 0x384a && (id == 143 || id == 146)) {
    if (id == 143) {
      if (d[0x2c4d] != 9) {
        marcar_accion_rechazada(d);
        return ResultadoMenu::accion;
      }
      guardar(d, 0x1a38, 231);
      guardar(d, 0x1a3a, 182);
      guardar(d, 0x0ad7, 0xab9f);
      guardar(d, 0x0adc, 0xabaa);
      guardar(d, 0x0ae1, 0xabb0);
    } else {
      guardar(d, 0x1a38, 159);
      guardar(d, 0x1a3a, 164);
      guardar(d, 0x0ad7, 0xabc4);
      guardar(d, 0x009c, 0x2c6d);
      guardar(d, 0x009e, 0x0203);
      guardar(d, 0x00a0, 0xabca);
    }
    return comenzar(d);
  }
  if (manejador == 0x3143 && id == 146) {
    if (d[0x2c4c]) {
      marcar_accion_rechazada(d);
      return ResultadoMenu::accion;
    }
    guardar(d, 0x1a38, 171);
    guardar(d, 0x1a3a, 165);
    guardar(d, 0x0ad7, 0xabfc);
    guardar(d, 0x0adc, 0xac0a);
    guardar(d, 0x0ae1, 0xac13);
    guardar(d, 0x35c7, static_cast<Palabra>(palabra(d, 0x35c7) + 5));
    return comenzar(d);
  }
  if (manejador == 0x2fca && id == 148) {
    if (d[0x2c4c] != 1) {
      marcar_accion_rechazada(d);
      return ResultadoMenu::accion;
    }
    retirar_identificador(d, 148);
    d[0x4b9c] = 0;
    guardar(d, 0x1a38, 142);
    guardar(d, 0x1a3a, 165);
    guardar(d, 0x0ad7, 0xabd6);
    guardar(d, 0x0adc, 0xabe1);
    guardar(d, 0x009c, 0x2caf);
    guardar(d, 0x009e, 0x2805);
    guardar(d, 0x00a0, 0x1a32);
    guardar(d, 0x00a6, 0x2c4c);
    guardar(d, 0x00a8, 0x0211);
    guardar(d, 0x00aa, 0xabe9);
    guardar(d, 0x35c7, static_cast<Palabra>(palabra(d, 0x35c7) + 5));
    return comenzar(d);
  }
  // BAA6–BB8C: la escoba baja el arma o carámbano solo con actividad1 eY89.
  if (manejador == 0x2fca && id == 141 && (escena == 52 || escena == 55)) {
    if (d[0x2c4c] != 1 || palabra(d, 0x2c50) != 89) {
      marcar_accion_rechazada(d);
      return ResultadoMenu::accion;
    }
    const auto persistente = static_cast<Palabra>(escena == 52 ? 0x4c12 : 0x4c60);
    guardar(d, persistente, static_cast<Palabra>(palabra(d, persistente) + 62));
    guardar(d, 0x1a38, escena == 52 ? 141 : 157);
    guardar(d, 0x1a3a, escena == 52 ? 151 : 159);
    guardar(d, 0x0ad7, 0xac1c);
    guardar(d, 0x0adc, 0xac2a);
    guardar(d, 0x0ae1, 0xac31);
    guardar(d, 0x009c, 0x2c6d);
    guardar(d, 0x009e, 0x0203);
    guardar(d, 0x00a0, 0xac38);
    guardar(d, 0x00a4, 0xac3c);
    guardar(d, 0x00a6, 0x2c4c);
    guardar(d, 0x00a8, 0x0109);
    guardar(d, 0x00aa, escena == 52 ? 0xac49 : 0xac41);
    guardar(d, 0x00ae, 0xac50);
    guardar(d, 0x35c7, static_cast<Palabra>(palabra(d, 0x35c7) + 5));
    return comenzar(d);
  }
  const auto previo = palabra(d, 0xb638), verbo = palabra(d, 0xb634);
  // BB8D–BC6E: los cuatro útiles pueden colocarse en cualquiera de los cuatro soportes.
  if (escena == 56 && manejador == 0x3b53 && (verbo == 7 || verbo == 1)) {
    const auto soporte = id;
    const auto pieza = previo;
    const auto imagen = pieza == 150   ? 30
                        : pieza == 153 ? 27
                        : pieza == 154 ? 34
                        : pieza == 145 ? 14
                                       : 0;
    if (verbo == 7 && soporte >= 155 && soporte <= 158 && imagen) {
      const auto entidad = static_cast<Palabra>(0x2cd0 + (soporte - 155) * 33);
      if (d[entidad] == 1) {
        marcar_accion_rechazada(d);
        return ResultadoMenu::accion;
      }
      retirar_identificador(d, pieza);
      guardar(d, entidad + 14, pieza);
      d[entidad + 1] = static_cast<unsigned char>(imagen);
      d[entidad] = 3;
      guardar(d, 0x1a38, palabra(d, entidad + 2));
      guardar(d, 0x1a3a, static_cast<Palabra>(palabra(d, entidad + 4) + 7));
      guardar(d, 0x0ad7, 0x96cf);
      guardar(d, 0x0adc, 0x96d8);
      guardar(d, 0x009c, entidad);
      guardar(d, 0x009e, 0x0107);
      guardar(d, 0x00a0, 0xacbe);
      return comenzar(d);
    }
    // BC71–BCDC: el gato mecánico eleva la reja cuando ya no está su protector.
    if (verbo == 1 && ((id == 149 && previo == 159) || (id == 159 && previo == 149))) {
      if (d[0x2d96]) {
        marcar_accion_rechazada(d);
        return ResultadoMenu::accion;
      }
      retirar_identificador(d, 149);
      guardar(d, 0x1a38, 251);
      guardar(d, 0x1a3a, 163);
      guardar(d, 0x0ad7, 0xac5a);
      guardar(d, 0x0adc, 0xac76);
      guardar(d, 0x0ae1, 0xac8b);
      guardar(d, 0x009c, 0x2e3b);
      guardar(d, 0x009e, 0x080f);
      guardar(d, 0x00a4, 0xaca0);
      guardar(d, 0x00a6, 0x2e5c);
      guardar(d, 0x00a8, 0x0403);
      guardar(d, 0x00aa, 0xaca3);
      guardar(d, 0x00ac, 0xacb0);
      guardar(d, 0x00b0, 0x2e7d);
      guardar(d, 0x00b2, 0x0403);
      guardar(d, 0x00b4, 0xacb7);
      guardar(d, 0x35c7, static_cast<Palabra>(palabra(d, 0x35c7) + 5));
      return comenzar(d);
    }
  }
  if (manejador == 0x3b53 && verbo == 1 &&
      ((id == 142 && previo == 151) || (id == 151 && previo == 142))) {
    retirar_identificador(d, 151);
    d[0x4b00] = 12;
    guardar(d, 0x4b0c, static_cast<Palabra>(palabra(d, 0x4b0c) + 24));
    d[0x4b11] = d[0x4b1a] = d[0x2c8e] = d[0x2caf] = 7;
    guardar(d, 0x1a38, 89);
    guardar(d, 0x1a3a, 180);
    guardar(d, 0x0ad7, 0x96de);
    guardar(d, 0x0adc, 0x96e7);
    guardar(d, 0x009c, 0x2c4c);
    guardar(d, 0x009e, 0x0209);
    guardar(d, 0x00a0, 0xabb6);
    guardar(d, 0x00a6, 0x2c6d);
    guardar(d, 0x00a8, 0x0209);
    guardar(d, 0x00ae, 0xabc0);
    guardar(d, 0x35c7, static_cast<Palabra>(palabra(d, 0x35c7) + 5));
    return comenzar(d);
  }
  if (manejador == 0x3b53 && verbo == 7 &&
      ((id == 147 && previo == 152) || (id == 152 && previo == 147))) {
    if (!retirar_identificador(d, 147)) {
      marcar_accion_rechazada(d);
      return ResultadoMenu::accion;
    }
    retirar_identificador(d, 152);
    insertar_identificador(d, 148);
    guardar(d, 0x2c3b, 0x96de);
    guardar(d, 0x2c40, 0x96e7);
    guardar(d, 0x35c7, static_cast<Palabra>(palabra(d, 0x35c7) + 5));
    return comenzar(d);
  }
  // 49DA/BAA6: ventilador y escoba solo tienen efectos especiales en sus habitaciones posteriores.
  if (id != 76 && id != 124 && (id < 140 || id > 159))
    return ResultadoMenu::pendiente;
  if (manejador == 0x384a)
    return ResultadoMenu::actualizado;
  if (manejador == 0x2fca || manejador == 0x3143 || manejador == 0x3b53) {
    marcar_accion_rechazada(d);
    return ResultadoMenu::accion;
  }
  return ResultadoMenu::pendiente;
}
} // namespace motor
