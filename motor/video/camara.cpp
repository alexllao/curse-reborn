#include "camara.hpp"
#include "../estado/segmento.hpp"

namespace motor {
// Sustituye el origen CRTC y conserva los 32 píxeles de la franja exterior que mantiene el VGA.
bool componer_camara(const unsigned char *anterior, const unsigned char *destino, int direccion,
                     unsigned int paso, unsigned char *salida) {
  if ((direccion != -1 && direccion != 1) || paso > 8)
    return false;
  const auto avance = paso * 4u;
  for (unsigned int y = 0; y < 200; ++y)
    for (unsigned int x = 0; x < 320; ++x) {
      const auto p = y * 320u;
      if (direccion > 0) {
        const auto origen = x + avance;
        salida[p + x] = origen < 32 ? anterior[p + origen] : destino[p + origen - 32];
      } else {
        const auto origen = x + 32u - avance;
        salida[p + x] = origen < 320 ? destino[p + origen] : anterior[p + origen - 32];
      }
    }
  return true;
}
// Los umbrales y el centro usan comparaciones de palabra con signo, como CMP/JG y CMP/JL.
int decidir_camara(const unsigned char *d) {
  const auto centro = static_cast<Palabra>(palabra(d, 0x2c2d) + (palabra(d, 0x2c31) >> 1));
  if (con_signo(centro) <= con_signo(palabra(d, 0x358d)))
    return d[0x3597] ? -1 : 0;
  if (con_signo(centro) < con_signo(palabra(d, 0x358f)))
    return 0;
  const auto restantes = static_cast<unsigned char>(d[0x0aec] - d[0x3597]);
  const int firmado = restantes < 128 ? restantes : static_cast<int>(restantes) - 256;
  return firmado > 10 ? 1 : 0;
}

// Conserva el ajuste de todos los registros, incluso los inactivos; los nodos 1A36/3C/42 son
// locales.
bool desplazar_camara(unsigned char *d, int direccion) {
  if ((direccion != -1 && direccion != 1) || palabra(d, 0x358a) > 10)
    return false;
  const auto restantes = static_cast<unsigned char>(d[0x0aec] - d[0x3597]);
  if ((direccion < 0 && !d[0x3597]) || (direccion > 0 && (restantes <= 10 || restantes >= 128)))
    return false;
  d[0x3597] = static_cast<unsigned char>(d[0x3597] + direccion);
  guardar(d, 0x991e, 0);
  const int delta = -direccion * 32;
  constexpr unsigned short campos[] = {0x35a0, 0x1a38, 0x1a3e, 0x1a44};
  for (const auto campo : campos)
    guardar(d, static_cast<Palabra>(campo),
            static_cast<Palabra>(palabra(d, static_cast<Palabra>(campo)) + delta));
  guardar(d, 0x3591, static_cast<Palabra>(palabra(d, 0x3591) - delta));
  for (unsigned int i = 0; i < 69; ++i) {
    const auto x = static_cast<Palabra>(0x2c2d + i * 33);
    guardar(d, x, static_cast<Palabra>(palabra(d, x) + delta));
  }
  for (unsigned int i = 0; i < palabra(d, 0x358a); ++i) {
    const auto x = static_cast<Palabra>(0x3510 + i * 12);
    guardar(d, x, static_cast<Palabra>(palabra(d, x) + delta));
  }
  // 6B46 mantiene el cursor en sus límites sin trasladar su destino con la cámara.
  for (unsigned int i = 0; i < 2; ++i) {
    auto valor = palabra(d, static_cast<Palabra>(0x2c0c + i * 2));
    const auto minimo = palabra(d, static_cast<Palabra>(0x9916 + i * 4));
    const auto maximo = palabra(d, static_cast<Palabra>(0x9918 + i * 4));
    if (con_signo(valor) < con_signo(minimo))
      valor = minimo;
    else if (con_signo(valor) > con_signo(maximo))
      valor = maximo;
    guardar(d, static_cast<Palabra>(0x9912 + i * 2), valor);
  }
  guardar(d, 0x0358, 0);
  guardar(d, 0x0616, 0);
  return true;
}

// 4E98–4EC8: dispone las escenas integradas; la plataforma resuelve nombres y recursos.
bool preparar_vista_escena(unsigned char *d) {
  const auto escena = palabra(d, 0x8082);
  if (escena > 31 && escena != 32 && escena != 33 && escena != 34 && escena != 35 && escena != 36 &&
      escena != 37 && escena != 38 && escena != 39 && escena != 40 && escena != 41 &&
      escena != 42 && escena != 43 && escena != 44 && escena != 45 && escena != 46 &&
      escena != 47 && escena != 48 && escena != 49)
    return false;
  const auto tabla = palabra(d, static_cast<Palabra>(0x757e + escena * 2));
  if (d[tabla] < 10 || static_cast<unsigned int>(tabla) + d[tabla] >= 65536u)
    return false;
  d[0x0aec] = d[tabla];
  guardar(d, 0x0aed, static_cast<Palabra>(tabla + 1));
  const auto exceso = static_cast<unsigned char>(d[tabla] - 10 - d[0x3597]);
  if (exceso >= 128) {
    const auto ajuste = static_cast<unsigned char>(0 - exceso);
    d[0x3597] = static_cast<unsigned char>(d[0x3597] - ajuste);
    guardar(d, 0x2c2d, static_cast<Palabra>(palabra(d, 0x2c2d) + ajuste * 32));
  }
  return true;
}
} // namespace motor
