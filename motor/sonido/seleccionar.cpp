#include "seleccionar.hpp"
#include "../estado/segmento.hpp"

namespace motor {

OrdenSonido seleccionar_sonido(unsigned char *datos, EstadoSonido &estado,
                               const LecturaSonido &lectura, unsigned short entidad,
                               unsigned char identificador) {
  const auto campo = [&](int desplazamiento) {
    return palabra(datos, static_cast<Palabra>(entidad + desplazamiento));
  };
  const auto x = campo(2), y = campo(4);
  if (con_signo(x) > 319 || con_signo(x) + con_signo(campo(6)) <= 0 || con_signo(y) > 199 ||
      con_signo(y) + con_signo(campo(8)) <= 0)
    return {true, 0, 0, 0, 0, 0};
  const auto modalidad = datos[0x0ad4];
  if (modalidad == 0 || modalidad == 2)
    return {true, 0, 0, 0, 0, 0};
  if (modalidad > 4)
    return {false, 0, 0, 0, 0, 0};
  if (identificador == 0xff) {
    if (modalidad == 4)
      return {true, 0x7d6a, estado.referencia_datos, 0x8c90, 0, 0};
    return {true, 0, 0, 0, 0, 0};
  }
  Palabra tabla = modalidad == 1 ? 0x8e14 : modalidad == 3 ? 0x8fd3 : 0x92ef;
  auto indice = identificador;
  auto recurso = estado.recurso_general;
  if (indice >= 15) {
    const auto grupo = datos[static_cast<Palabra>(0x9048 + datos[0x8082])];
    const Palabra tablas = modalidad == 1 ? 0x8e41 : modalidad == 3 ? 0x901e : 0x92fe;
    tabla = palabra(datos, static_cast<Palabra>(tablas + grupo * 2));
    indice = static_cast<unsigned char>(indice - 15);
    recurso = estado.recurso_escena;
    if (modalidad == 3 && recurso == 0)
      return {true, 0, 0, 0, 0, 0};
  }
  const int tamano = modalidad == 1 ? 3 : modalidad == 3 ? 5 : 1;
  const auto entrada = static_cast<Palabra>(tabla + indice * tamano);
  const auto prioridad = datos[entrada];
  if (modalidad == 1) {
    const auto secuencia = palabra(datos, static_cast<Palabra>(entrada + 1));
    if (secuencia == 0 || (datos[0x9924] != 0 && prioridad <= estado.prioridad_secuencia))
      return {true, 0, 0, 0, 0, 0};
    estado.prioridad_secuencia = prioridad;
    return {true, 0x7a46, estado.referencia_datos, secuencia, 0, 0};
  }
  if (modalidad == 4) {
    if (prioridad == 0xff)
      return {true, 0, 0, 0, 0, 0};
    datos[0x92ec] = prioridad;
    return {true, 0x7d6a, estado.referencia_datos, 0x92e9, 0, 0};
  }
  if (estado.muestra_activa != 0 && prioridad <= estado.prioridad_muestra)
    return {true, 0, 0, 0, 0, 0};
  estado.prioridad_muestra = prioridad;
  const auto canal = datos[static_cast<Palabra>(entrada + 1)];
  const auto salto = palabra(datos, static_cast<Palabra>(entrada + 2));
  const auto casilla = static_cast<Palabra>(datos[static_cast<Palabra>(entrada + 4)] * 2);
  Palabra muestra = 0, longitud = 0;
  if (!lectura.palabra || !lectura.palabra(lectura.contexto, recurso, casilla, muestra) ||
      !lectura.palabra(lectura.contexto, recurso, muestra, longitud))
    return {false, 0, 0, 0, 0, 0};
  return {true,
          0x7fb3,
          recurso,
          static_cast<Palabra>(muestra + 2),
          static_cast<Palabra>(longitud - salto),
          canal};
}

} // namespace motor
