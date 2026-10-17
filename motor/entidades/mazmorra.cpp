#include "mazmorra.hpp"
#include "actualizar.hpp"
#include "../estado/segmento.hpp"
#include "../acciones/lista_identificadores.hpp"
#include "../movimiento/planificar.hpp"

namespace motor {
namespace {
// Prepara referencia, fase, contador e intervalo sin alterar el resto del registro.
void configurar_secuencia(unsigned char *datos, Palabra posicion, Palabra referencia,
                          unsigned char contador, unsigned char intervalo) {
  guardar(datos, posicion, referencia);
  datos[static_cast<Palabra>(posicion + 2)] = 0;
  datos[static_cast<Palabra>(posicion + 3)] = contador;
  datos[static_cast<Palabra>(posicion + 4)] = intervalo;
}
} // namespace

// Conserva las condiciones y coordenadas de 183F, incluidas las sumas de palabra con envoltura.
void actualizar_escena_cinco(unsigned char *datos) {
  if (datos[0x2cf1] == 0 || palabra(datos, 0x2d01) != 0 || datos[0x3594] == 0)
    return;
  guardar(datos, 0x2cf3, static_cast<Palabra>(palabra(datos, 0x2d14) + 0x79));
  guardar(datos, 0x2cf5, static_cast<Palabra>(palabra(datos, 0x2d16) + 0x19));
  configurar_secuencia(datos, 0x2d01, 0x1211, 1, 2);
  configurar_secuencia(datos, 0x2d06, 0x124b, 1, 2);
  configurar_secuencia(datos, 0x2d0b, 0x1280, 1, 2);
  if (datos[0x2d54] != 1)
    return;
  datos[0x3ac7] = 0;
  datos[0x3af4] = 1;
  guardar(datos, 0x2d01, 0x12b5);
  configurar_secuencia(datos, 0x2da6, 0x1a2c, 0x35, 2);
  configurar_secuencia(datos, 0x2d85, 0x12ee, 0x2f, 2);
  configurar_secuencia(datos, 0x2d8f, 0x12f7, 0x2f, 2);
}
// La tabla CS:17A3 contiene cero para la mazmorra (índice 0) y 183F para el índice 5.
bool actualizar_rutina_escena(unsigned char *datos) {
  const auto escena = palabra(datos, 0x8082);
  if (escena == 0 || escena == 2 || escena == 4 || escena == 7 || escena == 9 || escena == 10 ||
      escena == 11 || escena == 12 || escena == 13 || escena == 14 || escena == 15 ||
      escena == 18 || escena == 21 || escena == 22 || escena == 29)
    return true;
  if (escena == 17) {
    // 19A7–19C0: la fuente delimita la zona próxima mientras su entidad permanece activa.
    if (datos[0x2c6d] == 1) {
      guardar(datos, 0x18f9, 319);
      guardar(datos, 0x18f7, 190);
      guardar(datos, 0x18fb, 199);
    }
    return true;
  }
  if (escena == 16) {
    // 1993–19A6: conserva la franja de proximidad del asaltante en la entrada siguiente.
    if (datos[0x2c4c]) {
      guardar(datos, 0x18f9, 228);
      guardar(datos, 0x18fb, 199);
    }
    return true;
  }
  if (escena == 1) {
    // BEA8–BEFC: reacción del corredor en sus ventanas 24 y cero, con coordenadas con signo.
    if (datos[0x3597] == 24 && (datos[0x2cf1] & 2)) {
      datos[0x2cf1] &= 0xfd;
      guardar(datos, 0x2d01, 0xadd3);
    }
    if (datos[0x3597] == 0 && datos[0x3594] && con_signo(palabra(datos, 0x2c2d)) < 90) {
      retirar_identificador(datos, 1);
      guardar(datos, 0x1a38, 55);
      guardar(datos, 0x1a3a, 173);
      guardar(datos, 0x0dfa, 0x1a36);
      unsigned short pasos = 0;
      if (!planificar_movimiento(datos, 0x1a36, pasos))
        return false;
      datos[0x3594] = datos[0x3595] = 0;
    }
    return true;
  }
  if (escena == 5) {
    actualizar_escena_cinco(datos);
    return true;
  }
  return false;
}
// Reproduce BEFD sin inventar los efectos de sonido ni alterar sus consumos de azar.
bool actualizar_rutina_escena(unsigned char *d, EstadoAleatorio &azar,
                              const RecursosEntidades &recursos) {
  if (palabra(d, 0x8082) == 6) {
    // 1905–1962: habilita el efecto de la franja 832–896 y conserva sus cuatro consumos de azar.
    const auto x = static_cast<Palabra>(palabra(d, 0x2c2d) + palabra(d, 0x3591));
    d[0x0def] = con_signo(x) >= 832 && con_signo(x) <= 896 ? 1 : 0;
    Palabra valor = 0;
    if (!generar_aleatorio(azar, 3, valor))
      return false;
    if (!valor)
      return true;
    if (!generar_aleatorio(azar, 4, valor))
      return false;
    d[0x120a] = static_cast<unsigned char>(valor + 2);
    if (!generar_aleatorio(azar, 2, valor))
      return false;
    d[0x120a] |= static_cast<unsigned char>(valor << 7);
    if (!generar_aleatorio(azar, 63, valor))
      return false;
    guardar(d, 0x120b, static_cast<Palabra>(valor + 832 - palabra(d, 0x3591)));
    return crear_entidad(d, azar, recursos, 0x1208).valido;
  }
  if (palabra(d, 0x8082) == 8) {
    // 1963–1992: la poza crea partículas solo cuando la presentación original está activa.
    if (!d[0x0def])
      return true;
    Palabra valor = 0;
    if (!generar_aleatorio(azar, 2, valor))
      return false;
    if (valor)
      return true;
    if (!generar_aleatorio(azar, 200, valor))
      return false;
    guardar(d, 0x1301, static_cast<Palabra>(valor + 50));
    if (!generar_aleatorio(azar, 10, valor))
      return false;
    guardar(d, 0x1303, static_cast<Palabra>(valor - 20));
    return crear_entidad(d, azar, recursos, 0x12fe).valido;
  }
  if (palabra(d, 0x8082) != 3)
    return actualizar_rutina_escena(d);
  const auto izquierda = static_cast<Palabra>(palabra(d, 0x3591) + palabra(d, 0x2e3d) - 11);
  guardar(d, 0x18f5, izquierda);
  guardar(d, 0x18f9, static_cast<Palabra>(izquierda + 42));
  guardar(d, 0x18fb, 199);
  const bool sumergido = d[0x358c] == 5 && static_cast<signed char>(d[0x2c3d]) <= 12;
  bool emitir = sumergido;
  if (!sumergido && d[0x358c] != 10) {
    --d[0xadce];
    if (!d[0xadce]) {
      Palabra valor = 0;
      if (!generar_aleatorio(azar, 50, valor))
        return false;
      d[0xadce] = static_cast<unsigned char>(valor + 1);
      emitir = true;
      // BF3C–BF41: burbujeo de Brad mediante el selector común, sin consumir azar.
      if (!emitir_sonido(recursos.sonido, d, 0x2c2b, 0x11))
        return false;
    }
  }
  if (emitir) {
    Palabra valor = 0;
    if (!generar_aleatorio(azar, 4, valor))
      return false;
    const unsigned int cantidad = valor + 2u;
    for (unsigned int i = 0; i < cantidad; ++i) {
      if (!generar_aleatorio(azar, 3, valor))
        return false;
      const auto descriptor = static_cast<Palabra>(0xaddf + valor * 9);
      if (!generar_aleatorio(azar, 24, valor))
        return false;
      const auto x = static_cast<Palabra>(valor - 12 + palabra(d, 0x2c31));
      guardar(d, static_cast<Palabra>(descriptor + 3),
              static_cast<Palabra>((x >> 1) + palabra(d, 0x2c2d)));
      if (!generar_aleatorio(azar, 6, valor))
        return false;
      guardar(d, static_cast<Palabra>(descriptor + 5),
              static_cast<Palabra>(valor + 10 + palabra(d, 0x2c2f)));
      const auto creada = crear_entidad(d, azar, recursos, descriptor);
      if (!creada.valido)
        return false;
      if (!generar_aleatorio(azar, 12, valor))
        return false;
      // 8B6F devuelve DI=3510 cuando no queda registro libre; el original escribe también en ese
      // caso.
      const auto posicion = creada.creada ? creada.posicion : static_cast<Palabra>(0x3510);
      d[static_cast<Palabra>(posicion + 0x17)] = static_cast<unsigned char>(valor);
    }
  }
  if (con_signo(palabra(d, 0x2c6f)) < -400) {
    guardar(d, 0x2c6f, static_cast<Palabra>(palabra(d, 0x2c2d) + 100));
    guardar(d, 0x2c71, 0xfff2);
    configurar_secuencia(d, 0x2c7d, 0xae69, 1, 1);
    configurar_secuencia(d, 0x2c82, 0xae6e, 1, 1);
    configurar_secuencia(d, 0x2c87, 0xae79, 1, 1);
    guardar(d, 0x2cd2, static_cast<Palabra>(palabra(d, 0x2c2d) + 97));
    guardar(d, 0x2cd4, 0xfff4);
    configurar_secuencia(d, 0x2ce0, 0xaeb0, 1, 1);
    configurar_secuencia(d, 0x2ce5, 0xaeb2, 1, 1);
    configurar_secuencia(d, 0x2cea, 0xaeed, 1, 1);
  }
  if (d[0x389e] && static_cast<signed char>(d[0x3597]) >= 10 &&
      static_cast<signed char>(d[0x3597]) <= 15 && d[0x2df9] == 3) {
    d[0x2df9] = 1;
    guardar(d, 0x2dfb, 0xff70);
    guardar(d, 0x2dfd, 100);
    configurar_secuencia(d, 0x2e09, 0xaf2d, 1, 2);
    configurar_secuencia(d, 0x2e13, 0xaf32, 1, 2);
  }
  return true;
}
} // namespace motor
