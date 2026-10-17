#include "../../motor/video/camara.hpp"
#include "../../motor/video/fundido.hpp"
#include "../../motor/recursos/escena.hpp"
#include "mazmorra.hpp"
#include "../../motor/estado/paso_escena.hpp"
#include "../../motor/movimiento/geometria.hpp"
#include "../../motor/movimiento/planificar.hpp"
#include "../../motor/movimiento/direccion.hpp"
#include "../../motor/acciones/menu.hpp"
#include "../../motor/video/menu.hpp"
#include "../../motor/recursos/rnc.hpp"
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
constexpr unsigned short referencia_raton = 0x4300;
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
                       : referencia == referencia_raton     ? &estado.raton
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
      !cargar_banco("MANA.BRD", modelo) || !cargar_banco("ICONS.SPR", iconos) ||
      !cargar_banco("MOUSE.SPR", raton))
    return false;
  unsigned short ancho = 0, alto = 0;
  if (!dimensiones(this, referencia_modelo, 0, ancho, alto))
    return false;
  motor::preparar_entrada_mazmorra(datos, referencia_modelo, ancho, alto);
  const motor::RecursosEntidades recursos{this, dimensiones_escena, referencia_escena,
                                          parrafo_escena};
  if (!motor::cargar_entidades_escena(datos, aleatorio, recursos))
    return false;
  std::vector<unsigned char> menu_comprimido;
  if (!cargar_banco("MENU.DAT", menu_comprimido))
    return false;
  const auto descomprimido = motor::descomprimir_rnc1(
      menu_comprimido.data(), static_cast<unsigned int>(menu_comprimido.size()), fondo_menu, 10240);
  if (descomprimido.estado != motor::EstadoRnc::correcto || descomprimido.escritos != 10240)
    return false;
  std::vector<unsigned char> boxdet;
  unsigned int inicio = 0, cantidad = 0;
  if (!cargar_banco("BOXDET.DAT", boxdet) ||
      !motor::localizar_geometria(boxdet.data(), static_cast<unsigned int>(boxdet.size()), 0,
                                  inicio, cantidad))
    return false;
  geometria.assign(boxdet.begin() + inicio, boxdet.begin() + inicio + cantidad * 10u);
  motor::guardar(datos, 0x0df0, static_cast<unsigned short>(cantidad));
  motor::guardar(datos, 0x0df2, 0);
  direccion = 0xffff;
  menu_visible = false;
  animacion = paso_animacion = 0;
  if (!cargar_banco("DUNGEON.MAP", mapa) || !motor::preparar_vista_escena(datos))
    return false;
  motor::guardar(datos, 0x2c29, referencia_raton);
  fondo = imagen_fondo;
  return true;
}

// 06E8–0BC1: integra entrada, guiones, paredes e historial antes del despacho de la escena.
bool Mazmorra::avanzar() {
  if (menu_visible || en_presentacion())
    return true;
  const int camara = motor::decidir_camara(datos);
  if (camara) {
    if (!dibujar(imagen_anterior) || !motor::desplazar_camara(datos, camara) ||
        !cargar_fondo_visible() || !dibujar(imagen_destino))
      return false;
    sentido_camara = camara;
    animacion = 2;
    paso_animacion = 0;
    inicio_animacion = SDL_GetTicksNS() * 70u / 1000000000u;
    return true;
  }
  // EAD5 presenta la posición anterior al paso que detectará la salida.
  if (!dibujar(imagen_anterior))
    return false;
  const motor::RecursosSecuencias secuencias{
      this,          dimensiones,  referencia_personaje, referencia_escena, referencia_modelo,
      omitir_sonido, cargar_modelo};
  const motor::RecursosEntidades entidades{this, dimensiones_escena, referencia_escena,
                                           parrafo_escena};
  const auto resultado =
      motor::avanzar_escena(datos, aleatorio, secuencias, entidades, geometria.data(),
                            static_cast<unsigned int>(geometria.size()), direccion);
  if (!resultado.valida)
    return false;
  if (resultado.acciones) {
    const auto actual = escena_actual();
    if ((resultado.acciones == 2 || resultado.acciones == 16) &&
        ((actual == 0 && resultado.parametro == 0) ||
         (actual == 1 && (resultado.parametro == 1 || resultado.parametro == 2)) ||
         (actual == 2 && resultado.parametro == 3))) {
      salida_pendiente = resultado.parametro;
      animacion = 1;
      paso_animacion = 0;
      inicio_animacion = SDL_GetTicksNS() * 70u / 1000000000u;
      return true;
    }
    // Las otras salidas se reconocen, pero sus habitaciones todavía no tienen un adaptador
    // completo.
    motor::guardar(datos, 0x2c2d, motor::palabra(datos, 0x0ae6));
    motor::guardar(datos, 0x2c2f, motor::palabra(datos, 0x0ae8));
    motor::guardar(datos, 0x359a, 0);
    motor::guardar(datos, 0x0dfa, 0);
    if (!motor::seleccionar_direccion(datos, 0))
      return false;
    SDL_Log("La salida %u de la escena %u aún no está integrada.", resultado.parametro, actual);
  }
  return true;
}

