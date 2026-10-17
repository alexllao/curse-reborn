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
                                           unsigned short id) {
  if (palabra(d, 0x8082) != 0)
    return ResultadoMenu::pendiente;
  if (manejador == 0x32de && id <= 1) {
    if (d[0x0ad4] != 0)
      return ResultadoMenu::pendiente;
    if (id == 1 || d[0x3607] == 0)
      return ResultadoMenu::accion;
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
  if (manejador == 0x563a && (id == 1 || id == 3)) {
    if (!insertar_identificador(d, id)) {
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
    } else {
      d[0x3610] = d[0x2d33] = 0;
      guardar(d, 0x1a38, 0xc8);
      guardar(d, 0x1a3a, 0x9d);
      guardar(d, 0x0ad7, 0x972c);
      guardar(d, 0x0adc, 0x9738);
    }
    return comenzar(d);
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
ResultadoMenu seleccionar_menu(unsigned char *d, unsigned short casilla) {
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
    } else if (palabra(d, 0xb634) == 0 &&
               ((objeto == 0 && previo == 1) || (objeto == 1 && previo == 0)))
      return ejecutar_interaccion_inicial(d, 0x989e, objeto);
    return ResultadoMenu::pendiente;
  } else if (manejador == 0x32de || manejador == 0x563a)
    return ejecutar_interaccion_inicial(
        d, manejador, static_cast<Palabra>(id - (manejador == 0x32de ? 0x17 : 0x1a)));
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
