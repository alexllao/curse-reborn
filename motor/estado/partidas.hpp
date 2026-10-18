#pragma once
namespace motor {
inline constexpr unsigned int ranuras_partida = 8;
inline constexpr unsigned int caracteres_nombre_partida = 13;
inline constexpr unsigned int inicio_estado_partida = 0x2c0a;
inline constexpr unsigned int bytes_estado_partida = 10151;
inline constexpr unsigned int bytes_ranura_partida =
    caracteres_nombre_partida + bytes_estado_partida;
inline constexpr unsigned int bytes_archivo_partidas = 1 + ranuras_partida * bytes_ranura_partida;
// E19F/E325/E793: comprueba las ranuras activas, permitiendo archivos DOS sin ranuras finales.
bool validar_partidas(const unsigned char *archivo, unsigned int longitud);
// Lee un nombre DOS de trece bytes, terminado en cero para la plataforma; no modifica el archivo.
bool nombre_partida(const unsigned char *archivo, unsigned int longitud, unsigned int ranura,
                    char *nombre);
// E253–E27A: copia el bloque DOS a DS; los recursos y referencias se reconstruyen posteriormente.
bool leer_partida(const unsigned char *archivo, unsigned int longitud, unsigned int ranura,
                  unsigned char *datos);
// E718–E745: modifica solo la ranura elegida y la máscara; el destino admite ocho registros.
bool escribir_partida(unsigned char *archivo, unsigned int capacidad, unsigned int ranura,
                      const char *nombre, const unsigned char *datos);
// E824–E84D: borra el bit de ocupación, conservando bytes y ranuras restantes como DOS.
bool borrar_partida(unsigned char *archivo, unsigned int longitud, unsigned int ranura);
} // namespace motor
