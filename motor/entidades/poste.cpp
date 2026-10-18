#include "mazmorra.hpp"
#include "actualizar.hpp"
#include "../acciones/lista_identificadores.hpp"
#include "../estado/segmento.hpp"

namespace motor {
// C794–C8B0: el encuentro solo consume111/112 fuera del tramo central; las partículas son comunes.
bool actualizar_poste(unsigned char *d, EstadoAleatorio &azar, const RecursosEntidades &recursos) {
  if (!d[0x2c4c] && (contiene_identificador(d, 111) || contiene_identificador(d, 112))) {
    const auto global = static_cast<Palabra>(palabra(d, 0x2c2d) + palabra(d, 0x3591));
    if (global < 320 || global >= 600) {
      const bool derecha = global >= 600;
      guardar(d, 0x2c4e, static_cast<Palabra>((derecha ? 600 : 320) - palabra(d, 0x3591)));
      guardar(d, 0x4621, derecha ? 471 : 449);
      d[0x4620] = derecha ? 31 : 159;
      guardar(d, 0x2c5c, derecha ? 0xb1bc : 0xb227);
      guardar(d, 0x2c61, derecha ? 0xb1da : 0xb245);
      retirar_identificador(d, 111);
      retirar_identificador(d, 112);
      d[0x2c4c] = 3;
      guardar(d, 0x2c50, static_cast<Palabra>(palabra(d, 0x2c2f) + 30));
      guardar(d, 0x4623, static_cast<Palabra>(palabra(d, 0x2c2f) + 36));
      d[0x461f] = 1;
      for (unsigned int i = 0; i < 3; ++i) {
        const auto campo = static_cast<Palabra>(0x2c5c + i * 5);
        d[campo + 2] = 0;
        d[campo + 3] = d[campo + 4] = 1;
      }
      guardar(d, 0x2c66, 0xb201);
    }
  }
  Palabra indice = 0, valor = 0;
  if (!generar_aleatorio(azar, 3, indice))
    return false;
  const auto descriptor = static_cast<Palabra>(0xb26c + indice * 9);
  d[descriptor + 2] = static_cast<unsigned char>(indice * 3);
  if (!generar_aleatorio(azar, 3, valor))
    return false;
  d[descriptor + 2] = static_cast<unsigned char>(d[descriptor + 2] + valor + 51);
  if (!generar_aleatorio(azar, 280, valor))
    return false;
  guardar(d, descriptor + 3, static_cast<Palabra>(valor + 40 - palabra(d, 0x3591)));
  const auto limite = static_cast<Palabra>((indice + 1) * 15);
  if (!generar_aleatorio(azar, limite, valor))
    return false;
  guardar(d, descriptor + 5, static_cast<Palabra>(valor - limite));
  return crear_entidad(d, azar, recursos, descriptor).valido;
}
} // namespace motor
