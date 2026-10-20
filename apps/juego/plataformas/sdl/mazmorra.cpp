#include "../../motor/video/panel_partidas.hpp"
#include "../../motor/video/sprites.hpp"
#include "../../motor/acciones/lista_identificadores.hpp"
#include "../../motor/video/camara.hpp"
#include "../../motor/video/fundido.hpp"
#include "../../motor/recursos/escena.hpp"
#include "mazmorra.hpp"
#include "../../motor/estado/partidas.hpp"
#include "registro.hpp"
#include "vistas.hpp"
#include "interfaz/creditos_port.hpp"
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

// 5288: sirve a las secuencias FB, respuestas del menú y rutina del lago con el mismo selector.
bool Mazmorra::reproducir_sonido(void *contexto, unsigned char *datos, unsigned short entidad,
                                 unsigned char identificador) {
  return static_cast<Mazmorra *>(contexto)->audio.reproducir(datos, entidad, identificador);
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
  fase_probador = 0;
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
      !cargar_banco("MOUSE.SPR", raton) || !cargar_banco("FONT.SPR", fuente))
    return false;
  unsigned short ancho = 0, alto = 0;
  if (!dimensiones(this, referencia_modelo, 0, ancho, alto))
    return false;
  motor::preparar_entrada_mazmorra(datos, referencia_modelo, ancho, alto);
  if (!audio.conectar(datos))
    return false;
  const motor::RecursosEntidades recursos{
      this,           dimensiones_escena,        referencia_escena,
      parrafo_escena, {this, reproducir_sonido}, &fase_probador};
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
  paso_tras_camara = false;
  if (!cargar_banco("DUNGEON.MAP", mapa) || !motor::preparar_vista_escena(datos))
    return false;
  motor::guardar(datos, 0x2c29, referencia_raton);
  fondo = imagen_fondo;
  registro_preparado = false;
  registrar_accion("Inicio de sesión nueva: %s", nombre_escena(0));
  registrar_estado();
  return true;
}

