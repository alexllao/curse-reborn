#pragma once

namespace motor {

enum class ContinuacionAccion : unsigned int {
  continuar = 0,
  finalizar_con_restauracion = 1,
  finalizar_sin_restauracion = 2,
};

// Decisión del llamador en 0000:DE2D–DE59. No ejecuta el dibujo ni la restauración.
ContinuacionAccion interpretar_retorno_accion(unsigned short ax, bool acarreo);

}
