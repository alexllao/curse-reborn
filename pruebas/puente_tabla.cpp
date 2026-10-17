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
#include "../motor/entidades/actualizar.hpp"
#include "../motor/entidades/secuencias.hpp"
#include "../motor/sonido/seleccionar.hpp"
#include "../motor/sonido/altavoz.hpp"
#include "../motor/video/planos.hpp"
#include "../motor/recursos/rnc.hpp"

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

namespace {
bool dimensiones_entidad(void* contexto, unsigned short indice, unsigned short& ancho,
                         unsigned short& alto) {
  if (indice >= 128) return false;
  const auto* dimensiones = static_cast<const unsigned short*>(contexto);
  ancho = dimensiones[indice * 2]; alto = dimensiones[indice * 2 + 1];
  return true;
}

bool dimensiones_secuencia(void* contexto, unsigned short referencia, unsigned short indice,
                           unsigned short& ancho, unsigned short& alto) {
  if (referencia < 0x4000 || referencia > 0x4200 || (referencia & 0xff) != 0 || indice >= 128) return false;
  const auto* dimensiones = static_cast<const unsigned short*>(contexto);
  const unsigned int posicion = ((referencia - 0x4000) / 256 * 128 + indice) * 2;
  ancho = dimensiones[posicion]; alto = dimensiones[posicion + 1];
  return true;
}
}

// Recursos sintéticos de prueba: las cabeceras originales se preparan en 4000h.
extern "C" EXPORTAR int procesar_entidades(unsigned int operacion, unsigned char* datos,
    unsigned short* palabras, unsigned short* dimensiones, unsigned short posicion,
    unsigned short* salida) {
  motor::EstadoAleatorio estado{palabras[0], palabras[1], palabras[2], palabras[3]};
  const motor::RecursosEntidades recursos{dimensiones, dimensiones_entidad, 0x4000};
  bool valido = true;
  if (operacion == 0) {
    const auto creacion = motor::crear_entidad(datos, estado, recursos, posicion);
    valido = creacion.valido;
    salida[0] = creacion.creada ? 1 : 0; salida[1] = creacion.posicion;
  } else if (operacion == 1) {
    valido = motor::actualizar_entidad(datos, estado, recursos, posicion);
  } else if (operacion == 2) {
    valido = motor::actualizar_entidades(datos, estado, recursos);
  } else {
    motor::activar_transicion_entidad(datos);
  }
  palabras[0] = estado.primera; palabras[1] = estado.segunda;
  palabras[2] = estado.tercera; palabras[3] = estado.cuarta;
  return valido ? 1 : 0;
}

extern "C" EXPORTAR int procesar_secuencias(unsigned int operacion, unsigned char* datos,
    unsigned short* palabras, unsigned short* dimensiones, unsigned short posicion) {
  motor::EstadoAleatorio estado{palabras[0], palabras[1], palabras[2], palabras[3]};
  const motor::RecursosSecuencias recursos{dimensiones, dimensiones_secuencia, 0x4100, 0x4000, 0x4200};
  auto resultado = motor::EstadoSecuencias::correcto;
  if (operacion == 0) resultado = motor::avanzar_secuencias_entidad(datos, estado, recursos, posicion);
  else if (operacion == 1) motor::retirar_entidad_fuera_de_limites(datos, posicion);
  else resultado = motor::avanzar_secuencias(datos, estado, recursos);
  palabras[0] = estado.primera; palabras[1] = estado.segunda;
  palabras[2] = estado.tercera; palabras[3] = estado.cuarta;
  return static_cast<int>(resultado) + 1;
}

