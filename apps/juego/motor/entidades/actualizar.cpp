#include "actualizar.hpp"
#include "../acciones/iniciar_accion.hpp"
#include "../movimiento/planificar.hpp"
#include "../estado/segmento.hpp"

namespace motor {
namespace {

void secuencia(unsigned char *datos, Palabra base, Palabra puntero, unsigned char intervalo,
               int modo) {
  guardar(datos, base, puntero);
  datos[static_cast<Palabra>(base + 2)] = 0;
  datos[static_cast<Palabra>(base + 3)] = intervalo;
  if (modo >= 0)
    datos[static_cast<Palabra>(base + 4)] = static_cast<unsigned char>(modo);
}

Palabra absoluto(Palabra valor) {
  return con_signo(valor) < 0 ? static_cast<Palabra>(0 - valor) : valor;
}

int signo_byte(unsigned char valor) {
  return valor < 128 ? valor : static_cast<int>(valor) - 256;
}

void configurar_personaje(unsigned char *datos, Palabra a, Palabra b, Palabra c) {
  guardar(datos, 0x2c3b, a);
  guardar(datos, 0x2c40, b);
  guardar(datos, 0x2c45, c);
}

} // namespace

void activar_transicion_entidad(unsigned char *datos) {
  if (datos[0x3594] == 0)
    return;
  reiniciar_estado_accion(datos);
  secuencia(datos, 0x2c3b, 0xaff7, 1, 1);
  secuencia(datos, 0x2c40, 0xb015, 1, 1);
  secuencia(datos, 0x2c45, 0xb021, 1, 1);
  guardar(datos, 0x2cf3, static_cast<Palabra>(palabra(datos, 0x2c2d) - 2));
  guardar(datos, 0x2cf5, static_cast<Palabra>(palabra(datos, 0x2c2f) + 13));
  secuencia(datos, 0x2d01, 0xb04e, 10, 2);
  secuencia(datos, 0x2d06, 0xb054, 10, 2);
  secuencia(datos, 0x2d0b, 0xb058, 10, 2);
  guardar(datos, 0x2db9, static_cast<Palabra>(palabra(datos, 0x2c2d) - 10));
  guardar(datos, 0x2dbb, 0x00d2);
  secuencia(datos, 0x2dc7, 0xb05c, 20, 1);
  secuencia(datos, 0x2dcc, 0xb06a, 22, 1);
  secuencia(datos, 0x2dd1, 0xb071, 22, 1);
}

bool actualizar_entidad(unsigned char *datos, EstadoAleatorio &estado,
                        const RecursosEntidades &recursos, unsigned short posicion) {
  const auto direccion = [posicion](int desplazamiento) {
    return static_cast<Palabra>(posicion + desplazamiento);
  };
  const auto p = [datos, &direccion](int desplazamiento) {
    return palabra(datos, direccion(desplazamiento));
  };
  const auto b = [datos, &direccion](int desplazamiento) {
    return datos[direccion(desplazamiento)];
  };
  const auto w = [datos, &direccion](int desplazamiento, Palabra valor) {
    guardar(datos, direccion(desplazamiento), valor);
  };
  const auto configurar_secuencia = [datos, &direccion](int desplazamiento, Palabra puntero,
                                                        unsigned char intervalo, int modo) {
    secuencia(datos, direccion(desplazamiento), puntero, intervalo, modo);
  };
  const auto crear = [&](Palabra descriptor) {
    return crear_entidad(datos, estado, recursos, descriptor).valido;
  };
  const auto limites = [&]() {
    const auto x = static_cast<Palabra>(palabra(datos, 0x3591) + p(2));
    guardar(datos, 0x18f5, x);
    guardar(datos, 0x18f9, static_cast<Palabra>(x + p(6)));
    guardar(datos, 0x18fb, 199);
  };
  const auto reiniciar_personaje = [&](Palabra a, Palabra s, Palabra c) {
    reiniciar_estado_accion(datos);
    configurar_personaje(datos, a, s, c);
  };
  if (b(0) != 1)
    return true;
  const auto tipo = p(14);
  switch (tipo) {
  case 0x400: {
    const auto centro = static_cast<Palabra>((p(-27) >> 1) + p(-31));
    const auto distancia = absoluto(static_cast<Palabra>(centro - palabra(datos, 0x2c2d)));
    Palabra paso = 21;
    if (con_signo(distancia) > 50)
      paso = con_signo(centro) >= con_signo(palabra(datos, 0x2c2d)) ? 20 : 22;
    w(2, static_cast<Palabra>(paso + p(-31)));
    break;
  }
  case 0x401: {
    if ((p(16) | p(21) | p(26)) != 0)
      break;
    const auto modelo = datos[0x358c];
    const auto indice = static_cast<Palabra>(static_cast<unsigned char>(modelo + 1) * 4);
    const auto tabla_x = palabra(datos, static_cast<Palabra>(0x0e56 + indice));
    if (tabla_x == 0)
      break;
    const auto tabla_y = palabra(datos, static_cast<Palabra>(0x0e58 + indice));
    auto imagen = datos[0x2c2c];
    if (modelo == 0xff && (imagen & 0x80) != 0)
      imagen = static_cast<unsigned char>(imagen - 0x5e);
    else if (modelo == 2)
      imagen &= 0x7f;
    w(4, static_cast<Palabra>(signo_byte(datos[static_cast<Palabra>(tabla_y + imagen)]) +
                              palabra(datos, 0x2c2f)));
    w(2, static_cast<Palabra>(signo_byte(datos[static_cast<Palabra>(tabla_x + imagen)]) +
                              palabra(datos, 0x2c2d)));
    break;
  }
  case 0x406: {
    if (con_signo(p(4)) >= 200) {
      datos[posicion] = 0;
      break;
    }
    if (palabra(datos, 0x8082) != 8 || con_signo(p(4)) < 145)
      break;
    Palabra azar = 0;
    generar_aleatorio(estado, 4, azar);
    if (azar != 0)
      break;
    w(14, 0xffff);
    configurar_secuencia(16, 0x131b, 1, 2);
    configurar_secuencia(21, 0x1322, 1, 2);
    w(26, 0);
    break;
  }
  case 0x407: {
    if (datos[0x3594] == 0)
      break;
    const auto dx = static_cast<Palabra>(palabra(datos, 0x2c2d) - p(2));
    if (p(16) == 0) {
      if (con_signo(dx) <= -24 || con_signo(dx) >= 24)
        break;
      configurar_secuencia(16, 0x1008, 6, -1);
      configurar_secuencia(21, 0x101f, 6, -1);
      configurar_secuencia(26, 0x1032, 6, -1);
      guardar(datos, 0x0ff0, static_cast<Palabra>(p(2) - 28));
      if (!crear(0x0fed))
        return false;
    } else if (b(18) == 5 && con_signo(dx) > -16 && con_signo(dx) < 16) {
      w(14, 0xffff);
      reiniciar_estado_accion(datos);
      guardar(datos, 0x2c2f, 138);
      configurar_personaje(datos, 0x0f94, 0x0faa, 0x0fb9);
    }
    break;
  }
  case 0x408:
    limites();
    if (con_signo(static_cast<Palabra>(p(2) + p(6) + 64)) <= 0)
      datos[posicion] = 0;
    break;
  case 0x409:
    limites();
    if (datos[0x3594] == 0)
      break;
    if (con_signo(static_cast<Palabra>(p(2) - palabra(datos, 0x2c2d))) >= 28) {
      if (b(18) != 0 && b(18) != 3)
        w(2, static_cast<Palabra>(p(2) - 2));
    } else {
      reiniciar_personaje(0x0fc8, 0x0fd9, 0x0fe3);
      guardar(datos, 0x2c2f, 138);
    }
    break;
  case 0x40a: {
    const auto actual = palabra(datos, 0x2c3b);
    if (actual == 0x11f0 || actual == 0x104a)
      break;
    const auto delta = static_cast<Palabra>(p(2) - palabra(datos, 0x2c2d));
    if (con_signo(delta) < 0) {
      if (con_signo(static_cast<Palabra>(delta + p(6) - 10)) <= 0)
        break;
      reiniciar_personaje(0x11f0, 0x0e26, 0x0e46);
    } else {
      if (con_signo(static_cast<Palabra>(delta - palabra(datos, 0x2c31))) >= 0)
        break;
      reiniciar_personaje(0x11f0, 0x0e36, 0x0e46);
    }
    datos[0x2c3f] = datos[0x2c44] = datos[0x2c49] = 1;
    break;
  }
  case 0x40b:
    if (p(16) == 0x1111)
      break;
    if (con_signo(static_cast<Palabra>(p(4) - palabra(datos, 0x2c2f))) >= 26) {
      w(4, static_cast<Palabra>(palabra(datos, 0x2c2f) + 26));
      configurar_secuencia(16, 0x1111, 19, 2);
      configurar_secuencia(21, 0x1143, 19, 1);
      configurar_secuencia(26, 0x1186, 19, 1);
      secuencia(datos, 0x2c3b, 0x104a, 1, 1);
      secuencia(datos, 0x2c40, 0x106a, 1, 1);
      secuencia(datos, 0x2c45, 0x10b1, 1, 1);
    } else if (p(26) != 0x110f && b(18) == 13) {
      configurar_secuencia(16, 0x110d, 1, 1);
      w(21, 0);
      configurar_secuencia(26, 0x110f, 1, 1);
    }
    break;
  case 0x40c: {
    const auto x = p(2);
    if (con_signo(x) >= 320) {
      datos[posicion] = 3;
      break;
    }
    w(2, static_cast<Palabra>(x + 8));
    if (con_signo(static_cast<Palabra>(x + 96)) >= con_signo(palabra(datos, 0x2c2d))) {
      w(4, static_cast<Palabra>(p(4) - 1));
      w(2, static_cast<Palabra>(p(2) + 8));
      break;
    }
    const int dy = con_signo(static_cast<Palabra>(p(4) - palabra(datos, 0x2c2f)));
    if (dy < -20)
      w(4, static_cast<Palabra>(p(4) + 1));
    else if (dy > -20)
      w(4, static_cast<Palabra>(p(4) - 1));
    if (p(16) == 0x11c6)
      break;
    const int dx = con_signo(static_cast<Palabra>(palabra(datos, 0x2c2d) - p(2)));
    if (dx > 144)
      break;
    if (dx >= 124 && datos[0x358c] == 8 && signo_byte(datos[0x2c2c]) >= 1 &&
        signo_byte(datos[0x2c2c]) <= 2) {
      configurar_secuencia(16, 0x11c6, 1, 1);
      configurar_secuencia(26, 0x11dc, 1, 1);
      datos[0x389e] = 0;
      break;
    }
    if (datos[0x358c] != 8 && datos[0x3594] == 0)
      break;
    if (palabra(datos, 0x2c3b) == 0x10f8)
      break;
    reiniciar_personaje(0x10f8, 0x1104, 0);
    datos[0x2c3f] = datos[0x2c44] = 1;
    configurar_secuencia(21, 0x1104, 1, 1);
    break;
  }
  case 0x40d: {
    if (p(16) == 0)
      break;
    for (int i = 0; i < 5; ++i) {
      Palabra azar = 0;
      generar_aleatorio(estado, 4, azar);
      datos[0x1330] = static_cast<unsigned char>(azar + 26);
      if (!generar_aleatorio(estado, p(6), azar))
        return false;
      guardar(datos, 0x1331, static_cast<Palabra>(azar + p(2)));
      if (!generar_aleatorio(estado, p(8), azar))
        return false;
      guardar(datos, 0x1333, static_cast<Palabra>(azar + p(4)));
      if (!crear(0x132e))
        return false;
    }
    break;
  }
  case 0x40f: {
    auto x = static_cast<Palabra>(p(2) + p(6));
    if ((datos[0x2c4c] & 2) != 0) {
      guardar(datos, 0x18f9, x);
      guardar(datos, 0x18fb, 199);
      break;
    }
    x = static_cast<Palabra>(x + 16);
    if (con_signo(x) < con_signo(palabra(datos, 0x2c2d)) || p(16) != 0)
      break;
    configurar_secuencia(16, 0x1324, 1, 2);
    configurar_secuencia(21, 0x132a, 1, 2);
    guardar(datos, 0x1a38, 15);
    guardar(datos, 0x1a3a,
            static_cast<Palabra>(palabra(datos, 0x2c2f) + palabra(datos, 0x2c33) - 1));
    guardar(datos, 0x0dfa, 0x1a36);
    Palabra pasos = 0;
    if (!planificar_movimiento(datos, 0x1a36, pasos))
      return false;
    break;
  }
  case 0x410: {
    auto x = static_cast<Palabra>(palabra(datos, 0x2c2d) + 8);
    auto y = static_cast<Palabra>(palabra(datos, 0x2c2f) - 7);
    const auto imagen = datos[0x2c2c];
    if (imagen < 9) {
      datos[direccion(1)] = 0;
      x = static_cast<Palabra>(x + 3);
      y = static_cast<Palabra>(y - 2);
    } else if (imagen < 18)
      datos[direccion(1)] = 3;
    else if (imagen < 128)
      datos[direccion(1)] = 1;
    else {
      datos[direccion(1)] = 2;
      x = static_cast<Palabra>(x - 5);
      y = static_cast<Palabra>(y - 2);
    }
    w(2, x);
    w(4, y);
    break;
  }
  case 0x411: {
    if (con_signo(palabra(datos, 0x2c2f)) <= 69)
      break;
    const auto ancho = palabra(datos, 0x2c31);
    auto x = static_cast<Palabra>(palabra(datos, 0x2c2d) + ancho - p(2));
    if (con_signo(x) <= 0)
      break;
    x = static_cast<Palabra>(x - ancho - p(6));
    if (con_signo(x) < 0)
      activar_transicion_entidad(datos);
    break;
  }
  case 0x412: {
    if (b(1) != 0x3c || datos[0x3594] == 0)
      break;
    if (con_signo(absoluto(static_cast<Palabra>(p(2) - palabra(datos, 0x2c2d)))) > 20)
      break;
    if (con_signo(absoluto(static_cast<Palabra>(p(4) + p(8) - palabra(datos, 0x2c2f) -
                                                palabra(datos, 0x2c33)))) > 10)
      break;
    reiniciar_estado_accion(datos);
    const unsigned short xs[] = {4, 12, 8}, ys[] = {5, 10, 15};
    for (int i = 0; i < 3; ++i) {
      guardar(datos, 0x1361, static_cast<Palabra>(palabra(datos, 0x2c2d) + xs[i]));
      guardar(datos, 0x1363, static_cast<Palabra>(palabra(datos, 0x2c2f) + ys[i]));
      if (!crear(0x135e))
        return false;
    }
    configurar_personaje(datos, 0x1349, 0x1354, 0x1359);
    break;
  }
  default:
    break;
  }
  return true;
}

bool actualizar_entidades(unsigned char *datos, EstadoAleatorio &estado,
                          const RecursosEntidades &recursos) {
  datos[0x18f3] = datos[0x18f4] = 0;
  guardar(datos, 0x18f5, 0);
  guardar(datos, 0x18f7, 0);
  guardar(datos, 0x18f9, 0);
  guardar(datos, 0x18fb, 0);
  Palabra posicion = 0x2c4c;
  for (int i = 0; i < 68; ++i) {
    if (!actualizar_entidad(datos, estado, recursos, posicion))
      return false;
    posicion = static_cast<Palabra>(posicion + 33);
  }
  return true;
}

} // namespace motor
