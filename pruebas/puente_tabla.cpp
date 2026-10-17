#include "../motor/sonido/digital.hpp"
#include "../motor/estado/segmento.hpp"
#include "../motor/video/camara.hpp"
#include "../motor/movimiento/geometria.hpp"
#include "../motor/estado/paso_escena.hpp"
#include "../motor/acciones/menu.hpp"
#include "../motor/estado/partidas.hpp"
#include "../motor/video/menu.hpp"
#include "../motor/video/introduccion.hpp"
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
#include "../motor/sonido/pcm.hpp"
#include "../motor/sonido/musica.hpp"
#include "../motor/sonido/musica_rs.hpp"
#include "../motor/video/planos.hpp"
#include "../motor/recursos/rnc.hpp"
#include "../motor/recursos/escena.hpp"
#include "../motor/video/sprites.hpp"
#include "../motor/video/fundido.hpp"
#include "../motor/video/orden.hpp"
#include "../motor/video/entidades.hpp"
#include "../motor/entidades/mazmorra.hpp"
#include "../motor/acciones/objetos_cercanos.hpp"
#include "../motor/estado/inicializar.hpp"
#include "../motor/recursos/banco.hpp"

#ifdef _WIN32
#define EXPORTAR __declspec(dllexport)
#else
#define EXPORTAR __attribute__((visibility("default")))
#endif

namespace {
motor::AltavozPcm altavoz_pcm;
struct TrazaMusica {
  unsigned short *salida;
  unsigned int cantidad;
};
// Registra las parejas OPL conservando el orden; la primera palabra cuenta las escrituras.
void escribir_musica(void *contexto, unsigned short registro, unsigned char valor) {
  auto &traza = *static_cast<TrazaMusica *>(contexto);
  if (traza.cantidad >= 1024)
    return;
  traza.salida[1 + traza.cantidad * 2] = registro;
  traza.salida[2 + traza.cantidad * 2] = valor;
  traza.salida[0] = static_cast<unsigned short>(++traza.cantidad);
}
// Permite contrastar las respuestas de menú con el mismo selector que la salida PCM.
bool sonido_pcm(void *, unsigned char *datos, unsigned short entidad, unsigned char identificador) {
  return altavoz_pcm.reproducir(datos, entidad, identificador);
}
} // namespace

// Contrasta paquetes RS con el alimentador MPU original y conserva el puntero de DS:9926.
extern "C" EXPORTAR int procesar_musica_rs(unsigned char *datos, const unsigned char *banco,
                                           unsigned int longitud, unsigned char *estado,
                                           unsigned short *salida) {
  auto posicion = motor::palabra(datos, 0x9926);
  motor::EventoMusicaRS evento{};
  if (!motor::leer_evento_musica_rs(banco, longitud, posicion, *estado, evento))
    return 0;
  datos[0x994a] = 1;
  motor::guardar(datos, 0x9926, posicion);
  salida[0] = evento.cantidad;
  for (unsigned int i = 0; i < evento.cantidad; ++i)
    salida[i + 1] = evento.bytes[i];
  return 1;
}

// Expone la selección común sin introducir una tabla paralela de pistas en la plataforma.
extern "C" EXPORTAR int seleccionar_musica_rs(const unsigned char *datos, unsigned short escena,
                                              unsigned short *salida) {
  const auto referencia = motor::seleccionar_musica_rs(datos, escena);
  salida[0] = referencia.banco;
  salida[1] = referencia.tempo;
  salida[2] = referencia.nombre;
  return referencia.valida;
}

// Expone estado CS y registros AdLib, sin incluir el sintetizador externo en la comparación DOS.
extern "C" EXPORTAR int procesar_musica(unsigned int operacion, unsigned char *codigo,
                                        unsigned short parametro, unsigned short *registros) {
  registros[0] = 0;
  TrazaMusica traza{registros, 0};
  const motor::SalidaAdlib salida{&traza, escribir_musica};
  if (operacion == 0)
    return motor::iniciar_musica(codigo, parametro);
  if (operacion == 1)
    return motor::avanzar_musica(codigo, salida);
  if (operacion == 2)
    return motor::avanzar_canal_musica(codigo, parametro, salida);
  if (operacion == 3)
    return motor::inicializar_chip_adlib(codigo, salida);
  return 0;
}

// Expone el reloj PCM por bloques y escrituras de puerto para pruebas contra IRQ originales.
extern "C" EXPORTAR int procesar_pcm(unsigned int operacion, unsigned char *datos,
                                     unsigned short primero, unsigned short segundo, short *salida,
                                     unsigned int cantidad) {
  if (operacion == 0) {
    altavoz_pcm.reiniciar();
    return 1;
  }
  if (operacion == 1)
    return altavoz_pcm.reproducir(datos, primero, static_cast<unsigned char>(segundo));
  if (operacion == 2)
    return altavoz_pcm.generar(datos, salida, cantidad);
  if (operacion == 3) {
    altavoz_pcm.escribir(primero, static_cast<unsigned char>(segundo));
    return 1;
  }
  if (operacion == 4)
    return altavoz_pcm.detener(datos);
  if (operacion == 5)
    return static_cast<int>(motor::ejecutar_interaccion_inicial(datos, primero, segundo,
                                                                {nullptr, sonido_pcm})) ==
           static_cast<int>(motor::ResultadoMenu::accion);
  return 0;
}

// Interfaz de prueba para llamar al bloque C++ desde el comparador de memoria.
extern "C" EXPORTAR void reconstruir_tabla(unsigned char *datos) {
  motor::construir_tabla_acciones(datos);
}

