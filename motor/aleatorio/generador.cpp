#include "generador.hpp"

namespace motor {
namespace {

using Palabra = unsigned short;
static_assert(static_cast<Palabra>(-1) == 0xffff, "Se necesitan palabras de 16 bits.");

void rotar_con_acarreo(Palabra &valor, bool &acarreo) {
  const bool siguiente = (valor & 0x8000) != 0;
  valor = static_cast<Palabra>((valor << 1) | (acarreo ? 1 : 0));
  acarreo = siguiente;
}

} // namespace

bool generar_aleatorio(EstadoAleatorio &estado, unsigned short limite, unsigned short &resultado) {
  if (limite == 0) {
    return false;
  }
  auto tercera = estado.tercera;
  auto cuarta = estado.cuarta;
  bool acarreo = (cuarta & 0x8000) != 0;
  cuarta = static_cast<Palabra>(cuarta << 1); // SHL DX,1, antes de las seis RCL.
  for (int pareja = 0; pareja < 3; ++pareja) {
    rotar_con_acarreo(tercera, acarreo);
    rotar_con_acarreo(cuarta, acarreo);
  }
  // CLC / SUB CX,7 / JNC / DEC DX: resta con préstamo entre las dos palabras.
  const bool prestamo = tercera < 7;
  tercera = static_cast<Palabra>(tercera - 7);
  if (prestamo) {
    cuarta = static_cast<Palabra>(cuarta - 1);
  }
  const auto primera = static_cast<Palabra>(estado.primera ^ tercera);
  const auto segunda = static_cast<Palabra>(estado.segunda ^ cuarta);
  estado = {tercera, segunda, primera, cuarta};
  resultado = static_cast<Palabra>(segunda % limite);
  return true;
}

} // namespace motor
