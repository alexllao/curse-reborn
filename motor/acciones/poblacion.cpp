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
// Traduce las respuestas y los intercambios ofrecidos en el trayecto de la población.
ResultadoMenu ejecutar_interaccion_poblacion(unsigned char *d, unsigned short manejador,
                                             unsigned short id, const ServicioSonido &sonido) {
  const auto escena = palabra(d, 0x8082);
  if (escena < 17 || escena > 29)
    return ResultadoMenu::pendiente;
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
