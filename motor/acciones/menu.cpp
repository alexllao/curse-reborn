#include "corredor.hpp"
#include "asaltante.hpp"
#include "poblacion.hpp"
#include "borde.hpp"
#include "lago.hpp"
#include "poza.hpp"
#include "cuevas.hpp"
#include "catapulta.hpp"
#include "agujeros.hpp"
#include "mascara.hpp"
#include "pozo.hpp"
#include "menu.hpp"
#include "construir_tabla.hpp"
#include "objetos_cercanos.hpp"
#include "iniciar_accion.hpp"
#include "lista_identificadores.hpp"
#include "../estado/segmento.hpp"
#include "../movimiento/avanzar.hpp"
#include "../movimiento/direccion.hpp"
#include "../movimiento/planificar.hpp"

namespace motor {
namespace {
// Copia una lista de diez palabras, adapta sus identificadores y rellena desde el primer FFFF.
void copiar_lista(unsigned char *d, Palabra origen, Palabra ajuste) {
  bool fin = false;
  for (Palabra i = 0; i < 10; ++i) {
    const auto valor = palabra(d, static_cast<Palabra>(origen + i * 2));
    fin = fin || valor == 0xffff;
    guardar(d, static_cast<Palabra>(0xb620 + i * 2),
            fin ? 0xffff : static_cast<Palabra>(valor + ajuste));
  }
}
// E07F: prepara destinos compatibles; la primera casilla permite volver al inventario.
bool preparar_destinos(unsigned char *d) {
  const auto accion = palabra(d, 0xb634);
  const bool inventario = accion != 1 && accion != 6;
  if (inventario)
    guardar(d, 0xb620, 0);
  if (!buscar_objetos_cercanos(d, 1, inventario ? 0xb622 : 0xb620, inventario ? 9 : 10).valido)
    return false;
  guardar(d, 0xb636, 0x3b53);
  construir_tabla_acciones(d);
  if (inventario)
    guardar(d, 0xb60c, 0xe0c0);
  return true;
}
// 886B: inicia el contrato común y traduce el acarreo de salida a una decisión portable.
ResultadoMenu comenzar(unsigned char *d) {
  unsigned short ax = 0;
  return iniciar_accion(d, ax) ? ResultadoMenu::accion : ResultadoMenu::error;
}
} // namespace

// DE96: B638 pierde el objeto previo; DEFB construye la tabla y DEF1/DEF4 bloquean dos entradas.
void preparar_menu_principal(unsigned char *d) {
  guardar(d, 0xb638, 0xffff);
  for (Palabra i = 0; i < 10; ++i)
    guardar(d, static_cast<Palabra>(0xb620 + i * 2), i);
  construir_tabla_acciones(d);
  if (palabra(d, 0x2c3b) != 0) {
    guardar(d, 0xb60e, 0xde96);
    guardar(d, 0xb610, 0xde96);
  }
}

// DB02–DB4B: los punteros 1A36/3C/42 se limpian al abrir, no al cambiar de submenú.
void abrir_menu(unsigned char *d, unsigned short x, unsigned short y) {
  guardar(d, 0x1a36, 0);
  guardar(d, 0x1a3c, 0);
  guardar(d, 0x1a42, 0);
  preparar_menu_principal(d);
  guardar(d, 0xb63a, 1);
  guardar(d, 0xb63c, 0);
  d[0xb607] = 0;
  guardar(d, 0xb60a, x);
  guardar(d, 0xb608, y < 100 ? 0 : 0x4980);
}

// El ámbito explícito impide ejecutar una aproximación del mismo verbo en otras escenas.
ResultadoMenu ejecutar_interaccion_inicial(unsigned char *d, unsigned short manejador,
                                           unsigned short id, const ServicioSonido &sonido,
                                           const ServicioCreacion &creacion) {
  if (manejador == 0xa1d4 || manejador == 0xa1fa || manejador == 0xa220 || manejador == 0xa245 ||
      manejador == 0xa26a || manejador == 0xa28f || manejador == 0xa2d8)
    return ejecutar_interaccion_mascara(d, manejador);
  if (manejador == 0xa383)
    return ejecutar_interaccion_catapulta(d, manejador, id, sonido);
  if (manejador == 0x9994 && palabra(d, 0x8082) != 3) {
    marcar_accion_rechazada(d);
    return ResultadoMenu::accion;
  }
  if (palabra(d, 0x8082) == 3)
    return ejecutar_interaccion_lago(d, manejador, id, sonido);
  if (palabra(d, 0x8082) == 8)
    return ejecutar_interaccion_poza(d, manejador, id, sonido);
  if (palabra(d, 0x8082) == 9)
    return ejecutar_interaccion_pozo(d, manejador, id, sonido, creacion);
  if (palabra(d, 0x8082) == 14)
    return ejecutar_interaccion_agujeros(d, manejador, id, sonido);
  if (manejador == 0xa383 || palabra(d, 0x8082) == 11 || palabra(d, 0x8082) == 12 ||
      palabra(d, 0x8082) == 13)
    return ejecutar_interaccion_catapulta(d, manejador, id, sonido);
  if ((palabra(d, 0x8082) >= 4 && palabra(d, 0x8082) <= 7) || palabra(d, 0x8082) == 10)
    return ejecutar_interaccion_cuevas(d, manejador, id, sonido);
  if ((palabra(d, 0x8082) >= 32 && palabra(d, 0x8082) <= 35) || palabra(d, 0x8082) == 38)
    return ejecutar_interaccion_poste(d, manejador, id, sonido);
  if (palabra(d, 0x8082) == 30)
    return ejecutar_interaccion_borde(d, manejador, id, sonido, creacion);
  if ((palabra(d, 0x8082) >= 17 && palabra(d, 0x8082) <= 29) || palabra(d, 0x8082) == 31)
    return ejecutar_interaccion_poblacion(d, manejador, id, sonido);
  if (palabra(d, 0x8082) == 16)
    return ejecutar_interaccion_asaltante(d, manejador, id, sonido, creacion);
  if (palabra(d, 0x8082) == 15) {
    // 3545, 37FF, 3B4F y 88A0: respuestas comunes de la explanada tras caer la máscara.
    if (manejador == 0x32de && id <= 1)
      return emitir_sonido(sonido, d, 0x2c2b, static_cast<unsigned char>(id))
                 ? ResultadoMenu::accion
                 : ResultadoMenu::error;
    if (id == 3 || id == 70 || id == 74) {
      if (manejador == 0x384a)
        return ResultadoMenu::actualizado;
      if (manejador == 0x2fca || manejador == 0x3143) {
        marcar_accion_rechazada(d);
        return ResultadoMenu::accion;
      }
    }
    return ResultadoMenu::pendiente;
  }
  if (palabra(d, 0x8082) == 1)
    return ejecutar_interaccion_corredor(d, manejador, id, sonido);
  if (palabra(d, 0x8082) != 0)
    return ResultadoMenu::pendiente;
  if (manejador == 0x32de && id <= 1) {
    // 3545 y 37FF: las respuestas sin animación emiten los identificadores cero y uno.
    if (id == 1 || d[0x3607] == 0)
      return emitir_sonido(sonido, d, 0x2c2b, static_cast<unsigned char>(id))
                 ? ResultadoMenu::accion
                 : ResultadoMenu::pendiente;
    guardar(d, 0x2c3b, 0x241a);
    guardar(d, 0x2d22, 0x2495);
    d[0x2d24] = 0;
    d[0x2d25] = 1;
    guardar(d, 0x2d27, 0x24dc);
    d[0x2d29] = 0;
    d[0x2d2a] = 0x2a;
    guardar(d, 0x2d2c, 0x24f7);
    d[0x2d2e] = 0;
    d[0x2d2f] = 0x2a;
    guardar(d, 0x2cbf, 0x2512);
    d[0x2cc2] = 0x28;
    guardar(d, 0x2cc4, 0x2521);
    d[0x2cc7] = 0x28;
    guardar(d, 0x2d85, 0x252c);
    d[0x2d88] = 0x54;
    guardar(d, 0x2d8a, 0x2537);
    d[0x2d8d] = 0x56;
    guardar(d, 0x2d8f, 0x254b);
    d[0x2d92] = 0x56;
    d[0x3607] = d[0x3622] = 0;
    return comenzar(d);
  }
  if (manejador == 0x563a && (id == 1 || id == 3 || id == 5)) {
    const auto icono = d[0x524e + id * 2];
    if (icono == 0x1f || icono == 0x22)
      guardar(d, 0x35c7, static_cast<Palabra>(palabra(d, 0x35c7) + (icono == 0x1f ? 1 : 2)));
    else if (!insertar_identificador(d, id)) {
      marcar_accion_rechazada(d);
      return ResultadoMenu::accion;
    }
    if (id == 1) {
      d[0x3622] = 0;
      guardar(d, 0x2d85, 0x1a2e);
      d[0x2d87] = 0;
      d[0x2d88] = 3;
      d[0x2d89] = 1;
      guardar(d, 0x2c3b, 0x9723);
    } else if (id == 3) {
      d[0x3610] = d[0x2d33] = 0;
      guardar(d, 0x1a38, 0xc8);
      guardar(d, 0x1a3a, 0x9d);
      guardar(d, 0x0ad7, 0x972c);
      guardar(d, 0x0adc, 0x9738);
    }
    if (id == 5) {
      // 56CB y 615C–61A6: conserva el objeto hasta que su canal de recogida lo desactive.
      guardar(d, 0x1a38, static_cast<Palabra>(palabra(d, 0x2cd2) + 5));
      guardar(d, 0x1a3a, static_cast<Palabra>(palabra(d, 0x2cd4) + palabra(d, 0x2cd8) - 2));
      // 623D: anula la entrada persistente de la lista de entidades, no solo su dibujo actual.
      const auto lista = palabra(d, 0x53b5);
      d[static_cast<Palabra>(lista + ((0x2cd0 - 0x2c0a) / 33 - 2) * 9 + 1)] = 0;
      guardar(d, 0x2ce0, 0x1a2e);
      d[0x2ce2] = 0;
      d[0x2ce4] = 1;
      guardar(d, 0x0ad7, d[0x358c] == 0x14 ? 0x96f6 : 0x96c0);
      guardar(d, 0x0adc, d[0x358c] == 0x14 ? 0x96ff : 0x96c9);
      unsigned short pasos = 0;
      if (!iniciar_accion(d, pasos))
        return ResultadoMenu::error;
      d[0x2ce3] = static_cast<unsigned char>(pasos + 6);
      return ResultadoMenu::accion;
    }
    return comenzar(d);
  }
  if (manejador == 0x9900) {
    // 9900–9951: empujar la pared exige el fotograma tres de la entidad.
    if (d[0x2c8f] != 3) {
      marcar_accion_rechazada(d);
      return ResultadoMenu::accion;
    }
    guardar(d, 0x2c9e, 0x9a59);
    d[0x2ca1] = 10;
    guardar(d, 0x2ca8, 0x9a64);
    d[0x2cab] = 10;
    d[0x35e4] = 10;
    guardar(d, 0x35e7, static_cast<Palabra>(palabra(d, 0x35e7) + 27));
    d[0x3611] = 0x2a;
    d[0x2d33] = 1;
    guardar(d, 0x1a38, 210);
    guardar(d, 0x1a3a, 157);
    guardar(d, 0x0ad7, 0x9a14);
    guardar(d, 0x0adc, 0x9a26);
    guardar(d, 0x35c7, static_cast<Palabra>(palabra(d, 0x35c7) + 5));
    return comenzar(d);
  }
  if (manejador == 0x9954) {
    // 9954–9993: programa la apertura de la puerta al terminar el desplazamiento de Brad.
    d[0x35ed] = 0x10;
    guardar(d, 0x35ee, static_cast<Palabra>(palabra(d, 0x35ee) + 17));
    guardar(d, 0x1a38, 55);
    guardar(d, 0x1a3a, 156);
    guardar(d, 0x0ad7, 0x9a2c);
    guardar(d, 0x0adc, 0x9a46);
    guardar(d, 0x009c, 0x2caf);
    guardar(d, 0x009e, 0x0215);
    guardar(d, 0x00a0, 0x9a6c);
    guardar(d, 0x00a2, 0x9a73);
    guardar(d, 0x35c7, static_cast<Palabra>(palabra(d, 0x35c7) + 5));
    return comenzar(d);
  }
  if ((manejador == 0x2fca || manejador == 0x3143 || manejador == 0x384a) && id <= 5) {
    // 3B4F no altera el estado y devuelve sin acarreo; 313E y 32D9 marcan el rechazo.
    if (manejador == 0x384a)
      return ResultadoMenu::actualizado;
    marcar_accion_rechazada(d);
    return ResultadoMenu::accion;
  }
  if (manejador == 0x989e) {
    if (d[0x358c] != 0) {
      marcar_accion_rechazada(d);
      return ResultadoMenu::accion;
    }
    d[0x3622] = 0;
    guardar(d, 0x2d85, 0x1a2e);
    d[0x2d87] = 0;
    d[0x2d88] = 3;
    d[0x2d89] = 1;
    retirar_identificador(d, 1);
    insertar_identificador(d, 1);
    guardar(d, 0x2c3b, 0x99cb);
    guardar(d, 0x2c40, 0x99e8);
    guardar(d, 0x2c45, 0x99fe);
    guardar(d, 0x35c7, static_cast<Palabra>(palabra(d, 0x35c7) + 5));
    const bool invertido = d[0x2c2c] != 0;
    const auto resultado = comenzar(d);
    if (invertido)
      d[0x2c3d] = d[0x2c42] = d[0x2c47] = 3;
    return resultado;
  }
  return ResultadoMenu::pendiente;
}

// Conserva los destinos originales como etiquetas; nunca salta a direcciones de código DOS.
ResultadoMenu seleccionar_menu(unsigned char *d, unsigned short casilla,
                               const ServicioSonido &sonido, const ServicioCreacion &creacion) {
  if (casilla >= 10)
    return ResultadoMenu::error;
  const auto manejador = palabra(d, static_cast<Palabra>(0xb60c + casilla * 2));
  const auto id = palabra(d, static_cast<Palabra>(0xb620 + casilla * 2));
  if (manejador == 0 || manejador == 0xe15e)
    return ResultadoMenu::actualizado;
  if (manejador == 0xde96) {
    preparar_menu_principal(d);
    return ResultadoMenu::actualizado;
  }
  if (manejador == 0xdfd3 || manejador == 0xe0c0 || manejador == 0xe0fd || manejador == 0xe010) {
    copiar_lista(d, manejador == 0xe010 ? 0x5239 : 0x5225, manejador == 0xe010 ? 0x17 : 0x1a);
    guardar(d, 0xb636,
            manejador == 0xdfd3   ? 0xde96
            : manejador == 0xe0fd ? 0x2fca
            : manejador == 0xe010 ? 0x32de
                                  : 0x3b53);
  } else if (manejador == 0xdffc || manejador == 0xe0e9 || manejador == 0xe14a) {
    if (!buscar_objetos_cercanos(d,
                                 manejador == 0xdffc   ? 2
                                 : manejador == 0xe14a ? 5
                                                       : 1,
                                 0xb620, 10)
             .valido)
      return ResultadoMenu::error;
    guardar(d, 0xb636, manejador == 0xdffc ? 0x563a : manejador == 0xe14a ? 0x3143 : 0x384a);
  } else if (manejador == 0xe039 || manejador == 0xe126) {
    const Palabra cantidad = manejador == 0xe039 ? 8 : 3;
    for (Palabra i = 0; i < 10; ++i)
      guardar(d, static_cast<Palabra>(0xb620 + i * 2),
              i < cantidad ? static_cast<Palabra>((manejador == 0xe039 ? 15 : 10) + i) : 0xffff);
  } else if (manejador == 0xe05b) {
    guardar(d, 0xb634, casilla);
    if (casilla == 1 || (casilla >= 3 && casilla <= 7)) {
      copiar_lista(d, 0x5225, 0x1a);
      guardar(d, 0xb636, 0x3b53);
    } else
      return preparar_destinos(d) ? ResultadoMenu::actualizado : ResultadoMenu::error;
  } else if (manejador == 0x3b53) {
    const auto objeto = static_cast<Palabra>(id - 0x1a);
    const auto previo = palabra(d, 0xb638);
    if (objeto == previo)
      return preparar_destinos(d) ? ResultadoMenu::actualizado : ResultadoMenu::error;
    if (previo == 0xffff) {
      const auto accion = palabra(d, 0xb634);
      if (accion == 0 || accion == 1 || accion == 6 || accion == 7) {
        guardar(d, 0xb638, objeto);
        return preparar_destinos(d) ? ResultadoMenu::actualizado : ResultadoMenu::error;
      }
      if (accion == 3 && objeto == 84)
        return ejecutar_interaccion_inicial(d, 0x419e, objeto, sonido, creacion);
      if (accion == 4 && objeto == 6)
        return ejecutar_interaccion_inicial(d, 0x9994, objeto, sonido, creacion);
      if (accion == 2 && objeto == 15)
        return ejecutar_interaccion_inicial(d, 0x9a48, objeto, sonido, creacion);
      if (accion == 2 && objeto == 48)
        return ejecutar_interaccion_inicial(d, 0x9de4, objeto, sonido, creacion);
      if (accion == 2 && objeto == 2)
        return ejecutar_interaccion_inicial(d, 0x9900, objeto, sonido, creacion);
      if (accion == 5 && objeto == 47 && palabra(d, 0x8082) == 9)
        return ejecutar_interaccion_inicial(d, 0x9e21, objeto, sonido, creacion);
      if (accion == 5 && objeto == 61 && palabra(d, 0x8082) == 11)
        return ejecutar_interaccion_inicial(d, 0xa0db, objeto, sonido, creacion);
      if (accion == 5 && objeto == 70 && palabra(d, 0x8082) == 13)
        return ejecutar_interaccion_inicial(d, 0xa3b8, objeto, sonido, creacion);
      if (accion == 4 && objeto == 53 && palabra(d, 0x8082) >= 4 && palabra(d, 0x8082) <= 7)
        return ejecutar_interaccion_inicial(d, 0x9cbe, objeto, sonido, creacion);
      if (palabra(d, 0x8082) == 30) {
        if (accion == 2 && objeto == 92)
          return ejecutar_interaccion_inicial(d, 0xa406, objeto, sonido, creacion);
        if (accion == 2 && objeto >= 88 && objeto <= 91)
          return ejecutar_interaccion_inicial(d, static_cast<Palabra>(0xa4a3 + (objeto - 88) * 10),
                                              objeto, sonido, creacion);
        if (accion == 4 && (objeto == 86 || objeto == 87))
          return ejecutar_interaccion_inicial(d, objeto == 86 ? 0xa46a : 0xa52b, objeto, sonido,
                                              creacion);
      }
      if (accion == 4 && objeto == 74)
        return ejecutar_interaccion_inicial(d, 0xa2d8, objeto, sonido, creacion);
    } else if (palabra(d, 0xb634) == 6 && objeto == 20 && previo == 17) {
      return ejecutar_interaccion_inicial(d, 0x9aa8, objeto, sonido, creacion);
    } else if (palabra(d, 0xb634) == 6 && objeto == 22 && previo == 18) {
      return ejecutar_interaccion_inicial(d, 0x9b86, objeto, sonido, creacion);
    } else if (palabra(d, 0xb634) == 1 &&
               ((objeto == 25 && previo == 23) || (objeto == 23 && previo == 25))) {
      // 3D72–3D89: insertar picana y tapón admite ambos órdenes de selección.
      return ejecutar_interaccion_inicial(d, 0x9bb8, objeto, sonido, creacion);
    } else if (palabra(d, 0xb634) == 1 && palabra(d, 0x8082) == 13 &&
               ((objeto == 62 && previo == 70) || (objeto == 70 && previo == 62))) {
      // 3D8C–3DA5: insertar el imán con cordel en el agujero equivale a lanzarlo, en ambos órdenes.
      return ejecutar_interaccion_inicial(d, 0xa3b8, objeto, sonido, creacion);
    } else if (palabra(d, 0xb634) == 6 && objeto == 54 && palabra(d, 0x8082) == 10) {
      // 3E89–3F25: el destino es el picapedrero y el objeto previo determina el tamaño.
      return ejecutar_interaccion_inicial(d, 0x9f39, previo, sonido, creacion);
    } else if (palabra(d, 0xb634) == 7) {
      if (objeto == 36 && previo == 63 && palabra(d, 0x8082) == 5)
        return ejecutar_interaccion_inicial(d, 0x9c73, objeto, sonido, creacion);
      // 3FF9–407F: cada receta admite los dos órdenes originales.
      const auto pareja = [&](Palabra a, Palabra b) {
        return (objeto == a && previo == b) || (objeto == b && previo == a);
      };
      if (pareja(37, 64))
        return ejecutar_interaccion_inicial(d, 0xa1d4, objeto, sonido, creacion);
      if (pareja(37, 49))
        return ejecutar_interaccion_inicial(d, 0xa1fa, objeto, sonido, creacion);
      if (pareja(64, 49))
        return ejecutar_interaccion_inicial(d, 0xa220, objeto, sonido, creacion);
      if (pareja(49, 71))
        return ejecutar_interaccion_inicial(d, 0xa245, objeto, sonido, creacion);
      if (pareja(64, 72))
        return ejecutar_interaccion_inicial(d, 0xa26a, objeto, sonido, creacion);
      if (pareja(37, 73))
        return ejecutar_interaccion_inicial(d, 0xa28f, objeto, sonido, creacion);
      if (objeto == 58 && previo == 56 && palabra(d, 0x8082) == 11)
        return ejecutar_interaccion_inicial(d, 0xa07e, objeto, sonido, creacion);
      if ((objeto == 57 && previo == 55) || (objeto == 55 && previo == 57))
        return ejecutar_interaccion_inicial(d, 0xa383, objeto, sonido, creacion);
    } else if (palabra(d, 0xb634) == 0) {
      if ((objeto == 0 && previo == 1) || (objeto == 1 && previo == 0))
        return ejecutar_interaccion_inicial(d, 0x989e, objeto, sonido, creacion);
      if ((objeto == 3 && previo == 4) || (objeto == 4 && previo == 3))
        return ejecutar_interaccion_inicial(d, 0x9954, objeto, sonido, creacion);
    }
    const auto escena = palabra(d, 0x8082);
    if ((escena >= 32 && escena <= 35) || escena == 38)
      return ejecutar_interaccion_poste(d, 0x3b53, objeto, sonido);
    if (escena == 30)
      return ejecutar_interaccion_borde(d, 0x3b53, objeto, sonido, creacion);
    if ((escena >= 17 && escena <= 29) || escena == 31) {
      // 3CDF–3CE4: llevar el vestido comprueba después la habitación en 430F.
      if (palabra(d, 0xb634) == 4 && objeto == 79)
        return ejecutar_interaccion_inicial(d, 0x430f, objeto, sonido, creacion);
      if (palabra(d, 0xb634) == 6 && previo == 76 &&
          (objeto == 83 || objeto == 81 || objeto == 77 || objeto == 78))
        return ejecutar_interaccion_inicial(d,
                                            objeto == 83   ? 0x41da
                                            : objeto == 78 ? 0x44ab
                                            : objeto == 77 ? 0x43b4
                                                           : 0x420f,
                                            objeto, sonido, creacion);
      return ejecutar_interaccion_poblacion(d, 0x3b53, objeto, sonido);
    }
    if ((palabra(d, 0x8082) == 0 && objeto <= 5 && (previo == 0xffff || previo <= 5)) ||
        (palabra(d, 0x8082) == 1 && objeto <= 14 && (previo == 0xffff || previo <= 14)) ||
        (palabra(d, 0x8082) == 3 && objeto <= 26 && (previo == 0xffff || previo <= 26)) ||
        (palabra(d, 0x8082) == 8 && (objeto == 3 || (objeto >= 48 && objeto <= 50)) &&
         (previo == 0xffff || previo == 3 || (previo >= 48 && previo <= 50))) ||
        ((palabra(d, 0x8082) >= 4 && palabra(d, 0x8082) <= 14) &&
         (objeto == 3 || (objeto >= 27 && objeto <= 74)) &&
         (previo == 0xffff || previo == 3 || (previo >= 27 && previo <= 74)))) {
      marcar_accion_rechazada(d);
      return ResultadoMenu::accion;
    }
    if (palabra(d, 0x8082) == 15 && (objeto == 3 || objeto == 70 || objeto == 74) &&
        (previo == 0xffff || previo == 3 || previo == 70 || previo == 74)) {
      marcar_accion_rechazada(d);
      return ResultadoMenu::accion;
    }
    if (palabra(d, 0x8082) == 16 && (objeto == 3 || objeto == 70 || objeto == 75 || objeto == 76) &&
        (previo == 0xffff || previo == 3 || previo == 70 || previo == 75 || previo == 76)) {
      marcar_accion_rechazada(d);
      return ResultadoMenu::accion;
    }
    return ResultadoMenu::pendiente;
  } else if (manejador == 0x32de || manejador == 0x563a || manejador == 0x2fca ||
             manejador == 0x3143 || manejador == 0x384a)
    return ejecutar_interaccion_inicial(
        d, manejador, static_cast<Palabra>(id - (manejador == 0x32de ? 0x17 : 0x1a)), sonido,
        creacion);
  else
    return ResultadoMenu::pendiente;
  construir_tabla_acciones(d);
  return ResultadoMenu::actualizado;
}

// 096D–0A58: no hace colisiones ni decide nuevos destinos; consume los nodos que preparó una
// acción.
bool avanzar_accion_pendiente(unsigned char *d) {
  if (palabra(d, 0x359a) != 0 && avanzar_movimiento(d))
    return true;
  auto nodo = palabra(d, 0x0dfa);
  if (nodo == 0)
    return true;
  const auto siguiente = palabra(d, nodo);
  if (siguiente != 0) {
    guardar(d, 0x0dfa, siguiente);
    unsigned short pasos = 0;
    return planificar_movimiento(d, siguiente, pasos);
  }
  guardar(d, 0x0dfa, 0);
  if (palabra(d, 0x0ad7) == 0)
    return seleccionar_direccion(d, 0);
  guardar(d, 0x2c2d,
          static_cast<Palabra>(palabra(d, static_cast<Palabra>(nodo + 2)) -
                               ((palabra(d, 0x2c31) >> 1) - 1)));
  guardar(
      d, 0x2c2f,
      static_cast<Palabra>(palabra(d, static_cast<Palabra>(nodo + 4)) - palabra(d, 0x2c33) + 1));
  for (Palabra i = 0; i < 15; ++i)
    d[0x2c3b + i] = d[0x0ad7 + i];
  guardar(d, 0x0ad7, 0);
  guardar(d, 0x0adc, 0);
  guardar(d, 0x0ae1, 0);
  for (Palabra i = 0; i < 70; ++i) {
    const auto registro = static_cast<Palabra>(0x009c + i * 10);
    const auto entidad = palabra(d, registro);
    guardar(d, registro, 0);
    if (entidad == 0)
      break;
    const auto intervalo = palabra(d, static_cast<Palabra>(registro + 2));
    for (Palabra c = 0; c < 3; ++c) {
      const auto campo = static_cast<Palabra>(registro + 4 + c * 2);
      const auto referencia = palabra(d, campo);
      guardar(d, campo, 0);
      if (referencia != 0) {
        const auto destino = static_cast<Palabra>(entidad + 0x10 + c * 5);
        guardar(d, destino, referencia);
        d[static_cast<Palabra>(destino + 2)] = 0;
        guardar(d, static_cast<Palabra>(destino + 3), intervalo);
      }
    }
  }
  return true;
}
} // namespace motor
