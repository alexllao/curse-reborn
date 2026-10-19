#include "mazmorra.hpp"
#include "actualizar.hpp"
#include "../acciones/lista_identificadores.hpp"
#include "../estado/segmento.hpp"

namespace motor {
// C5E0–C793: umbrales de DINO/CELESTE y canales originales de reacción de REMOTE.
bool actualizar_ramas_valle(unsigned char *d) {
  const auto escena = palabra(d, 0x8082);
  if (escena == 32) {
    // C5E0–C637: las comparaciones de posición son firmadas y conservan los umbrales ajenos.
    const auto global = static_cast<Palabra>(palabra(d, 0x2c2d) + palabra(d, 0x3591));
    const Palabra entidad = con_signo(global) <= 250 ? 0x2c4c : 0x2c8e;
    if (d[entidad] == 1) {
      const auto x = static_cast<Palabra>(palabra(d, entidad + 2) + palabra(d, 0x3591));
      const auto y = static_cast<Palabra>(palabra(d, entidad + 4) + palabra(d, entidad + 8));
      guardar(d, 0x18f5, x);
      guardar(d, 0x18f9, static_cast<Palabra>(x + palabra(d, entidad + 6)));
      guardar(d, 0x18fb, y);
      if (entidad == 0x2c8e)
        guardar(d, 0x18f7, static_cast<Palabra>(y - 15));
    }
    if (d[0x2c6d] != 1) {
      const int cantidad = d[0x3597] < 128 ? d[0x3597] : static_cast<int>(d[0x3597]) - 256;
      if (cantidad <= 8 && contiene_identificador(d, 121) && contiene_identificador(d, 119) &&
          contiene_identificador(d, 120) && contiene_identificador(d, 115)) {
        const Palabra guiones[] = {0xb181, 0xb18d, 0xb197};
        for (unsigned int i = 0; i < 3; ++i) {
          const auto campo = static_cast<Palabra>(0x2c7d + i * 5);
          guardar(d, campo, guiones[i]);
          d[campo + 2] = 0;
          d[campo + 3] = 1;
          d[campo + 4] = 2;
        }
      }
      return true;
    }
    const auto x = static_cast<Palabra>(palabra(d, 0x2c6f) + palabra(d, 0x3591));
    const auto y = static_cast<Palabra>(palabra(d, 0x2c71) + palabra(d, 0x2c75));
    guardar(d, 0x18f5, x);
    if (con_signo(static_cast<Palabra>(y - 5 - palabra(d, 0x2c2f))) <=
        con_signo(palabra(d, 0x2c33))) {
      guardar(d, 0x18f9, static_cast<Palabra>(x + (palabra(d, 0x2c73) >> 1) - 4));
      guardar(d, 0x18fb, static_cast<Palabra>(y - 1));
    } else {
      guardar(d, 0x18f9, static_cast<Palabra>(x + palabra(d, 0x2c73)));
      guardar(d, 0x18f7, static_cast<Palabra>(y - 10));
      guardar(d, 0x18fb, 199);
    }
    return true;
  }
  if (escena == 33) {
    if (d[0x2c4d] != 19) {
      guardar(d, 0x18fb, static_cast<Palabra>(palabra(d, 0x2c50) + palabra(d, 0x2c54)));
      const auto x = static_cast<Palabra>(palabra(d, 0x2c4e) + palabra(d, 0x3591));
      guardar(d, 0x18f5, x);
      guardar(d, 0x18f9, static_cast<Palabra>(x + palabra(d, 0x2c52)));
    }
    return true;
  }
  if (escena != 34)
    return false;
  if (palabra(d, 0x2c5c) || palabra(d, 0x2c2d) < 65 || palabra(d, 0x2c2d) > 200 ||
      palabra(d, 0x2c2f) < 110)
    return true;
  const Palabra secuencias[] = {0xb19c, 0xb1a8, 0xb1b2};
  for (unsigned int i = 0; i < 3; ++i) {
    const auto campo = static_cast<Palabra>(0x2c5c + i * 5);
    guardar(d, campo, secuencias[i]);
    d[campo + 2] = 0;
    d[campo + 3] = d[campo + 4] = 1;
  }
  return true;
}
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
// C8F9–CADE: deriva firmada de Brad y cuatro nubes; solo reciclar una nube consume azar.
bool actualizar_nube(unsigned char *d, EstadoAleatorio &azar) {
  const int deriva = con_signo(static_cast<Palabra>(palabra(d, 0x35c5) - 150)) / 10;
  guardar(d, 0x2c2d, static_cast<Palabra>(palabra(d, 0x2c2d) - deriva));
  const auto cuadro = d[0x2c2c];
  if (palabra(d, 0x2c3b) != 0x9808 && d[0x358c] == 0xff &&
      ((cuadro <= 8 && deriva >= 4) || (cuadro >= 0x80 && deriva <= -4))) {
    guardar(d, 0x359a, 0);
    guardar(d, 0x0dfa, 0);
    guardar(d, 0x2c3b, 0);
    guardar(d, 0x2c40, 0);
    guardar(d, 0x2c45, 0);
    d[0x2c2c] = cuadro <= 8 ? 0 : 0x80;
  }
  const int viento = con_signo(static_cast<Palabra>(palabra(d, 0x35b7) - 150)) / 10;
  for (unsigned int i = 0; i < 4; ++i) {
    const auto x = static_cast<Palabra>(0x2c6f + i * 33);
    guardar(d, x, static_cast<Palabra>(palabra(d, x) - viento));
  }
  for (unsigned int i = 0; i < 4; ++i) {
    const auto actor = static_cast<Palabra>(0x2c6d + i * 33);
    const int x = con_signo(palabra(d, actor + 2));
    if (x > 480)
      guardar(d, actor + 2, static_cast<Palabra>(x - 600));
    else if (x <= -160)
      guardar(d, actor + 2, static_cast<Palabra>(x + 600));
    else
      continue;
    Palabra valor = 0;
    if (!generar_aleatorio(azar, 3, valor))
      return false;
    d[actor + 1] = static_cast<unsigned char>(valor + 4);
    if (!generar_aleatorio(azar, 50, valor))
      return false;
    guardar(d, actor + 4, static_cast<Palabra>(valor + i * 50 - 5));
    if (!generar_aleatorio(azar, 2, valor))
      return false;
    guardar(d, actor + 21, static_cast<Palabra>(0xb2e0 + valor * 2));
    d[actor + 23] = 0;
    d[actor + 24] = d[actor + 25] = 1;
  }
  return true;
}
} // namespace motor
