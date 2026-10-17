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
                                        unsigned short id, const ServicioSonido &sonido) {
  if (palabra(d, 0x8082) != 3)
    return ResultadoMenu::pendiente;
  if (manejador == 0x32de && id <= 1) {
    // 3558–3587: el tema uno activa la reacción de la tienda únicamente a menos de 64 píxeles.
    auto distancia = static_cast<Palabra>(palabra(d, 0x2c2d) - palabra(d, 0x2cf3));
    if (con_signo(distancia) < 0)
      distancia = static_cast<Palabra>(0 - distancia);
    if (id == 1 && con_signo(distancia) < 64) {
      guardar(d, 0x2dc7, 0x1a2f);
      d[0x2dc9] = 0;
      d[0x2dca] = 1;
      d[0x2dcb] = 40;
    }
    // 3545/37FF: los temas cero y uno mantienen sus efectos mediante el servicio común.
    return emitir_sonido(sonido, d, 0x2c2b, static_cast<unsigned char>(id)) ? ResultadoMenu::accion
                                                                            : ResultadoMenu::error;
  }
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
  if (manejador == 0x9b86) {
    // 9B86–9BB7: la concha activa el transporte de la tortuga y repone la reserva indicada por el
    // guion.
    retirar_identificador(d, 18);
    guardar(d, 0x358d, 160);
    guardar(d, 0x2da4, 0x040b);
    guardar(d, 0x1a38, static_cast<Palabra>(palabra(d, 0x2e1c) + 38));
    guardar(d, 0x1a3a, static_cast<Palabra>(palabra(d, 0x2e1e) + 40));
    guardar(d, 0x0ad7, 0x99c8);
    guardar(d, 0x35c7, static_cast<Palabra>(palabra(d, 0x35c7) + 5));
    return comenzar(d);
  }
  if (manejador == 0x2fca && id == 23) {
    // 2FD3–300A: la picana exige un tiburón activo y mantiene habilitada la consulta del
    // inventario.
    if (d[0x2df9] != 1) {
      marcar_accion_rechazada(d);
      return ResultadoMenu::accion;
    }
    guardar(d, 0x359a, 0);
    guardar(d, 0x0dfa, 0);
    guardar(d, 0x2c3b, 0x1ab2);
    guardar(d, 0x2c40, 0x1ac3);
    guardar(d, 0x2c45, 0);
    const auto resultado = comenzar(d);
    d[0x3596] = 1;
    return resultado;
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
  if ((manejador == 0x2fca && id <= 26) || (manejador == 0x3143 && id <= 26 && id != 24)) {
    // 313E/32D9: rechazos originales; el ataque de la almeja se mantiene pendiente de traslado.
    marcar_accion_rechazada(d);
    return ResultadoMenu::accion;
  }
  if (manejador == 0x384a && id <= 26 && id != 8) {
    // 3B4F: examinar sin una rama especial conserva el menú y retorna sin acarreo.
    return ResultadoMenu::actualizado;
  }
  return ResultadoMenu::pendiente;
}
} // namespace motor