namespace {
struct ContextoOperaciones {
  unsigned char* modelo;
  const unsigned char* fuente;
  unsigned int longitud;
  unsigned short* bitacora;
};

unsigned short leer_palabra_prueba(const unsigned char* datos, unsigned short posicion) {
  return static_cast<unsigned short>(datos[posicion] | (datos[static_cast<unsigned short>(posicion + 1)] << 8));
}

bool leer_muestra(void*, unsigned short recurso, unsigned short posicion, unsigned short& valor) {
  if (recurso != 0x5000 && recurso != 0x5100) return false;
  // Tabla y cabeceras sintéticas que el comparador dispone también para el original.
  if (posicion < 512 && (posicion & 1) == 0) valor = static_cast<unsigned short>(0x3000 + posicion * 2);
  else if (posicion >= 0x3000 && posicion < 0x3400 && (posicion & 3) == 0) {
    valor = static_cast<unsigned short>(1000 + posicion - 0x3000 + (recurso == 0x5100 ? 7 : 0));
  } else return false;
  return true;
}

bool cargar_modelo_prueba(void* contexto, const unsigned char*, unsigned short nombre, unsigned short capacidad) {
  auto& prueba = *static_cast<ContextoOperaciones*>(contexto);
  ++prueba.bitacora[0]; prueba.bitacora[1] = nombre; prueba.bitacora[2] = capacidad;
  const auto cantidad = prueba.longitud < capacidad ? prueba.longitud : capacidad;
  for (unsigned int i = 0; i < cantidad; ++i) prueba.modelo[i] = prueba.fuente[i];
  return true;
}

bool dimensiones_modelo(void* contexto, unsigned short recurso, unsigned short imagen,
                       unsigned short& ancho, unsigned short& alto) {
  const auto& prueba = *static_cast<ContextoOperaciones*>(contexto);
  if (recurso != 0x4200 || imagen >= 128) return false;
  const auto parrafo = leer_palabra_prueba(prueba.modelo, static_cast<unsigned short>(imagen * 2));
  const unsigned int posicion = parrafo * 16u;
  if (posicion > 65532) return false;
  ancho = leer_palabra_prueba(prueba.modelo, static_cast<unsigned short>(posicion));
  alto = leer_palabra_prueba(prueba.modelo, static_cast<unsigned short>(posicion + 2));
  return true;
}

bool sonido_sin_dispositivo(void*, unsigned char* datos, unsigned short entidad, unsigned char id) {
  motor::EstadoSonido estado{0, 0, 0, 0, 0, 0x0f31};
  const auto orden = motor::seleccionar_sonido(datos, estado, {nullptr, nullptr}, entidad, id);
  return orden.valida && orden.rutina == 0;
}
}

extern "C" EXPORTAR int seleccionar_sonido(unsigned char* datos, unsigned short* palabras,
    unsigned short entidad, unsigned char identificador, unsigned short* salida) {
  motor::EstadoSonido estado{static_cast<unsigned char>(palabras[0]), static_cast<unsigned char>(palabras[1]),
    static_cast<unsigned char>(palabras[2]), palabras[3], palabras[4], palabras[5]};
  const auto orden = motor::seleccionar_sonido(datos, estado, {nullptr, leer_muestra}, entidad, identificador);
  palabras[0] = estado.prioridad_secuencia; palabras[1] = estado.prioridad_muestra;
  salida[0] = orden.rutina; salida[1] = orden.referencia; salida[2] = orden.posicion;
  salida[3] = orden.longitud; salida[4] = orden.canal;
  return orden.valida ? 1 : 0;
}

extern "C" EXPORTAR int ejecutar_operaciones_secuencia(unsigned char* datos, unsigned short* palabras,
    unsigned char* modelo, const unsigned char* fuente, unsigned int longitud, unsigned short* bitacora,
    unsigned short posicion) {
  ContextoOperaciones contexto{modelo, fuente, longitud, bitacora};
  const motor::RecursosSecuencias recursos{&contexto, dimensiones_modelo, 0x4100, 0x4000, 0x4200,
                                          sonido_sin_dispositivo, cargar_modelo_prueba};
  motor::EstadoAleatorio estado{palabras[0], palabras[1], palabras[2], palabras[3]};
  const auto resultado = motor::avanzar_secuencias_entidad(datos, estado, recursos, posicion);
  palabras[0] = estado.primera; palabras[1] = estado.segunda;
  palabras[2] = estado.tercera; palabras[3] = estado.cuarta;
  return static_cast<int>(resultado) + 1;
}

namespace {
void registrar_puerto(unsigned short* salida, unsigned short operacion, unsigned short puerto, unsigned char valor) {
  const auto indice = salida[0]++;
  if (indice < 10) {
    salida[1 + indice * 3] = operacion; salida[2 + indice * 3] = puerto; salida[3 + indice * 3] = valor;
  }
}
unsigned char leer_altavoz(void* contexto, unsigned short puerto) {
  auto* salida = static_cast<unsigned short*>(contexto);
  const auto valor = static_cast<unsigned char>(salida[31]);
  registrar_puerto(salida, 0, puerto, valor); return valor;
}
void escribir_altavoz(void* contexto, unsigned short puerto, unsigned char valor) {
  auto* salida = static_cast<unsigned short*>(contexto);
  registrar_puerto(salida, 1, puerto, valor);
  if (puerto == 0x61) salida[31] = valor;
}
bool sonido_altavoz(void* contexto, unsigned char* datos, unsigned short entidad, unsigned char id) {
  auto& estado = *static_cast<motor::EstadoSonido*>(contexto);
  return motor::atender_orden_altavoz(datos,
    motor::seleccionar_sonido(datos, estado, {nullptr, nullptr}, entidad, id));
}
}

