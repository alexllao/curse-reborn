#pragma once
#include <string>
#include <vector>
#include "../../motor/video/introduccion.hpp"

class Introduccion {
  unsigned char planos_[262144]{};
  unsigned char temporal_[64000]{};
  unsigned char paleta_[768]{};
  unsigned char paletas_[3][768]{};
  std::vector<unsigned char> recursos_[16];
  unsigned int indice_ = 0;
  unsigned short pagina_ = 0;
  unsigned long long hasta_ = 0, inicio_fundido_ = 0;
  bool fundiendo_ = false, omitir_ = false, salida_anticipada_ = false, terminada_ = false;

public:
  // Prepara los archivos originales; los eventos y el dibujo permanecen en el núcleo portable.
  bool iniciar(const std::string &carpeta, motor::ImagenIndexada &imagen);
  // Procesa retrazos nominales desde la entrada; una pulsación se consume en los controles 2F8E.
  bool avanzar(unsigned long long retrazos, bool entrada, motor::ImagenIndexada &imagen);
  // Permite al bucle SDL entrar en la mazmorra cuando concluye el fundido de salida.
  bool terminada() const {
    return terminada_;
  }
};
