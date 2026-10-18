#include "partidas.hpp"
namespace motor {
// E19F–E324: una máscara puede referenciar únicamente registros completos presentes en el archivo.
bool validar_partidas(const unsigned char *archivo, unsigned int longitud) {
  if (!archivo || !longitud || longitud > bytes_archivo_partidas)
    return false;
  for (unsigned int i = 0; i < ranuras_partida; ++i)
    if ((archivo[0] & (1u << i)) && longitud < 1 + (i + 1) * bytes_ranura_partida)
      return false;
  return true;
}
// E997: consulta una de las ocho descripciones sin interpretar sus bytes como instrucciones.
bool nombre_partida(const unsigned char *archivo, unsigned int longitud, unsigned int ranura,
                    char *nombre) {
  if (!nombre || ranura >= ranuras_partida || !validar_partidas(archivo, longitud) ||
      !(archivo[0] & (1u << ranura)))
    return false;
  const auto inicio = 1 + ranura * bytes_ranura_partida;
  for (unsigned int i = 0; i < caracteres_nombre_partida; ++i)
    nombre[i] = static_cast<char>(archivo[inicio + i]);
  nombre[caracteres_nombre_partida] = 0;
  return true;
}
// E257–E27A: el tamaño original está en DS:53B1; esta versión conserva sus 10.151 bytes.
bool leer_partida(const unsigned char *archivo, unsigned int longitud, unsigned int ranura,
                  unsigned char *datos) {
  if (!datos || ranura >= ranuras_partida || !validar_partidas(archivo, longitud) ||
      !(archivo[0] & (1u << ranura)))
    return false;
  const auto inicio = 1 + ranura * bytes_ranura_partida + caracteres_nombre_partida;
  for (unsigned int i = 0; i < bytes_estado_partida; ++i)
    datos[inicio_estado_partida + i] = archivo[inicio + i];
  return true;
}
// E3C4 y E718–E745: rellena con espacios el nombre y conserva los otros siete registros.
bool escribir_partida(unsigned char *archivo, unsigned int capacidad, unsigned int ranura,
                      const char *nombre, const unsigned char *datos) {
  if (!archivo || !nombre || !nombre[0] || !datos || capacidad != bytes_archivo_partidas ||
      ranura >= ranuras_partida || !validar_partidas(archivo, capacidad))
    return false;
  const auto inicio = 1 + ranura * bytes_ranura_partida;
  bool fin = false;
  for (unsigned int i = 0; i < caracteres_nombre_partida; ++i) {
    if (!fin && !nombre[i])
      fin = true;
    archivo[inicio + i] = fin ? ' ' : static_cast<unsigned char>(nombre[i]);
  }
  for (unsigned int i = 0; i < bytes_estado_partida; ++i)
    archivo[inicio + caracteres_nombre_partida + i] = datos[inicio_estado_partida + i];
  archivo[0] |= static_cast<unsigned char>(1u << ranura);
  return true;
}
// E831–E84D: la eliminación original modifica exclusivamente el byte de ocupación.
bool borrar_partida(unsigned char *archivo, unsigned int longitud, unsigned int ranura) {
  if (ranura >= ranuras_partida || !validar_partidas(archivo, longitud) ||
      !(archivo[0] & (1u << ranura)))
    return false;
  archivo[0] &= static_cast<unsigned char>((1u << ranura) ^ 0x7f);
  return true;
}
} // namespace motor
