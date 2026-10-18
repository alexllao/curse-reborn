#include "inicializar.hpp"
#include "segmento.hpp"

namespace motor {
// Traduce los fragmentos de estado del cargador sin ejecutar sus servicios de memoria y vídeo DOS.
bool preparar_transicion_escena(unsigned char *d, unsigned short salida,
                                unsigned short referencia_estandar,
                                unsigned short referencia_modelo, unsigned short ancho,
                                unsigned short alto) {
  constexpr unsigned short origenes[] = {0,  1,  1,  2,  3,  4,  4,  5,  5,  5,  6,  6,  6,
                                         6,  7,  7,  7,  8,  9,  10, 11, 12, 13, 14, 12, 15,
                                         16, 25, 17, 17, 17, 17, 18, 18, 18, 19, 24, 19, 20,
                                         20, 20, 21, 21, 21, 22, 23, 24, 22, 26, 27, 28, 29};
  // Accesos nativos contrastados del valle y WISE; los demás destinos tardíos siguen detenidos.
  if (salida == 59   ? palabra(d, 0x8082) != 32
      : salida == 54 ? palabra(d, 0x8082) != 33
      : salida == 57 ? palabra(d, 0x8082) != 35
      : salida == 58 ? palabra(d, 0x8082) != 32
      : salida == 60 ? palabra(d, 0x8082) != 35
      : salida == 56 ? palabra(d, 0x8082) != 34
      : salida == 55 ? palabra(d, 0x8082) != 33
      : salida == 61 ? palabra(d, 0x8082) != 33
      : salida == 71 ? palabra(d, 0x8082) != 35
      : salida == 52 ? palabra(d, 0x8082) != 20
      : salida == 72 ? (palabra(d, 0x8082) != 30 && palabra(d, 0x8082) != 22)
      : salida == 73 ? palabra(d, 0x8082) != 31
                     : (salida >= sizeof(origenes) / sizeof(origenes[0]) ||
                        origenes[salida] == 0xffff || palabra(d, 0x8082) != origenes[salida]))
    return false;
  const auto descriptor = static_cast<Palabra>(0x85ac + salida * 14);
  const auto modelo = d[static_cast<Palabra>(descriptor + 13)];
  if (modelo != 0xff && modelo != 5 && modelo != 11 && modelo != 14 && modelo != 18 &&
      modelo != 19 && modelo != 20 && modelo != 22 && modelo != 23)
    return false;
  guardar(d, 0x3598, salida);
  d[0x2c0b] = 0;
  guardar(d, 0x2c1a, 0);
  for (unsigned int i = 0; i < 6; ++i)
    d[0x2c2b + i] = d[descriptor + i];
  guardar(d, 0x2c31, modelo == 0xff ? 32 : ancho);
  guardar(d, 0x2c33, modelo == 0xff ? 48 : alto);
  guardar(d, 0x2c35, 0);
  guardar(d, 0x2c37, 0);
  guardar(d, 0x2c39, 0);
  for (unsigned int c = 0; c < 3; ++c) {
    const auto destino = static_cast<Palabra>(0x2c3b + c * 5);
    guardar(d, destino, palabra(d, static_cast<Palabra>(descriptor + 6 + c * 2)));
    d[destino + 2] = 0;
    d[destino + 3] = d[destino + 4] = 1;
  }
  d[0x3597] = d[descriptor + 12];
  guardar(d, 0x3591, static_cast<Palabra>(d[0x3597] << 5));
  for (unsigned int i = 0; i < 10; ++i)
    guardar(d, static_cast<Palabra>(0x35b3 + i * 2), palabra(d, 0x2c2d));
  d[0x358c] = modelo;
  if (modelo != 0xff) {
    d[0x2c3f] = d[0x2c44] = d[0x2c49] = 2;
    d[0x9974] = modelo;
  }
  guardar(d, 0x2c4a, modelo == 0xff ? referencia_estandar : referencia_modelo);
  guardar(d, 0x8082, palabra(d, static_cast<Palabra>(0x7482 + salida * 2)));
  guardar(d, 0x358d, 64);
  guardar(d, 0x358f, 223);
  d[0x3594] = d[0x3595] = d[0x3596] = d[0x3593] = 0;
  guardar(d, 0x359a, 0);
  guardar(d, 0x35b0, 0);
  guardar(d, 0x35ae, 0);
  guardar(d, 0x0dfa, 0);
  d[0x0def] = 0;
  return true;
}
// Conserva el contrato de las entradas ya comparadas entre la celda y el corredor.
bool preparar_transicion_mazmorra(unsigned char *d, unsigned short salida,
                                  unsigned short referencia_estandar) {
  return (salida == 0 || salida == 2) &&
         preparar_transicion_escena(d, salida, referencia_estandar, referencia_estandar, 32, 48);
}
// La copia inicial acaba en el archivo; el resto de DS corresponde a memoria reservada a cero.
bool leer_datos_iniciales(const unsigned char *ejecutable, unsigned int longitud,
                          unsigned char *datos) {
  constexpr unsigned int inicio = 512u + 0x0f31u * 16u;
  if (longitud != 111606 || ejecutable[0] != 'M' || ejecutable[1] != 'Z' || ejecutable[8] != 32 ||
      ejecutable[9] != 0)
    return false;
  for (unsigned int i = 0; i < 65536; ++i)
    datos[i] = inicio + i < longitud ? ejecutable[inicio + i] : 0;
  return true;
}

// Reconstruye el registro reservado del personaje a partir del descriptor DS:859E.
void preparar_entrada_mazmorra(unsigned char *datos, unsigned short modelo, unsigned short ancho,
                               unsigned short alto) {
  guardar(datos, 0x3598, 0xffff);
  datos[0x2c0b] = 0;
  guardar(datos, 0x2c1a, 0);
  for (unsigned int i = 0; i < 6; ++i)
    datos[0x2c2b + i] = datos[0x859e + i];
  guardar(datos, 0x2c31, ancho);
  guardar(datos, 0x2c33, alto);
  guardar(datos, 0x2c35, 0);
  guardar(datos, 0x2c37, 0);
  guardar(datos, 0x2c39, 0);
  for (unsigned int canal = 0; canal < 3; ++canal) {
    const auto base = static_cast<Palabra>(0x2c3b + canal * 5);
    guardar(datos, base, palabra(datos, static_cast<Palabra>(0x85a4 + canal * 2)));
    datos[static_cast<Palabra>(base + 2)] = 0;
    datos[static_cast<Palabra>(base + 3)] = 1;
    datos[static_cast<Palabra>(base + 4)] = 2;
  }
  datos[0x3597] = datos[0x85aa];
  guardar(datos, 0x3591, static_cast<Palabra>(datos[0x3597] << 5));
  for (unsigned int i = 0; i < 10; ++i)
    guardar(datos, static_cast<Palabra>(0x35b3 + i * 2), palabra(datos, 0x2c2d));
  datos[0x358c] = datos[0x85ab];
  datos[0x9974] = datos[0x358c];
  guardar(datos, 0x2c4a, modelo);
  guardar(datos, 0x8082, palabra(datos, 0x7480));
  guardar(datos, 0x358d, 64);
  guardar(datos, 0x358f, 223);
  datos[0x3594] = datos[0x3595] = datos[0x3596] = datos[0x3593] = 0;
  guardar(datos, 0x359a, 0);
  guardar(datos, 0x35b0, 0);
  guardar(datos, 0x35ae, 0);
  guardar(datos, 0x0dfa, 0);
  datos[0x0def] = 0;
  // La modalidad cero del selector original consume las órdenes de sonido sin dispositivo.
  datos[0x0ad4] = 0;
}
} // namespace motor
