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
  if (escena == 32 || escena == 33 || escena == 34 || escena == 36 || escena == 39)
    return actualizar_ramas_valle(datos);
  if (escena == 46) {
    // CFA9–CFD1: detiene la puerta de los dados al recortar sus límites verticales.
    const auto y = con_signo(palabra(datos, 0x2d16));
    if (y < 30 || y > 89) {
      guardar(datos, 0x2d16, y < 30 ? 30 : 89);
      guardar(datos, 0x2d2c, 0);
    }
    return true;
  }
  if (escena == 45) {
    // CF34–CFA8: recicla los cuatro grupos flotantes conservando referencias y contadores.
    if (con_signo(palabra(datos, 0x2d98)) >= 600) {
      static constexpr Palabra alturas[] = {152, 153, 155, 157};
      for (unsigned int i = 0; i < 4; ++i) {
        const auto entidad = static_cast<Palabra>(0x2d33 + i * 33);
        guardar(datos, entidad + 2, static_cast<Palabra>(-20 - static_cast<int>(i) * 10));
        guardar(datos, entidad + 4, alturas[i]);
        datos[entidad + 18] = datos[entidad + 23] = datos[entidad + 28] = 0;
      }
    }
    return true;
  }
  if (escena == 44) {
    // CF26–CF33: la criatura del lago bloquea el desplazamiento derecho al llegar a cuadro34.
    if (datos[0x2cd1] == 34)
      guardar(datos, 0x358f, 0);
    return true;
  }
  if (escena >= 40 && escena <= 43)
    return actualizar_nieve(datos);
  if (escena == 0 || escena == 2 || escena == 4 || escena == 7 || escena == 9 || escena == 10 ||
      escena == 11 || escena == 12 || escena == 13 || escena == 14 || escena == 15 ||
      escena == 18 || escena == 19 || escena == 21 || escena == 22 || escena == 23 ||
      escena == 26 || escena == 27 || escena == 28 || escena == 29 || escena == 31 || escena == 37)
    return true;
  if (escena == 20) {
    // 19C1–19DE: límite de proximidad de la calle de la adivina según el segundo actor.
    guardar(datos, 0x18f9, 60);
    guardar(datos, 0x18fb, 199);
    if (datos[0x2c6e] != 23)
      guardar(datos, 0x18fb, static_cast<Palabra>(palabra(datos, 0x2c71) + palabra(datos, 0x2c75)));
    return true;
  }
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
// Despacha BEFD, C093 y las rutinas de población con servicios comunes y azar original.
bool actualizar_rutina_escena(unsigned char *d, EstadoAleatorio &azar,
                              const RecursosEntidades &recursos) {
  if (palabra(d, 0x8082) == 38)
    return actualizar_nube(d, azar);
  if (palabra(d, 0x8082) == 35)
    return actualizar_poste(d, azar, recursos);
  if (palabra(d, 0x8082) == 30)
    return actualizar_borde(d, azar, recursos);
  if (palabra(d, 0x8082) == 25) {
    // 1A74–1ABE: siete intentos de creación, conservando orden y escrituras del descriptor.
    if (d[0x2c4d] == 12) {
      constexpr Palabra descriptores[] = {0x13ae, 0x13b7, 0x13c0};
      for (const Palabra descriptor : descriptores) {
        const auto campo = static_cast<Palabra>(palabra(d, descriptor + 7) + 8);
        const unsigned int limite = descriptor == 0x13ae ? 3 : 2;
        for (unsigned int i = 0; i < limite; ++i) {
          d[campo] = static_cast<unsigned char>(1 + i * (descriptor == 0x13ae ? 4 : 3));
          if (!crear_entidad(d, azar, recursos, descriptor).valido)
            return false;
        }
      }
    }
    if (d[0x358c] != 19)
      return true;
    if (!recursos.fase_probador)
      return false;
    const auto fotograma = d[0x2c2c];
    if (fotograma == 0 || fotograma == 3) {
      *recursos.fase_probador = fotograma == 3 ? 1 : 0;
      return true;
    }
    if (!((fotograma == 1 && *recursos.fase_probador == 0) ||
          (fotograma == 2 && *recursos.fase_probador == 1)))
      return true;
    // 1B11–1BA1: consume el azar también cuando se agotan los registros libres.
    const Palabra bases[] = {0x2c90, 0x2c2d, 0x2c6f};
    for (unsigned int grupo = fotograma == 2 ? 0u : 1u; grupo < 3; ++grupo) {
      const auto base = bases[grupo];
      const Palabra ancho = grupo == 1 ? 19 : palabra(d, base + 4);
      const Palabra alto = grupo == 1 ? 43 : palabra(d, base + 6);
      for (unsigned int i = 0; i < (grupo == 1 ? 10u : 6u); ++i) {
        Palabra valor = 0;
        if (!generar_aleatorio(azar, ancho, valor))
          return false;
        guardar(d, 0x14a0, static_cast<Palabra>(valor + palabra(d, base)));
        if (!generar_aleatorio(azar, alto, valor))
          return false;
        guardar(d, 0x14a2, static_cast<Palabra>(valor + palabra(d, base + 2)));
        if (!generar_aleatorio(azar, 3, valor))
          return false;
        d[0x149f] = static_cast<unsigned char>(valor + 15);
        if (!crear_entidad(d, azar, recursos, 0x149d).valido)
          return false;
      }
    }
    return true;
  }
  if (palabra(d, 0x8082) == 24) {
    // 19DF–1A73: proximidad de Benn y quince partículas cuando ya se ha recuperado el ventilador.
    if (!d[0x2c4c])
      return true;
    guardar(d, 0x18f5, palabra(d, 0x2c4e));
    guardar(d, 0x18f9, static_cast<Palabra>(palabra(d, 0x2c4e) + palabra(d, 0x2c52)));
    const auto pie = static_cast<Palabra>(palabra(d, 0x2c50) + palabra(d, 0x2c54));
    guardar(d, 0x18fb, pie);
    guardar(d, 0x18f7, static_cast<Palabra>(pie - 5));
    if (!(d[0x35c9] & 4) || palabra(d, 0x2c5c) == 0x1383)
      return true;
    if (!palabra(d, 0x2c5c)) {
      d[0x41ec] = 0;
      configurar_secuencia(d, 0x2c5c, 0x1383, 5, 2);
      return true;
    }
    if (d[0x2c4c] != 1)
      return true;
    for (unsigned int i = 0; i < 15; ++i) {
      Palabra valor = 0;
      if (!generar_aleatorio(azar, 24, valor))
        return false;
      guardar(d, 0x138d, static_cast<Palabra>(valor + 4 + palabra(d, 0x2c2d)));
      if (!generar_aleatorio(azar, 47, valor))
        return false;
      guardar(d, 0x138f, static_cast<Palabra>(valor + 1 + palabra(d, 0x2c2f)));
      if (!generar_aleatorio(azar, 4, valor))
        return false;
      d[0x138c] = static_cast<unsigned char>(valor + 17);
      if (!crear_entidad(d, azar, recursos, 0x138a).valido)
        return false;
    }
    return true;
  }
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