// Una imagen completa sustituye la restauración de rectángulos entre páginas VGA sin añadir
// efectos.
bool Mazmorra::dibujar(motor::ImagenIndexada &imagen, unsigned int paso_menu) {
  if (animacion == 1) {
    imagen = imagen_anterior;
    return motor::calcular_fundido(imagen_anterior.paleta, 256, 32,
                                   static_cast<unsigned short>(paso_animacion), imagen.paleta);
  }
  if (animacion == 2) {
    imagen = imagen_destino;
    return motor::componer_camara(imagen_anterior.indices, imagen_destino.indices, sentido_camara,
                                  paso_animacion, imagen.indices);
  }
  imagen = fondo;
  if (!motor::dibujar_entidades(datos, {this, banco, referencia_escena}, imagen.indices))
    return false;
  if (menu_visible) {
    motor::componer_menu(datos, panel_menu, casilla_menu, imagen.indices);
    // DB68–DB8D: la selección usa el color 255 de la tabla original de 24 colores.
    const auto color = paso_menu % 24u;
    for (unsigned int i = 0; i < 3; ++i)
      imagen.paleta[765 + i] = datos[0xb63e + color * 3 + i];
  }
  return true;
}

// DB34 y DE10: conserva la mitad superior/inferior elegida al abrir y divide x entre 32.
bool Mazmorra::entrada_menu(unsigned short x, unsigned short y, bool seleccionar, bool alternar) {
  if (en_presentacion())
    return true;
  if (alternar) {
    if (menu_visible) {
      menu_visible = false;
      return true;
    }
    if (datos[0x3595] == 0)
      return true;
    motor::abrir_menu(datos, x, y);
    menu_visible = true;
  }
  if (!menu_visible)
    return seleccionar ? mover_a(x, y) : true;
  casilla_menu = x < 320 ? x / 32 : 9;
  motor::guardar(datos, 0xb60a, x);
  if (seleccionar) {
    const auto resultado = motor::seleccionar_menu(datos, casilla_menu);
    if (resultado == motor::ResultadoMenu::error)
      return false;
    if (resultado == motor::ResultadoMenu::accion)
      menu_visible = false;
    if (resultado == motor::ResultadoMenu::pendiente)
      SDL_Log("Esta acción todavía está pendiente de integrar.");
  }
  return motor::dibujar_menu(datos, fondo_menu, iconos.data(),
                             static_cast<unsigned int>(iconos.size()), panel_menu);
}