extern "C" EXPORTAR int procesar_altavoz(unsigned int operacion, unsigned char* datos,
    unsigned short posicion, unsigned short* prioridad, unsigned short* puertos) {
  const motor::PuertosAltavoz servicio{puertos, leer_altavoz, escribir_altavoz};
  if (operacion == 0) { motor::iniciar_altavoz(datos, posicion); return 1; }
  if (operacion == 1) return motor::detener_altavoz(datos, servicio) ? 1 : 0;
  if (operacion == 2) return motor::avanzar_altavoz(datos, servicio) ? 1 : 0;
  motor::EstadoSonido estado{static_cast<unsigned char>(*prioridad), 0, 0, 0, 0, 0x0f31};
  bool valido = true;
  if (operacion == 3) valido = sonido_altavoz(&estado, datos, 0x2c4c, static_cast<unsigned char>(posicion));
  else {
    const motor::RecursosSecuencias recursos{&estado, nullptr, 0, 0, 0, sonido_altavoz, nullptr};
    motor::EstadoAleatorio aleatorio{0, 0, 0, 0};
    valido = motor::avanzar_secuencias_entidad(datos, aleatorio, recursos, 0x2c4c) == motor::EstadoSecuencias::correcto;
  }
  *prioridad = estado.prioridad_secuencia;
  return valido ? 1 : 0;
}

namespace {
struct ContextoVideo { unsigned int* traza; unsigned char control; };
unsigned char leer_video(void* contexto, unsigned short puerto) {
  auto& c = *static_cast<ContextoVideo*>(contexto);
  const auto indice = c.traza[0]++;
  c.traza[1 + indice * 4] = 0; c.traza[2 + indice * 4] = puerto;
  c.traza[3 + indice * 4] = c.control; c.traza[4 + indice * 4] = 1;
  return c.control;
}
void escribir_video(void* contexto, unsigned short puerto, unsigned short valor, unsigned char tamano) {
  auto& c = *static_cast<ContextoVideo*>(contexto);
  const auto indice = c.traza[0]++;
  c.traza[1 + indice * 4] = 1; c.traza[2 + indice * 4] = puerto;
  c.traza[3 + indice * 4] = valor; c.traza[4 + indice * 4] = tamano;
  if (puerto == 0x3cf) c.control = static_cast<unsigned char>(valor);
}
}

extern "C" EXPORTAR int procesar_video(unsigned int operacion, unsigned char* datos,
    unsigned char* planos, const unsigned short* parametros, unsigned int* traza) {
  ContextoVideo contexto{traza, static_cast<unsigned char>(parametros[6])};
  const motor::PuertosVideo puertos{&contexto, leer_video, escribir_video};
  const auto ancho = parametros[0], alto = parametros[1], paso = parametros[2];
  if (operacion == 0) return motor::subir_imagen(datos, planos, puertos, ancho, alto, paso, parametros[4]) ? 1 : 0;
  if (operacion == 1) return motor::recuperar_imagen(planos, datos, puertos, ancho, alto, paso, parametros[3]) ? 1 : 0;
  if (operacion == 2) return motor::copiar_planos(planos, puertos, ancho, alto, paso, parametros[5], parametros[3], parametros[4]) ? 1 : 0;
  return motor::enviar_paleta(datos, puertos, parametros[3], static_cast<unsigned char>(parametros[4]), ancho) ? 1 : 0;
}

extern "C" EXPORTAR unsigned int descomprimir_rnc1(const unsigned char* origen, unsigned int longitud,
    unsigned char* destino, unsigned int capacidad, unsigned int* escritos) {
  const auto resultado = motor::descomprimir_rnc1(origen, longitud, destino, capacidad);
  *escritos = resultado.escritos;
  return static_cast<unsigned int>(resultado.estado);
}