// 06E8–0BC1: integra entrada, guiones, paredes e historial antes del despacho de la escena.
bool Mazmorra::avanzar() {
  if (menu_visible || en_presentacion())
    return true;
  // EAD5 comprueba la cámara una vez por ciclo; tras sus ocho retrazos sigue el paso lógico.
  const int camara = paso_tras_camara ? 0 : motor::decidir_camara(datos);
  paso_tras_camara = false;
  if (camara) {
    if (!dibujar(imagen_anterior) || !motor::desplazar_camara(datos, camara) ||
        !cargar_fondo_visible() || !dibujar(imagen_destino))
      return false;
    paso_tras_camara = true;
    sentido_camara = camara;
    animacion = 2;
    paso_animacion = 0;
    inicio_animacion = SDL_GetTicksNS() * 70u / 1000000000u;
    return true;
  }
  // EAD5 presenta la posición anterior al paso que detectará la salida.
  if (!dibujar(imagen_anterior))
    return false;
  const motor::RecursosSecuencias secuencias{this,
                                             dimensiones,
                                             referencia_personaje,
                                             referencia_escena,
                                             referencia_modelo,
                                             reproducir_sonido,
                                             cargar_modelo};
  const motor::RecursosEntidades entidades{
      this,           dimensiones_escena,        referencia_escena,
      parrafo_escena, {this, reproducir_sonido}, &fase_probador};
  const auto portero_anterior = datos[0x2c4d];
  const auto resultado =
      motor::avanzar_escena(datos, aleatorio, secuencias, entidades, geometria.data(),
                            static_cast<unsigned int>(geometria.size()), direccion);
  if (!resultado.valida)
    return false;
  if (escena_actual() == 21 && datos[0x2c4d] == 19 && portero_anterior != 19)
    registrar_accion("Red Dragon: reacción del portero original al acercarse a la posada");
  registrar_estado();
  if (resultado.acciones) {
    const auto actual = escena_actual();
    if ((resultado.acciones == 2 || resultado.acciones == 16) &&
        salida_integrada(actual, resultado.parametro)) {
      salida_pendiente = resultado.parametro;
      registrar_accion("Salida %u desde escena %u: comienza el fundido", salida_pendiente, actual);
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
    registrar_accion("Ruta pendiente: salida %u desde %s (escena %u). La habitación de destino aún "
                     "no está integrada.",
                     resultado.parametro, nombre_escena(actual), actual);
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

// D623 centra la placa negra; D87A usa FONT.SPR con pasos de ocho píxeles y diez por línea.
bool Mazmorra::dibujar_informacion(unsigned int pagina, motor::ImagenIndexada &imagen,
                                   SDL_FRect &placa) const {
  imagen = fondo;
  unsigned int x = 0, y = 0, ancho = 0, alto = 0;
  if (pagina == 4) {
    placa = {24, 51, 272, 98};
    return componer_creditos_port(datos, fuente.data(), static_cast<unsigned int>(fuente.size()),
                                  imagen.indices);
  }
  if (!motor::componer_informacion(datos, fuente.data(), static_cast<unsigned int>(fuente.size()),
                                   pagina, imagen.indices, x, y, ancho, alto))
    return false;
  if (idioma_interfaz == IdiomaInterfaz::castellano &&
      !componer_informacion_castellano(datos, fuente.data(),
                                       static_cast<unsigned int>(fuente.size()), pagina,
                                       imagen.indices, x, y, ancho, alto))
    return false;
  placa = {static_cast<float>(x), static_cast<float>(y), static_cast<float>(ancho),
           static_cast<float>(alto)};
  return true;
}

// E89D y E46D: utiliza la placa, el teclado y el cursor DOS sobre un fondo centinela.
bool Mazmorra::dibujar_panel_partidas(unsigned int modo, const std::vector<unsigned char> &archivo,
                                      bool editando, const std::string &nombre, int cursor_x,
                                      int cursor_y, unsigned char fondo_transparente,
                                      motor::ImagenIndexada &imagen) const {
  imagen = fondo;
  for (auto &indice : imagen.indices)
    indice = fondo_transparente;
  std::array<unsigned char, 65536> traduccion{};
  const unsigned char *textos = datos;
  if (idioma_interfaz == IdiomaInterfaz::castellano) {
    std::copy(std::begin(datos), std::end(datos), traduccion.begin());
    traducir_panel_partidas(traduccion);
    textos = traduccion.data();
  }
  if (!motor::componer_panel_partidas(textos, fuente.data(),
                                      static_cast<unsigned int>(fuente.size()), archivo.data(),
                                      static_cast<unsigned int>(archivo.size()), modo, editando,
                                      reinterpret_cast<const unsigned char *>(nombre.data()),
                                      static_cast<unsigned int>(nombre.size()), imagen.indices))
    return SDL_SetError("Panel original no válido: modo%u, fuente%zu, archivo%zu, nombre%zu", modo,
                        fuente.size(), archivo.size(), nombre.size());
  if (!datos[0x2c0a])
    return true;
  if (!motor::dibujar_fotograma(raton.data(), static_cast<unsigned int>(raton.size()),
                                datos[0x2c0b], imagen.indices, cursor_x, cursor_y, false))
    return SDL_SetError("Cursor original no válido: imagen%u, banco%zu, posición%d,%d",
                        datos[0x2c0b], raton.size(), cursor_x, cursor_y);
  return true;
}
// MOUSE.SPR conserva la misma mano que el selector DOS; dos fondos distinguen el negro opaco.
bool Mazmorra::preparar_cursor_rgba(unsigned char *rgba) const {
  std::vector<unsigned char> oscuro(64000, 0), claro(64000, 255);
  if (!rgba ||
      !motor::dibujar_fotograma(raton.data(), static_cast<unsigned int>(raton.size()), 0,
                                oscuro.data(), 0, 0, false) ||
      !motor::dibujar_fotograma(raton.data(), static_cast<unsigned int>(raton.size()), 0,
                                claro.data(), 0, 0, false))
    return false;
  for (unsigned int y = 0; y < 16; ++y)
    for (unsigned int x = 0; x < 16; ++x) {
      const auto p = y * 320 + x;
      auto *color = rgba + (y * 16 + x) * 4;
      for (unsigned int c = 0; c < 3; ++c)
        color[c] = static_cast<unsigned char>(fondo.paleta[oscuro[p] * 3u + c] * 255u / 63u);
      color[3] = oscuro[p] == claro[p] ? 255 : 0;
    }
  return true;
}

// BA70 contiene siete teclas por fila, separadas por un espacio y con tres acciones especiales.
unsigned char Mazmorra::tecla_nombre_partida(unsigned int x, unsigned int y) const {
  return x >= 8 && x <= 111 && y >= 15 && y <= 74 ? datos[0xba70 + ((y - 15) / 10) * 15 + x / 8]
                                                  : 0xff;
}
// D897/D89E: los índices con signo negativo no tienen glifo en la fuente original.
bool Mazmorra::admite_letra_partida(unsigned char letra) const {
  return letra >= 32 && letra < 127 && fuente.size() >= 2 && datos[0xb6b4 + letra] < 128 &&
         datos[0xb6b4 + letra] * 2u + 2u <= motor::palabra(fuente.data(), 0) * 16u;
}

// 8B6F: los fragmentos del pozo usan el mismo creador de entidades que las animaciones.
bool Mazmorra::crear_desde_menu(void *contexto, unsigned char *datos, unsigned short descriptor,
                                unsigned short &posicion) {
  auto &partida = *static_cast<Mazmorra *>(contexto);
  const motor::RecursosEntidades recursos{contexto,
                                          dimensiones_escena,
                                          referencia_escena,
                                          parrafo_escena,
                                          {contexto, reproducir_sonido}};
  const auto resultado = motor::crear_entidad(datos, partida.aleatorio, recursos, descriptor);
  posicion = resultado.creada ? resultado.posicion : 0x3510;
  return resultado.valido;
}

// DB34 y DE10: conserva la mitad superior/inferior elegida al abrir y divide x entre 32.
bool Mazmorra::entrada_menu(unsigned short x, unsigned short y, bool seleccionar, bool alternar) {
  if (en_presentacion()) {
    if (seleccionar || alternar)
      registrar_accion("Entrada de menú ignorada durante el cambio de pantalla");
    return true;
  }
  if (alternar) {
    if (menu_visible) {
      menu_visible = false;
      registrar_accion("Menú cerrado");
      return true;
    }
    if (datos[0x3595] == 0) {
      registrar_accion("Abrir menú: bloqueado por la secuencia original");
      return true;
    }
    registrar_accion("Menú abierto en (%u, %u)", x, y);
    motor::abrir_menu(datos, x, y);
    menu_visible = true;
  }
  if (!menu_visible)
    return seleccionar ? mover_a(x, y) : true;
  casilla_menu = x < 320 ? x / 32 : 9;
  motor::guardar(datos, 0xb60a, x);
  if (seleccionar) {
    const auto manejador = motor::palabra(datos, 0xb60c + casilla_menu * 2);
    const char *accion = manejador == 0xdfd3                          ? "inventario"
                         : manejador == 0xdffc || manejador == 0x563a ? "recoger"
                         : manejador == 0xe039 || manejador == 0x3b53 ? "manipular objetos"
                         : manejador == 0xe0e9 || manejador == 0x384a ? "examinar"
                         : manejador == 0xe010 || manejador == 0x32de ? "hablar"
                         : manejador == 0xe14a || manejador == 0x3143 ? "atacar"
                         : manejador == 0xe0fd || manejador == 0x2fca ? "usar objeto"
                         : manejador == 0xe160                        ? "información"
                         : manejador == 0xe126                        ? "partidas"
                                                                      : "elegir objeto o submenú";
    constexpr const char *manipulaciones[] = {"desbloquear",   "insertar",       "empujar o tirar",
                                              "comer o beber", "ponerse",        "lanzar",
                                              "dar",           "unir o combinar"};
    if (manejador == 0xe05b && casilla_menu < 8)
      accion = manipulaciones[casilla_menu];
    registrar_accion("Selección de menú: %s; casilla %u, identificador %04X, manejador %04X",
                     accion, casilla_menu + 1, motor::palabra(datos, 0xb620 + casilla_menu * 2),
                     manejador);
    if (manejador == 0xe19f || manejador == 0xe325 || manejador == 0xe793 || manejador == 0xe160) {
      solicitud_partidas = manejador == 0xe160   ? 4
                           : manejador == 0xe19f ? 1
                           : manejador == 0xe325 ? 2
                                                 : 3;
      menu_visible = false;
      registrar_accion("Partidas: abrir %s", solicitud_partidas == 1   ? "cargar"
                                             : solicitud_partidas == 2 ? "guardar"
                                             : solicitud_partidas == 3 ? "borrar"
                                                                       : "información");
      return true;
    }
    const auto resultado = motor::seleccionar_menu(datos, casilla_menu, {this, reproducir_sonido},
                                                   {this, crear_desde_menu, &aleatorio});
    if (resultado == motor::ResultadoMenu::error) {
      registrar_accion("Error al despachar la selección del menú");
      return false;
    }
    if (resultado == motor::ResultadoMenu::accion)
      menu_visible = false;
    registrar_accion("Resultado de selección: %s", resultado == motor::ResultadoMenu::accion
                                                       ? "acción despachada al guion original"
                                                   : resultado == motor::ResultadoMenu::pendiente
                                                       ? "acción pendiente de integrar"
                                                       : "submenú actualizado");
    registrar_estado();
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

// E718–E745: guarda únicamente el bloque de estado que utiliza el archivo original de DOS.
bool Mazmorra::guardar_partida(unsigned char *archivo, unsigned int capacidad, unsigned int ranura,
                               const char *nombre) const {
  return motor::escribir_partida(archivo, capacidad, ranura, nombre, datos);
}

// E27E y 4AF0/AL=1: reconstruye recursos en un estado temporal antes de sustituir la sesión viva.
bool Mazmorra::cargar_partida(const unsigned char *archivo, unsigned int longitud,
                              unsigned int ranura) {
  Mazmorra candidata;
  for (unsigned int i = 0; i < sizeof(datos); ++i)
    candidata.datos[i] = datos[i];
  if (!motor::leer_partida(archivo, longitud, ranura, candidata.datos))
    return false;
  const auto salida = motor::palabra(candidata.datos, 0x3598);
  if (salida != 0xffff && salida > 125)
    return false;
  const auto escena_guardada =
      motor::palabra(candidata.datos, static_cast<unsigned short>(0x7482 + salida * 2));
  bool integrada = false;
  for (const auto &vista : vistas_desarrollo)
    integrada = integrada || vista.escena == escena_guardada;
  if (!integrada)
    return false;
  candidata.carpeta = carpeta;
  candidata.aleatorio = aleatorio;
  candidata.personaje = personaje;
  candidata.modelo = modelo;
  candidata.raton = raton;
  candidata.iconos = iconos;
  candidata.fondo = fondo;
  for (unsigned int i = 0; i < sizeof(fondo_menu); ++i)
    candidata.fondo_menu[i] = fondo_menu[i];
  motor::guardar(candidata.datos, 0x8082, escena_guardada);
  motor::guardar(candidata.datos, 0x2c29, referencia_raton);
  const auto modelo_guardado = candidata.datos[0x358c];
  if (modelo_guardado < 254) {
    if (modelo_guardado >= 32 ||
        !cargar_modelo(&candidata, candidata.datos,
                       motor::palabra(candidata.datos, 0x0918 + modelo_guardado * 2), 0xfa00))
      return false;
    candidata.datos[0x9974] = modelo_guardado;
  }
  const auto referencia = modelo_guardado == 255   ? referencia_personaje
                          : modelo_guardado == 254 ? referencia_escena
                                                   : referencia_modelo;
  motor::guardar(candidata.datos, 0x2c4a, referencia);
  if (modelo_guardado != 255)
    candidata.datos[0x2c3f] = candidata.datos[0x2c44] = candidata.datos[0x2c49] = 2;
  // 513F–5143 omite 5145–5182 al cargar: conserva control, secuencias y entidades guardadas.
  if (!motor::preparar_vista_escena(candidata.datos) ||
      !candidata.cargar_habitacion_actual(salida, false, false))
    return false;
  unsigned short ancho = 0, alto = 0;
  if (modelo_guardado != 255 &&
      !dimensiones(&candidata, referencia, candidata.datos[0x2c2c] & 0x7f, ancho, alto))
    return false;
  if (modelo_guardado != 255) {
    motor::guardar(candidata.datos, 0x2c31, ancho);
    motor::guardar(candidata.datos, 0x2c33, alto);
  }
  motor::ImagenIndexada comprobacion{};
  if (!candidata.dibujar(comprobacion))
    return false;
  for (unsigned int i = 0; i < sizeof(datos); ++i)
    datos[i] = candidata.datos[i];
  aleatorio = candidata.aleatorio;
  escena.swap(candidata.escena);
  modelo.swap(candidata.modelo);
  mapa.swap(candidata.mapa);
  geometria.swap(candidata.geometria);
  fondo = candidata.fondo;
  menu_visible = false;
  animacion = paso_animacion = 0;
  paso_tras_camara = false;
  direccion = 0xffff;
  registro_preparado = false;
  solicitud_partidas = 0;
  if (!audio.conectar(datos) || !audio.cambiar_escena(escena_guardada))
    registrar_accion("Partida recuperada; no se ha podido restablecer el audio");
  registrar_accion("Partida cargada: ranura %u, %s (escena %u)", ranura + 1,
                   nombre_escena(escena_guardada), escena_guardada);
  registrar_estado();
  return true;
}

// 08C8–08DA: el ratón solo propone rutas cuando el guion original ha habilitado el movimiento.
bool Mazmorra::mover_a(unsigned short x, unsigned short y) {
  if (en_presentacion() || menu_visible || !datos[0x3594]) {
    registrar_accion("Caminar a (%u, %u): bloqueado por el menú, presentación o guion", x, y);
    return true;
  }
  if (x >= 320 || y >= 200)
    return true;
  registrar_accion("Caminar a (%u, %u)", x, y);
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
  const bool anterior = direccion != 0xffff && (direccion & bit);
  if (anterior != pulsada)
    registrar_accion("Flecha %s: %s%s",
                     bit == 8   ? "izquierda"
                     : bit == 4 ? "derecha"
                     : bit == 2 ? "arriba"
                                : "abajo",
                     pulsada ? "pulsada" : "soltada",
                     pulsada && !datos[0x3594] ? " (movimiento bloqueado por el guion)" : "");
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
  if (!salida_integrada(escena_actual(), salida))
    return false;
  unsigned short ancho = 32, alto = 48;
  const auto modelo_entrada = datos[0x85ac + salida * 14 + 13];
  if (modelo_entrada != 0xff &&
      (!cargar_modelo(this, datos, motor::palabra(datos, 0x0918 + modelo_entrada * 2), 0xfa00) ||
       !dimensiones(this, referencia_modelo, datos[0x85ac + salida * 14 + 1] & 0x7f, ancho, alto)))
    return false;
  if (!motor::preparar_transicion_escena(datos, salida, referencia_personaje, referencia_modelo,
                                         ancho, alto) ||
      !motor::preparar_vista_escena(datos))
    return false;
  return cargar_habitacion_actual(salida);
}

// 4E98–5182: carga recursos de la escena y reconstruye sus entidades persistentes.
bool Mazmorra::cargar_habitacion_actual(unsigned short salida, bool actualizar_sonido,
                                        bool reconstruir_entidades) {
  const auto actual = escena_actual();
  const char *nombre_banco = actual == 30 ? "EDGE" : nullptr;
  for (const auto &vista : vistas_desarrollo)
    if (vista.escena == actual)
      nombre_banco = vista.banco;
  // 784E: MAP y SPR tienen nombres independientes; SNOWAST1 usa SNOWAST3.SPR.
  const auto nombre_recurso = [&](unsigned int indice) {
    const auto referencia =
        motor::palabra(datos, static_cast<unsigned short>(0x784e + actual * 8 + indice * 2));
    std::string nombre;
    for (unsigned int i = 0; i < 13; ++i) {
      const auto caracter = datos[static_cast<unsigned short>(referencia + i)];
      if (!caracter)
        break;
      nombre.push_back(static_cast<char>(caracter));
    }
    return nombre;
  };
  if (!nombre_banco || !cargar_banco(nombre_recurso(1), escena) ||
      !cargar_banco(nombre_recurso(0), mapa) || !cargar_fondo_visible())
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
  if (actualizar_sonido && !audio.cambiar_escena(actual))
    return false;
  const motor::RecursosEntidades recursos{
      this,           dimensiones_escena,        referencia_escena,
      parrafo_escena, {this, reproducir_sonido}, &fase_probador};
  if (reconstruir_entidades) {
    if (!motor::cargar_entidades_escena(datos, aleatorio, recursos))
      return false;
  } else {
    // E28B–E2B3: sustituye los segmentos DOS, conservando todos los registros guardados.
    if (motor::palabra(datos, 0x3588) >= 70 || motor::palabra(datos, 0x358a) > 10)
      return false;
    for (unsigned int i = 0; i < 68; ++i)
      motor::guardar(datos, static_cast<unsigned short>(0x2c4c + i * 33 + 31), referencia_escena);
    for (unsigned int i = 0; i < motor::palabra(datos, 0x358a); ++i) {
      const auto registro = static_cast<unsigned short>(0x3510 + i * 12);
      motor::guardar(
          datos, registro + 8,
          static_cast<unsigned short>(referencia_escena + motor::palabra(datos, registro + 10)));
    }
  }
  motor::guardar(datos, 0x0358, 0);
  motor::guardar(datos, 0x0616, 0);
  menu_visible = false;
  direccion = 0xffff;
  registrar_accion("Escena cargada: %s (índice %u, recurso %s, salida %u)", nombre_escena(actual),
                   actual, nombre_banco, salida);
  registrar_estado();
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
  return iniciar_vista(ruta, imagen_fondo, 2);
}

// Carga entradas originales a corredor, caída o lago; la navegación de prueba no altera una
// partida.
bool Mazmorra::iniciar_vista(const std::string &ruta, const motor::ImagenIndexada &imagen_fondo,
                             unsigned short escena_vista) {
  if (escena_vista >= cantidad_vistas || !iniciar(ruta, imagen_fondo))
    return false;
  if (!escena_vista)
    return true;
  const auto &vista = vistas_desarrollo[escena_vista];
  motor::guardar(datos, 0x8082, vista.origen);
  return cambiar_habitacion(vista.salida);
}

// Registra solo los cambios de lista y de habilitación de movimiento, conservando todos los datos.
void Mazmorra::registrar_estado() {
  bool cambiado = !registro_preparado;
  for (unsigned int i = 0; i < 10; ++i) {
    const auto valor = motor::palabra(datos, static_cast<unsigned short>(0x5225 + i * 2));
    cambiado = cambiado || valor != inventario_registrado[i];
    inventario_registrado[i] = valor;
  }
  if (cambiado) {
    std::string lista;
    for (unsigned int i = 0; i < 10 && inventario_registrado[i] != 0xffff; ++i) {
      if (!lista.empty())
        lista += ", ";
      const auto id = inventario_registrado[i];
      lista += id == 1                                              ? "llave"
               : id == 3                                            ? "clip"
               : id == 6                                            ? "pecera"
               : id == 7                                            ? "agua"
               : id == 17                                           ? "gusano"
               : id == 18                                           ? "concha"
               : id == 23                                           ? "picana"
               : ((id >= 27 && id <= 35) || (id >= 38 && id <= 46)) ? "piedra"
               : id == 49                                           ? "algas"
               : id == 55                                           ? "cuerda"
                                                                    : "objeto";
      lista += " [" + std::to_string(id) + "]";
    }
    registrar_accion("Inventario: %s", lista.empty() ? "vacío" : lista.c_str());
  }
  if (!registro_preparado || control_registrado != datos[0x3594])
    registrar_accion("Movimiento de Brad: %s",
                     datos[0x3594] ? "habilitado" : "bloqueado por el guion");
  control_registrado = datos[0x3594];
  registro_preparado = true;
}

// Lee el canal principal de Brad; no altera sus controles ni sus contadores.
bool Mazmorra::secuencia_personaje_activa() const {
  return motor::palabra(datos, 0x2c3b) != 0;
}
// Conserva los valores de 16 bits de los objetos, incluidas sus posiciones fuera de pantalla.
bool Mazmorra::posicion_objeto(unsigned short id, unsigned short &x, unsigned short &y,
                               unsigned short &alto) const {
  for (unsigned short p = id == 0 ? 0x2c2b : 0x2c4c; p < 0x3500;
       p = static_cast<unsigned short>(p + 33)) {
    if (datos[p] && motor::palabra(datos, p + 14) == id) {
      x = motor::palabra(datos, p + 2);
      y = motor::palabra(datos, p + 4);
      alto = motor::palabra(datos, p + 8);
      return true;
    }
  }
  return false;
}
// Busca en las diez entradas originales sin introducir una selección alternativa.
int Mazmorra::casilla_objeto(unsigned short id) const {
  for (unsigned short i = 0; i < 10; ++i)
    if (motor::palabra(datos, 0xb620 + i * 2) == id + 0x1a)
      return i;
  return -1;
}

// 88BC: consulta de diez identificadores; conserva orden, contenido y resto del segmento.
bool Mazmorra::conserva_objeto(unsigned short identificador) const {
  return motor::contiene_identificador(datos, identificador);
}
// El estado uno corresponde al tiburón visible que admite la acción 2FCA.
bool Mazmorra::tiburon_activo() const {
  return datos[0x2df9] == 1;
}
// 040C activa 11C6 únicamente cuando la descarga coincide con su intervalo original.
bool Mazmorra::tiburon_aturdido() const {
  return motor::palabra(datos, 0x2e09) == 0x11c6;
}

// Comprobación de una fase del personaje, sin forzar fotogramas ni detener sus entidades.
bool Mazmorra::golpe_estatua() const {
  return motor::palabra(datos, 0x2c3b) == 0x0f94;
}
