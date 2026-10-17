#include "musica_rs.hpp"
#include "../estado/segmento.hpp"

namespace motor {
// Reconstruye 4DD5–4E36: DS:80CD selecciona uno de cinco descriptores DS:80B9 para 78 escenas.
ReferenciaMusicaRS seleccionar_musica_rs(const unsigned char *d, unsigned short escena) {
  if (!d || (escena != 0xffff && escena >= 78))
    return {false, 0, 0, 0};
  const auto indice = escena == 0xffff ? 4 : d[0x80cd + escena];
  if (indice > 4)
    return {false, 0, 0, 0};
  const auto descriptor = static_cast<Palabra>(0x80b9 + indice * 4);
  return {true, static_cast<unsigned char>(indice), d[descriptor], palabra(d, descriptor + 1)};
}

// Reconstruye 7E3B–7EA1: conserva los paquetes MPU, los saltos absolutos y el estado MIDI
// implícito.
bool leer_evento_musica_rs(const unsigned char *b, unsigned int longitud, unsigned short &posicion,
                           unsigned char &estado, EventoMusicaRS &e) {
  e = {};
  if (!b || longitud > 65536 || posicion >= longitud)
    return false;
  auto p = posicion;
  if (b[p] == 0xff) {
    if (p + 2u >= longitud)
      return false;
    p = static_cast<Palabra>(b[p + 1] | (b[p + 2] << 8));
    if (p >= longitud)
      return false;
  }
  // Acota las lecturas del paquete sin aceptar un recurso truncado.
  const auto tomar = [&](unsigned char &valor) {
    if (p >= longitud)
      return false;
    valor = b[p++];
    e.bytes[e.cantidad++] = valor;
    return true;
  };
  unsigned char tiempo = 0, primero = 0;
  if (!tomar(tiempo))
    return false;
  e.tiempo = tiempo < 0xf0 ? tiempo : 240;
  if (tiempo < 0xf0) {
    if (!tomar(primero))
      return false;
    if (primero < 0xf0) {
      auto nuevo_estado = estado;
      unsigned char dato = primero, segundo = 0;
      if (primero >= 0x80) {
        nuevo_estado = primero;
        if (!tomar(dato))
          return false;
      }
      if (nuevo_estado < 0x80 || nuevo_estado >= 0xf0)
        return false;
      if ((nuevo_estado < 0xc0 || nuevo_estado >= 0xe0) && !tomar(segundo))
        return false;
      estado = nuevo_estado;
      e.mensaje = nuevo_estado | (static_cast<unsigned int>(dato) << 8) |
                  (static_cast<unsigned int>(segundo) << 16);
    }
  }
  posicion = p;
  return true;
}
} // namespace motor
