#pragma once

namespace motor {

struct SalidaAdlib {
  void *contexto;
  void (*escribir)(void *, unsigned short registro, unsigned char valor);
};

// 8DB0–8DFD: dispone nueve canales desde las tablas de los dos temas internos del EXE.
bool iniciar_musica(unsigned char *codigo, unsigned short tema);
// 8FFC–900B: envía la tabla de inicialización 9749 al chip; no emula la calibración DOS.
bool inicializar_chip_adlib(const unsigned char *codigo, const SalidaAdlib &salida);
// 8E5F–8F4E: consume duración, patrones, repeticiones, instrumentos y notas de un canal.
bool avanzar_canal_musica(unsigned char *codigo, unsigned short canal, const SalidaAdlib &salida);
// 8DFE–8E5A: una IRQ del juego; respeta el divisor musical de 8E5B/8E5D.
bool avanzar_musica(unsigned char *codigo, const SalidaAdlib &salida);

} // namespace motor
