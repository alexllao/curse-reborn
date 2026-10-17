#include "../motor/acciones/construir_tabla.hpp"
#include "../motor/acciones/seleccionar_coincidencias.hpp"
#include "../motor/acciones/retorno_accion.hpp"
#include "../motor/aleatorio/generador.hpp"
#include "../motor/acciones/preparar_resultados.hpp"
#include "../motor/acciones/lista_identificadores.hpp"
#include "../motor/acciones/iniciar_accion.hpp"
#include "../motor/movimiento/direccion.hpp"
#include "../motor/movimiento/planificar.hpp"
#include "../motor/movimiento/avanzar.hpp"

#ifdef _WIN32
#define EXPORTAR __declspec(dllexport)
#else
#define EXPORTAR __attribute__((visibility("default")))
#endif

// Interfaz de prueba para llamar al bloque C++ desde el comparador de memoria.
extern "C" EXPORTAR void reconstruir_tabla(unsigned char* datos) {
  motor::construir_tabla_acciones(datos);
}

extern "C" EXPORTAR int seleccionar_coincidencias(unsigned char* datos,
    unsigned char primero, unsigned char segundo, unsigned char tercero) {
  return motor::seleccionar_coincidencias(datos, primero, segundo, tercero) ? 1 : 0;
}

extern "C" EXPORTAR unsigned int interpretar_retorno(unsigned short ax, unsigned int acarreo) {
  return static_cast<unsigned int>(motor::interpretar_retorno_accion(ax, acarreo != 0));
}

extern "C" EXPORTAR int generar_valor(unsigned short* palabras, unsigned short limite,
                                     unsigned short* resultado) {
  motor::EstadoAleatorio estado{palabras[0], palabras[1], palabras[2], palabras[3]};
  if (!motor::generar_aleatorio(estado, limite, *resultado)) {
    return 0;
  }
  palabras[0] = estado.primera;
  palabras[1] = estado.segunda;
  palabras[2] = estado.tercera;
  palabras[3] = estado.cuarta;
  return 1;
}

extern "C" EXPORTAR void preparar_resultados(unsigned char* datos, unsigned short* palabras) {
  motor::EstadoAleatorio estado{palabras[0], palabras[1], palabras[2], palabras[3]};
  motor::preparar_resultados(datos, estado);
  palabras[0] = estado.primera;
  palabras[1] = estado.segunda;
  palabras[2] = estado.tercera;
  palabras[3] = estado.cuarta;
}

extern "C" EXPORTAR void reiniciar_accion(unsigned char* datos) {
  motor::reiniciar_estado_accion(datos);
}

extern "C" EXPORTAR void rechazar_accion(unsigned char* datos) {
  motor::marcar_accion_rechazada(datos);
}

extern "C" EXPORTAR int gestionar_lista(unsigned int operacion, unsigned char* datos, unsigned short valor) {
  if (operacion == 0) return motor::contiene_identificador(datos, valor) ? 1 : 0;
  if (operacion == 1) return motor::insertar_identificador(datos, valor) ? 1 : 0;
  return motor::retirar_identificador(datos, valor) ? 1 : 0;
}

extern "C" EXPORTAR int seleccionar_direccion(unsigned char* datos, unsigned short indice) {
  return motor::seleccionar_direccion(datos, indice) ? 1 : 0;
}

extern "C" EXPORTAR int planificar_movimiento(unsigned char* datos, unsigned short nodo, unsigned short* ax) {
  return motor::planificar_movimiento(datos, nodo, *ax) ? 1 : 0;
}

extern "C" EXPORTAR int iniciar_accion(unsigned char* datos, unsigned short* ax) {
  return motor::iniciar_accion(datos, *ax) ? 1 : 0;
}

extern "C" EXPORTAR int ejecutar_accion_resultados(unsigned char* datos, unsigned short* palabras,
                                                unsigned short* ax) {
  motor::EstadoAleatorio estado{palabras[0], palabras[1], palabras[2], palabras[3]};
  const bool valido = motor::ejecutar_accion_resultados(datos, estado, *ax);
  palabras[0] = estado.primera;
  palabras[1] = estado.segunda;
  palabras[2] = estado.tercera;
  palabras[3] = estado.cuarta;
  return valido ? 1 : 0;
}

extern "C" EXPORTAR int avanzar_movimiento(unsigned char* datos) {
  return motor::avanzar_movimiento(datos) ? 1 : 0;
}
