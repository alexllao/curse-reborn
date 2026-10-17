#include "../motor/acciones/construir_tabla.hpp"

#ifdef _WIN32
#define EXPORTAR __declspec(dllexport)
#else
#define EXPORTAR __attribute__((visibility("default")))
#endif

// Interfaz de prueba para llamar al bloque C++ desde el comparador de memoria.
extern "C" EXPORTAR void reconstruir_tabla(unsigned char* datos) {
  motor::construir_tabla_acciones(datos);
}