// Representa cajas de entidades (verde), Brad (cian), disponibilidad (magenta) y límites
// (amarillo).
bool Mazmorra::depurar(SDL_Renderer *dibujante, bool objetos, bool interacciones,
                       bool limites) const {
  // Traza un rectángulo en coordenadas lógicas; el dibujante adapta escala y bandas negras.
  const auto rectangulo = [&](int x, int y, int ancho, int alto, unsigned char r, unsigned char g,
                              unsigned char b) {
    const SDL_FRect caja{static_cast<float>(x), static_cast<float>(y), static_cast<float>(ancho),
                         static_cast<float>(alto)};
    return ancho <= 0 || alto <= 0 ||
           (SDL_SetRenderDrawColor(dibujante, r, g, b, 255) && SDL_RenderRect(dibujante, &caja));
  };
  for (unsigned int i = 0; i < 69; ++i) {
    const auto entidad = static_cast<unsigned short>(0x2c2b + i * 33);
    if (!datos[entidad])
      continue;
    const int x = motor::con_signo(motor::palabra(datos, entidad + 2)),
              y = motor::con_signo(motor::palabra(datos, entidad + 4));
    const int ancho = motor::palabra(datos, entidad + 6), alto = motor::palabra(datos, entidad + 8);
    if (objetos && !rectangulo(x, y, ancho, alto, i ? 40 : 0, i ? 255 : 220, i ? 80 : 255))
      return false;
    const auto tipo = motor::palabra(datos, entidad + 14);
    if (interacciones && i && tipo < 0x400 && datos[0x524d + tipo * 2] &&
        (!(datos[entidad] & 2) || (datos[entidad] & 4))) {
      const auto margen = motor::palabra(datos, entidad + 12);
      if (!rectangulo(x, y - margen, ancho, alto + margen * 2, 255, 60, 220))
        return false;
    }
  }
  if (interacciones) {
    const int x = motor::con_signo(motor::palabra(datos, 0x2c2d)),
              y = motor::con_signo(motor::palabra(datos, 0x2c2f));
    if (!rectangulo(x - 5, y + motor::palabra(datos, 0x2c33) - 8,
                    motor::palabra(datos, 0x2c31) + 10, 12, 255, 150, 0))
      return false;
  }
  if (limites) {
    for (unsigned int p = 0; p + 10 <= geometria.size(); p += 10) {
      // Lee cada campo del registro original de diez bytes, sin cambiar la geometría.
      const auto leer = [&](unsigned int campo) {
        return static_cast<unsigned short>(
            geometria[p + campo] | (static_cast<unsigned int>(geometria[p + campo + 1]) << 8));
      };
      const int x = motor::con_signo(leer(2)) - motor::con_signo(motor::palabra(datos, 0x3591));
      const int y = motor::con_signo(leer(4));
      const auto tipo = geometria[p];
      if (!rectangulo(x, y, motor::con_signo(leer(6)) - motor::con_signo(leer(2)) + 1,
                      motor::con_signo(leer(8)) - y + 1, tipo == 2 ? 40 : 255,
                      tipo == 2   ? 140
                      : tipo == 1 ? 50
                                  : 255,
                      tipo == 2 ? 255 : 0))
        return false;
    }
    if (!rectangulo(0, 0, 320, 200, 255, 255, 0))
      return false;
    for (const auto campo : {0x358d, 0x358f}) {
      const auto x = static_cast<float>(
          motor::con_signo(motor::palabra(datos, static_cast<unsigned short>(campo))));
      if (!SDL_SetRenderDrawColor(dibujante, 255, 255, 0, 255) ||
          !SDL_RenderLine(dibujante, x, 0, x, 199))
        return false;
    }
  }
  return true;
}

// 08C8–08DA: el ratón solo propone rutas cuando el guion original ha habilitado el movimiento.
bool Mazmorra::mover_a(unsigned short x, unsigned short y) {
  if (en_presentacion() || menu_visible || !datos[0x3594])
    return true;
  if (x >= 320 || y >= 200)
    return true;
  direccion = 0xffff;
  datos[0x0ad5] = 1;
  datos[0x2c0a] = 1;
  motor::guardar(datos, 0x2c0c, x);
  motor::guardar(datos, 0x2c0e, y);
  if (!motor::preparar_camino(datos, geometria.data(), static_cast<unsigned int>(geometria.size())))
    return false;
  unsigned short pasos = 0;
  return motor::planificar_movimiento(datos, motor::palabra(datos, 0x0dfa), pasos);
}

// 083D–086D: izquierda=8, derecha=4, arriba=2 y abajo=1; los guiones conservan su bloqueo.
void Mazmorra::tecla_direccion(unsigned short bit, bool pulsada) {
  if (!pulsada && direccion == 0xffff)
    return;
  if (direccion == 0xffff)
    direccion = 0;
  if (pulsada)
    direccion = static_cast<unsigned short>(direccion | bit);
  else
    direccion = static_cast<unsigned short>(direccion & ~bit);
  if (datos[0x3594]) {
    datos[0x0ad5] = 0;
    datos[0x2c0a] = 0;
  }
}