extern "C" EXPORTAR int seleccionar_coincidencias(unsigned char *datos, unsigned char primero,
                                                  unsigned char segundo, unsigned char tercero) {
  return motor::seleccionar_coincidencias(datos, primero, segundo, tercero) ? 1 : 0;
}

extern "C" EXPORTAR unsigned int interpretar_retorno(unsigned short ax, unsigned int acarreo) {
  return static_cast<unsigned int>(motor::interpretar_retorno_accion(ax, acarreo != 0));
}

extern "C" EXPORTAR int generar_valor(unsigned short *palabras, unsigned short limite,
                                      unsigned short *resultado) {
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

extern "C" EXPORTAR void preparar_resultados(unsigned char *datos, unsigned short *palabras) {
  motor::EstadoAleatorio estado{palabras[0], palabras[1], palabras[2], palabras[3]};
  motor::preparar_resultados(datos, estado);
  palabras[0] = estado.primera;
  palabras[1] = estado.segunda;
  palabras[2] = estado.tercera;
  palabras[3] = estado.cuarta;
}

extern "C" EXPORTAR void reiniciar_accion(unsigned char *datos) {
  motor::reiniciar_estado_accion(datos);
}

extern "C" EXPORTAR void rechazar_accion(unsigned char *datos) {
  motor::marcar_accion_rechazada(datos);
}

extern "C" EXPORTAR int gestionar_lista(unsigned int operacion, unsigned char *datos,
                                        unsigned short valor) {
  if (operacion == 0)
    return motor::contiene_identificador(datos, valor) ? 1 : 0;
  if (operacion == 1)
    return motor::insertar_identificador(datos, valor) ? 1 : 0;
  return motor::retirar_identificador(datos, valor) ? 1 : 0;
}

extern "C" EXPORTAR int seleccionar_direccion(unsigned char *datos, unsigned short indice) {
  return motor::seleccionar_direccion(datos, indice) ? 1 : 0;
}

extern "C" EXPORTAR int planificar_movimiento(unsigned char *datos, unsigned short nodo,
                                              unsigned short *ax) {
  return motor::planificar_movimiento(datos, nodo, *ax) ? 1 : 0;
}

extern "C" EXPORTAR int iniciar_accion(unsigned char *datos, unsigned short *ax) {
  return motor::iniciar_accion(datos, *ax) ? 1 : 0;
}

extern "C" EXPORTAR int ejecutar_accion_resultados(unsigned char *datos, unsigned short *palabras,
                                                   unsigned short *ax) {
  motor::EstadoAleatorio estado{palabras[0], palabras[1], palabras[2], palabras[3]};
  const bool valido = motor::ejecutar_accion_resultados(datos, estado, *ax);
  palabras[0] = estado.primera;
  palabras[1] = estado.segunda;
  palabras[2] = estado.tercera;
  palabras[3] = estado.cuarta;
  return valido ? 1 : 0;
}

extern "C" EXPORTAR int avanzar_movimiento(unsigned char *datos) {
  return motor::avanzar_movimiento(datos) ? 1 : 0;
}

namespace {
bool dimensiones_entidad(void *contexto, unsigned short indice, unsigned short &ancho,
                         unsigned short &alto) {
  if (indice >= 128)
    return false;
  const auto *dimensiones = static_cast<const unsigned short *>(contexto);
  ancho = dimensiones[indice * 2];
  alto = dimensiones[indice * 2 + 1];
  return true;
}

bool dimensiones_secuencia(void *contexto, unsigned short referencia, unsigned short indice,
                           unsigned short &ancho, unsigned short &alto) {
  if (referencia < 0x4000 || referencia > 0x4200 || (referencia & 0xff) != 0 || indice >= 128)
    return false;
  const auto *dimensiones = static_cast<const unsigned short *>(contexto);
  const unsigned int posicion = ((referencia - 0x4000) / 256 * 128 + indice) * 2;
  ancho = dimensiones[posicion];
  alto = dimensiones[posicion + 1];
  return true;
}
} // namespace

// Recursos sintéticos de prueba: las cabeceras originales se preparan en 4000h.
extern "C" EXPORTAR int procesar_entidades(unsigned int operacion, unsigned char *datos,
                                           unsigned short *palabras, unsigned short *dimensiones,
                                           unsigned short posicion, unsigned short *salida) {
  motor::EstadoAleatorio estado{palabras[0], palabras[1], palabras[2], palabras[3]};
  const motor::RecursosEntidades recursos{dimensiones, dimensiones_entidad, 0x4000};
  bool valido = true;
  if (operacion == 0) {
    const auto creacion = motor::crear_entidad(datos, estado, recursos, posicion);
    valido = creacion.valido;
    salida[0] = creacion.creada ? 1 : 0;
    salida[1] = creacion.posicion;
  } else if (operacion == 1) {
    valido = motor::actualizar_entidad(datos, estado, recursos, posicion);
  } else if (operacion == 2) {
    valido = motor::actualizar_entidades(datos, estado, recursos);
  } else {
    motor::activar_transicion_entidad(datos);
  }
  palabras[0] = estado.primera;
  palabras[1] = estado.segunda;
  palabras[2] = estado.tercera;
  palabras[3] = estado.cuarta;
  return valido ? 1 : 0;
}

extern "C" EXPORTAR int procesar_secuencias(unsigned int operacion, unsigned char *datos,
                                            unsigned short *palabras, unsigned short *dimensiones,
                                            unsigned short posicion) {
  motor::EstadoAleatorio estado{palabras[0], palabras[1], palabras[2], palabras[3]};
  const motor::RecursosSecuencias recursos{dimensiones, dimensiones_secuencia, 0x4100, 0x4000,
                                           0x4200};
  auto resultado = motor::EstadoSecuencias::correcto;
  if (operacion == 0)
    resultado = motor::avanzar_secuencias_entidad(datos, estado, recursos, posicion);
  else if (operacion == 1)
    motor::retirar_entidad_fuera_de_limites(datos, posicion);
  else
    resultado = motor::avanzar_secuencias(datos, estado, recursos);
  palabras[0] = estado.primera;
  palabras[1] = estado.segunda;
  palabras[2] = estado.tercera;
  palabras[3] = estado.cuarta;
  return static_cast<int>(resultado) + 1;
}

namespace {
struct ContextoOperaciones {
  unsigned char *modelo;
  const unsigned char *fuente;
  unsigned int longitud;
  unsigned short *bitacora;
};

unsigned short leer_palabra_prueba(const unsigned char *datos, unsigned short posicion) {
  return static_cast<unsigned short>(datos[posicion] |
                                     (datos[static_cast<unsigned short>(posicion + 1)] << 8));
}

bool leer_muestra(void *, unsigned short recurso, unsigned short posicion, unsigned short &valor) {
  if (recurso != 0x5000 && recurso != 0x5100)
    return false;
  // Tabla y cabeceras sintéticas que el comparador dispone también para el original.
  if (posicion < 512 && (posicion & 1) == 0)
    valor = static_cast<unsigned short>(0x3000 + posicion * 2);
  else if (posicion >= 0x3000 && posicion < 0x3400 && (posicion & 3) == 0) {
    valor = static_cast<unsigned short>(1000 + posicion - 0x3000 + (recurso == 0x5100 ? 7 : 0));
  } else
    return false;
  return true;
}

bool cargar_modelo_prueba(void *contexto, const unsigned char *, unsigned short nombre,
                          unsigned short capacidad) {
  auto &prueba = *static_cast<ContextoOperaciones *>(contexto);
  ++prueba.bitacora[0];
  prueba.bitacora[1] = nombre;
  prueba.bitacora[2] = capacidad;
  const auto cantidad = prueba.longitud < capacidad ? prueba.longitud : capacidad;
  for (unsigned int i = 0; i < cantidad; ++i)
    prueba.modelo[i] = prueba.fuente[i];
  return true;
}

bool dimensiones_modelo(void *contexto, unsigned short recurso, unsigned short imagen,
                        unsigned short &ancho, unsigned short &alto) {
  const auto &prueba = *static_cast<ContextoOperaciones *>(contexto);
  if (recurso != 0x4200 || imagen >= 128)
    return false;
  const auto parrafo = leer_palabra_prueba(prueba.modelo, static_cast<unsigned short>(imagen * 2));
  const unsigned int posicion = parrafo * 16u;
  if (posicion > 65532)
    return false;
  ancho = leer_palabra_prueba(prueba.modelo, static_cast<unsigned short>(posicion));
  alto = leer_palabra_prueba(prueba.modelo, static_cast<unsigned short>(posicion + 2));
  return true;
}

bool sonido_sin_dispositivo(void *, unsigned char *datos, unsigned short entidad,
                            unsigned char id) {
  motor::EstadoSonido estado{0, 0, 0, 0, 0, 0x0f31};
  const auto orden = motor::seleccionar_sonido(datos, estado, {nullptr, nullptr}, entidad, id);
  return orden.valida && orden.rutina == 0;
}
} // namespace

extern "C" EXPORTAR int seleccionar_sonido(unsigned char *datos, unsigned short *palabras,
                                           unsigned short entidad, unsigned char identificador,
                                           unsigned short *salida) {
  motor::EstadoSonido estado{static_cast<unsigned char>(palabras[0]),
                             static_cast<unsigned char>(palabras[1]),
                             static_cast<unsigned char>(palabras[2]),
                             palabras[3],
                             palabras[4],
                             palabras[5]};
  const auto orden =
      motor::seleccionar_sonido(datos, estado, {nullptr, leer_muestra}, entidad, identificador);
  palabras[0] = estado.prioridad_secuencia;
  palabras[1] = estado.prioridad_muestra;
  salida[0] = orden.rutina;
  salida[1] = orden.referencia;
  salida[2] = orden.posicion;
  salida[3] = orden.longitud;
  salida[4] = orden.canal;
  return orden.valida ? 1 : 0;
}

extern "C" EXPORTAR int
ejecutar_operaciones_secuencia(unsigned char *datos, unsigned short *palabras,
                               unsigned char *modelo, const unsigned char *fuente,
                               unsigned int longitud, unsigned short *bitacora,
                               unsigned short posicion) {
  ContextoOperaciones contexto{modelo, fuente, longitud, bitacora};
  const motor::RecursosSecuencias recursos{
      &contexto,           dimensiones_modelo, 0x4100, 0x4000, 0x4200, sonido_sin_dispositivo,
      cargar_modelo_prueba};
  motor::EstadoAleatorio estado{palabras[0], palabras[1], palabras[2], palabras[3]};
  const auto resultado = motor::avanzar_secuencias_entidad(datos, estado, recursos, posicion);
  palabras[0] = estado.primera;
  palabras[1] = estado.segunda;
  palabras[2] = estado.tercera;
  palabras[3] = estado.cuarta;
  return static_cast<int>(resultado) + 1;
}

namespace {
void registrar_puerto(unsigned short *salida, unsigned short operacion, unsigned short puerto,
                      unsigned char valor) {
  const auto indice = salida[0]++;
  if (indice < 10) {
    salida[1 + indice * 3] = operacion;
    salida[2 + indice * 3] = puerto;
    salida[3 + indice * 3] = valor;
  }
}
unsigned char leer_altavoz(void *contexto, unsigned short puerto) {
  auto *salida = static_cast<unsigned short *>(contexto);
  const auto valor = static_cast<unsigned char>(salida[31]);
  registrar_puerto(salida, 0, puerto, valor);
  return valor;
}
void escribir_altavoz(void *contexto, unsigned short puerto, unsigned char valor) {
  auto *salida = static_cast<unsigned short *>(contexto);
  registrar_puerto(salida, 1, puerto, valor);
  if (puerto == 0x61)
    salida[31] = valor;
}
bool sonido_altavoz(void *contexto, unsigned char *datos, unsigned short entidad,
                    unsigned char id) {
  auto &estado = *static_cast<motor::EstadoSonido *>(contexto);
  return motor::atender_orden_altavoz(
      datos, motor::seleccionar_sonido(datos, estado, {nullptr, nullptr}, entidad, id));
}
} // namespace

extern "C" EXPORTAR int procesar_altavoz(unsigned int operacion, unsigned char *datos,
                                         unsigned short posicion, unsigned short *prioridad,
                                         unsigned short *puertos) {
  const motor::PuertosAltavoz servicio{puertos, leer_altavoz, escribir_altavoz};
  if (operacion == 0) {
    motor::iniciar_altavoz(datos, posicion);
    return 1;
  }
  if (operacion == 1)
    return motor::detener_altavoz(datos, servicio) ? 1 : 0;
  if (operacion == 2)
    return motor::avanzar_altavoz(datos, servicio) ? 1 : 0;
  motor::EstadoSonido estado{static_cast<unsigned char>(*prioridad), 0, 0, 0, 0, 0x0f31};
  bool valido = true;
  if (operacion == 3)
    valido = sonido_altavoz(&estado, datos, 0x2c4c, static_cast<unsigned char>(posicion));
  else {
    const motor::RecursosSecuencias recursos{&estado, nullptr, 0, 0, 0, sonido_altavoz, nullptr};
    motor::EstadoAleatorio aleatorio{0, 0, 0, 0};
    valido = motor::avanzar_secuencias_entidad(datos, aleatorio, recursos, 0x2c4c) ==
             motor::EstadoSecuencias::correcto;
  }
  *prioridad = estado.prioridad_secuencia;
  return valido ? 1 : 0;
}

namespace {
struct ContextoVideo {
  unsigned int *traza;
  unsigned char control;
};
unsigned char leer_video(void *contexto, unsigned short puerto) {
  auto &c = *static_cast<ContextoVideo *>(contexto);
  const auto indice = c.traza[0]++;
  c.traza[1 + indice * 4] = 0;
  c.traza[2 + indice * 4] = puerto;
  c.traza[3 + indice * 4] = c.control;
  c.traza[4 + indice * 4] = 1;
  return c.control;
}
void escribir_video(void *contexto, unsigned short puerto, unsigned short valor,
                    unsigned char tamano) {
  auto &c = *static_cast<ContextoVideo *>(contexto);
  const auto indice = c.traza[0]++;
  c.traza[1 + indice * 4] = 1;
  c.traza[2 + indice * 4] = puerto;
  c.traza[3 + indice * 4] = valor;
  c.traza[4 + indice * 4] = tamano;
  if (puerto == 0x3cf)
    c.control = static_cast<unsigned char>(valor);
}
} // namespace

extern "C" EXPORTAR int procesar_video(unsigned int operacion, unsigned char *datos,
                                       unsigned char *planos, const unsigned short *parametros,
                                       unsigned int *traza) {
  ContextoVideo contexto{traza, static_cast<unsigned char>(parametros[6])};
  const motor::PuertosVideo puertos{&contexto, leer_video, escribir_video};
  const auto ancho = parametros[0], alto = parametros[1], paso = parametros[2];
  if (operacion == 0)
    return motor::subir_imagen(datos, planos, puertos, ancho, alto, paso, parametros[4]) ? 1 : 0;
  if (operacion == 1)
    return motor::recuperar_imagen(planos, datos, puertos, ancho, alto, paso, parametros[3]) ? 1
                                                                                             : 0;
  if (operacion == 2)
    return motor::copiar_planos(planos, puertos, ancho, alto, paso, parametros[5], parametros[3],
                                parametros[4])
               ? 1
               : 0;
  return motor::enviar_paleta(datos, puertos, parametros[3],
                              static_cast<unsigned char>(parametros[4]), ancho)
             ? 1
             : 0;
}

extern "C" EXPORTAR unsigned int descomprimir_rnc1(const unsigned char *origen,
                                                   unsigned int longitud, unsigned char *destino,
                                                   unsigned int capacidad, unsigned int *escritos) {
  const auto resultado = motor::descomprimir_rnc1(origen, longitud, destino, capacidad);
  *escritos = resultado.escritos;
  return static_cast<unsigned int>(resultado.estado);
}

// Expone el método dos para contrastar la salida C++ con las instrucciones originales.
extern "C" EXPORTAR unsigned int descomprimir_rnc2(const unsigned char *origen,
                                                   unsigned int longitud, unsigned char *destino,
                                                   unsigned int capacidad, unsigned int *escritos) {
  const auto resultado = motor::descomprimir_rnc2(origen, longitud, destino, capacidad);
  *escritos = resultado.escritos;
  return static_cast<unsigned int>(resultado.estado);
}

// Expone el montaje MAP completo con imagen y matrices aportadas por el comparador.
extern "C" EXPORTAR int preparar_fondo_mapa(const unsigned char *mapa, unsigned int longitud,
                                            const unsigned char *seleccion,
                                            motor::ImagenIndexada *imagen, unsigned char *planos,
                                            unsigned char *tira) {
  return motor::preparar_fondo_mapa(mapa, longitud, seleccion, *imagen, planos, tira) ? 1 : 0;
}

// Expone la presentación del fotograma para compararla con el dibujado VGA de 6D8C.
extern "C" EXPORTAR int dibujar_fotograma(const unsigned char *banco, unsigned int longitud,
                                          unsigned int indice, unsigned char *destino, int x, int y,
                                          int invertido) {
  return motor::dibujar_fotograma(banco, longitud, indice, destino, x, y, invertido != 0) ? 1 : 0;
}

// Expone un paso de paleta para contrastarlo con las escrituras DAC del fundido original.
extern "C" EXPORTAR int calcular_fundido(const unsigned char *paleta, unsigned short colores,
                                         unsigned short pasos, unsigned short paso,
                                         unsigned char *salida) {
  return motor::calcular_fundido(paleta, colores, pasos, paso, salida) ? 1 : 0;
}

namespace {
struct BancoPrueba {
  const unsigned char *datos;
  unsigned int longitud;
};
// Aporta las cabeceras reales de la escena al cargador de entidades comparado.
bool dimensiones_banco(void *contexto, unsigned short indice, unsigned short &ancho,
                       unsigned short &alto) {
  const auto &banco = *static_cast<const BancoPrueba *>(contexto);
  unsigned short parrafo = 0;
  return motor::consultar_fotograma(banco.datos, banco.longitud, indice, parrafo, ancho, alto);
}
// Aporta la referencia de cada superposición, sin convertirla en una dirección del anfitrión.
bool parrafo_banco(void *contexto, unsigned short indice, unsigned short &parrafo) {
  const auto &banco = *static_cast<const BancoPrueba *>(contexto);
  unsigned short ancho = 0, alto = 0;
  return motor::consultar_fotograma(banco.datos, banco.longitud, indice, parrafo, ancho, alto);
}
// Resuelve las dos referencias usadas por las composiciones comparadas con VGA.
const unsigned char *banco_dibujo_prueba(void *contexto, unsigned short referencia,
                                         unsigned int &longitud) {
  const auto *bancos = static_cast<const BancoPrueba *>(contexto);
  if (referencia != 0x4000 && referencia != 0x5000)
    return nullptr;
  const auto &banco = bancos[referencia == 0x4000 ? 0 : 1];
  longitud = banco.longitud;
  return banco.datos;
}
// Consulta dimensiones reales al avanzar los guiones de la mazmorra comparada.
bool dimensiones_dibujo_prueba(void *contexto, unsigned short referencia, unsigned short indice,
                               unsigned short &ancho, unsigned short &alto) {
  unsigned int longitud = 0;
  const auto *banco = banco_dibujo_prueba(contexto, referencia, longitud);
  unsigned short parrafo = 0;
  return banco && motor::consultar_fotograma(banco, longitud, indice, parrafo, ancho, alto);
}
// Equivale a la modalidad cero del selector, que no accede a dispositivos ni detiene la secuencia.
bool omitir_sonido_prueba(void *, unsigned char *datos, unsigned short, unsigned char) {
  return datos[0x0ad4] == 0;
}
} // namespace

// Expone carga, rutina particular, ordenación, proximidad y entrada para sus comparaciones
// completas de DS.
extern "C" EXPORTAR int procesar_objetos(unsigned int operacion, unsigned char *datos,
                                         unsigned short *palabras, const unsigned char *banco,
                                         unsigned int longitud, unsigned short mascara,
                                         unsigned short parametro, unsigned short *salida) {
  motor::EstadoAleatorio estado{palabras[0], palabras[1], palabras[2], palabras[3]};
  BancoPrueba contexto{banco, longitud};
  const motor::RecursosEntidades recursos{&contexto, dimensiones_banco, 0x4000, parrafo_banco};
  bool valido = true;
  if (operacion == 0)
    valido = motor::cargar_entidades_escena(datos, estado, recursos);
  else if (operacion == 1)
    motor::actualizar_escena_cinco(datos);
  else if (operacion == 2)
    motor::ordenar_entidades(datos);
  else if (operacion == 3) {
    const auto resultado = motor::buscar_objetos_cercanos(
        datos, static_cast<unsigned char>(mascara), 0xa800, parametro);
    valido = resultado.valido;
    *salida = resultado.cantidad;
  } else if (operacion == 4) {
    unsigned short parrafo = 0, ancho = 0, alto = 0;
    valido = motor::consultar_fotograma(banco, longitud, 0, parrafo, ancho, alto);
    if (valido)
      motor::preparar_entrada_mazmorra(datos, 0x4000, ancho, alto);
  } else
    valido = false;
  palabras[0] = estado.primera;
  palabras[1] = estado.segunda;
  palabras[2] = estado.tercera;
  palabras[3] = estado.cuarta;
  return valido ? 1 : 0;
}

// Compone una escena sobre el fondo de prueba y conserva la tabla de orden resultante en DS.
extern "C" EXPORTAR int componer_objetos(unsigned char *datos, unsigned char *imagen,
                                         const unsigned char *escena, unsigned int longitud_escena,
                                         const unsigned char *personaje,
                                         unsigned int longitud_personaje) {
  BancoPrueba bancos[2] = {{escena, longitud_escena}, {personaje, longitud_personaje}};
  return motor::dibujar_entidades(datos, {bancos, banco_dibujo_prueba, 0x4000}, imagen) ? 1 : 0;
}

// Integra un paso de secuencias y comportamientos con bancos reales y la rutina particular de la
// escena seleccionada mediante el despacho original.
extern "C" EXPORTAR int avanzar_objetos(unsigned char *datos, unsigned short *palabras,
                                        const unsigned char *escena, unsigned int longitud_escena,
                                        const unsigned char *personaje,
                                        unsigned int longitud_personaje) {
  BancoPrueba bancos[2] = {{escena, longitud_escena}, {personaje, longitud_personaje}};
  motor::EstadoAleatorio estado{palabras[0], palabras[1], palabras[2], palabras[3]};
  const motor::RecursosSecuencias secuencias{bancos, dimensiones_dibujo_prueba, 0x5000, 0x4000,
                                             0x5000, omitir_sonido_prueba,      nullptr};
  const motor::RecursosEntidades entidades{&bancos[0], dimensiones_banco, 0x4000, parrafo_banco};
  const bool valido =
      motor::avanzar_secuencias(datos, estado, secuencias) == motor::EstadoSecuencias::correcto &&
      motor::actualizar_entidades(datos, estado, entidades);
  if (valido)
    motor::actualizar_rutina_escena(datos);
  palabras[0] = estado.primera;
  palabras[1] = estado.segunda;
  palabras[2] = estado.tercera;
  palabras[3] = estado.cuarta;
  return valido ? 1 : 0;
}

// Comprueba la copia de datos MZ por separado de las rutinas de juego que los consumen.
extern "C" EXPORTAR int leer_datos_iniciales(const unsigned char *ejecutable, unsigned int longitud,
                                             unsigned char *datos) {
  return motor::leer_datos_iniciales(ejecutable, longitud, datos) ? 1 : 0;
}

// Expone los eventos originales de la introducción sin copiar tipos C++ al comparador.
extern "C" EXPORTAR int evento_introduccion(unsigned int indice, unsigned short *salida) {
  motor::EventoIntroduccion evento{};
  if (!motor::obtener_evento_introduccion(indice, evento))
    return 0;
  salida[0] = static_cast<unsigned short>(evento.operacion);
  salida[1] = evento.valor;
  salida[2] = evento.pagina;
  return 1;
}

// Comprueba RNC2, subida y recuperación de cada página de la introducción.
extern "C" EXPORTAR int pagina_introduccion(const unsigned char *datos, unsigned int longitud,
                                            unsigned short pagina, unsigned char *planos,
                                            unsigned char *temporal, unsigned char *salida) {
  static motor::ImagenIndexada imagen;
  if (!motor::cargar_pagina_introduccion(datos, longitud, pagina, planos, temporal) ||
      !motor::mostrar_pagina_introduccion(planos, pagina, imagen))
    return 0;
  for (unsigned int i = 0; i < 64000; ++i)
    salida[i] = imagen.indices[i];
  return 1;
}

// Expone las decisiones y escrituras del menú y la activación de acciones al comparador.
struct ContextoCreacionMenu {
  BancoPrueba banco;
  motor::EstadoAleatorio azar;
};
// Aporta el mismo creador usado por SDL con el banco real de la habitación que prueba Unicorn.
bool crear_menu_prueba(void *contexto, unsigned char *datos, unsigned short descriptor,
                       unsigned short &posicion) {
  auto &estado = *static_cast<ContextoCreacionMenu *>(contexto);
  const motor::RecursosEntidades recursos{&estado.banco, dimensiones_banco, 0x4000, parrafo_banco};
  const auto resultado = motor::crear_entidad(datos, estado.azar, recursos, descriptor);
  posicion = resultado.creada ? resultado.posicion : 0x3510;
  return resultado.valido;
}
// Permite comparar selecciones que crean fragmentos sin introducir recursos en el núcleo.
extern "C" EXPORTAR int seleccionar_menu_recursos(unsigned char *datos, unsigned short casilla,
                                                  const unsigned char *banco, unsigned int longitud,
                                                  unsigned short *semilla) {
  ContextoCreacionMenu contexto{{banco, longitud},
                                {semilla[0], semilla[1], semilla[2], semilla[3]}};
  const auto resultado =
      motor::seleccionar_menu(datos, casilla, {}, {&contexto, crear_menu_prueba, &contexto.azar});
  semilla[0] = contexto.azar.primera;
  semilla[1] = contexto.azar.segunda;
  semilla[2] = contexto.azar.tercera;
  semilla[3] = contexto.azar.cuarta;
  return static_cast<int>(resultado);
}

// Expone las cuatro operaciones del archivo DOS sin permitir acceso del núcleo al disco.
extern "C" EXPORTAR int procesar_partidas(unsigned int operacion, unsigned char *archivo,
                                          unsigned int longitud, unsigned int ranura,
                                          const char *nombre, unsigned char *datos) {
  if (operacion == 0)
    return motor::validar_partidas(archivo, longitud);
  if (operacion == 1)
    return motor::escribir_partida(archivo, longitud, ranura, nombre, datos);
  if (operacion == 2)
    return motor::leer_partida(archivo, longitud, ranura, datos);
  if (operacion == 3)
    return motor::borrar_partida(archivo, longitud, ranura);
  return 0;
}

// Conserva el puente sin recursos para las comparaciones de los primeros manejadores.
extern "C" EXPORTAR int procesar_menu(unsigned int operacion, unsigned char *datos,
                                      unsigned short parametro, unsigned short otro) {
  if (operacion == 0) {
    motor::abrir_menu(datos, parametro, otro);
    return 0;
  }
  if (operacion == 1)
    return static_cast<int>(motor::seleccionar_menu(datos, parametro));
  if (operacion == 2)
    return static_cast<int>(motor::ejecutar_interaccion_inicial(datos, parametro, otro));
  if (operacion == 3)
    return motor::avanzar_accion_pendiente(datos) ? 0 : 3;
  if (operacion == 4)
    return motor::preparar_transicion_mazmorra(datos, parametro, otro) ? 0 : 3;
  return 3;
}

// Contrasta los iconos y las máscaras del panel con DEFB y las rutinas de sprites originales.
extern "C" EXPORTAR int dibujar_menu(const unsigned char *datos, const unsigned char *fondo,
                                     const unsigned char *iconos, unsigned int longitud,
                                     unsigned char *destino) {
  return motor::dibujar_menu(datos, fondo, iconos, longitud, destino) ? 1 : 0;
}

// Aporta servicios de bancos reales durante las secuencias iniciadas por las nuevas interacciones.
extern "C" EXPORTAR int avanzar_menu(unsigned char *datos, unsigned short *palabras,
                                     bool (*dimensiones)(void *, unsigned short, unsigned short,
                                                         unsigned short &, unsigned short &),
                                     bool (*modelo)(void *, const unsigned char *, unsigned short,
                                                    unsigned short)) {
  motor::EstadoAleatorio estado{palabras[0], palabras[1], palabras[2], palabras[3]};
  const motor::RecursosSecuencias recursos{nullptr, dimensiones,          0x6400, 0x4000,
                                           0x5400,  omitir_sonido_prueba, modelo};
  if (!motor::avanzar_accion_pendiente(datos))
    return -1;
  const auto secuencias = motor::avanzar_secuencias(datos, estado, recursos);
  if (secuencias != motor::EstadoSecuencias::correcto)
    return -10 - static_cast<int>(secuencias);
  // Los comportamientos de esta escena consultan el banco de escena mediante el mismo servicio.
  // Reutiliza el servicio de dimensiones para el banco fijo de las entidades de escena.
  const auto dimensiones_escena = [](void *contexto, unsigned short indice, unsigned short &ancho,
                                     unsigned short &alto) {
    const auto funcion = *static_cast<decltype(dimensiones) *>(contexto);
    return funcion(nullptr, 0x4000, indice, ancho, alto);
  };
  const motor::RecursosEntidades entidades{&dimensiones, dimensiones_escena, 0x4000, nullptr};
  if (!motor::actualizar_entidades(datos, estado, entidades))
    return -20;
  motor::actualizar_rutina_escena(datos);
  palabras[0] = estado.primera;
  palabras[1] = estado.segunda;
  palabras[2] = estado.tercera;
  palabras[3] = estado.cuarta;
  return 1;
}

namespace {
// Resuelve los tres bancos de la comparación de acciones, sin mezclar STD y el modelo sustituible.
const unsigned char *banco_menu_prueba(void *contexto, unsigned short referencia,
                                       unsigned int &longitud) {
  const auto *bancos = static_cast<const BancoPrueba *>(contexto);
  const int indice = referencia == 0x4000   ? 0
                     : referencia == 0x5400 ? 1
                     : referencia == 0x6400 ? 2
                                            : -1;
  if (indice < 0)
    return nullptr;
  longitud = bancos[indice].longitud;
  return bancos[indice].datos;
}
} // namespace
// Compone el resultado de una acción con modelo dinámico y banco estándar separados.
extern "C" EXPORTAR int componer_menu_escena(unsigned char *datos, unsigned char *imagen,
                                             const unsigned char *escena, unsigned int n_escena,
                                             const unsigned char *modelo, unsigned int n_modelo,
                                             const unsigned char *estandar,
                                             unsigned int n_estandar) {
  BancoPrueba bancos[] = {{escena, n_escena}, {modelo, n_modelo}, {estandar, n_estandar}};
  return motor::dibujar_entidades(datos, {bancos, banco_menu_prueba, 0x4000}, imagen) ? 1 : 0;
}

// Expone caminos, detección y corrección de colisiones, historial y despacho para contrastar DS.
extern "C" EXPORTAR int procesar_geometria(unsigned int operacion, unsigned char *datos,
                                           const unsigned char *geometria, unsigned int longitud,
                                           unsigned short *palabras, unsigned short *parametros) {
  motor::EstadoAleatorio azar{palabras[0], palabras[1], palabras[2], palabras[3]};
  motor::Colision r{true, 0, 0};
  if (operacion == 1)
    r.valida = motor::preparar_camino(datos, geometria, longitud);
  else if (operacion == 2)
    r = motor::comprobar_colision(datos, azar, geometria, longitud, parametros[0], parametros[1],
                                  parametros[2], parametros[3]);
  else if (operacion == 3)
    r = motor::resolver_colision_personaje(datos, azar, geometria, longitud);
  else if (operacion == 4)
    motor::actualizar_historial_personaje(datos);
  else if (operacion == 5)
    r.valida = motor::actualizar_rutina_escena(datos);
  else
    r.valida = false;
  palabras[0] = azar.primera;
  palabras[1] = azar.segunda;
  palabras[2] = azar.tercera;
  palabras[3] = azar.cuarta;
  parametros[4] = r.acciones;
  parametros[5] = r.parametro;
  return r.valida ? 1 : 0;
}

// Recorre un paso jugable completo con servicios de bancos reales aportados por el comparador.
extern "C" EXPORTAR int avanzar_jugabilidad(
    unsigned char *datos, unsigned short *palabras,
    bool (*dimensiones)(void *, unsigned short, unsigned short, unsigned short &, unsigned short &),
    bool (*modelo)(void *, const unsigned char *, unsigned short, unsigned short),
    const unsigned char *geometria, unsigned int longitud, unsigned short direccion) {
  motor::EstadoAleatorio azar{palabras[0], palabras[1], palabras[2], palabras[3]};
  const motor::RecursosSecuencias secuencias{nullptr, dimensiones,          0x6400, 0x4000,
                                             0x5400,  omitir_sonido_prueba, modelo};
  // Adapta la consulta de dimensiones al banco fijo de las entidades de escena.
  const auto consultar = [](void *contexto, unsigned short indice, unsigned short &ancho,
                            unsigned short &alto) {
    return (*static_cast<decltype(dimensiones) *>(contexto))(nullptr, 0x4000, indice, ancho, alto);
  };
  const motor::RecursosEntidades entidades{&dimensiones, consultar, 0x4000, nullptr};
  const auto r =
      motor::avanzar_escena(datos, azar, secuencias, entidades, geometria, longitud, direccion);
  palabras[0] = azar.primera;
  palabras[1] = azar.segunda;
  palabras[2] = azar.tercera;
  palabras[3] = azar.cuarta;
  return r.valida ? static_cast<int>(r.acciones) : -1;
}

// Expone exclusivamente el estado de cámara y de su tabla; no ejecuta puertos VGA ni DOS.
extern "C" EXPORTAR int procesar_camara(unsigned int operacion, unsigned char *datos,
                                        int direccion) {
  if (operacion == 0)
    return motor::decidir_camara(datos);
  if (operacion == 1)
    return motor::desplazar_camara(datos, direccion) ? 1 : 0;
  if (operacion == 2)
    return motor::preparar_vista_escena(datos) ? 1 : 0;
  return -2;
}

// Expone los ocho retrazos de cámara para contrastarlos con el origen CRTC de las páginas
// originales.
extern "C" EXPORTAR int componer_camara(const unsigned char *anterior, const unsigned char *destino,
                                        int direccion, unsigned int paso, unsigned char *salida) {
  return motor::componer_camara(anterior, destino, direccion, paso, salida) ? 1 : 0;
}

// Expone la preparación de entradas con modelo y dimensiones cargados por la plataforma de pruebas.
extern "C" EXPORTAR int preparar_transicion(unsigned char *datos, unsigned short salida,
                                            unsigned short estandar, unsigned short modelo,
                                            unsigned short ancho, unsigned short alto) {
  return motor::preparar_transicion_escena(datos, salida, estandar, modelo, ancho, alto) ? 1 : 0;
}

// Expone el primer bloque DMA para contrastarlo con las escrituras y el estado de 7FB3.
extern "C" EXPORTAR void preparar_dma_digital(unsigned short segmento, unsigned short posicion,
                                              unsigned int longitud, unsigned int *salida) {
  const auto bloque = motor::preparar_dma(segmento, posicion, longitud);
  salida[0] = bloque.direccion;
  salida[1] = bloque.cuenta;
  salida[2] = bloque.pagina;
  salida[3] = bloque.restantes;
}
// Conserva una sesión digital entre llamadas para comprobar interrupciones, prioridades y bloques.
extern "C" EXPORTAR int procesar_digital(unsigned int operacion, unsigned char *datos,
                                         const unsigned char *general,
                                         unsigned int longitud_general, const unsigned char *escena,
                                         unsigned int longitud_escena, unsigned short entidad,
                                         unsigned char identificador, short *salida,
                                         unsigned int cantidad) {
  static motor::DigitalPcm audio;
  if (operacion == 0) {
    audio.conectar(general, longitud_general, escena, longitud_escena);
    return 1;
  }
  if (operacion == 1)
    return audio.reproducir(datos, entidad, identificador) ? 1 : 0;
  if (operacion == 2) {
    audio.generar(salida, cantidad);
    return 1;
  }
  if (operacion == 3)
    return audio.activa() ? 1 : 0;
  if (operacion == 4) {
    audio.detener();
    return 1;
  }
  return 0;
}
// Obtiene referencias de bancos desde la misma tabla de 78 registros que utiliza el cargador DOS.
extern "C" EXPORTAR int nombre_sfx(const unsigned char *datos, unsigned short escena,
                                   char *nombre) {
  return motor::nombre_banco_sonido(datos, escena, nombre) ? 1 : 0;
}

// Expone las cuatro fichas originales para contrastar números, glifos y los 64.000 píxeles.
extern "C" EXPORTAR int componer_informacion(const unsigned char *datos,
                                             const unsigned char *fuente, unsigned int longitud,
                                             unsigned int pagina, unsigned char *destino,
                                             unsigned int *rectangulo) {
  return motor::componer_informacion(datos, fuente, longitud, pagina, destino, rectangulo[0],
                                     rectangulo[1], rectangulo[2], rectangulo[3])
             ? 1
             : 0;
}
