#include "borde.hpp"
#include "iniciar_accion.hpp"
#include "lista_identificadores.hpp"
#include "../estado/segmento.hpp"

namespace motor {
namespace {
// 886B conserva su resultado AL para las demoras relativas de las animaciones.
ResultadoMenu comenzar(unsigned char *d) {
  unsigned short pasos = 0;
  return iniciar_accion(d, pasos) ? ResultadoMenu::accion : ResultadoMenu::error;
}
// Programa los canales diferidos de 0A58, conservando los campos que no escribe el original.
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
// 623D: retira la oferta persistente asociada al registro que se recoge.
void retirar_oferta(unsigned char *d, Palabra entidad) {
  const auto indice = static_cast<Palabra>((entidad - 0x2c0a) / 33 - 2);
  const auto lista = palabra(d, static_cast<Palabra>(0x53b5 + palabra(d, 0x8082) * 2));
  d[static_cast<Palabra>(lista + indice * 9 + 1)] = 0;
}
} // namespace

// Limita la reconstrucción a EDGE; los manejadores posteriores no se simulan como rechazos.
ResultadoMenu ejecutar_interaccion_borde(unsigned char *d, unsigned short manejador,
                                         unsigned short id, const ServicioSonido &sonido,
                                         const ServicioCreacion &creacion) {
  if (palabra(d, 0x8082) != 30)
    return ResultadoMenu::pendiente;
  if (manejador == 0x563a && (id == 86 || id == 87 || id == 95)) {
    // 563A–5679 y 5CF6–5D2E: capacidad y puntos antes de cambiar actor y oferta.
    const auto icono = d[0x524e + id * 2];
    if (icono == 0x1f || icono == 0x22)
      guardar(d, 0x35c7, static_cast<Palabra>(palabra(d, 0x35c7) + (icono == 0x1f ? 1 : 2)));
    else if (!insertar_identificador(d, id)) {
      marcar_accion_rechazada(d);
      return ResultadoMenu::accion;
    }
    const Palabra entidad = id == 86 ? 0x2c6d : id == 87 ? 0x2c8e : 0x2e1a;
    if (id == 87)
      guardar(d, 0x2c9c, 0xffff);
    guardar(d, 0x1a38, static_cast<Palabra>(palabra(d, entidad + 2) + (id == 86 ? 4 : 5)));
    const int dy = id == 86 ? 1 : id == 87 ? 2 : -2;
    guardar(d, 0x1a3a,
            static_cast<Palabra>(palabra(d, entidad + 4) + palabra(d, entidad + 8) + dy));
    retirar_oferta(d, entidad);
    guardar(d, entidad + 16, 0x1a2e);
    d[entidad + 18] = 0;
    d[entidad + 20] = 1;
    const bool pequeno = d[0x358c] == 20;
    const Palabra a = id == 95 ? (pequeno ? 0x96f6 : 0x96c0) : (pequeno ? 0x9705 : 0x96cf);
    const Palabra b = id == 95 ? (pequeno ? 0x96ff : 0x96c9) : (pequeno ? 0x970e : 0x96d8);
    guardar(d, 0x0ad7, a);
    guardar(d, 0x0adc, b);
    unsigned short pasos = 0;
    if (!iniciar_accion(d, pasos))
      return ResultadoMenu::error;
    d[entidad + 19] = static_cast<unsigned char>(pasos + 6);
    return ResultadoMenu::accion;
  }
  if (manejador == 0xa406) {
    // A406–A467: empuja la roca solo cuando todavía descansa a altura76.
    if (palabra(d, 0x2cb3) != 76) {
      marcar_accion_rechazada(d);
      return ResultadoMenu::accion;
    }
    guardar(d, 0x1a38, static_cast<Palabra>(453 - palabra(d, 0x3591)));
    guardar(d, 0x1a3a, 113);
    guardar(d, 0x0ad7, 0xa035);
    guardar(d, 0x0adc, 0xa04d);
    actor(d, 0x009c, 0x2caf, 0x0205, 0xa0d2, 0xa0dc, 0xa0ed);
    actor(d, 0x00a6, 0x2ebf, 0x0229, 0xa196, 0xa19a, 0xa19e);
    guardar(d, 0x35c7, static_cast<Palabra>(palabra(d, 0x35c7) + 5));
    return comenzar(d);
  }
  if (manejador == 0x3143 && id == 92) {
    // 324C–327D: el salto se ofrece después de desplazar la roca, sin puntos adicionales.
    if (palabra(d, 0x2cb3) == 76) {
      marcar_accion_rechazada(d);
      return ResultadoMenu::accion;
    }
    guardar(d, 0x1a38, static_cast<Palabra>(469 - palabra(d, 0x3591)));
    guardar(d, 0x1a3a, 112);
    guardar(d, 0x0ad7, 0x22b8);
    guardar(d, 0x0adc, 0x22d0);
    guardar(d, 0x0ae1, 0x22e3);
    return comenzar(d);
  }
  if (manejador == 0xa46a || (manejador == 0x2fca && id == 86)) {
    // A46A–A4A0: usar y llevar guantes comparten la comparación con signo, sin valor absoluto.
    if (con_signo(static_cast<Palabra>(palabra(d, 0x2c4e) - palabra(d, 0x2c2d))) > 40) {
      marcar_accion_rechazada(d);
      return ResultadoMenu::accion;
    }
    retirar_identificador(d, 86);
    guardar(d, 0x1a38, static_cast<Palabra>(974 - palabra(d, 0x3591)));
    guardar(d, 0x1a3a, 122);
    guardar(d, 0x0ad7, 0xa061);
    guardar(d, 0x0adc, 0xa072);
    guardar(d, 0x35c7, static_cast<Palabra>(palabra(d, 0x35c7) + 5));
    return comenzar(d);
  }
  if (manejador >= 0xa4a3 && manejador <= 0xa4c1 && (manejador - 0xa4a3) % 10 == 0) {
    // A4A3–A528: cada botón invierte su bit; el puente solo se programa con la combinación B.
    const auto indice = static_cast<Palabra>((manejador - 0xa4a3) / 10);
    const auto entidad = static_cast<Palabra>(0x2d33 + indice * 33);
    d[0x35c9] ^= static_cast<unsigned char>(8u << indice);
    guardar(d, 0x1a38, static_cast<Palabra>(palabra(d, entidad + 2) + 4));
    guardar(d, 0x1a3a, static_cast<Palabra>(palabra(d, entidad + 4) + 19));
    guardar(d, 0x0ad7, 0xa082);
    guardar(d, 0x0adc, 0xa08c);
    actor(d, 0x009c, entidad, 0x0107, d[entidad + 1] == 20 ? 0xa105 : 0xa107);
    if (((d[0x35c9] >> 3) & 15) == 11)
      actor(d, 0x00a6, 0x2d12, 0x020b, 0xa12c);
    return comenzar(d);
  }
  if (manejador == 0xa52b) {
    // A52B–A598: llevar el caramelo cerca de la piedra; AL+4 enlaza su entidad con el movimiento.
    const auto objeto = palabra(d, 0x2dfb), brad = palabra(d, 0x2c2d);
    auto distancia = static_cast<Palabra>(objeto - brad);
    if (con_signo(objeto) < con_signo(brad))
      distancia = static_cast<Palabra>(0 - distancia);
    if (distancia > 30) {
      marcar_accion_rechazada(d);
      return ResultadoMenu::accion;
    }
    retirar_identificador(d, 87);
    guardar(d, 0x1a38, static_cast<Palabra>(objeto + 2));
    guardar(d, 0x1a3a, 114);
    guardar(d, 0x0ad7, 0xa09a);
    guardar(d, 0x0adc, 0xa09f);
    guardar(d, 0x2c90, static_cast<Palabra>(1354 - palabra(d, 0x3591)));
    guardar(d, 0x2c92, 92);
    guardar(d, 0x2c98, 100);
    d[0x2c8e] = 3;
    guardar(d, 0x2c9e, 0x1a2c);
    d[0x2ca0] = 0;
    d[0x2ca2] = 1;
    unsigned short pasos = 0;
    if (!iniciar_accion(d, pasos))
      return ResultadoMenu::error;
    d[0x2ca1] = static_cast<unsigned char>(pasos + 4);
    guardar(d, 0x35c7, static_cast<Palabra>(palabra(d, 0x35c7) + 5));
    return ResultadoMenu::accion;
  }
  if (manejador == 0x3b53 && palabra(d, 0xb638) == 0xffff && palabra(d, 0xb634) == 5 && id == 94) {
    // A599–A7BB: la piedra dispara cinco fragmentos; las demoras se asignan en orden de pila.
    auto distancia = static_cast<Palabra>(palabra(d, 0x2c2d) + palabra(d, 0x3591) - 0x551);
    if (con_signo(distancia) <= 0)
      distancia = static_cast<Palabra>(0 - distancia);
    if (con_signo(distancia) > 30) {
      marcar_accion_rechazada(d);
      return ResultadoMenu::accion;
    }
    if (!creacion.crear)
      return ResultadoMenu::error;
    retirar_identificador(d, 94);
    guardar(d, 0x1a38, static_cast<Palabra>(0x558 - palabra(d, 0x3591)));
    guardar(d, 0x1a3a, 0x71);
    guardar(d, 0x0ad7, 0xa0a4);
    guardar(d, 0x0adc, 0xa0af);
    guardar(d, 0x2dfb, static_cast<Palabra>(0x56c - palabra(d, 0x3591)));
    guardar(d, 0x2dfd, 0x57);
    d[0x2df9] = 3;
    const Palabra campos[] = {0x2e09, 0x2e0e, 0x2e13, 0x2ce5, 0x2cea, 0x2f11, 0x2f16,
                              0x2f1b, 0x2e8d, 0x2e92, 0x2e97, 0x2eae, 0x2eb3, 0x2eb8};
    const Palabra guiones[] = {0xa136, 0xa146, 0xa152, 0xa163, 0xa169, 0xa1ae, 0xa1b6,
                               0xa1ba, 0xa196, 0xa1a6, 0xa1a2, 0xa196, 0xa1aa, 0xa1a2};
    for (unsigned int n = 0; n < 14; ++n) {
      guardar(d, campos[n], guiones[n]);
      d[campos[n] + 2] = 0;
      d[campos[n] + 4] = n < 5 ? 1 : 2;
    }
    guardar(d, 0x2e7f, static_cast<Palabra>(0x55c - palabra(d, 0x3591)));
    guardar(d, 0x2e81, 0x70);
    guardar(d, 0x2ea0, static_cast<Palabra>(0x5b1 - palabra(d, 0x3591)));
    guardar(d, 0x2ea2, 0x6f);
    const Palabra posiciones[][2] = {
        {0x562, 0x37}, {0x5a5, 0x43}, {0x57e, 0x50}, {0x599, 0x4f}, {0x56a, 0x46}};
    Palabra fragmentos[5];
    for (unsigned int n = 0; n < 5; ++n) {
      guardar(d, 0xa17e, static_cast<Palabra>(posiciones[n][0] - palabra(d, 0x3591)));
      guardar(d, 0xa180, posiciones[n][1]);
      if (!creacion.crear(creacion.contexto, d, 0xa17b, fragmentos[n]))
        return ResultadoMenu::error;
    }
    unsigned short pasos = 0;
    if (!iniciar_accion(d, pasos))
      return ResultadoMenu::error;
    const unsigned int ajustes[] = {6, 6, 6, 10, 10, 16, 16, 16, 24, 24, 24, 24, 24, 24};
    for (unsigned int n = 0; n < 14; ++n)
      d[campos[n] + 3] = static_cast<unsigned char>(pasos + ajustes[n]);
    const unsigned int demoras[] = {18, 16, 14, 12, 10};
    for (unsigned int n = 0; n < 5; ++n)
      for (unsigned int canal = 0; canal < 3; ++canal)
        d[fragmentos[4 - n] + 19 + canal * 5] = static_cast<unsigned char>(pasos + demoras[4 - n]);
    guardar(d, 0x35c7, static_cast<Palabra>(palabra(d, 0x35c7) + 5));
    return ResultadoMenu::accion;
  }
  if (manejador == 0x3b53 && palabra(d, 0xb638) == 0xffff && palabra(d, 0xb634) == 5 && id == 95) {
    // A7BC–A819: lanzar la cuerda transforma su destino96 y conserva los otros canales diferidos.
    auto distancia = static_cast<Palabra>(0x61f - palabra(d, 0x3591) - palabra(d, 0x2c2d));
    if (con_signo(distancia) < 0)
      distancia = static_cast<Palabra>(0 - distancia);
    if (con_signo(distancia) > 40) {
      marcar_accion_rechazada(d);
      return ResultadoMenu::accion;
    }
    retirar_identificador(d, 95);
    guardar(d, 0x2e6a, 96);
    guardar(d, 0x2e68, 30);
    guardar(d, 0x1a38, static_cast<Palabra>(0x626 - palabra(d, 0x3591)));
    guardar(d, 0x1a3a, 0x70);
    guardar(d, 0x0ad7, 0xa0b9);
    guardar(d, 0x0adc, 0xa0c7);
    actor(d, 0x009c, 0x2e5c, 0x0109, 0xa179);
    guardar(d, 0x35c7, static_cast<Palabra>(palabra(d, 0x35c7) + 5));
    return comenzar(d);
  }
  if (manejador == 0x3143 && id == 96) {
    // 3280–32BE: saltar el abismo exige estar a treinta unidades del punto original.
    auto distancia = static_cast<Palabra>(0x637 - palabra(d, 0x3591) - palabra(d, 0x2c2d));
    if (con_signo(distancia) < 0)
      distancia = static_cast<Palabra>(0 - distancia);
    if (con_signo(distancia) > 30) {
      marcar_accion_rechazada(d);
      return ResultadoMenu::accion;
    }
    guardar(d, 0x1a38, static_cast<Palabra>(0x63e - palabra(d, 0x3591)));
    guardar(d, 0x1a3a, 0x71);
    guardar(d, 0x0ad7, 0x22f0);
    guardar(d, 0x0adc, 0x2356);
    guardar(d, 0x0ae1, 0x23bb);
    return comenzar(d);
  }
  if (manejador == 0x384a && id == 97) {
    // 3AFD–3B3D: examinar la inscripción ofrece el tema2 si la puerta no está abierta.
    if (d[0x2f22] != 1)
      guardar(d, 0x523d, 2);
    guardar(d, 0x1a38, static_cast<Palabra>(0x820 - palabra(d, 0x3591)));
    guardar(d, 0x1a3a, 0x71);
    guardar(d, 0x0ad7, 0x270d);
    guardar(d, 0x2e4b, 0x1a2f);
    d[0x2e4d] = 0;
    d[0x2e4f] = 30;
    unsigned short pasos = 0;
    if (!iniciar_accion(d, pasos))
      return ResultadoMenu::error;
    d[0x2e4e] = static_cast<unsigned char>(pasos + 4);
    return ResultadoMenu::accion;
  }
  if (manejador == 0x32de && id == 2) {
    // 3809–3848: las palabras retiran el tema y abren la puerta con el servicio sonoro común19.
    guardar(d, 0x523d, 0xffff);
    guardar(d, 0x2f32, 0x1a2c);
    d[0x2f34] = 0;
    d[0x2f35] = 5;
    d[0x2f36] = 1;
    guardar(d, 0x2f53, 0x2561);
    d[0x2f55] = 0;
    d[0x2f56] = 1;
    d[0x2f57] = 4;
    return emitir_sonido(sonido, d, 0x2c2b, 19) ? ResultadoMenu::accion : ResultadoMenu::error;
  }
  if (manejador == 0x32de && id <= 1)
    return emitir_sonido(sonido, d, 0x2c2b, static_cast<unsigned char>(id)) ? ResultadoMenu::accion
                                                                            : ResultadoMenu::error;
  if (manejador == 0x384a)
    return ResultadoMenu::actualizado;
  if (manejador == 0x2fca || manejador == 0x3143 || manejador == 0x3b53) {
    marcar_accion_rechazada(d);
    return ResultadoMenu::accion;
  }
  return ResultadoMenu::pendiente;
}
} // namespace motor
