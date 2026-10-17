#include "geometria.hpp"
#include "../estado/segmento.hpp"
#include "../acciones/lista_identificadores.hpp"
namespace motor {
namespace {
// Obtiene una palabra de un recurso con capacidad explícita; el llamador valida previamente el
// rango.
Palabra leer(const unsigned char *datos, unsigned int p) {
  return static_cast<Palabra>(datos[p] | (static_cast<unsigned int>(datos[p + 1]) << 8));
}
// Conserva las comparaciones inclusivas con signo del detector original.
bool contiene(const unsigned char *g, unsigned int p, Palabra x, Palabra y) {
  return con_signo(x) >= con_signo(leer(g, p)) && con_signo(x) <= con_signo(leer(g, p + 4)) &&
         con_signo(y) >= con_signo(leer(g, p + 2)) && con_signo(y) <= con_signo(leer(g, p + 6));
}
// 25D9–2770: predicados de los pasos entre escenas, sin adelantar su efecto.
bool permite_transicion(const unsigned char *d, unsigned char id) {
  switch (id) {
  case 0:
    return d[0x2cb0] == 0x10;
  case 0x11:
    return d[0x2c4d] == 0x0d;
  case 0x23:
  case 0x26:
    return d[0x2c4c] != 1;
  case 0x24:
    return !(d[0x35c9] & 4);
  case 0x3f:
    return d[0x358c] == 0xff;
  case 0x42:
    return d[0x2cd0] == 1;
  case 0x44:
    return d[0x2c6d] == 1;
  case 0x45:
    return palabra(d, 0x2c90) == 0x127;
  case 0x48:
    return d[0x2f44] == 0x44;
  case 0x4a:
    return d[0x2c8e] == 1;
  case 0x53:
    return d[0x2c6d] == 0;
  case 0x54:
    return palabra(d, 0x2d16) == 0x1e;
  case 0x5b:
    return d[0x4b2c] == 1;
  case 0x5d:
    return d[0x4b35] == 1;
  case 0x5f:
    return d[0x4b3e] == 1;
  case 0x61:
    return d[0x4b47] == 1;
  case 0x63:
    return d[0x4b50] == 1;
  case 0x65:
    return d[0x4b59] == 1;
  case 0x67:
    return d[0x358c] == 0x18;
  case 0x74:
    return d[0x2cd1] != 0x16 && d[0x2c4c] == 0;
  case 0x7c:
    return static_cast<Palabra>(palabra(d, 0x2c6f) + palabra(d, 0x3591)) == 0x9b;
  default:
    return true;
  }
}
} // namespace

// El índice inicial tiene 78 palabras; los bloques contienen una cuenta seguida de registros de
// diez bytes.
bool localizar_geometria(const unsigned char *archivo, unsigned int longitud, unsigned short escena,
                         unsigned int &inicio, unsigned int &cantidad) {
  if (escena >= 78 || longitud < 156)
    return false;
  const auto p = leer(archivo, escena * 2u);
  if (!p) {
    inicio = cantidad = 0;
    return true;
  }
  if (p > longitud || longitud - p < 2)
    return false;
  cantidad = leer(archivo, p);
  inicio = p + 2u;
  return cantidad <= 32767 && cantidad * 10u <= longitud - inicio;
}

// Los nodos enlazan seis bytes: siguiente, x e y. Se reserva hasta 99B4 para no invadir el
// cargador.
bool preparar_camino(unsigned char *d, const unsigned char *g, unsigned int longitud) {
  Palabra destino = 0x9978;
  const auto n = palabra(d, 0x0df2), cajas = palabra(d, 0x0df6);
  if (n) {
    if (!g || longitud < 0x0dfa || cajas > longitud || n * 8u > longitud - cajas)
      return false;
    const auto rutas = leer(g, 0x0df8);
    Palabra origen = n, objetivo = n;
    for (Palabra i = 0; i < n; ++i) {
      if (origen == n && contiene(g, cajas + i * 8u, palabra(d, 0x2c2d),
                                  static_cast<Palabra>(palabra(d, 0x2c2f) + palabra(d, 0x2c33))))
        origen = i;
      if (objetivo == n && contiene(g, cajas + i * 8u, palabra(d, 0x2c0c), palabra(d, 0x2c0e)))
        objetivo = i;
    }
    if (origen < n && objetivo < n && origen != objetivo) {
      unsigned int p = rutas + origen * 2u;
      if (p + 2 > longitud)
        return false;
      p = rutas + leer(g, p) + objetivo * 2u;
      if (p + 2 > longitud)
        return false;
      p = rutas + leer(g, p);
      if (p + 2 > longitud)
        return false;
      const auto pasos = leer(g, p);
      p += 2;
      if (pasos > 9 || pasos * 4u > longitud - p)
        return false;
      for (Palabra i = 0; i < pasos; ++i) {
        guardar(d, destino, static_cast<Palabra>(destino + 6));
        guardar(d, destino + 2, leer(g, p));
        guardar(d, destino + 4, leer(g, p + 2));
        destino = static_cast<Palabra>(destino + 6);
        p += 4;
      }
    }
  }
  guardar(d, destino, 0);
  guardar(d, destino + 2, palabra(d, 0x2c0c));
  guardar(d, destino + 4, palabra(d, 0x2c0e));
  guardar(d, 0x0dfa, 0x9978);
  return true;
}

// Mantiene la prioridad de registros, los retornos anticipados y la última pared encontrada.
Colision comprobar_colision(unsigned char *d, EstadoAleatorio &azar, const unsigned char *g,
                            unsigned int longitud, Palabra izquierda, Palabra arriba,
                            Palabra derecha, Palabra abajo) {
  d[0x18f0] = 0xff;
  if (con_signo(izquierda) <= con_signo(palabra(d, 0x18f9)) &&
      con_signo(arriba) <= con_signo(palabra(d, 0x18fb)) &&
      con_signo(derecha) >= con_signo(palabra(d, 0x18f5)) &&
      con_signo(abajo) >= con_signo(palabra(d, 0x18f7)))
    d[0x18f0] = d[0x18f4];
  const auto cantidad = palabra(d, 0x0df0);
  if (cantidad && (!g || cantidad * 10u > longitud))
    return {false, 0, 0};
  if (cantidad)
    guardar(d, 0x18f1, cantidad);
  for (unsigned int i = 0; i < cantidad; ++i) {
    const auto p = i * 10u;
    if (con_signo(izquierda) <= con_signo(leer(g, p + 6)) &&
        con_signo(arriba) <= con_signo(leer(g, p + 8)) &&
        con_signo(derecha) >= con_signo(leer(g, p + 2)) &&
        con_signo(abajo) >= con_signo(leer(g, p + 4))) {
      const auto tipo = g[p], id = g[p + 1];
      if (tipo == 0)
        d[0x18f0] = id;
      else if (tipo == 1)
        return {true, 4, id};
      else if (tipo == 3 && (id <= 4 || (id >= 9 && id <= 11)))
        return {true, 8, id};
      else if (tipo == 2 && permite_transicion(d, id)) {
        Palabra secuencia = id == 0x38   ? 0x18fd
                            : id == 0x4c ? 0x1907
                            : id == 0x4e ? 0x1911
                            : id == 0x50 ? 0x191b
                                         : 0;
        if (secuencia) {
          guardar(d, 0x2c3b, secuencia);
          d[0x3594] = d[0x3595] = d[0x3596] = 0;
          guardar(d, 0x359a, 0);
          guardar(d, 0x0dfa, 0);
          for (Palabra c = 0; c < 3; ++c) {
            const auto base = static_cast<Palabra>(0x2c3b + c * 5);
            d[base + 2] = 0;
            d[base + 3] = d[base + 4] = 1;
          }
          guardar(d, 0x2c40, 0x1925);
          guardar(d, 0x2c45, 0x192e);
          return {true, 0, 0};
        }
        unsigned char destino = id;
        Palabra valor = 0;
        if (id == 0x46 || id == 0x69 || id == 0x6c) {
          if (!generar_aleatorio(azar, 2, valor))
            return {false, 0, 0};
          if (valor)
            destino = id == 0x46 ? 0x34 : static_cast<unsigned char>(id + 1);
        } else if (id == 0x6b && !contiene_identificador(d, 0xa1))
          destino = static_cast<unsigned char>(id + 1);
        return {true, 2, destino};
      }
    }
    guardar(d, 0x18f1, static_cast<Palabra>(palabra(d, 0x18f1) - 1));
    if (con_signo(palabra(d, 0x18f1)) <= 0)
      break;
  }
  return {true, static_cast<Palabra>(d[0x18f0] == 0xff ? 0 : 1), d[0x18f0]};
}

// La corrección de un píxel y el orden de restauración de x/y reproducen 0AF6–0B7F.
Colision resolver_colision_personaje(unsigned char *d, EstadoAleatorio &azar,
                                     const unsigned char *g, unsigned int longitud) {
  if (!d[0x3596])
    return {true, 0, 0};
  guardar(d, 0x0aea, palabra(d, 0x2c2f));
  bool ajustar = true;
  for (unsigned int intento = 0; intento < 8; ++intento) {
    const auto margen_x = d[0x358c] == 0x14 ? 3 : 6, margen_y = d[0x358c] == 0x14 ? 2 : 4;
    const auto x = static_cast<Palabra>(palabra(d, 0x2c2d) + palabra(d, 0x3591));
    const auto pies = static_cast<Palabra>(palabra(d, 0x2c2f) + palabra(d, 0x2c33));
    const auto colision =
        comprobar_colision(d, azar, g, longitud, static_cast<Palabra>(x + margen_x),
                           static_cast<Palabra>(pies - margen_y),
                           static_cast<Palabra>(x + palabra(d, 0x2c31) - 1 - margen_x),
                           static_cast<Palabra>(pies - 1));
    if (!colision.valida || colision.acciones > 1)
      return colision;
    if (colision.acciones == 0)
      break;
    if (ajustar) {
      ajustar = false;
      if (palabra(d, 0x2c45) == 0 && (colision.parametro == 1 || colision.parametro == 2)) {
        const int cambio = colision.parametro == 1 ? 1 : -1;
        guardar(d, 0x0aea, static_cast<Palabra>(palabra(d, 0x0aea) + cambio));
        guardar(d, 0x0ae8, static_cast<Palabra>(palabra(d, 0x0ae8) + cambio));
        guardar(d, 0x2c2f, static_cast<Palabra>(palabra(d, 0x2c2f) + cambio));
        continue;
      }
    }
    if (palabra(d, 0x0ae8) != palabra(d, 0x2c2f)) {
      guardar(d, 0x35a4, palabra(d, 0x0ae8));
      guardar(d, 0x35a2, 0);
      guardar(d, 0x2c2f, palabra(d, 0x0ae8));
      if (palabra(d, 0x0ae6) != palabra(d, 0x2c2d))
        continue;
    } else if (palabra(d, 0x0ae6) != palabra(d, 0x2c2d)) {
      guardar(d, 0x35a0, palabra(d, 0x0ae6));
      guardar(d, 0x359e, 0);
      guardar(d, 0x2c2d, palabra(d, 0x0ae6));
      guardar(d, 0x35a4, palabra(d, 0x0aea));
      guardar(d, 0x35a2, 0);
      guardar(d, 0x2c2f, palabra(d, 0x0aea));
      continue;
    }
    break;
  }
  if (palabra(d, 0x0ae6) == palabra(d, 0x2c2d) && palabra(d, 0x0ae8) == palabra(d, 0x2c2f)) {
    const auto espera = static_cast<Palabra>(palabra(d, 0x359c) - 1);
    guardar(d, 0x359c, espera);
    if (con_signo(espera) <= 0) {
      guardar(d, 0x359a, 0);
      guardar(d, 0x359c, 2);
    }
  } else
    guardar(d, 0x359c, 2);
  return {true, 0, 0};
}

// Desplaza de derecha a izquierda para conservar el solapamiento de REP MOVSW con DF activado.
void actualizar_historial_personaje(unsigned char *d) {
  for (Palabra i = 9; i > 0; --i)
    guardar(d, static_cast<Palabra>(0x35b3 + i * 2),
            palabra(d, static_cast<Palabra>(0x35b1 + i * 2)));
  guardar(d, 0x35b3, palabra(d, 0x2c2d));
}
} // namespace motor
