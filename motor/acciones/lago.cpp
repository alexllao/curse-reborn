#include "lago.hpp"
#include "iniciar_accion.hpp"
#include "lista_identificadores.hpp"
#include "../estado/segmento.hpp"

namespace motor {
namespace {
// 886B: entrega los canales y el destino al mismo planificador de acciones del ejecutable.
ResultadoMenu comenzar(unsigned char *d) {
  unsigned short pasos = 0;
  return iniciar_accion(d, pasos) ? ResultadoMenu::accion : ResultadoMenu::error;
}
// Escribe un canal de cinco bytes conservando secuencia, fase, demora e intervalo originales.
void canal(unsigned char *d, Palabra destino, Palabra secuencia, unsigned char demora,
           unsigned char intervalo) {
  guardar(d, destino, secuencia);
  d[destino + 2] = 0;
  d[destino + 3] = demora;
  d[destino + 4] = intervalo;
}
// 615C–61A6 y 623D: retira el descriptor persistente y programa la recogida al llegar al objeto.
ResultadoMenu recoger(unsigned char *d, Palabra entidad) {
  guardar(d, 0x1a38, static_cast<Palabra>(palabra(d, entidad + 2) + 5));
  guardar(d, 0x1a3a, static_cast<Palabra>(palabra(d, entidad + 4) + palabra(d, entidad + 8) - 2));
  const auto lista = palabra(d, 0x53bb);
  d[static_cast<Palabra>(lista + ((entidad - 0x2c0a) / 33 - 2) * 9 + 1)] = 0;
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
} // namespace

// Conserva las escrituras originales; las acciones submarinas posteriores siguen pendientes.
ResultadoMenu ejecutar_interaccion_lago(unsigned char *d, unsigned short manejador,
                                        unsigned short id) {
  if (palabra(d, 0x8082) != 3)
    return ResultadoMenu::pendiente;
  if (manejador == 0x9994) {
    // 999E–9A47: consumir pecera y agua, animar su colocación y sumar cinco puntos.
    retirar_identificador(d, 6);
    retirar_identificador(d, 7);
    guardar(d, 0x2c4e, palabra(d, 0x2c2d));
    guardar(d, 0x2c50, palabra(d, 0x2c2f));
    canal(d, 0x2c5c, 0x9a91, 7, 2);
    canal(d, 0x2c61, 0x9a95, 7, 2);
    canal(d, 0x2c66, 0x9a9b, 7, 2);
    canal(d, 0x2e8d, 0x9bc8, 7, 2);
    canal(d, 0x2e92, 0x9bcc, 7, 2);
    canal(d, 0x2e97, 0x9bce, 7, 2);
    guardar(d, 0x2c3b, 0x9a78);
    guardar(d, 0x2c40, 0x9a87);
    guardar(d, 0x35c7, static_cast<Palabra>(palabra(d, 0x35c7) + 5));
    return comenzar(d);
  }
  if (manejador == 0x9a48) {
    // 9A48–9AA7: empujar los barrotes solo si su entidad continúa activa.
    if (!d[0x2c6d]) {
      marcar_accion_rechazada(d);
      return ResultadoMenu::accion;
    }
    canal(d, 0x2c7d, 0x9b21, 15, d[0x2c81]);
    canal(d, 0x2c82, 0x9b26, 15, d[0x2c86]);
    canal(d, 0x2c87, 0x9b2b, 15, d[0x2c8b]);
    guardar(d, 0x1a38, static_cast<Palabra>(palabra(d, 0x2c6f) - 16));
    guardar(d, 0x1a3a, static_cast<Palabra>(palabra(d, 0x2c71) + 24));
    guardar(d, 0x0ad7, 0x9aa1);
    guardar(d, 0x0adc, 0x9ab2);
    guardar(d, 0x35c7, static_cast<Palabra>(palabra(d, 0x35c7) + 5));
    return comenzar(d);
  }
  if (manejador == 0x384a && id == 16) {
    // 38A4–38D2: examinar la arena y descubrir el gusano mediante los canales originales.
    guardar(d, 0x1a38, static_cast<Palabra>(palabra(d, 0x2c90) + 4));
    guardar(d, 0x1a3a, palabra(d, 0x2c92));
    guardar(d, 0x0ad7, 0x1a84);
    guardar(d, 0x0adc, 0x1a96);
    guardar(d, 0x009c, 0x2caf);
    guardar(d, 0x009e, 0x0207);
    guardar(d, 0x00a0, 0x1bfa);
    return comenzar(d);
  }
  if (manejador == 0x9aa8) {
    // 9AA8–9B85: entregar el gusano al pez programa su respuesta y la recuperación de oxígeno.
    retirar_identificador(d, 17);
    d[0x2d96] = 1;
    canal(d, 0x2da6, 0x9b35, 1, 2);
    canal(d, 0x2dab, 0x9b50, 1, 1);
    canal(d, 0x2d43, 0x1a2e, 58, 1);
    canal(d, 0x2d64, 0x1a2e, 50, 1);
    d[0x2db7] = 0;
    canal(d, 0x2d06, 0x9b85, 50, 1);
    canal(d, 0x2d0b, 0x9b8f, 50, 1);
    guardar(d, 0x2d27, 0x9b85);
    d[0x2d29] = 8;
    d[0x2d2a] = 64;
    d[0x2d2b] = 1;
    guardar(d, 0x2d2c, 0x9b8f);
    d[0x2d2e] = 8;
    d[0x2d2f] = 64;
    d[0x2d30] = 1;
    guardar(d, 0x1a38, static_cast<Palabra>(palabra(d, 0x2cf3) + 12));
    guardar(d, 0x1a3a, static_cast<Palabra>(palabra(d, 0x2cf5) + 52));
    guardar(d, 0x0ad7, 0x9abc);
    guardar(d, 0x0adc, 0x9ad6);
    guardar(d, 0x35c7, static_cast<Palabra>(palabra(d, 0x35c7) + 5));
    return comenzar(d);
  }
  if (manejador == 0x563a && (id == 17 || id == 18 || id == 23 || id == 26)) {
    const auto icono = d[0x524e + id * 2];
    if (icono == 0x1f || icono == 0x22)
      guardar(d, 0x35c7, static_cast<Palabra>(palabra(d, 0x35c7) + (icono == 0x1f ? 1 : 2)));
    else if (!insertar_identificador(d, id)) {
      marcar_accion_rechazada(d);
      return ResultadoMenu::accion;
    }
    // 5807–581D: la concha que aún se mueve no puede conservarse en el inventario.
    if (id == 18 && (palabra(d, 0x2ce0) | palabra(d, 0x2ce5) | palabra(d, 0x2cea))) {
      retirar_identificador(d, 18);
      marcar_accion_rechazada(d);
      return ResultadoMenu::accion;
    }
    return recoger(d, id == 17 ? 0x2caf : id == 18 ? 0x2cd0 : id == 23 ? 0x2dd8 : 0x2d75);
  }
  return ResultadoMenu::pendiente;
}
} // namespace motor
