#include "catapulta.hpp"
#include "iniciar_accion.hpp"
#include "lista_identificadores.hpp"
#include "../estado/segmento.hpp"

namespace motor {
namespace {
// 886B: entrega la acción al planificador común sin sustituir su recorrido.
ResultadoMenu comenzar(unsigned char *d) {
  unsigned short pasos = 0;
  return iniciar_accion(d, pasos) ? ResultadoMenu::accion : ResultadoMenu::error;
}
// Dispone fase e intervalo; conserva la demora hasta que el manejador original la sustituye.
void canal(unsigned char *d, Palabra campo, Palabra secuencia) {
  guardar(d, campo, secuencia);
  d[campo + 2] = 0;
  d[campo + 4] = 1;
}
// 5652–5679: puntuación por icono o inserción en las diez casillas originales.
bool incorporar(unsigned char *d, Palabra id) {
  const auto icono = d[0x524e + id * 2];
  if (icono == 0x1f || icono == 0x22) {
    guardar(d, 0x35c7, static_cast<Palabra>(palabra(d, 0x35c7) + (icono == 0x1f ? 1 : 2)));
    return true;
  }
  return insertar_identificador(d, id);
}
// SUB/JGE/NEG: conserva la decisión con signo antes de envolver el resultado de 16 bits.
Palabra distancia_original(int resta) {
  const auto resultado = static_cast<Palabra>(resta);
  return resta < 0 ? static_cast<Palabra>(0 - resultado) : resultado;
}
} // namespace

// Reconstruye los manejadores y sus escrituras; las secuencias del EXE ejecutan las animaciones.
ResultadoMenu ejecutar_interaccion_catapulta(unsigned char *d, unsigned short manejador,
                                             unsigned short id, const ServicioSonido &sonido) {
  const auto escena = palabra(d, 0x8082);
  if (manejador == 0xa383) {
    if (!d[0x3594] || !retirar_identificador(d, 57)) {
      marcar_accion_rechazada(d);
      return ResultadoMenu::accion;
    }
    retirar_identificador(d, 55);
    insertar_identificador(d, 70);
    guardar(d, 0x2c3b, 0x9fd6);
    guardar(d, 0x2c40, 0x9fe3);
    guardar(d, 0x35c7, static_cast<Palabra>(palabra(d, 0x35c7) + 5));
    return comenzar(d);
  }
  if (escena != 11 && escena != 12 && escena != 13)
    return ResultadoMenu::pendiente;
  if (manejador == 0xa07e && escena == 11) {
    retirar_identificador(d, 56);
    d[0x3593] = 0;
    guardar(d, 0x2c3b, 0x9f1d);
    guardar(d, 0x2c40, 0x9f25);
    guardar(d, 0x2c45, 0x9f42);
    d[0x3e7c] = 6;
    guardar(d, 0x3e7d, 124);
    guardar(d, 0x2c4e, 124);
    guardar(d, 0x3e7f, 121);
    guardar(d, 0x2c50, 121);
    guardar(d, 0x2c56, 10);
    guardar(d, palabra(d, 0x3e81), 10);
    canal(d, 0x2c5c, 0x9f5f);
    d[0x2c5f] = 0x3a;
    guardar(d, 0x35c7, static_cast<Palabra>(palabra(d, 0x35c7) + 5));
    return comenzar(d);
  }
  if (manejador == 0x3143 && escena == 11 && id == 59) {
    // 3196–320A: solo permite bajar de la cornisa con el imán o su combinación en inventario.
    if (!contiene_identificador(d, 57) && !contiene_identificador(d, 70)) {
      marcar_accion_rechazada(d);
      return ResultadoMenu::accion;
    }
    guardar(d, 0x2c3b, 0x2280);
    guardar(d, 0x2c40, 0x228d);
    guardar(d, 0x2c45, 0x2293);
    d[0x3e7c] = 7;
    guardar(d, palabra(d, 0x3e81), 0xfff4);
    guardar(d, 0x2c56, 0xfff4);
    guardar(d, 0x3e7d, 133);
    guardar(d, 0x3e7f, 110);
    d[0x3e8e] = 11;
    d[0x3e8d] &= 0xfd;
    guardar(d, palabra(d, 0x3e93), 0xfffc);
    guardar(d, 0x2c98, 0xfffc);
    guardar(d, 0x3e8f, palabra(d, 0x2c90));
    guardar(d, 0x3e91, palabra(d, 0x2c92));
    guardar(d, 0x35c7, static_cast<Palabra>(palabra(d, 0x35c7) + 5));
    return comenzar(d);
  }
  if (manejador == 0x3143 && (escena == 11 || escena == 12) && id == 60) {
    // 320B–324B: salto sobre el apoyo; el modelo de Brad catorce lo rechaza.
    if (d[0x358c] == 14) {
      marcar_accion_rechazada(d);
      return ResultadoMenu::accion;
    }
    guardar(d, 0x2c56, 0xffec);
    guardar(d, 0x1a38, static_cast<Palabra>(palabra(d, 0x2c4e) - 18));
    guardar(d, 0x1a3a, static_cast<Palabra>(palabra(d, 0x2c50) + 183));
    guardar(d, 0x0ad7, 0x2299);
    guardar(d, 0x0adc, 0x22ab);
    guardar(d, 0x0ae1, 0x22b1);
    guardar(d, 0x35c7, static_cast<Palabra>(palabra(d, 0x35c7) + 5));
    return comenzar(d);
  }
  if (manejador == 0xa0db && escena == 11) {
    const auto horizontal =
        distancia_original(con_signo(palabra(d, 0x2c2d)) - con_signo(palabra(d, 0x2c4e)));
    const auto pies =
        static_cast<Palabra>(palabra(d, 0x2c2f) + palabra(d, 0x2c33) - palabra(d, 0x2c50));
    const auto vertical = distancia_original(con_signo(pies) - con_signo(palabra(d, 0x2c54)));
    if (d[0x2c4d] != 6 || con_signo(horizontal) > 60 || con_signo(vertical) > 5) {
      marcar_accion_rechazada(d);
      return ResultadoMenu::accion;
    }
    retirar_identificador(d, 61);
    d[0x52bf] |= 2;
    guardar(d, 0x1a38, static_cast<Palabra>(palabra(d, 0x2c4e) - 1));
    guardar(d, 0x2c90, static_cast<Palabra>(palabra(d, 0x2c4e) + 5));
    guardar(d, 0x1a3a, static_cast<Palabra>(palabra(d, 0x2c50) + 39));
    guardar(d, 0x2c92, static_cast<Palabra>(palabra(d, 0x2c50) + 4));
    canal(d, 0x2c9e, 0x9fa1);
    canal(d, 0x2ca3, 0x9fa8);
    canal(d, 0x2ca8, 0x9fbc);
    guardar(d, 0x2c56, 180);
    canal(d, 0x2c5c, 0x9fd0);
    canal(d, 0x2c61, 0x9fd2);
    canal(d, 0x2c66, 0x9fd4);
    guardar(d, 0x0ad7, 0x9f64);
    guardar(d, 0x0adc, 0x9f7d);
    guardar(d, 0x0ae1, 0x9f8f);
    unsigned short pasos = 0;
    if (!iniciar_accion(d, pasos))
      return ResultadoMenu::error;
    constexpr Palabra demoras_monitor[] = {0x2ca1, 0x2ca6, 0x2cab};
    constexpr Palabra demoras_tablon[] = {0x2c5f, 0x2c64, 0x2c69};
    for (const auto campo : demoras_monitor)
      d[campo] = static_cast<unsigned char>(pasos + 8);
    for (const auto campo : demoras_tablon)
      d[campo] = static_cast<unsigned char>(pasos + 26);
    guardar(d, 0x35c7, static_cast<Palabra>(palabra(d, 0x35c7) + 5));
    return ResultadoMenu::accion;
  }
  if (manejador == 0x563a &&
      ((escena == 11 && (id == 56 || id == 57)) || (escena == 13 && id == 61))) {
    if (!incorporar(d, id)) {
      marcar_accion_rechazada(d);
      return ResultadoMenu::accion;
    }
    if (id == 56) {
      d[0x52bd] &= 0xfd;
      d[0x3593] = 1;
      canal(d, 0x2c5c, 0x1a34);
      guardar(d, 0x1a38, static_cast<Palabra>(palabra(d, 0x2c4e) + 5));
      guardar(d, 0x1a3a, static_cast<Palabra>(palabra(d, 0x2c50) + 2));
      guardar(d, 0x0ad7, 0x975a);
      guardar(d, 0x0adc, 0x9765);
      guardar(d, 0x0ae1, 0x976b);
      const auto resultado = comenzar(d);
      d[0x2c5f] = static_cast<unsigned char>(palabra(d, 0x359a) + 6);
      return resultado;
    }
    if (id == 57) {
      d[0x3e84] = 0;
      canal(d, 0x2c7d, 0x1a2e);
      d[0x2c80] = 6;
      guardar(d, 0x2c3b, 0x96ed);
      guardar(d, 0x2c40, 0x96c9);
      return comenzar(d);
    }
    d[0x52c7] &= 0xfd;
    guardar(d, 0x1a38, static_cast<Palabra>(palabra(d, 0x2cb1) + 5));
    guardar(d, 0x1a3a, static_cast<Palabra>(palabra(d, 0x2cb3) + palabra(d, 0x2cb7) - 4));
    const auto lista = palabra(d, 0x53b5 + 13 * 2);
    d[static_cast<Palabra>(lista + 3 * 9 + 1)] = 0;
    canal(d, 0x2cbf, 0x1a2e);
    guardar(d, 0x0ad7, d[0x358c] == 0x14 ? 0x96f6 : 0x96c0);
    guardar(d, 0x0adc, d[0x358c] == 0x14 ? 0x96ff : 0x96c9);
    unsigned short pasos = 0;
    if (!iniciar_accion(d, pasos))
      return ResultadoMenu::error;
    d[0x2cc2] = static_cast<unsigned char>(pasos + 6);
    return ResultadoMenu::accion;
  }
  if (manejador == 0xa3b8 || (manejador == 0x384a && id == 62)) {
    if (escena != 13) {
      marcar_accion_rechazada(d);
      return ResultadoMenu::accion;
    }
    guardar(d, 0x1a38, static_cast<Palabra>(palabra(d, 0x2c6f) + 19));
    guardar(d, 0x1a3a, static_cast<Palabra>(palabra(d, 0x2c71) + 42));
    guardar(d, 0x0ad7, 0xa018);
    guardar(d, 0x0adc, 0xa02a);
    const auto resultado = comenzar(d);
    d[0x3596] = 1;
    if (d[0x2c8e]) {
      if (manejador == 0xa3b8) {
        insertar_identificador(d, 63);
        d[0x3ed9] = d[0x2c8e] = 0;
        guardar(d, 0x35c7, static_cast<Palabra>(palabra(d, 0x35c7) + 5));
      } else {
        guardar(d, 0x2c4e, palabra(d, 0x2c2d));
        guardar(d, 0x2c50, static_cast<Palabra>(palabra(d, 0x2c2f) - 30));
        canal(d, 0x2c5c, 0x1db0);
        d[0x2c5f] = 1;
        d[0x2c60] = 10;
        d[0x2c8e] |= 4;
      }
    }
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
