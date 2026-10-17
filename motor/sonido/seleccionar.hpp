#pragma once

namespace motor {

struct EstadoSonido {
  unsigned char prioridad_secuencia;
  unsigned char prioridad_muestra;
  unsigned char muestra_activa;
  unsigned short recurso_general;
  unsigned short recurso_escena;
  unsigned short referencia_datos;
};

struct LecturaSonido {
  void *contexto;
  bool (*palabra)(void *, unsigned short recurso, unsigned short posicion, unsigned short &valor);
};

// La orden describe la entrada a la rutina de dispositivo, sin ejecutarla.
struct OrdenSonido {
  bool valida;
  unsigned short rutina;
  unsigned short referencia;
  unsigned short posicion;
  unsigned short longitud;
  unsigned char canal;
};

// 5288–53B2, hasta RET o entrada a 7A46/7FB3/7D6A.
OrdenSonido seleccionar_sonido(unsigned char *datos, EstadoSonido &estado,
                               const LecturaSonido &lectura, unsigned short entidad,
                               unsigned char identificador);

} // namespace motor
