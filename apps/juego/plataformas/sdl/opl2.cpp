#include "opl2.hpp"
#include <new>
#include <deque>
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#elif defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4100)
#endif
#include "../../terceros/ymfm/ymfm_opl.h"
#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#pragma warning(pop)
#endif

struct SintesisOpl2::Estado {
  ymfm::ymfm_interface interfaz;
  ymfm::ym3812 chip{interfaz};
  struct Orden {
    unsigned long long instante;
    unsigned char registro, valor;
  };
  std::deque<Orden> ordenes;
  unsigned long long reloj = 0, ultima_orden = 0;
  unsigned int fraccion = 0;
  int anterior = 0, actual = 0;
};

// Solo esta unidad necesita conocer la estructura del sintetizador externo.
SintesisOpl2::~SintesisOpl2() {
  delete estado;
}
// YM3812 divide su reloj de 3.579.545 Hz por 72; no se redondea esa frecuencia a un entero.
bool SintesisOpl2::reiniciar() {
  delete estado;
  estado = new (std::nothrow) Estado;
  if (!estado)
    return false;
  estado->chip.reset();
  estado->fraccion = 0;
  estado->anterior = estado->actual = 0;
  return true;
}
// Espacia escrituras dos muestras nativas: evita fusionar apagado y encendido de una nota; solo
// banco bajo.
void SintesisOpl2::escribir(unsigned short registro, unsigned char valor) {
  if (estado && registro < 256) {
    auto instante = estado->ultima_orden + 2;
    if (instante < estado->reloj)
      instante = estado->reloj;
    estado->ordenes.push_back({instante, static_cast<unsigned char>(registro), valor});
    estado->ultima_orden = instante;
  }
}
// Interpolación de frecuencia necesaria para SDL; conserva el DAC de mantisa/exponente de ymfm.
short SintesisOpl2::generar() {
  if (!estado)
    return 0;
  constexpr unsigned int periodo = 48000 * 72;
  estado->fraccion += 3579545;
  while (estado->fraccion >= periodo) {
    ymfm::ym3812::output_data salida;
    estado->chip.generate(&salida);
    while (!estado->ordenes.empty() && estado->ordenes.front().instante <= estado->reloj) {
      const auto orden = estado->ordenes.front();
      estado->chip.write_address(orden.registro);
      estado->chip.write_data(orden.valor);
      estado->ordenes.pop_front();
    }
    ++estado->reloj;
    estado->anterior = estado->actual;
    estado->actual = salida.data[0];
    estado->fraccion -= periodo;
  }
  const auto mezcla = static_cast<long long>(estado->anterior) * (periodo - estado->fraccion) +
                      static_cast<long long>(estado->actual) * estado->fraccion;
  return static_cast<short>(mezcla / periodo / 2);
}
