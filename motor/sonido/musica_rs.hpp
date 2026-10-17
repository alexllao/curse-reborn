#pragma once

namespace motor {
struct ReferenciaMusicaRS {
  bool valida;
  unsigned char banco;
  unsigned char tempo;
  unsigned short nombre;
};
struct EventoMusicaRS {
  unsigned short tiempo;
  unsigned int mensaje;
  unsigned char bytes[4];
  unsigned char cantidad;
};
// 4DD5–4E16: tabla musical común a las 78 escenas; FFFF selecciona la presentación RS5.
ReferenciaMusicaRS seleccionar_musica_rs(const unsigned char *datos, unsigned short escena);
// 7E3B–7EA1: un paquete del recurso pedido por MPU-401; FF enlaza con otra posición.
bool leer_evento_musica_rs(const unsigned char *banco, unsigned int longitud,
                           unsigned short &posicion, unsigned char &estado, EventoMusicaRS &evento);
} // namespace motor
