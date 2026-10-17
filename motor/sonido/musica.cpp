#include "musica.hpp"
#include "../estado/segmento.hpp"

namespace motor {
namespace {
// 902E–90BA: conserva el orden de los trece registros, incluidos los dos operadores FM.
void tocar_nota(const unsigned char *codigo, Palabra canal, Palabra instrumento, Palabra frecuencia,
                const SalidaAdlib &salida) {
  const auto operador = codigo[static_cast<Palabra>(0x90bb + canal)];
  constexpr unsigned char bases[] = {0x20, 0x23, 0x40, 0x43, 0x60, 0x80, 0x63, 0x83, 0xe0, 0xe3};
  constexpr unsigned char campos[] = {4, 9, 5, 10, 6, 7, 11, 12, 8, 13};
  for (unsigned int i = 0; i < 10; ++i)
    salida.escribir(salida.contexto, static_cast<unsigned char>(bases[i] + operador),
                    codigo[static_cast<Palabra>(instrumento + campos[i])]);
  salida.escribir(salida.contexto, static_cast<Palabra>(0xc0 + canal),
                  codigo[static_cast<Palabra>(instrumento + 14)]);
  salida.escribir(salida.contexto, static_cast<Palabra>(0xa0 + canal),
                  static_cast<unsigned char>(frecuencia));
  salida.escribir(salida.contexto, static_cast<Palabra>(0xb0 + canal),
                  static_cast<unsigned char>((frecuencia >> 8) | 0x20));
}
} // namespace

// Los bytes 10h de los canales y el reloj global se conservan, como en 8DB0.
bool iniciar_musica(unsigned char *codigo, unsigned short tema) {
  if (!codigo || tema > 1)
    return false;
  auto tabla = palabra(codigo, static_cast<Palabra>(0x8dac + tema * 2));
  for (Palabra i = 0; i < 9; ++i) {
    const auto canal = static_cast<Palabra>(0x8ce2 + i * 17);
    codigo[canal] = static_cast<unsigned char>(i);
    guardar(codigo, canal + 1, 0);
    guardar(codigo, canal + 3, 0);
    guardar(codigo, canal + 5, 0x8dab);
    guardar(codigo, canal + 7, 0x8dab);
    guardar(codigo, canal + 9, 0);
    guardar(codigo, canal + 11, palabra(codigo, tabla));
    guardar(codigo, canal + 13, 0);
    codigo[canal + 15] = 0;
    tabla = static_cast<Palabra>(tabla + 2);
  }
  return true;
}

// La tabla termina en FFFF; el límite evita recorrer indefinidamente datos ajenos al contrato.
bool inicializar_chip_adlib(const unsigned char *codigo, const SalidaAdlib &salida) {
  if (!codigo || !salida.escribir)
    return false;
  Palabra posicion = 0x9749;
  for (unsigned int i = 0; i < 32768; ++i) {
    const auto orden = palabra(codigo, posicion);
    if (orden == 0xffff)
      return true;
    salida.escribir(salida.contexto, static_cast<unsigned char>(orden),
                    static_cast<unsigned char>(orden >> 8));
    posicion = static_cast<Palabra>(posicion + 2);
  }
  return false;
}

// Las operaciones sobre bytes, los signos y las envolturas de palabra siguen las instrucciones.
bool avanzar_canal_musica(unsigned char *c, unsigned short canal, const SalidaAdlib &salida) {
  if (!c || !salida.escribir || canal < 0x8ce2 || canal > 0x8d6a || (canal - 0x8ce2) % 17)
    return false;
  if (palabra(c, canal + 11) == 0xffff)
    return true;
  const auto contador = static_cast<Palabra>(palabra(c, canal + 1) + 1);
  if (contador < palabra(c, canal + 3)) {
    guardar(c, canal + 1, contador);
    return true;
  }
  guardar(c, canal + 1, 0);
  auto posicion = palabra(c, canal + 5);
  for (unsigned int pasos = 0; pasos < 4096; ++pasos) {
    const auto orden = c[posicion];
    if (orden < 0x80) {
      guardar(c, canal + 5, static_cast<Palabra>(posicion + 1));
      salida.escribir(salida.contexto, static_cast<Palabra>(0xb0 + c[canal]),
                      static_cast<unsigned char>(c[canal + 16] & 0xdf));
      if (!orden)
        return true;
      const auto nota = static_cast<unsigned char>(orden + c[canal + 15]);
      const auto frecuencia = palabra(c, static_cast<Palabra>(0x97d1 + nota * 2));
      c[canal + 16] = static_cast<unsigned char>(frecuencia >> 8);
      tocar_nota(c, c[canal], palabra(c, canal + 9), frecuencia, salida);
      return true;
    }
    if (orden == 0xff) {
      --c[canal + 13];
      if (static_cast<signed char>(c[canal + 13]) >= 0) {
        posicion = palabra(c, canal + 7);
        guardar(c, canal + 5, posicion);
        continue;
      }
      c[canal + 13] = 0;
      auto lista = palabra(c, canal + 11);
      bool encontrado = false;
      for (unsigned int saltos = 0; saltos < 4096; ++saltos) {
        const auto patron = palabra(c, lista);
        if (patron == 0xfffe) {
          guardar(c, canal + 13, palabra(c, static_cast<Palabra>(lista + 2)));
          lista = static_cast<Palabra>(lista + 4);
        } else if (patron == 0xffff) {
          guardar(c, canal + 11, 0xffff);
          return true;
        } else if (patron == 0xfffd)
          lista = palabra(c, static_cast<Palabra>(lista + 2));
        else {
          guardar(c, canal + 11, static_cast<Palabra>(lista + 2));
          guardar(c, canal + 7, patron);
          guardar(c, canal + 5, patron);
          guardar(c, canal + 1, 0);
          posicion = patron;
          encontrado = true;
          break;
        }
      }
      if (!encontrado)
        return false;
    } else if (orden == 0xfe) {
      const auto instrumento = c[static_cast<Palabra>(posicion + 1)];
      guardar(c, canal + 9,
              static_cast<Palabra>(palabra(c, static_cast<Palabra>(0x8d7b + instrumento * 2)) +
                                   0x958f));
      posicion = static_cast<Palabra>(posicion + 2);
    } else if (orden == 0xfd) {
      c[canal + 15] = c[static_cast<Palabra>(posicion + 1)];
      posicion = static_cast<Palabra>(posicion + 2);
    } else {
      c[canal + 3] = static_cast<unsigned char>(orden & 0x7f);
      posicion = static_cast<Palabra>(posicion + 1);
    }
  }
  return false;
}

// El contador se decrementa como palabra; cero inicial tarda FFFF pasos antes del siguiente pulso.
bool avanzar_musica(unsigned char *codigo, const SalidaAdlib &salida) {
  if (!codigo || !salida.escribir)
    return false;
  const auto contador = static_cast<Palabra>(palabra(codigo, 0x8e5b) - 1);
  guardar(codigo, 0x8e5b, contador);
  if (contador)
    return true;
  guardar(codigo, 0x8e5b, palabra(codigo, 0x8e5d));
  for (Palabra i = 0; i < 9; ++i)
    if (!avanzar_canal_musica(codigo, static_cast<Palabra>(0x8ce2 + i * 17), salida))
      return false;
  return true;
}

} // namespace motor
