#pragma once
#include <array>
#include <string>
#include <vector>

// Audio grabado propio: precarga fuera del callback y reproduce sin tocar el estado DOS.
class MusicaRemaster {
public:
  bool iniciar(const std::string &catalogo);
  bool disponible() const {
    return preparada;
  }
  void cambiar_banco(unsigned int siguiente, bool reiniciar = false);
  void generar(short *salida, unsigned int fotogramas);
  std::string titulo() const {
    return preparada ? pistas[banco].titulo : "";
  }
  size_t posicion() const {
    return cursor;
  }
  size_t fotogramas(unsigned int indice) const {
    return pistas[indice].muestras.size() / 2;
  }

private:
  struct Pista {
    std::string titulo;
    std::vector<short> muestras;
  };
  std::array<Pista, 5> pistas;
  bool preparada = false;
  unsigned int banco = 4;
  size_t cursor = 0;
};
