#pragma once
#include "../../../motor/estado/segmento.hpp"
#include <algorithm>

struct BalanceSonido {
  bool ubicado = false;
  float x = 160, izquierda = 1, derecha = 1;
};

// Solo interpreta registros de entidades de 33 bytes; las coordenadas ya son de pantalla.
inline BalanceSonido balance_sonido(const unsigned char *datos, unsigned short entidad,
                                    bool habilitado) {
  BalanceSonido b;
  if (!datos || entidad < 0x2c2b || entidad >= 0x3510 || (entidad - 0x2c2b) % 33)
    return b;
  const auto ancho = motor::con_signo(motor::palabra(datos, entidad + 6));
  const auto alto = motor::con_signo(motor::palabra(datos, entidad + 8));
  if (ancho <= 0 || alto <= 0)
    return b;
  b.ubicado = true;
  b.x = motor::con_signo(motor::palabra(datos, entidad + 2)) + ancho / 2.0f;
  const auto pan = habilitado ? std::clamp((b.x - 160) / 160, -1.0f, 1.0f) : 0.0f;
  // Centro idéntico al mono anterior, sin amplificar ni apagar completamente el otro oído.
  b.izquierda = 1 - std::max(pan, 0.0f) * .65f;
  b.derecha = 1 + std::min(pan, 0.0f) * .65f;
  return b;
}

class EstereoPosicional {
public:
  void configurar(bool activar) {
    habilitado = activar;
  }
  bool activo() const {
    return habilitado;
  }
  void iniciar(const unsigned char *datos, unsigned short origen) {
    entidad = origen;
    const auto b = balance_sonido(datos, entidad, habilitado);
    izquierda = b.izquierda;
    derecha = b.derecha;
  }
  void reiniciar() {
    entidad = 0;
    izquierda = derecha = 1;
  }
  void procesar(const unsigned char *datos, const short *mono, short *estereo,
                unsigned int cantidad) {
    const auto b = balance_sonido(datos, entidad, habilitado);
    // Cinco milisegundos de rampa evitan clics al moverse, desplazar cámara o pulsar P.
    for (unsigned int i = 0; i < cantidad; ++i) {
      izquierda += std::clamp(b.izquierda - izquierda, -1.0f / 240, 1.0f / 240);
      derecha += std::clamp(b.derecha - derecha, -1.0f / 240, 1.0f / 240);
      estereo[i * 2] = static_cast<short>(mono[i] * izquierda);
      estereo[i * 2 + 1] = static_cast<short>(mono[i] * derecha);
    }
  }

private:
  bool habilitado = true;
  unsigned short entidad = 0;
  float izquierda = 1, derecha = 1;
};