// Mantiene el índice original, independiente de las vistas de presentación F5/F6.
unsigned short Mazmorra::escena_actual() const {
  return motor::palabra(datos, 0x8082);
}

// 50D2: recompone el fondo completo como sustitución portable de las copias solapadas VGA.
bool Mazmorra::cargar_fondo_visible() {
  const auto inicio = motor::palabra(datos, 0x0aed) + datos[0x3597];
  return motor::preparar_fondo_mapa(mapa.data(), static_cast<unsigned int>(mapa.size()),
                                    datos + inicio, fondo, planos_fondo, tira_fondo);
}

// 4AF0: conserva inventario y descriptores mutados; vuelve a cargar las entidades desde esas
// listas.
bool Mazmorra::cambiar_habitacion(unsigned short salida) {
  if (salida > 3)
    return false;
  unsigned short ancho = 32, alto = 48;
  if (salida == 3 && (!cargar_banco("PLUNGE.BRD", modelo) ||
                      !dimensiones(this, referencia_modelo, 0, ancho, alto)))
    return false;
  if (!motor::preparar_transicion_escena(datos, salida, referencia_personaje, referencia_modelo,
                                         ancho, alto) ||
      !motor::preparar_vista_escena(datos))
    return false;
  constexpr const char *bancos[] = {"DUNGEON", "CORRIDOR", "DROP", "LAKEBED"};
  const auto actual = escena_actual();
  if (actual > 3 || !cargar_banco(std::string(bancos[actual]) + ".SPR", escena) ||
      !cargar_banco(std::string(bancos[actual]) + ".MAP", mapa) || !cargar_fondo_visible())
    return false;
  // 4EE5 conserva los 192 colores de fondo en DS para los siguientes fundidos.
  for (unsigned int i = 0; i < 576; ++i)
    datos[0x0aef + i] = mapa[i];
  std::vector<unsigned char> boxdet;
  unsigned int inicio = 0, cantidad = 0;
  if (!cargar_banco("BOXDET.DAT", boxdet) ||
      !motor::localizar_geometria(boxdet.data(), static_cast<unsigned int>(boxdet.size()),
                                  escena_actual(), inicio, cantidad))
    return false;
  geometria.assign(boxdet.begin() + inicio, boxdet.begin() + inicio + cantidad * 10u);
  motor::guardar(datos, 0x0df0, static_cast<unsigned short>(cantidad));
  motor::guardar(datos, 0x0df2, 0);
  const motor::RecursosEntidades recursos{this, dimensiones_escena, referencia_escena,
                                          parrafo_escena};
  if (!motor::cargar_entidades_escena(datos, aleatorio, recursos))
    return false;
  motor::guardar(datos, 0x0358, 0);
  motor::guardar(datos, 0x0616, 0);
  menu_visible = false;
  direccion = 0xffff;
  return true;
}

// El original detiene la lógica durante el fundido y los ocho retrazos del movimiento de cámara.
bool Mazmorra::avanzar_presentacion(unsigned long long tick) {
  if (!en_presentacion())
    return true;
  const auto transcurrido = tick > inicio_animacion ? tick - inicio_animacion : 0;
  const unsigned int limite = animacion == 1 ? 32u : 8u;
  paso_animacion = transcurrido < limite ? static_cast<unsigned int>(transcurrido) : limite;
  if (transcurrido > limite) {
    const auto anterior = animacion;
    animacion = 0;
    if (anterior == 1 && !cambiar_habitacion(salida_pendiente))
      return false;
  }
  return true;
}

// Dispone exclusivamente la vista de desarrollo; nunca transforma una partida iniciada en otra
// escena.
bool Mazmorra::iniciar_caida(const std::string &ruta, const motor::ImagenIndexada &imagen_fondo) {
  if (!iniciar(ruta, imagen_fondo))
    return false;
  motor::guardar(datos, 0x8082, 1);
  return cambiar_habitacion(1);
}
