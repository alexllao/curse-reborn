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
  // CS17A3: los tres corredores y las seis habitaciones laterales tienen manejador nulo.
  if ((escena >= 47 && escena <= 55) || (escena >= 59 && escena <= 62) || escena == 64 ||
      escena == 66 || escena == 67 || escena == 68 || escena == 69 || escena == 70 ||
      escena == 71 || escena == 73)
    return true;
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
  if (palabra(d, 0x8082) == 72)
    return actualizar_bruja(d, azar, recursos);
  if (palabra(d, 0x8082) == 65 || palabra(d, 0x8082) == 77)
    return actualizar_cementerio(d, azar, recursos);
  // D41C–D578: sueño del guardia detrás de la columna, reacción y diez partículas por intento.
  if (palabra(d, 0x8082) == 63) {
    if (d[0x2c4c] == 1) {
      guardar(d, 0x18f5, 190);
      guardar(d, 0x18f9, 261);
      guardar(d, 0x18fb, 158);
      const auto dy = static_cast<Palabra>(palabra(d, 0x2c2f) - 122);
      const auto absdy = con_signo(palabra(d, 0x2c2f)) < 122 ? static_cast<Palabra>(0 - dy) : dy;
      if (con_signo(palabra(d, 0x2c2d)) <= 80 && con_signo(absdy) < 5) {
        const auto previo = con_signo(palabra(d, 0x35ce));
        auto dormir = static_cast<Palabra>(palabra(d, 0x35ce) + 1);
        guardar(d, 0x35ce, dormir);
        // INC/JGE conserva OF: el salto usa el incremento con signo antes de envolver.
        if (previo >= -1) {
          if (con_signo(dormir) > 100)
            guardar(d, 0x35ce, 100);
          if (palabra(d, 0x2c5c) != 0xb526) {
            configurar_secuencia(d, 0x2c5c, 0xb526, 1, 4);
            configurar_secuencia(d, 0x2c7d, 0xb533, 1, 2);
          }
        }
      } else {
        const auto previo = con_signo(palabra(d, 0x35ce));
        const auto dormir = static_cast<Palabra>(palabra(d, 0x35ce) - 1);
        guardar(d, 0x35ce, dormir);
        // DEC/JLE también distingue el desbordamiento de8000 a7FFF.
        if (previo <= 1) {
          guardar(d, 0x35ce, 0xffec);
          if (palabra(d, 0x2c5c) == 0xb526) {
            configurar_secuencia(d, 0x2c5c, 0xb531, 1, 4);
            configurar_secuencia(d, 0x2c7d, 0xb535, 1, 2);
          }
          if (palabra(d, 0x2c7d) == 0) {
            if (con_signo(palabra(d, 0x2c2d)) < 230 && d[0x2c6e] == 10)
              configurar_secuencia(d, 0x2c7d, 0xb520, 1, 2);
            else if (con_signo(palabra(d, 0x2c2d)) >= 230 && d[0x2c6e] == 8)
              configurar_secuencia(d, 0x2c7d, 0xb523, 1, 2);
          }
        }
      }
    }
    if (d[0x2d33] == 1 && d[0x2d34] != 30 && d[0x2d34] != 36 &&
        static_cast<signed char>(d[0x2d45]) >= 8) {
      for (unsigned int i = 0; i < 10; ++i) {
        Palabra valor = 0;
        if (!generar_aleatorio(azar, 24, valor))
          return false;
        guardar(d, 0xb53c, static_cast<Palabra>(valor + 4 + palabra(d, 0x2c2d)));
        if (!generar_aleatorio(azar, 47, valor))
          return false;
        guardar(d, 0xb53e, static_cast<Palabra>(valor + 1 + palabra(d, 0x2c2f)));
        if (!generar_aleatorio(azar, 4, valor))
          return false;
        d[0xb53b] = static_cast<unsigned char>(valor + 37);
        if (!crear_entidad(d, azar, recursos, 0xb539).valido)
          return false;
      }
    }
    return true;
  }
  // D36C–D41B: la mano atrapa al avanzar por la franja del suelo y ordena su profundidad.
  if (palabra(d, 0x8082) == 58) {
    if (d[0x2c4c] == 3 && con_signo(palabra(d, 0x2c2f)) <= 130 && d[0x3594]) {
      d[0x4d94] = 0;
      d[0x3594] = d[0x3595] = d[0x3596] = 0;
      guardar(d, 0x359a, 0);
      guardar(d, 0x0dfa, 0);
      guardar(d, 0x2c4e, static_cast<Palabra>(palabra(d, 0x2c2d) + 128));
      guardar(d, 0x2c50, static_cast<Palabra>(palabra(d, 0x2c2f) - 106));
      configurar_secuencia(d, 0x2c3b, 0xb4ee, 1, 1);
      configurar_secuencia(d, 0x2c40, 0xb4fa, 1, 1);
      configurar_secuencia(d, 0x2c45, 0xb503, 1, 1);
      guardar(d, 0x2c5c, 0xb4cd);
      guardar(d, 0x2c61, 0xb4d4);
      guardar(d, 0x2c66, 0xb4e1);
    }
    if (d[0x2c2b] & 2)
      guardar(d, 0x2c56, 200);
    return true;
  }
  // D114–D36B: proyectiles de la reina, retroceso al recibir un golpe y acceso al trono.
  if (palabra(d, 0x8082) == 57) {
    d[0x3593] = 1;
    if (con_signo(palabra(d, 0x2c2f)) < 125)
      return true;
    if (con_signo(palabra(d, 0x2c2d)) >= 197) {
      if (!d[0x3594])
        return true;
      d[0x3594] = d[0x3595] = d[0x3596] = 0;
      guardar(d, 0x1a38, 212);
      guardar(d, 0x1a3a, 144);
      d[0x2c3d] = d[0x2c42] = d[0x2c47] = 0;
      guardar(d, 0x0ad7, 0xb479);
      guardar(d, 0x0adc, 0xb487);
      guardar(d, 0x0ae1, 0xb490);
      guardar(d, 0x009c, 0x2cd0);
      guardar(d, 0x009e, 0x0211);
      guardar(d, 0x00a0, 0xb499);
      guardar(d, 0x00a2, 0xb4ab);
      guardar(d, 0x00a4, 0xb4bc);
      guardar(d, 0x00a6, 0x2c4c);
      guardar(d, 0x00a8, 0x0201);
      guardar(d, 0x00aa, 0xb3ea);
      guardar(d, 0x0dfa, 0x1a36);
      unsigned short pasos = 0;
      return planificar_movimiento(d, 0x1a36, pasos);
    }
    if (d[0x358c] == 24 && d[0x2c2c] == 47) {
      if (con_signo(palabra(d, 0x2c2d)) <= 40) {
        configurar_secuencia(d, 0x2c3b, 0xb3e0, 1, 2);
        configurar_secuencia(d, 0x2c45, 0xb3e5, 1, 2);
      } else {
        Palabra paso = static_cast<Palabra>(palabra(d, 0x2c2d) - 37);
        paso = static_cast<Palabra>(paso >> 2);
        if (con_signo(paso) > 10)
          paso = 10;
        guardar(d, 0x2c2d, static_cast<Palabra>(palabra(d, 0x2c2d) - paso));
      }
    } else if (con_signo(palabra(d, 0x2c2d)) > 40 && palabra(d, 0x2cbf) == 0) {
      configurar_secuencia(d, 0x2c5c, 0xb3e7, 7, 2);
      configurar_secuencia(d, 0x2c7d, 0xb3ec, 1, 2);
      configurar_secuencia(d, 0x2c82, 0xb3fb, 1, 2);
      configurar_secuencia(d, 0x2c87, 0xb40a, 1, 2);
      configurar_secuencia(d, 0x2cbf, 0xb429, 9, 1);
      configurar_secuencia(d, 0x2cc4, 0xb445, 9, 1);
      configurar_secuencia(d, 0x2cc9, 0xb460, 9, 1);
      configurar_secuencia(d, 0x2c9e, 0xb419, 11, 2);
      configurar_secuencia(d, 0x2ca3, 0xb41f, 11, 2);
      configurar_secuencia(d, 0x2ca8, 0xb424, 11, 2);
    }
    if (con_signo(palabra(d, 0x2c2d)) <= 40 || d[0x2caf] != 1)
      return true;
    if (d[0x358c] == 9 && (d[0x2c2c] == 0x82 || d[0x2c2c] == 0x83))
      return true;
    const auto diferencia =
        static_cast<Palabra>(palabra(d, 0x2c2d) + palabra(d, 0x2c31) - palabra(d, 0x2cb1));
    if (con_signo(diferencia) <= 0 ||
        con_signo(static_cast<Palabra>(diferencia - palabra(d, 0x2c31) - palabra(d, 0x2cb5))) >=
            0 ||
        d[0x2c2c] == 47)
      return true;
    d[0x3594] = d[0x3595] = d[0x3596] = 0;
    guardar(d, 0x359a, 0);
    guardar(d, 0x0dfa, 0);
    guardar(d, 0x2c2f, 130);
    configurar_secuencia(d, 0x2c3b, 0xb3da, 1, 2);
    guardar(d, 0x2c40, 0);
    guardar(d, 0x2c45, 0);
    return true;
  }
  // CFD2–D113: recorre los cuatro reflectores y destruye el emisor al cerrar el circuito.
  if (palabra(d, 0x8082) == 56) {
    // 10EC/1119–1129/1791: esta sala no usa manejadores de entidad >=0400.
    // AX conserva el tipo del último actor activo; BX recibe CFD2 del despachador.
    // F29A ordena también actores inactivos: conservar estas escrituras de D10C.
    if (!d[0x2d75]) {
      Palabra ax = 0x0f31;
      for (Palabra i = 0; i < 68; ++i) {
        const auto actor = static_cast<Palabra>(0x2c4c + i * 33);
        if (d[actor] != 1)
          continue;
        ax = palabra(d, actor + 14);
        if (con_signo(ax) >= 0x400)
          return false;
      }
      guardar(d, 0x2d77, ax);
      guardar(d, 0x2d79, 0xcfd2);
      return true;
    }
    if (d[0x2e1a] == 1) {
      d[0x2d75] = 0;
      constexpr Palabra canales[] = {0x2da6, 0x2dc7, 0x2de8, 0x2e09, 0x2e2a};
      for (const Palabra canal : canales) {
        guardar(d, canal, 0x1a2e);
        d[canal + 2] = 0;
        d[canal + 3] = 18;
        d[canal + 4] = 1;
      }
      constexpr Palabra banderas[] = {0x5379, 0x537f, 0x5381, 0x536f};
      for (const Palabra bandera : banderas)
        d[bandera] &= 0xfd;
      Palabra ax = 0;
      for (Palabra i = 0; i < 6; ++i) {
        guardar(d, 0xb399, palabra(d, static_cast<Palabra>(0xb3c2 + i * 4)));
        guardar(d, 0xb39b, palabra(d, static_cast<Palabra>(0xb3c4 + i * 4)));
        constexpr Palabra campos[] = {0xb3a7, 0xb3ab, 0xb3af};
        for (const Palabra campo : campos)
          d[campo] = static_cast<unsigned char>(12 + i * 3);
        const auto creada = crear_entidad(d, azar, recursos, 0xb396);
        if (!creada.valido)
          return false;
        // B396 tiene flags01: 8BEA devuelve el banco; sin hueco, 8B80 conserva la base reservada.
        ax = creada.creada ? recursos.segmento_original
                           : static_cast<Palabra>(0x2c0a + d[0x3588] * 33);
      }
      guardar(d, 0x2d77, ax);
      guardar(d, 0x2d79, 0xb396);
      return true;
    }
    d[0x2db7] = d[0x2dd8] = d[0x2df9] = 3;
    Palabra x = 257, y = 163;
    if (d[0x2cd0] == 1 && d[0x2cd1] == 30) {
      d[0x2db7] = 1;
      x = static_cast<Palabra>(x - 183);
      y = static_cast<Palabra>(y - 10);
      if (d[0x2cf1] == 1 && d[0x2cf2] == 27) {
        d[0x2dd8] = 1;
        x = static_cast<Palabra>(x - 40);
        y = static_cast<Palabra>(y + 35);
        if (d[0x2d12] == 1 && d[0x2d13] == 34) {
          d[0x2df9] = 1;
          x = static_cast<Palabra>(x + 239);
          if (d[0x2d33] == 1 && d[0x2d34] == 14)
            d[0x2e1a] = 1;
        }
      }
    }
    guardar(d, 0x2d77, x);
    guardar(d, 0x2d79, y);
    return true;
  }
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
