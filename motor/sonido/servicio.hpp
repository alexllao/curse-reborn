#pragma once

namespace motor {

struct ServicioSonido {
  void *contexto = nullptr;
  bool (*reproducir)(void *, unsigned char *, unsigned short, unsigned char) = nullptr;
};

// 5288: delega la selección al dispositivo; sin servicio solo acepta la modalidad desactivada.
inline bool emitir_sonido(const ServicioSonido &servicio, unsigned char *datos,
                          unsigned short entidad, unsigned char identificador) {
  return servicio.reproducir ? servicio.reproducir(servicio.contexto, datos, entidad, identificador)
                             : datos[0x0ad4] == 0;
}

} // namespace motor
