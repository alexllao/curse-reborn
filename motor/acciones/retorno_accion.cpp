#include "retorno_accion.hpp"

namespace motor {

ContinuacionAccion interpretar_retorno_accion(unsigned short ax, bool acarreo) {
  if (!acarreo) {
    return ContinuacionAccion::continuar;
  }
  return ax == 0x8000 ? ContinuacionAccion::finalizar_sin_restauracion
                      : ContinuacionAccion::finalizar_con_restauracion;
}

}
