#pragma once

#include <cstdarg>
#include <cstdio>

// Mantiene las acciones de plataforma visibles en la consola, sin registrar cada fotograma.
inline void registrar_accion(const char *formato, ...) {
  std::printf("[partida] ");
  va_list argumentos;
  va_start(argumentos, formato);
  std::vprintf(formato, argumentos);
  va_end(argumentos);
  std::printf("\n");
  std::fflush(stdout);
}
