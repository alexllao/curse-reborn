#include "mazmorra.hpp"
#include <SDL3/SDL.h>
#include "../../motor/estado/inicializar.hpp"
#include "../../motor/estado/segmento.hpp"
#include "../../motor/entidades/actualizar.hpp"
#include "../../motor/entidades/mazmorra.hpp"
#include "../../motor/entidades/secuencias.hpp"
#include "../../motor/recursos/banco.hpp"
#include "../../motor/video/entidades.hpp"

namespace {
constexpr unsigned short referencia_escena = 0x4000;
constexpr unsigned short referencia_personaje = 0x4100;
constexpr unsigned short referencia_modelo = 0x4200;
} // namespace

// Carga un banco en propiedad para que sus referencias sobrevivan a todos los pasos de escena.
bool Mazmorra::cargar_banco(const std::string &nombre, std::vector<unsigned char> &destino) {
  size_t longitud = 0;
  void *archivo = SDL_LoadFile((carpeta + "/" + nombre).c_str(), &longitud);
  if (!archivo)
    return false;
  if (longitud == 0 || longitud > 0xffffffffu) {
    SDL_free(archivo);
    return false;
  }
  const auto *bytes = static_cast<const unsigned char *>(archivo);
  destino.assign(bytes, bytes + longitud);
  SDL_free(archivo);
  return true;
}

// Conserva las referencias como identificadores portables, sin usar el mapa de memoria de DOS.
const unsigned char *Mazmorra::banco(void *contexto, unsigned short referencia,
                                     unsigned int &longitud) {
  const auto &estado = *static_cast<const Mazmorra *>(contexto);
  const auto *fuente = referencia == referencia_escena      ? &estado.escena
                       : referencia == referencia_personaje ? &estado.personaje
                       : referencia == referencia_modelo    ? &estado.modelo
                                                            : nullptr;
  if (!fuente || fuente->empty())
    return nullptr;
  longitud = static_cast<unsigned int>(fuente->size());
  return fuente->data();
}

// Comparte el lector de cabeceras ya contrastado con las rutinas originales de los bancos.
bool Mazmorra::dimensiones(void *contexto, unsigned short referencia, unsigned short imagen,
                           unsigned short &ancho, unsigned short &alto) {
  unsigned int longitud = 0;
  const auto *fuente = banco(contexto, referencia, longitud);
  unsigned short parrafo = 0;
  return fuente && motor::consultar_fotograma(fuente, longitud, imagen, parrafo, ancho, alto);
}

// Las entidades de escena comparten el banco que selecciona CS:4AEE en el original.
bool Mazmorra::dimensiones_escena(void *contexto, unsigned short imagen, unsigned short &ancho,
                                  unsigned short &alto) {
  return dimensiones(contexto, referencia_escena, imagen, ancho, alto);
}

// La referencia en párrafos queda como dato para las superposiciones, no como puntero del proceso.
bool Mazmorra::parrafo_escena(void *contexto, unsigned short imagen, unsigned short &referencia) {
  unsigned int longitud = 0;
  const auto *fuente = banco(contexto, referencia_escena, longitud);
  unsigned short ancho = 0, alto = 0;
  return fuente && motor::consultar_fotograma(fuente, longitud, imagen, referencia, ancho, alto);
}

// Modalidad cero: el sonido no bloquea los guiones aunque no haya un dispositivo abierto.
bool Mazmorra::omitir_sonido(void *, unsigned char *datos, unsigned short, unsigned char) {
  return datos[0x0ad4] == 0;
}

// Lee únicamente nombres DOS de recurso, sin permitir rutas tomadas del contenido del EXE.
bool Mazmorra::cargar_modelo(void *contexto, const unsigned char *datos, unsigned short nombre,
                             unsigned short capacidad) {
  auto &estado = *static_cast<Mazmorra *>(contexto);
  std::string archivo;
  for (unsigned int i = 0; i <= 12; ++i) {
    const auto letra = datos[static_cast<unsigned short>(nombre + i)];
    if (letra == 0)
      return !archivo.empty() && estado.cargar_banco(archivo, estado.modelo) &&
             estado.modelo.size() <= capacidad;
    if (!((letra >= 'a' && letra <= 'z') || (letra >= 'A' && letra <= 'Z') ||
          (letra >= '0' && letra <= '9') || letra == '_' || letra == '.'))
      return false;
    archivo += static_cast<char>(letra);
  }
  return false;
}

// Reconstruye el estado de entrada y carga la lista original de objetos, incluidos los opcionales.
bool Mazmorra::iniciar(const std::string &ruta, const motor::ImagenIndexada &imagen_fondo) {
  carpeta = ruta;
  size_t longitud = 0;
  void *archivo = SDL_LoadFile((carpeta + "/../CURSE.EXE").c_str(), &longitud);
  if (!archivo)
    return false;
  const auto *bytes = static_cast<const unsigned char *>(archivo);
  const bool valido =
      longitud <= 0xffffffffu &&
      motor::leer_datos_iniciales(bytes, static_cast<unsigned int>(longitud), datos);
  if (valido) {
    constexpr unsigned short semilla = 512 + 0x78de;
    aleatorio = {motor::palabra(bytes, semilla), motor::palabra(bytes, semilla + 2),
                 motor::palabra(bytes, semilla + 4), motor::palabra(bytes, semilla + 6)};
  }
  SDL_free(archivo);
  if (!valido || !cargar_banco("DUNGEON.SPR", escena) || !cargar_banco("STD.BRD", personaje) ||
      !cargar_banco("MANA.BRD", modelo))
    return false;
  unsigned short ancho = 0, alto = 0;
  if (!dimensiones(this, referencia_modelo, 0, ancho, alto))
    return false;
  motor::preparar_entrada_mazmorra(datos, referencia_modelo, ancho, alto);
  const motor::RecursosEntidades recursos{this, dimensiones_escena, referencia_escena,
                                          parrafo_escena};
  if (!motor::cargar_entidades_escena(datos, aleatorio, recursos))
    return false;
  fondo = imagen_fondo;
  return true;
}

// 0A59 y 0BC1: avanza los canales y los comportamientos antes de la rutina particular de la
// mazmorra.
bool Mazmorra::avanzar() {
  const motor::RecursosSecuencias secuencias{
      this,          dimensiones,  referencia_personaje, referencia_escena, referencia_modelo,
      omitir_sonido, cargar_modelo};
  const motor::RecursosEntidades entidades{this, dimensiones_escena, referencia_escena,
                                           parrafo_escena};
  if (motor::avanzar_secuencias(datos, aleatorio, secuencias) !=
          motor::EstadoSecuencias::correcto ||
      !motor::actualizar_entidades(datos, aleatorio, entidades))
    return false;
  motor::actualizar_mazmorra(datos);
  return true;
}

// Una imagen completa sustituye la restauración de rectángulos entre páginas VGA sin añadir
// efectos.
bool Mazmorra::dibujar(motor::ImagenIndexada &imagen) {
  imagen = fondo;
  return motor::dibujar_entidades(datos, {this, banco, referencia_escena}, imagen.indices);
}
