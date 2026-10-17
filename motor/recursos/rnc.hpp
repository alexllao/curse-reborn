#pragma once

namespace motor {

enum class EstadoRnc { correcto, sin_compresion, metodo_no_admitido, entrada_invalida };
struct ResultadoRnc {
  EstadoRnc estado;
  unsigned int escritos;
};

// 64E0–67AC: método uno, tablas canónicas invertidas y copias solapadas hacia delante.
// Búferes separados y capacidad explícita; no necesita segmentos DOS ni asignaciones dinámicas.
ResultadoRnc descomprimir_rnc1(const unsigned char *origen, unsigned int longitud,
                               unsigned char *destino, unsigned int capacidad);

// 67BC–69BD: método dos, bits de mayor a menor peso y referencias a la salida anterior.
// Entrada y salida separadas; comprueba límites sin depender de segmentos DOS.
ResultadoRnc descomprimir_rnc2(const unsigned char *origen, unsigned int longitud,
                               unsigned char *destino, unsigned int capacidad);

} // namespace motor
