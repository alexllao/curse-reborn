#include "poblacion.hpp"
#include "asaltante.hpp"
#include "iniciar_accion.hpp"
#include "lista_identificadores.hpp"
#include "../estado/segmento.hpp"
namespace motor {
namespace {
// 886B: conserva el resultado de planificación de la acción original.
ResultadoMenu comenzar(unsigned char *d) {
  unsigned short pasos = 0;
  return iniciar_accion(d, pasos) ? ResultadoMenu::accion : ResultadoMenu::error;
}
// Escribe los tres canales diferidos que 0A58 aplicará al finalizar el recorrido.
void actor(unsigned char *d, Palabra registro, Palabra entidad, Palabra intervalo, Palabra a,
           Palabra b = 0xffff, Palabra c = 0xffff) {
  guardar(d, registro, entidad);
  guardar(d, registro + 2, intervalo);
  guardar(d, registro + 4, a);
  if (b != 0xffff)
    guardar(d, registro + 6, b);
  if (c != 0xffff)
    guardar(d, registro + 8, c);
}
} // namespace
// 5D89/5DA4 y respuestas comunes: recogidas del valle32–35 y CLOUD38; los puzles posteriores siguen
// pendientes.
ResultadoMenu ejecutar_interaccion_poste(unsigned char *d, unsigned short manejador,
                                         unsigned short id, const ServicioSonido &sonido) {
  const auto escena_valle = palabra(d, 0x8082);
  if (escena_valle < 32 || (escena_valle > 35 && escena_valle != 37 && escena_valle != 38))
    return ResultadoMenu::pendiente;
  if (manejador == 0x32de && id <= 1) {
    // 337D–34D6: pedir ayuda en GHETTO solo derrumba las rocas con2DF9 en tipo3.
    if (escena_valle == 37 && id == 0) {
      if (d[0x2df9] != 3) {
        marcar_accion_rechazada(d);
        return ResultadoMenu::accion;
      }
      guardar(d, 0x1a38, 159);
      guardar(d, 0x1a3a, 176);
      guardar(d, 0x2c3b, 0);
      guardar(d, 0x0ad7, 0x262e);
      guardar(d, 0x0adc, 0x263f);
      d[0x4794] = 1;
      static constexpr Palabra primeros[] = {0x2647, 0x1a2e, 0x2647, 0x2647, 0x1a2e, 0x2647, 0x1a2e,
                                             0x1a2e, 0x2647, 0x1a2e, 0x1a2e, 0x1a2e, 0x2647};
      for (unsigned int i = 0; i < 13; ++i) {
        d[0x471f + i * 9] = 0;
        guardar(d, 0x2c5c + i * 33, primeros[i]);
      }
      static constexpr Palabra terceros[] = {0x2658, 0x2658, 0x2658, 0x2663,
                                             0x266d, 0x2663, 0x266d, 0x2663};
      for (unsigned int i = 0; i < 8; ++i)
        guardar(d, 0x2c66 + i * 33, terceros[i]);
      guardar(d, 0x2e09, 0x1a2c);
      d[0x2e0b] = 0;
      d[0x2e0d] = 1;
      unsigned short pasos = 0;
      if (!iniciar_accion(d, pasos))
        return ResultadoMenu::error;
      for (unsigned int i = 0; i < 8; ++i)
        d[0x2c69 + i * 33] = static_cast<unsigned char>(pasos + 4);
      d[0x2e0c] = static_cast<unsigned char>(pasos + 9);
      for (unsigned int i = 8; i < 13; ++i)
        d[0x2c5f + i * 33] = static_cast<unsigned char>(pasos + 9);
      static constexpr Palabra retardados[] = {0x2cc2, 0x2d04, 0x2d46};
      for (const auto campo : retardados)
        d[campo] = static_cast<unsigned char>(pasos + 10);
      d[0x2ce3] = d[0x2d25] = static_cast<unsigned char>(pasos + 12);
      d[0x2c80] = static_cast<unsigned char>(pasos + 13);
      d[0x2c5f] = d[0x2ca1] = static_cast<unsigned char>(pasos + 14);
      return ResultadoMenu::accion;
    }
    // 372A–3763: el saludo cercano al personaje98 activa su bocadillo, además del sonido común.
    if (palabra(d, 0x8082) == 32 && id == 1 && d[0x2c4c] == 1) {
      const auto origen = palabra(d, 0x2c4e), destino = palabra(d, 0x2c2d);
      auto distancia = static_cast<Palabra>(origen - destino);
      if (con_signo(origen) < con_signo(destino))
        distancia = static_cast<Palabra>(0 - distancia);
      if (con_signo(distancia) <= 64) {
        guardar(d, 0x2d22, 0x1a2f);
        d[0x2d24] = 0;
        d[0x2d25] = 1;
        d[0x2d26] = 40;
      }
    }
    return emitir_sonido(sonido, d, 0x2c2b, static_cast<unsigned char>(id)) ? ResultadoMenu::accion
                                                                            : ResultadoMenu::error;
  }
  // 5D2F–5DD9: entidad, desplazamientos y familia615C/61A7 de cada recogida contrastada.
  static constexpr Palabra recogidas[][6] = {
      {105, 33, 0x2cf1, 3, 1, 0},       {108, 34, 0x2c6d, 45, 0xffea, 1},
      {110, 35, 0x2d33, 45, 0xfff4, 1}, {111, 35, 0x2c4c, 7, 1, 0},
      {112, 35, 0x2c4c, 7, 1, 0},       {99, 32, 0x2cf1, 5, 1, 0},
      {100, 32, 0x2d75, 15, 0xfffc, 1}, {101, 32, 0x2d54, 35, 0xfffc, 1},
      {109, 32, 0x2d33, 45, 0xfffc, 1}, {121, 38, 0x2c4c, 0, 0, 2},
      {120, 37, 0x2df9, 5, 1, 0}};
  const Palabra *recogida = nullptr;
  for (const auto &fila : recogidas)
    if (fila[0] == id && fila[1] == palabra(d, 0x8082))
      recogida = fila;
  if (manejador == 0x563a && recogida) {
    const auto icono = d[0x524e + id * 2];
    if (icono == 0x1f || icono == 0x22)
      guardar(d, 0x35c7, static_cast<Palabra>(palabra(d, 0x35c7) + (icono == 0x1f ? 1 : 2)));
    else if (!insertar_identificador(d, id)) {
      marcar_accion_rechazada(d);
      return ResultadoMenu::accion;
    }
    if (id == 108 && d[0x2c4c]) {
      retirar_identificador(d, id);
      marcar_accion_rechazada(d);
      return ResultadoMenu::accion;
    }
    if (id == 99)
      d[0x5314] = 0x90;
    const Palabra entidad = recogida[2];
    // 5E68–5E97: no aproxima a la bolsa; conserva el tercer canal y usa 359A tras 886B.
    if (id == 121) {
      const auto lista = palabra(d, static_cast<Palabra>(0x53b5 + escena_valle * 2));
      d[static_cast<Palabra>(lista + 1)] = 0;
      guardar(d, entidad + 16, 0x1a2e);
      d[entidad + 18] = 0;
      d[entidad + 20] = 1;
      guardar(d, 0x2c3b, 0x9808);
      guardar(d, 0x2c40, 0x9814);
      const auto resultado = comenzar(d);
      d[entidad + 19] = static_cast<unsigned char>(palabra(d, 0x359a) + 6);
      return resultado;
    }
    const bool recogida_normal = recogida[5] != 0;
    guardar(d, 0x1a38, static_cast<Palabra>(palabra(d, entidad + 2) + recogida[3]));
    guardar(d, 0x1a3a,
            static_cast<Palabra>(palabra(d, entidad + 4) + palabra(d, entidad + 8) + recogida[4]));
    const auto lista = palabra(d, static_cast<Palabra>(0x53b5 + palabra(d, 0x8082) * 2));
    d[static_cast<Palabra>(lista + ((entidad - 0x2c4c) / 33) * 9 + 1)] = 0;
    guardar(d, entidad + 16, 0x1a2e);
    d[entidad + 18] = 0;
    d[entidad + 20] = 1;
    const bool pequeno = d[0x358c] == 20;
    guardar(d, 0x0ad7, recogida_normal ? (pequeno ? 0x96f6 : 0x96c0) : (pequeno ? 0x9705 : 0x96cf));
    guardar(d, 0x0adc, recogida_normal ? (pequeno ? 0x96ff : 0x96c9) : (pequeno ? 0x970e : 0x96d8));
    unsigned short pasos = 0;
    if (!iniciar_accion(d, pasos))
      return ResultadoMenu::error;
    d[entidad + 19] = static_cast<unsigned char>(pasos + 6);
    return ResultadoMenu::accion;
  }
  // CLOUD no hereda intercambios de POST; solo las respuestas comunes ya comprobadas.
  if (escena_valle == 38) {
    if (id != 3 && id != 70 && id != 76 && id != 99 && id != 101 && id != 121)
      return ResultadoMenu::pendiente;
    if (manejador == 0x384a)
      return ResultadoMenu::actualizado;
    if (manejador == 0x2fca || manejador == 0x3143 || manejador == 0x3b53) {
      marcar_accion_rechazada(d);
      return ResultadoMenu::accion;
    }
    return ResultadoMenu::pendiente;
  }
  // 3DA6–3DBD/A81A–A8AB: insertar el pelo110 en102 admite ambos órdenes y resta firmada.
  const auto previo = palabra(d, 0xb638);
  if (palabra(d, 0x8082) == 32 && manejador == 0x3b53 && palabra(d, 0xb634) == 1 &&
      ((id == 102 && previo == 110) || (id == 110 && previo == 102))) {
    const auto horizontal = static_cast<Palabra>(palabra(d, 0x2c2d) + palabra(d, 0x2c31) - 6);
    const auto vertical =
        static_cast<Palabra>(palabra(d, 0x2c2f) + palabra(d, 0x2c33) - palabra(d, 0x2c92));
    if (con_signo(horizontal) >= con_signo(palabra(d, 0x2c90)) &&
        con_signo(vertical) < con_signo(palabra(d, 0x2c96))) {
      marcar_accion_rechazada(d);
      return ResultadoMenu::accion;
    }
    retirar_identificador(d, 110);
    d[0x45e7] = 0;
    guardar(d, 0x1a38, static_cast<Palabra>(332 - palabra(d, 0x3591)));
    guardar(d, 0x1a3a, 141);
    guardar(d, 0x0ad7, 0xa1be);
    guardar(d, 0x0adc, 0xa1cc);
    d[0x2caf] = d[0x2cd0] = 0;
    d[0x450c] = 29;
    guardar(d, 0x450d, static_cast<Palabra>(palabra(d, 0x450d) + 55));
    guardar(d, 0x450f, static_cast<Palabra>(palabra(d, 0x450f) - 39));
    d[0x4514] = d[0x451d] = 0;
    actor(d, 0x009c, 0x2c8e, 0x0115, 0xa1d6, 0xa1fc, 0xa215);
    guardar(d, 0x35c7, static_cast<Palabra>(palabra(d, 0x35c7) + 5));
    return comenzar(d);
  }
  // 3FC5–3FDC/AA6C: combina carta111 y sello109 en ambos órdenes; exige retirar la carta.
  const auto verbo = palabra(d, 0xb634);
  if (manejador == 0x3b53 && verbo == 7 &&
      ((id == 111 && previo == 109) || (id == 109 && previo == 111))) {
    if (!retirar_identificador(d, 111)) {
      marcar_accion_rechazada(d);
      return ResultadoMenu::accion;
    }
    retirar_identificador(d, 109);
    insertar_identificador(d, 112);
    guardar(d, 0x4625, 0xa33d);
    guardar(d, 0x2c5a, 112);
    guardar(d, 0x2c3b, 0x96de);
    guardar(d, 0x2c40, 0x96e7);
    guardar(d, 0x35c7, static_cast<Palabra>(palabra(d, 0x35c7) + 5));
    return comenzar(d);
  }
  // 3DC0–3DD7/AAA3–AB5D: inserta carta112 en buzón114, preservando dos o tres nodos de
  // aproximación.
  if (manejador == 0x3b53 && verbo == 1 &&
      ((id == 114 && previo == 112) || (id == 112 && previo == 114))) {
    if (con_signo(palabra(d, 0x2c2f)) < 161) {
      guardar(d, 0x1a38, static_cast<Palabra>(427 - palabra(d, 0x3591)));
      guardar(d, 0x1a3a, 174);
      const auto vertical =
          static_cast<Palabra>(palabra(d, 0x2d16) + palabra(d, 0x2d1a) - palabra(d, 0x2c2f));
      if (con_signo(vertical) >= con_signo(palabra(d, 0x2c33))) {
        const auto derecha = static_cast<Palabra>(palabra(d, 0x2c2d) + palabra(d, 0x2c31));
        const auto diferencia = static_cast<Palabra>(palabra(d, 0x2c2d) - palabra(d, 0x2d14));
        if (con_signo(derecha) < con_signo(palabra(d, 0x2d14)) ||
            con_signo(diferencia) >= con_signo(palabra(d, 0x2d18))) {
          guardar(d, 0x1a3e, palabra(d, 0x1a38));
          guardar(d, 0x1a40, palabra(d, 0x1a3a));
          guardar(d, 0x1a36, 0x1a3c);
          guardar(d, 0x1a38, palabra(d, 0x2c2d));
        } else {
          guardar(d, 0x1a44, palabra(d, 0x1a38));
          guardar(d, 0x1a40, palabra(d, 0x1a3a));
          guardar(d, 0x1a46, palabra(d, 0x1a3a));
          guardar(d, 0x1a36, 0x1a3c);
          guardar(d, 0x1a3c, 0x1a42);
          const auto lateral = static_cast<Palabra>(palabra(d, 0x2d14) - 16);
          guardar(d, 0x1a38, lateral);
          guardar(d, 0x1a3e, lateral);
          guardar(d, 0x1a3a, static_cast<Palabra>(palabra(d, 0x2c2f) + 47));
        }
      }
    }
    retirar_identificador(d, 112);
    if (d[0x44f9])
      d[0x44fa] = 4;
    else
      d[0x4526] = 1;
    guardar(d, 0x0ad7, 0xa326);
    guardar(d, 0x0adc, 0xa335);
    guardar(d, 0x35c7, static_cast<Palabra>(palabra(d, 0x35c7) + 5));
    return comenzar(d);
  }
  // 3DDA–3DF1/AB60–ABD2: inserta cinta105 en113 sin cambiar los otros campos del actor diferido.
  if (manejador == 0x3b53 && verbo == 1 &&
      ((id == 113 && previo == 105) || (id == 105 && previo == 113))) {
    const auto horizontal = static_cast<Palabra>(palabra(d, 0x2c2d) + 8 - palabra(d, 0x2cf3));
    const auto vertical =
        static_cast<Palabra>(palabra(d, 0x2c2f) + palabra(d, 0x2c33) - palabra(d, 0x2cf5));
    if (con_signo(horizontal) < con_signo(palabra(d, 0x2cf7)) &&
        con_signo(vertical) < con_signo(palabra(d, 0x2cf9))) {
      marcar_accion_rechazada(d);
      return ResultadoMenu::accion;
    }
    retirar_identificador(d, 105);
    d[0x4628] = 1;
    d[0x4629] = 0x47;
    guardar(d, 0x462c, static_cast<Palabra>(palabra(d, 0x462c) + 7));
    guardar(d, 0x1a38, static_cast<Palabra>(752 - palabra(d, 0x3591)));
    guardar(d, 0x1a3a, 172);
    guardar(d, 0x0ad7, 0xa362);
    guardar(d, 0x0adc, 0xa370);
    guardar(d, 0x009c, 0x2c6d);
    guardar(d, 0x009e, 0x0209);
    guardar(d, 0x00a0, 0xa34f);
    guardar(d, 0x00a4, 0xa355);
    guardar(d, 0x35c7, static_cast<Palabra>(palabra(d, 0x35c7) + 5));
    return comenzar(d);
  }
  // 3BE8/ABD5–AC5B: el mando108 produce106, conservando las dos pruebas firmadas de posición.
  if (manejador == 0x3b53 && verbo == 2 && previo == 0xffff && id == 108) {
    bool permitido = d[0x2c6d] == 1;
    const auto global = static_cast<Palabra>(palabra(d, 0x2c2d) + palabra(d, 0x3591));
    const auto y = con_signo(palabra(d, 0x2c2f));
    if (permitido && !(con_signo(global) >= 735 && y >= 123)) {
      const auto parcial = static_cast<Palabra>(palabra(d, 0x2c2d) - palabra(d, 0x2cf3));
      if (con_signo(parcial) < con_signo(palabra(d, 0x2cf7))) {
        const auto distancia = static_cast<Palabra>(parcial - palabra(d, 0x2cf7));
        permitido = con_signo(distancia) + 50 >= 0 && y >= 123;
      }
    }
    if (!permitido) {
      marcar_accion_rechazada(d);
      return ResultadoMenu::accion;
    }
    retirar_identificador(d, 108);
    insertar_identificador(d, 106);
    d[0x4628] = 0;
    guardar(d, 0x1a38, static_cast<Palabra>(752 - palabra(d, 0x3591)));
    guardar(d, 0x1a3a, 172);
    guardar(d, 0x0ad7, 0xa379);
    guardar(d, 0x0adc, 0xa390);
    guardar(d, 0x009c, 0x2c6d);
    guardar(d, 0x009e, 0x023b);
    guardar(d, 0x00a0, 0xa359);
    guardar(d, 0x00a4, 0xa35f);
    guardar(d, 0x35c7, static_cast<Palabra>(palabra(d, 0x35c7) + 5));
    return comenzar(d);
  }
  // 3F5C–3F73/A8AE–A94F: conserva la respuesta a la cinta vacía y la entrega de la grabación.
  if (manejador == 0x3b53 && verbo == 6 && id == 98 && (previo == 105 || previo == 106)) {
    guardar(d, 0x1a38, static_cast<Palabra>(141 - palabra(d, 0x3591)));
    guardar(d, 0x1a3a, 165);
    guardar(d, 0x0ad7, 0x96de);
    guardar(d, 0x0adc, 0x96e7);
    if (previo == 105) {
      guardar(d, 0x009c, 0x2d12);
      guardar(d, 0x009e, 0x2805);
      guardar(d, 0x00a0, 0x1a2f);
      return comenzar(d);
    }
    retirar_identificador(d, 106);
    d[0x44f9] = 0;
    const bool correo = d[0x2c4d] == 4;
    if (correo) {
      insertar_identificador(d, 99);
      d[0x5314] = 0x90;
    }
    actor(d, 0x009c, 0x2c4c, 0x020d, correo ? 0xa22e : 0xa245, correo ? 0xa23b : 0xa265,
          correo ? 0xa240 : 0xa27f);
    guardar(d, 0x35c7, static_cast<Palabra>(palabra(d, 0x35c7) + 5));
    return comenzar(d);
  }
  // 3DF4–3E0B/AC5E–AC8C: el pase99 abre116 sin consumirse ni sumar puntos.
  if (manejador == 0x3b53 && verbo == 1 &&
      ((id == 116 && previo == 99) || (id == 99 && previo == 116))) {
    guardar(d, 0x1a38, static_cast<Palabra>(910 - palabra(d, 0x3591)));
    guardar(d, 0x1a3a, 140);
    guardar(d, 0x0ad7, 0xa3a2);
    guardar(d, 0x0adc, 0xa3b8);
    guardar(d, 0x009c, 0x2cd0);
    guardar(d, 0x009e, 0x0109);
    guardar(d, 0x00a0, 0x1a2c);
    return comenzar(d);
  }
  // 3F5C–3FF6: intercambios y combinaciones posteriores todavía sin trasladar.
  if (manejador == 0x3b53) {
    const auto verbo = palabra(d, 0xb634);
    if (verbo == 7 && ((id == 100 && previo == 107) || (id == 107 && previo == 100)))
      return ResultadoMenu::pendiente;
  }
  if (id != 3 && id != 70 && id != 76 && id != 98 && id != 99 && id != 100 && id != 101 &&
      id != 102 && id != 105 && id != 106 && id != 108 && id != 109 && id != 110 && id != 111 &&
      id != 112 && id != 113 && id != 114 && id != 116 && id != 120 && id != 121)
    return ResultadoMenu::pendiente;
  if (manejador == 0x384a)
    return ResultadoMenu::actualizado;
  if (manejador == 0x2fca || manejador == 0x3143 || manejador == 0x3b53) {
    marcar_accion_rechazada(d);
    return ResultadoMenu::accion;
  }
  return ResultadoMenu::pendiente;
}
// Traduce las respuestas y los intercambios ofrecidos en el trayecto de la población.
ResultadoMenu ejecutar_interaccion_poblacion(unsigned char *d, unsigned short manejador,
                                             unsigned short id, const ServicioSonido &sonido) {
  const auto escena = palabra(d, 0x8082);
  if (escena < 17 || (escena > 29 && escena != 31))
    return ResultadoMenu::pendiente;
  if (escena == 31 && manejador == 0x32de && id == 1) {
    // 361A–3727: primera visita y regreso comparten diálogo; el bit1 conserva la progresión.
    Palabra registro = 0x009c;
    guardar(d, 0x0adc, 0x247a);
    if (!(d[0x35c9] & 1)) {
      d[0x35c9] |= 1;
      d[0x41ec] = d[0x41f5] = 3;
      guardar(d, 0x0adc, 0x2442);
      actor(d, registro, 0x2c6d, 0x1e0b, 0x256c);
      registro = static_cast<Palabra>(registro + 10);
    }
    guardar(d, 0x1a38, 123);
    guardar(d, 0x1a3a, 186);
    guardar(d, 0x0ad7, 0x241d);
    guardar(d, 0x0ae1, 0x245d);
    actor(d, registro, 0x2c4c, 0x010b, 0x2571);
    registro = static_cast<Palabra>(registro + 10);
    const Palabra canales[][2] = {{0x25e6, 0x25ec}, {0x25f2, 0x25f8}, {0x25fe, 0x2604},
                                  {0x260a, 0x2610}, {0x2616, 0x261c}, {0x2622, 0x2628}};
    for (unsigned int n = 0; n < 6; ++n) {
      actor(d, registro, static_cast<Palabra>(0x2c8e + n * 33), n == 5 ? 0x0273 : 0x0247, 0x25e0,
            canales[n][0], canales[n][1]);
      registro = static_cast<Palabra>(registro + 10);
    }
    return comenzar(d);
  }
  // 5C8F elige 2C6D en DRAGON, frente a 2C8E en el camino del asaltante.
  if (escena == 21 && manejador == 0x563a && id == 76)
    return ejecutar_interaccion_asaltante(d, manejador, id, sonido, {});
  if (manejador == 0x32de && id <= 1) {
    // 358A–35B0: el saludo del mago dispara su respuesta antes del sonido común 37FF.
    if (id == 1 && escena == 22 && d[0x2c4c] == 1) {
      guardar(d, 0x2ce0, 0x1a2f);
      d[0x2ce2] = 0;
      d[0x2ce3] = 1;
      d[0x2ce4] = 40;
    }
    // 35DC–3617: Sally y Frank responden a «Hi» mediante su canal de conversación.
    if (id == 1 && (escena == 26 || escena == 29)) {
      guardar(d, 0x2c7d, 0x1a2f);
      d[0x2c7f] = 0;
      d[0x2c80] = 1;
      d[0x2c81] = 40;
    }
    // 35B3–35D9: el saludo de Benn depende de que su entidad siga activa.
    if (id == 1 && escena == 24 && d[0x2c4c] == 1) {
      guardar(d, 0x2cbf, 0x1a2f);
      d[0x2cc1] = 0;
      d[0x2cc2] = 1;
      d[0x2cc3] = 40;
    }
    return emitir_sonido(sonido, d, 0x2c2b, static_cast<unsigned char>(id)) ? ResultadoMenu::accion
                                                                            : ResultadoMenu::error;
  }
  if (manejador == 0x430f) {
    // 430F–43B1: vestir el disfraz solo dentro del probador; las demoras dependen de AL de 886B.
    if (escena != 25) {
      marcar_accion_rechazada(d);
      return ResultadoMenu::accion;
    }
    retirar_identificador(d, 79);
    d[0x422d] = d[0x421b] = 1;
    guardar(d, 0x4231, static_cast<Palabra>(palabra(d, 0x4231) + 2));
    d[0x421c] = 13;
    guardar(d, 0x1a38, 176);
    guardar(d, 0x1a3a, 157);
    guardar(d, 0x0ad7, 0x1b8e);
    guardar(d, 0x0adc, 0x1bb3);
    constexpr Palabra canales[] = {0x2c7d, 0x2c9e, 0x2ca8, 0x2c5c};
    constexpr Palabra referencias[] = {0x1fc7, 0x1fcb, 0x1fd1, 0x1fd7};
    for (unsigned int i = 0; i < 4; ++i) {
      guardar(d, canales[i], referencias[i]);
      d[canales[i] + 2] = 0;
      d[canales[i] + 4] = 2;
    }
    guardar(d, 0x2c6f, 136);
    guardar(d, 0x2c71, 137);
    unsigned short pasos = 0;
    if (!iniciar_accion(d, pasos))
      return ResultadoMenu::error;
    d[0x2c80] = static_cast<unsigned char>(pasos + 14);
    d[0x2ca1] = d[0x2cab] = static_cast<unsigned char>(pasos + 72);
    d[0x2c5f] = static_cast<unsigned char>(pasos + 82);
    guardar(d, 0x35c7, static_cast<Palabra>(palabra(d, 0x35c7) + 5));
    return ResultadoMenu::accion;
  }
  if (manejador == 0x563a && escena == 24 && id == 79) {
    // 563A–5679 y 5CAF–5CF3: capacidad antes de retirar el vestido y animar a Benn.
    const auto icono = d[0x524e + id * 2];
    if (icono == 0x1f || icono == 0x22)
      guardar(d, 0x35c7, static_cast<Palabra>(palabra(d, 0x35c7) + (icono == 0x1f ? 1 : 2)));
    else if (!insertar_identificador(d, id)) {
      marcar_accion_rechazada(d);
      return ResultadoMenu::accion;
    }
    d[0x2c6d] = d[0x41ec] = d[0x41f5] = 0;
    guardar(d, 0x2c5c, 0x97d7);
    d[0x2c5e] = 0;
    d[0x2c60] = 2;
    guardar(d, 0x1a38, 128);
    guardar(d, 0x1a3a, 151);
    guardar(d, 0x0ad7, 0x96de);
    guardar(d, 0x0adc, 0x96e7);
    unsigned short pasos = 0;
    if (!iniciar_accion(d, pasos))
      return ResultadoMenu::error;
    d[0x2c5f] = static_cast<unsigned char>(pasos + 6);
    return ResultadoMenu::accion;
  }
  if (manejador == 0x419e) {
    // 419E–41D7: comer consume la comida solo con el modelo normal de Brad.
    if (d[0x358c] != 255) {
      marcar_accion_rechazada(d);
      return ResultadoMenu::accion;
    }
    retirar_identificador(d, 84);
    const bool derecha = d[0x2c2c] >= 18 && d[0x2c2c] <= 26;
    guardar(d, 0x2c3b, derecha ? 0x1a69 : 0x1a4e);
    guardar(d, 0x2c40, derecha ? 0x1a77 : 0x1a5c);
    return comenzar(d);
  }
  if (manejador == 0x41da) {
    // 41DA–420C: pagar a Frank incorpora comida y prepara su animación de entrega.
    retirar_identificador(d, 84);
    insertar_identificador(d, 84);
    guardar(d, 0x1a38, 150);
    guardar(d, 0x1a3a, 120);
    guardar(d, 0x0ad7, 0x96de);
    guardar(d, 0x0adc, 0x96e7);
    actor(d, 0x009c, 0x2c4c, 0x0209, 0x224c);
    return comenzar(d);
  }
  if (manejador == 0x44ab) {
    // 44AB–44FD: dar dinero a Benn ofrece el vestido solo en la fase tres del objeto.
    if (d[0x2c6d] != 3) {
      marcar_accion_rechazada(d);
      return ResultadoMenu::accion;
    }
    d[0x2c6d] |= 4;
    d[0x41f5] |= 4;
    guardar(d, 0x41f2, 0x1f9e);
    guardar(d, 0x1a38, static_cast<Palabra>(palabra(d, 0x2c4e) + 16));
    guardar(d, 0x1a3a, static_cast<Palabra>(palabra(d, 0x2c50) + palabra(d, 0x2c54) + 5));
    guardar(d, 0x0ad7, 0x96de);
    guardar(d, 0x0adc, 0x96e7);
    actor(d, 0x009c, 0x2c4c, 0x0207, 0x1fb0);
    guardar(d, 0x35c7, static_cast<Palabra>(palabra(d, 0x35c7) + 5));
    return comenzar(d);
  }
  if (manejador == 0x43b4) {
    // 43B4–44A8: las tres visitas al mago conservan sus guiones y ofertas persistentes.
    retirar_identificador(d, 84);
    Palabra a = 0x1def, b = 0x1e20, c = 0x1e4e;
    if (d[0x35c9] & 1) {
      if (!(d[0x35c9] & 2)) {
        if (d[0x402e] == 1) {
          marcar_accion_rechazada(d);
          return ResultadoMenu::accion;
        }
        d[0x402e] = 1;
        d[0x41ec] = d[0x41f5] = 0;
        constexpr Palabra ofertas[] = {0x4025, 0x4066, 0x4083, 0x40d6, 0x418e,
                                       0x41e1, 0x41fe, 0x4236, 0x42dc};
        for (const auto oferta : ofertas)
          d[oferta] = 1;
        a = 0x1e7e;
        b = 0x1ea7;
        c = 0x1ecf;
        actor(d, 0x00ba, 0x2caf, 0x023d, 0x1a2c);
      } else {
        if (!(d[0x35c9] & 4)) {
          marcar_accion_rechazada(d);
          return ResultadoMenu::accion;
        }
        a = 0x1ef7;
        b = 0x1f28;
        c = 0x1f56;
      }
    }
    guardar(d, 0x0ad7, a);
    guardar(d, 0x0adc, b);
    guardar(d, 0x0ae1, c);
    guardar(d, 0x1a38, 142);
    guardar(d, 0x1a3a, 143);
    actor(d, 0x009c, 0x2c4c, 0x022f, 0x1f84);
    actor(d, 0x00a6, 0x2c6d, 0x0233, 0x1f90);
    actor(d, 0x00b0, 0x2c8e, 0x0234, 0x1f90);
    guardar(d, 0x35c7, static_cast<Palabra>(palabra(d, 0x35c7) + 5));
    return comenzar(d);
  }
  if (manejador == 0x420f) {
    // 420F–430C: dar oro al identificador 81 conserva las cinco respuestas de 35C9.
    guardar(d, 0x1a38, 199);
    guardar(d, 0x1a3a, 159);
    guardar(d, 0x0ad7, 0x1b2f);
    guardar(d, 0x0adc, 0x1b50);
    guardar(d, 0x0ae1, 0x1b6f);
    const auto progreso = d[0x35c9];
    Palabra a = 0x20eb, b = 0x2129, c = 0x2156, x = 0x2183, y = 0x2194, z = 0x21a2;
    if (progreso & 1) {
      if (!(progreso & 6)) {
        a = 0x2072;
        b = 0x20a3;
        c = 0x20c7;
        x = 0x21b0;
        y = 0x21c1;
        z = 0x21cf;
      } else if ((progreso & 6) == 6) {
        a = 0x1fdd;
        b = 0x1ff4;
        c = 0x2006;
        x = 0x220a;
        y = 0x2211;
        z = 0x2218;
      } else {
        a = 0x2018;
        b = 0x203c;
        c = 0x2057;
        if (progreso & 2) {
          x = 0x21dd;
          y = 0x21ee;
          z = 0x21fc;
        } else {
          x = 0x221f;
          y = 0x2230;
          z = 0x223e;
        }
      }
    }
    actor(d, 0x009c, 0x2c4c, 0x0219, a, b, c);
    actor(d, 0x00a6, 0x2c8e, 0x022f, x, y, z);
    return comenzar(d);
  }
  // 3B4F: examinar no modifica el estado; 88A0 rechaza los usos sin combinación especial.
  if (manejador == 0x384a)
    return ResultadoMenu::actualizado;
  if (manejador == 0x2fca || manejador == 0x3143 || manejador == 0x3b53) {
    marcar_accion_rechazada(d);
    return ResultadoMenu::accion;
  }
  return ResultadoMenu::pendiente;
}
} // namespace motor
