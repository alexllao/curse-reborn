#include "cementerio.hpp"
#include "iniciar_accion.hpp"
#include "lista_identificadores.hpp"
#include "../estado/segmento.hpp"

namespace motor {
// 563A/60CE/4502–4554: conserva capacidad, consumo ignorado, canales y puntuación envolvente.
ResultadoMenu ejecutar_interaccion_cementerio(unsigned char *d, unsigned short manejador,
                                              unsigned short id, const ServicioSonido &sonido,
                                              const ServicioCreacion &creacion) {
  const auto escena = palabra(d, 0x8082);
  if (escena != 64 && escena != 65 && escena != 77)
    return ResultadoMenu::pendiente;
  if (manejador == 0x32de && id <= 1)
    return emitir_sonido(sonido, d, 0x2c2b, static_cast<unsigned char>(id)) ? ResultadoMenu::accion
                                                                            : ResultadoMenu::error;
  // 60E1–6140/615C: seis útiles con baja persistente y recogida baja compartida.
  if (manejador == 0x563a &&
      (id == 167 || id == 168 || id == 169 || id == 170 || id == 171 || id == 173)) {
    const auto icono = d[0x524e + id * 2];
    if (icono == 31 || icono == 34)
      guardar(d, 0x35c7, static_cast<Palabra>(palabra(d, 0x35c7) + (icono == 31 ? 1 : 2)));
    else if (!insertar_identificador(d, id)) {
      marcar_accion_rechazada(d);
      return ResultadoMenu::accion;
    }
    const Palabra entidad = id == 170   ? 0x2d75
                            : id == 167 ? 0x2d96
                            : id == 168 ? 0x2db7
                            : id == 169 ? 0x2dd8
                            : id == 173 ? 0x2d54
                                        : 0x2d33;
    guardar(d, 0x1a38, static_cast<Palabra>(palabra(d, entidad + 2) + 5));
    guardar(d, 0x1a3a, static_cast<Palabra>(palabra(d, entidad + 4) + palabra(d, entidad + 8) - 2));
    const auto lista = palabra(d, static_cast<Palabra>(0x53b5 + escena * 2));
    d[static_cast<Palabra>(lista + (entidad - 0x2c4c) / 33 * 9 + 1)] = 0;
    guardar(d, entidad + 16, 0x1a2e);
    d[entidad + 18] = 0;
    d[entidad + 20] = 1;
    guardar(d, 0x0ad7, d[0x358c] == 20 ? 0x96f6 : 0x96c0);
    guardar(d, 0x0adc, d[0x358c] == 20 ? 0x96ff : 0x96c9);
    unsigned short pasos = 0;
    if (!iniciar_accion(d, pasos))
      return ResultadoMenu::error;
    d[entidad + 19] = static_cast<unsigned char>(pasos + 6);
    return ResultadoMenu::accion;
  }
  if (manejador == 0x563a && id == 165) {
    const auto icono = d[0x524e + id * 2];
    if (icono == 31 || icono == 34)
      guardar(d, 0x35c7, static_cast<Palabra>(palabra(d, 0x35c7) + (icono == 31 ? 1 : 2)));
    else if (!insertar_identificador(d, id)) {
      marcar_accion_rechazada(d);
      return ResultadoMenu::accion;
    }
    d[0x2c6d] = 0;
    guardar(d, 0x2c3b, 0x9858);
    unsigned short pasos = 0;
    return iniciar_accion(d, pasos) ? ResultadoMenu::accion : ResultadoMenu::error;
  }
  if (manejador == 0x2fca && id == 165) {
    retirar_identificador(d, 165);
    guardar(d, 0x2c3b, 0x2714);
    constexpr Palabra canales[] = {0x2742, 0x2752, 0x2758};
    for (unsigned int i = 0; i < 3; ++i) {
      const auto p = static_cast<Palabra>(0x2c5c + i * 5);
      guardar(d, p, canales[i]);
      d[p + 2] = 0;
      d[p + 3] = 9;
      d[p + 4] = 8;
    }
    guardar(d, 0x35c7, static_cast<Palabra>(palabra(d, 0x35c7) + 5));
    unsigned short pasos = 0;
    return iniciar_accion(d, pasos) ? ResultadoMenu::accion : ResultadoMenu::error;
  }
  // 4680–4837: ajo, cruz y platillos avanzan fases3/4/5; consumir no exige presencia previa.
  if ((manejador == 0x3b53 && id == 170 && palabra(d, 0xb638) == 0xffff &&
       palabra(d, 0xb634) == 3) ||
      (manejador == 0x2fca && (id == 173 || id == 169))) {
    const unsigned int fase = id == 170 ? 3 : id == 173 ? 4 : 5;
    if (((d[0x35ca] >> 3) & 7) != fase) {
      marcar_accion_rechazada(d);
      return ResultadoMenu::accion;
    }
    if (fase != 5 && !creacion.crear)
      return ResultadoMenu::pendiente;
    d[0x35ca] ^= fase == 3 ? 0x38 : fase == 4 ? 8 : 0x18;
    retirar_identificador(d, id);
    if (fase == 3)
      d[0x512d] = 3;
    guardar(d, 0x1a38,
            static_cast<Palabra>((fase == 3   ? 78
                                  : fase == 4 ? 167
                                              : 312) -
                                 palabra(d, 0x3591)));
    guardar(d, 0x1a3a, fase == 3 ? 177 : fase == 4 ? 165 : 143);
    constexpr Palabra pendientes[][3] = {
        {0x2904, 0x2920, 0x2938}, {0x29cd, 0x29d8, 0x29de}, {0x2a1c, 0x2a2b, 0x2a34}};
    constexpr Palabra canales[][3] = {
        {0x2882, 0x28ae, 0x28cf}, {0x2996, 0x299f, 0x29a1}, {0x2a3d, 0x2a4d, 0x2a59}};
    for (unsigned int i = 0; i < 3; ++i) {
      guardar(d, static_cast<Palabra>(0x0ad7 + i * 5), pendientes[fase - 3][i]);
      guardar(d, static_cast<Palabra>(0x00a0 + i * 2), canales[fase - 3][i]);
    }
    guardar(d, 0x009c, 0x2c4c);
    guardar(d, 0x009e, fase == 3 ? 0x0201 : fase == 4 ? 0x0205 : 0x0209);
    if (fase != 5) {
      guardar(d, 0x00a6, fase == 3 ? 0x2c8e : 0x2caf);
      guardar(d, 0x00a8, fase == 3 ? 0x0221 : 0x1203);
      guardar(d, 0x00aa, fase == 3 ? 0x28f0 : 0x1a32);
      if (fase == 3)
        guardar(d, 0x00ae, 0x28fb);
      for (unsigned int i = 0; i < (fase == 3 ? 5u : 7u); ++i) {
        const auto tabla = static_cast<Palabra>((fase == 3 ? 0x2950 : 0x29e4) + i * 8);
        guardar(d, 0x2761, static_cast<Palabra>(palabra(d, tabla) - palabra(d, 0x3591)));
        guardar(d, 0x2763, palabra(d, tabla + 2));
        Palabra entidad = 0;
        if (!creacion.crear(creacion.contexto, d, 0x275e, entidad))
          return ResultadoMenu::error;
        const auto aux = static_cast<Palabra>(0x00b0 + i * 10);
        guardar(d, aux, entidad);
        guardar(d, aux + 2, fase == 3 ? 0x0155 : 0x0113);
        guardar(d, aux + 4, 0x2779);
        guardar(d, aux + 6, palabra(d, tabla + 4));
        guardar(d, aux + 8, palabra(d, tabla + 6));
      }
    }
    guardar(d, 0x35c7, static_cast<Palabra>(palabra(d, 0x35c7) + 5));
    unsigned short pasos = 0;
    return iniciar_accion(d, pasos) ? ResultadoMenu::accion : ResultadoMenu::error;
  }
  // 4555–4604: la pala resuelve la fase1 y crea cuatro fragmentos conservando el agotamiento
  // normal. 4605–467F: usar172 en fase2 desplaza la piedra y habilita el siguiente vampiro.
  if (manejador == 0x3b53 && id == 172 && palabra(d, 0xb638) == 0xffff && palabra(d, 0xb634) == 2) {
    if (((d[0x35ca] >> 3) & 7) != 2) {
      marcar_accion_rechazada(d);
      return ResultadoMenu::accion;
    }
    d[0x35ca] ^= 8;
    d[0x512d] = 1;
    guardar(d, 0x1a38, static_cast<Palabra>(440 - palabra(d, 0x3591)));
    guardar(d, 0x1a3a, 159);
    guardar(d, 0x0ad7, 0x96de);
    guardar(d, 0x0adc, 0x96e7);
    d[0x4e9f] = 26;
    guardar(d, 0x4ea2, static_cast<Palabra>(palabra(d, 0x4ea2) + 16));
    guardar(d, 0x009c, 0x2c4c);
    guardar(d, 0x009e, 0x0201);
    guardar(d, 0x00a0, 0x281d);
    guardar(d, 0x00a2, 0x2846);
    guardar(d, 0x00a4, 0x2861);
    guardar(d, 0x00a6, 0x2cf1);
    guardar(d, 0x00a8, 0x0207);
    guardar(d, 0x00aa, 0x287c);
    guardar(d, 0x00ae, 0x287f);
    guardar(d, 0x35c7, static_cast<Palabra>(palabra(d, 0x35c7) + 5));
    unsigned short pasos = 0;
    return iniciar_accion(d, pasos) ? ResultadoMenu::accion : ResultadoMenu::error;
  }
  // 4555–4604: la pala resuelve la fase1 y crea cuatro fragmentos conservando el agotamiento
  // normal.
  if (manejador == 0x2fca && id == 168) {
    if (((d[0x35ca] >> 3) & 7) != 1) {
      marcar_accion_rechazada(d);
      return ResultadoMenu::accion;
    }
    if (!creacion.crear)
      return ResultadoMenu::pendiente;
    d[0x35ca] ^= 0x18;
    retirar_identificador(d, 168);
    guardar(d, 0x1a38, static_cast<Palabra>(312 - palabra(d, 0x3591)));
    guardar(d, 0x1a3a, 178);
    guardar(d, 0x0ad7, 0x27b9);
    guardar(d, 0x0adc, 0x27cb);
    guardar(d, 0x0ae1, 0x27d8);
    guardar(d, 0x009c, 0x2c4c);
    guardar(d, 0x009e, 0x0201);
    guardar(d, 0x00a0, 0x277d);
    guardar(d, 0x00a2, 0x2792);
    guardar(d, 0x00a4, 0x279b);
    guardar(d, 0x00a6, 0x2cd0);
    guardar(d, 0x00a8, 0x020d);
    guardar(d, 0x00aa, 0x27a4);
    guardar(d, 0x00ac, 0x27ab);
    guardar(d, 0x00ae, 0x27b2);
    for (unsigned int i = 0; i < 4; ++i) {
      const auto tabla = static_cast<Palabra>(0x27e5 + i * 8);
      guardar(d, 0x2761, static_cast<Palabra>(palabra(d, tabla) - palabra(d, 0x3591)));
      guardar(d, 0x2763, palabra(d, tabla + 2));
      Palabra entidad = 0;
      if (!creacion.crear(creacion.contexto, d, 0x275e, entidad))
        return ResultadoMenu::error;
      const auto aux = static_cast<Palabra>(0x00b0 + i * 10);
      guardar(d, aux, entidad);
      guardar(d, aux + 2, 0x012f);
      guardar(d, aux + 4, 0x2779);
      guardar(d, aux + 6, palabra(d, tabla + 4));
      guardar(d, aux + 8, palabra(d, tabla + 6));
    }
    guardar(d, 0x35c7, static_cast<Palabra>(palabra(d, 0x35c7) + 5));
    unsigned short pasos = 0;
    return iniciar_accion(d, pasos) ? ResultadoMenu::accion : ResultadoMenu::error;
  }
  if (id != 76 && id != 124 && (id < 164 || id > 174))
    return ResultadoMenu::pendiente;
  if (manejador == 0x384a)
    return ResultadoMenu::actualizado;
  if (manejador == 0x3b53 || manejador == 0x2fca || manejador == 0x3143) {
    marcar_accion_rechazada(d);
    return ResultadoMenu::accion;
  }
  return ResultadoMenu::pendiente;
}
} // namespace motor
