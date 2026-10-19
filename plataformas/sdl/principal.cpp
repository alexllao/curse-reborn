#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <cstdio>
#include <string>
#include <utility>
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif
#include "../../motor/recursos/titulo.hpp"
#include "../../motor/recursos/escena.hpp"
#include "../../motor/video/sprites.hpp"
#include "../../motor/video/fundido.hpp"
#include "mazmorra.hpp"
#include "introduccion.hpp"
#include "registro.hpp"
#include "filtros.hpp"
#include "contador_fps.hpp"
#include "tiempo_juego.hpp"
#include "comprobar_tiempo.hpp"
#include "vistas.hpp"
#include "comprobar_cuevas.hpp"
#include "comprobar_catapulta.hpp"
#include "comprobar_final_cuevas.hpp"
#include "comprobar_campo.hpp"
#include "comprobar_asaltante.hpp"
#include "comprobar_poblacion.hpp"
#include "partidas.hpp"
#include "comprobar_partidas.hpp"
#include "marcos.hpp"
#include "decorados.hpp"
#include "cursor.hpp"

namespace {
// Límite de presentación independiente del reloj lógico y de los temporizadores de audio.
constexpr Uint64 intervalo_presentacion_ns = 1000000000ull / 70u;

// Relaciona el índice DOS con las vistas disponibles, cuyo orden difiere del de los recursos.
int vista_de_escena(unsigned short escena) {
  for (unsigned int i = 0; i < cantidad_vistas; ++i)
    if (vistas_desarrollo[i].escena == escena)
      return static_cast<int>(i) + 3;
  return -1;
}

// Interpreta como UTF-8 los mensajes propios, también al heredar una consola OEM de Windows.
void configurar_consola() {
#ifdef _WIN32
  SetConsoleOutputCP(CP_UTF8);
  SetConsoleCP(CP_UTF8);
#endif
}

motor::ImagenTitulo imagen;
unsigned char planos[262144];
unsigned char pixeles[64000 * 4];
unsigned char tira[6400];
unsigned char paleta_visible[768];
Mazmorra mazmorra;
Introduccion introduccion;

struct Presentacion {
  SDL_Window *ventana = nullptr;
  SDL_Renderer *dibujante = nullptr;
  SDL_Texture *textura = nullptr;
  FiltrosPantalla filtros;
  MarcosPantalla marcos;
  DecoradosAmpliados decorados;
  CursorOriginal cursor;
  // Libera los objetos de la plataforma al cerrar o al fallar la inicialización.
  ~Presentacion() {
    mazmorra.cerrar_audio();
    filtros.cerrar();
    marcos.cerrar();
    decorados.cerrar();
    cursor.cerrar();
    if (textura)
      SDL_DestroyTexture(textura);
    if (dibujante)
      SDL_DestroyRenderer(dibujante);
    if (ventana)
      SDL_DestroyWindow(ventana);
    SDL_Quit();
  }
};

// Añade el diagnóstico SDL al error que impide continuar con la presentación.
int fallo(const char *mensaje) {
  std::fprintf(stderr, "%s: %s\n", mensaje, SDL_GetError());
  return 1;
}
} // namespace

// Reconstruye Core o el título desde sus archivos, usando el mismo recorrido gráfico original.
bool cargar_imagen(const std::string &carpeta, const char *nombre) {
  size_t longitud = 0, longitud_paleta = 0;
  void *comprimido = SDL_LoadFile((carpeta + "/" + nombre + ".DAT").c_str(), &longitud);
  if (!comprimido)
    return false;
  void *paleta = SDL_LoadFile((carpeta + "/" + nombre + ".PAL").c_str(), &longitud_paleta);
  if (!paleta) {
    SDL_free(comprimido);
    return false;
  }
  const bool preparado =
      longitud <= 0xffffffffu && longitud_paleta <= 0xffffffffu &&
      motor::preparar_titulo(static_cast<const unsigned char *>(comprimido),
                             static_cast<unsigned int>(longitud),
                             static_cast<const unsigned char *>(paleta),
                             static_cast<unsigned int>(longitud_paleta), imagen, planos);
  SDL_free(paleta);
  SDL_free(comprimido);
  return preparado;
}

// 06B2 y 50D2: prepara el fondo y delega la entrada, los objetos y sus guiones en el adaptador.
bool cargar_mazmorra(const std::string &carpeta, unsigned short escena_vista = 0) {
  // 0679–069A conserva los 64 colores altos de SPRITES.PAL al cargar fondos MAP.
  size_t longitud_paleta = 0;
  void *paleta = SDL_LoadFile((carpeta + "/SPRITES.PAL").c_str(), &longitud_paleta);
  if (!paleta)
    return false;
  if (longitud_paleta != 768) {
    SDL_free(paleta);
    return false;
  }
  const auto *colores = static_cast<const unsigned char *>(paleta);
  for (unsigned int i = 576; i < 768; ++i) {
    if (colores[i] > 63) {
      SDL_free(paleta);
      return false;
    }
    imagen.paleta[i] = colores[i];
  }
  SDL_free(paleta);
  size_t longitud = 0;
  void *mapa = SDL_LoadFile((carpeta + "/DUNGEON.MAP").c_str(), &longitud);
  if (!mapa)
    return false;
  // DS:7648 contiene diez tiras consecutivas para la mazmorra.
  const unsigned char seleccion[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  const bool preparada = longitud <= 0xffffffffu &&
                         motor::preparar_fondo_mapa(static_cast<const unsigned char *>(mapa),
                                                    static_cast<unsigned int>(longitud), seleccion,
                                                    imagen, planos, tira);
  SDL_free(mapa);
  return preparada && mazmorra.iniciar_vista(carpeta, imagen, escena_vista) &&
         mazmorra.dibujar(imagen);
}

// Adapta la paleta VGA a RGBA; los filtros opcionales se aplican después de la composición.
bool actualizar_textura(Presentacion &presentacion, bool con_decorado = false) {
  // Adaptación de color de seis bits a RGBA; no sustituye la lógica original del juego.
  for (unsigned int i = 0; i < 64000; ++i) {
    const auto color = imagen.indices[i] * 3u;
    for (unsigned int componente = 0; componente < 3; ++componente) {
      pixeles[i * 4 + componente] =
          static_cast<unsigned char>(paleta_visible[color + componente] * 255u / 63u);
    }
    pixeles[i * 4 + 3] = 255;
  }
  if (con_decorado) {
    unsigned int luz = 0;
    for (const auto componente : paleta_visible)
      luz = std::max(luz, static_cast<unsigned int>(componente));
    if (!presentacion.decorados.componer(pixeles, mazmorra.escena_actual(), mazmorra.franja_menu(),
                                         luz))
      return false;
  }
  return presentacion.filtros.actualizar(presentacion.textura, pixeles);
}

// Recorre las vistas de desarrollo; cada entrada a la mazmorra prepara una sesión nueva sin
// guardar.
bool cambiar_pantalla(const std::string &carpeta, int pantalla, Presentacion &presentacion) {
  if (pantalla < 0 || pantalla >= static_cast<int>(cantidad_vistas) + 3)
    return false;
  const char *nombre = pantalla == 0   ? "Core"
                       : pantalla == 1 ? "título"
                       : pantalla == 2 ? "introducción"
                                       : vistas_desarrollo[pantalla - 3].nombre;
  registrar_accion("Cambio de vista: %s", nombre);
  mazmorra.desconectar_audio();
  const bool cargada =
      pantalla >= 3   ? cargar_mazmorra(carpeta, static_cast<unsigned short>(pantalla - 3))
      : pantalla == 2 ? (cargar_imagen(carpeta, "TITLE") && introduccion.iniciar(carpeta, imagen))
                      : cargar_imagen(carpeta, pantalla == 0 ? "CORE" : "TITLE");
  if (!cargada || (pantalla < 3 && !mazmorra.audio_presentacion()))
    return false;
  for (unsigned int i = 0; i < 768; ++i)
    paleta_visible[i] = imagen.paleta[i];
  const std::string titulo =
      std::string("Curse Reborn — ") +
      (pantalla >= 3 ? nombre_numerado_escena(mazmorra.escena_actual()) : nombre);
  return actualizar_textura(presentacion) &&
         SDL_SetWindowTitle(presentacion.ventana, titulo.c_str());
}

// Reinicio opcional del monitor: vuelve a Core sin escribir partidas y conserva los ajustes
// externos.
bool reiniciar_presentacion(const std::string &carpeta, Presentacion &presentacion,
                            PartidasSDL &partidas) {
  for (const auto bit : {1, 2, 4, 8})
    mazmorra.tecla_direccion(static_cast<unsigned short>(bit), false);
  mazmorra.tomar_solicitud_partidas();
  if (partidas.abierta())
    partidas.cerrar(presentacion.ventana);
  return mazmorra.reiniciar_audio() && cambiar_pantalla(carpeta, 0, presentacion);
}

// Verifica que Frank y el pozo conservan su imagen original con los decorados aparcados.
bool comprobar_decorados(const std::string &carpeta, const std::string &prefijo) {
  Presentacion presentacion;
  if (!SDL_Init(SDL_INIT_VIDEO) ||
      !SDL_CreateWindowAndRenderer("Escenas sin decorados", 960, 600, SDL_WINDOW_HIDDEN,
                                   &presentacion.ventana, &presentacion.dibujante) ||
      !SDL_SetRenderLogicalPresentation(presentacion.dibujante, 320, 200,
                                        SDL_LOGICAL_PRESENTATION_LETTERBOX))
    return false;
  presentacion.textura = SDL_CreateTexture(presentacion.dibujante, SDL_PIXELFORMAT_RGBA32,
                                           SDL_TEXTUREACCESS_STATIC, 320, 200);
  if (!presentacion.textura)
    return false;
  for (unsigned short escena = 0; escena < 78; ++escena)
    if (DecoradosAmpliados::buscar(escena))
      return SDL_SetError("El catálogo conserva un decorado que debía estar aparcado");
  for (int vista : {21, 17}) {
    if (!cambiar_pantalla(carpeta, vista, presentacion))
      return false;
    const auto escena = mazmorra.escena_actual();
    const std::vector<unsigned char> original(pixeles, pixeles + sizeof(pixeles));
    presentacion.decorados.alternar();
    for (unsigned int modo = 0; modo < 6; ++modo) {
      if (!actualizar_textura(presentacion, true) ||
          !std::equal(original.begin(), original.end(), pixeles) ||
          !SDL_RenderTexture(presentacion.dibujante,
                             presentacion.filtros.textura_visible(presentacion.textura), nullptr,
                             nullptr))
        return SDL_SetError("Un decorado aparcado modifica la imagen original");
      SDL_Surface *captura = SDL_RenderReadPixels(presentacion.dibujante, nullptr);
      const bool guardada =
          captura && SDL_SavePNG(captura, (prefijo + "." + std::to_string(escena) + "." +
                                           std::to_string(modo) + ".png")
                                              .c_str());
      SDL_DestroySurface(captura);
      if (!guardada ||
          !presentacion.filtros.alternar(presentacion.dibujante, presentacion.textura, pixeles))
        return false;
    }
    presentacion.decorados.alternar();
  }
  registrar_accion("Decorados aparcados: catálogo vacío; Frank y pozo originales con seis filtros");
  return true;
}

// Verifica la mano original, el ajuste de ventana y el relevo de cursor en los paneles DOS.
bool comprobar_cursor(const std::string &carpeta, const std::string &destino) {
  Presentacion presentacion;
  if (!SDL_Init(SDL_INIT_VIDEO) ||
      !SDL_CreateWindowAndRenderer("Cursor original", 960, 600, SDL_WINDOW_HIDDEN,
                                   &presentacion.ventana, &presentacion.dibujante) ||
      !SDL_SetRenderLogicalPresentation(presentacion.dibujante, 320, 200,
                                        SDL_LOGICAL_PRESENTATION_LETTERBOX))
    return false;
  presentacion.textura = SDL_CreateTexture(presentacion.dibujante, SDL_PIXELFORMAT_RGBA32,
                                           SDL_TEXTUREACCESS_STATIC, 320, 200);
  if (!presentacion.textura || !cambiar_pantalla(carpeta, 3, presentacion))
    return false;
  for (int modo = 0; modo < 2; ++modo) {
    if (modo == 1) {
      presentacion.marcos.alternar(1000000000ull);
      presentacion.marcos.alternar(1300000000ull);
      SDL_Event zoom{};
      zoom.type = SDL_EVENT_KEY_DOWN;
      zoom.key.mod = SDL_KMOD_CTRL;
      zoom.key.key = SDLK_MINUS;
      for (int paso = 0; paso < 3; ++paso)
        presentacion.marcos.evento_zoom(zoom);
    }
    for (const auto &tamano : {std::pair<int, int>{640, 400}, {1280, 720}, {720, 1280}}) {
      if (!SDL_SetWindowSize(presentacion.ventana, tamano.first, tamano.second) ||
          !SDL_SyncWindow(presentacion.ventana) ||
          !presentacion.marcos.preparar_encuadre(presentacion.dibujante, 1600000000ull))
        return false;
      SDL_FPoint centro;
      if (!SDL_RenderCoordinatesToWindow(presentacion.dibujante, 160, 100, &centro.x, &centro.y) ||
          !presentacion.cursor.actualizar_puntero(presentacion.ventana, presentacion.dibujante,
                                                  mazmorra, true, false, &centro) ||
          SDL_GetCursor() == SDL_GetDefaultCursor() || !SDL_CursorVisible() ||
          !presentacion.cursor.actualizar_puntero(presentacion.ventana, presentacion.dibujante,
                                                  mazmorra, true, true, &centro) ||
          SDL_CursorVisible())
        return false;
      for (const SDL_FPoint &fuera : {SDL_FPoint{-1, 100}, {320, 100}, {160, -1}, {160, 200}}) {
        SDL_FPoint ventana;
        if (!SDL_RenderCoordinatesToWindow(presentacion.dibujante, fuera.x, fuera.y, &ventana.x,
                                           &ventana.y) ||
            !presentacion.cursor.actualizar_puntero(presentacion.ventana, presentacion.dibujante,
                                                    mazmorra, true, true, &ventana) ||
            SDL_GetCursor() != SDL_GetDefaultCursor() || !SDL_CursorVisible())
          return SDL_SetError(
              "El cursor del juego aparece fuera de la imagen o sigue oculto al salir del panel");
      }
      for (const SDL_FPoint &dentro : {SDL_FPoint{0.25f, 0.25f}, {319.75f, 199.75f}}) {
        SDL_FPoint ventana;
        if (!SDL_RenderCoordinatesToWindow(presentacion.dibujante, dentro.x, dentro.y, &ventana.x,
                                           &ventana.y) ||
            !presentacion.cursor.actualizar_puntero(presentacion.ventana, presentacion.dibujante,
                                                    mazmorra, true, false, &ventana) ||
            SDL_GetCursor() == SDL_GetDefaultCursor() || !SDL_CursorVisible())
          return SDL_SetError("No se recupera la mano al volver a la imagen jugable");
      }
      if (!presentacion.cursor.actualizar_puntero(presentacion.ventana, presentacion.dibujante,
                                                  mazmorra, true, true, nullptr) ||
          SDL_GetCursor() != SDL_GetDefaultCursor() || !SDL_CursorVisible())
        return SDL_SetError("Salir de la ventana no restaura la flecha normal");
    }
  }
  std::vector<unsigned char> rgba(16u * 16u * 4u);
  if (!mazmorra.preparar_cursor_rgba(rgba.data()))
    return false;
  SDL_Surface *imagen_cursor =
      SDL_CreateSurfaceFrom(16, 16, SDL_PIXELFORMAT_RGBA32, rgba.data(), 16 * 4);
  const bool guardada = imagen_cursor && SDL_SavePNG(imagen_cursor, destino.c_str());
  SDL_DestroySurface(imagen_cursor);
  if (!guardada ||
      !presentacion.cursor.actualizar(presentacion.ventana, presentacion.dibujante, mazmorra, false,
                                      false) ||
      SDL_GetCursor() != SDL_GetDefaultCursor())
    return false;
  registrar_accion("Cursor: tres formatos con y sin zoom; cuatro bordes, salida de ventana, "
                   "regreso de la mano y relevo del panel comprobados");
  return true;
}

// Conserva el diagnóstico antes de que la limpieza de SDL elimine su último error.
bool fallo_monitor(const char *motivo) {
  std::fprintf(stderr, "%s: %s\n", motivo, SDL_GetError());
  return false;
}

// Captura RGBA física y devuelve el estado lógico, sin modificar la imagen ya dibujada.
SDL_Surface *capturar_monitor(SDL_Renderer *dibujante) {
  int ancho = 0, alto = 0;
  SDL_RendererLogicalPresentation presentacion;
  SDL_Rect viewport;
  const bool viewport_fijado = SDL_RenderViewportSet(dibujante);
  float sx = 1, sy = 1;
  if (!SDL_GetRenderLogicalPresentation(dibujante, &ancho, &alto, &presentacion) ||
      !SDL_GetRenderViewport(dibujante, &viewport) || !SDL_GetRenderScale(dibujante, &sx, &sy) ||
      !SDL_SetRenderLogicalPresentation(dibujante, 0, 0, SDL_LOGICAL_PRESENTATION_DISABLED) ||
      !SDL_SetRenderViewport(dibujante, nullptr) || !SDL_SetRenderScale(dibujante, 1, 1))
    return nullptr;
  SDL_Surface *captura = SDL_RenderReadPixels(dibujante, nullptr);
  const bool restaurado = SDL_SetRenderLogicalPresentation(dibujante, ancho, alto, presentacion) &&
                          SDL_SetRenderScale(dibujante, sx, sy) &&
                          SDL_SetRenderViewport(dibujante, viewport_fijado ? &viewport : nullptr);
  SDL_Surface *rgba =
      captura && restaurado ? SDL_ConvertSurface(captura, SDL_PIXELFORMAT_RGBA32) : nullptr;
  SDL_DestroySurface(captura);
  return rgba;
}

// Contrasta clics visibles, piloto, apagado, reinicio único y conservación del encuadre.
bool comprobar_reinicio(const std::string &carpeta, const std::string &prefijo) {
  Presentacion p;
  if (!SDL_Init(SDL_INIT_VIDEO) ||
      !SDL_CreateWindowAndRenderer("Reinicio del monitor", 1280, 720,
                                   SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE, &p.ventana,
                                   &p.dibujante))
    return fallo_monitor("No se ha podido crear la prueba de reinicio");
  p.textura =
      SDL_CreateTexture(p.dibujante, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, 320, 200);
  if (!p.textura)
    return false;
  p.marcos.alternar(1000000000ull);
  p.marcos.alternar(1300000000ull);
  PartidasSDL partidas;
  unsigned int caso = 0;
  for (const auto &tamano : {std::pair<int, int>{1280, 720}, {3840, 1080}, {720, 1280}}) {
    const Uint64 inicio = 2000000000ull + caso++ * 2000000000ull;
    SDL_Event zoom{};
    zoom.type = SDL_EVENT_KEY_DOWN;
    zoom.key.mod = SDL_KMOD_CTRL;
    zoom.key.key = SDLK_0;
    p.marcos.evento_zoom(zoom);
    zoom.key.key = SDLK_MINUS;
    for (int paso = 0; paso < (tamano.first == 720 ? 5 : 3); ++paso)
      p.marcos.evento_zoom(zoom);
    if (!SDL_SetWindowSize(p.ventana, tamano.first, tamano.second) || !SDL_SyncWindow(p.ventana) ||
        !p.marcos.preparar_encuadre(p.dibujante, inicio) || !cambiar_pantalla(carpeta, 3, p))
      return fallo_monitor("No se ha podido preparar la partida de reinicio");
    SDL_FRect boton, piloto, antes;
    int ancho = 0, alto = 0, ventana_ancho = 0, ventana_alto = 0;
    if (!p.marcos.localizar_controles(p.dibujante, boton, piloto) ||
        !MarcosPantalla::area_juego(p.dibujante, antes) ||
        !SDL_GetRenderOutputSize(p.dibujante, &ancho, &alto) ||
        !SDL_GetWindowSize(p.ventana, &ventana_ancho, &ventana_alto))
      return false;
    // La captura comparte el dibujo del juego y de la carcasa, sin tocar los recursos originales.
    const auto capturar = [&](Uint64 reloj, const char *fase, bool rojo, bool apagado,
                              bool destello) {
      const auto destino = p.marcos.destino_juego(reloj);
      if (!SDL_SetRenderDrawColor(p.dibujante, 0, 0, 0, 255) || !SDL_RenderClear(p.dibujante) ||
          (destino.h > 0 && !SDL_RenderTexture(p.dibujante, p.textura, nullptr, &destino)) ||
          !p.marcos.dibujar(p.dibujante, "escena_0", reloj))
        return false;
      SDL_Surface *imagen_fase = capturar_monitor(p.dibujante);
      if (!imagen_fase)
        return false;
      Uint8 r = 0, g = 0, b = 0, alpha = 0;
      bool correcto = SDL_ReadSurfacePixel(imagen_fase, int(piloto.x + piloto.w / 2),
                                           int(piloto.y + piloto.h / 2), &r, &g, &b, &alpha) &&
                      (rojo ? r > g : g > r);
      if (apagado || destello) {
        correcto = correcto && SDL_ReadSurfacePixel(imagen_fase, int(antes.x + antes.w / 2),
                                                    int(antes.y + antes.h / 2), &r, &g, &b, &alpha);
        correcto =
            correcto && (apagado ? r == 0 && g == 0 && b == 0 : r > 150 && g > 150 && b > 150);
      }
      const auto ruta = prefijo + "." + std::to_string(tamano.first) + "x" +
                        std::to_string(tamano.second) + "." + fase + ".png";
      correcto = correcto && SDL_SavePNG(imagen_fase, ruta.c_str());
      SDL_DestroySurface(imagen_fase);
      return correcto;
    };
    SDL_Event clic{};
    clic.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
    clic.button.windowID = SDL_GetWindowID(p.ventana);
    clic.button.button = SDL_BUTTON_LEFT;
    clic.button.x = (antes.x + antes.w / 2) * ventana_ancho / ancho;
    clic.button.y = (antes.y + antes.h / 2) * ventana_alto / alto;
    if (p.marcos.evento_boton(p.ventana, p.dibujante, clic, inicio) ||
        !capturar(inicio, "verde", false, false, false))
      return fallo_monitor("El botón responde fuera de su zona o el piloto inicial no es verde");
    clic.button.x = (boton.x + boton.w / 2) * ventana_ancho / ancho;
    clic.button.y = (boton.y + boton.h / 2) * ventana_alto / alto;
    clic.button.button = SDL_BUTTON_RIGHT;
    if (p.marcos.evento_boton(p.ventana, p.dibujante, clic, inicio))
      return fallo_monitor("El botón responde al clic derecho");
    clic.button.button = SDL_BUTTON_LEFT;
    partidas.abrir(4, p.ventana);
    if (!p.marcos.evento_boton(p.ventana, p.dibujante, clic, inicio) || !p.marcos.reiniciando() ||
        !p.marcos.evento_boton(p.ventana, p.dibujante, clic, inicio + 100000000ull) ||
        !capturar(inicio + 50000000ull, "cierre", true, false, false) ||
        !capturar(inicio + 180000000ull, "linea", true, false, true) ||
        !capturar(inicio + 400000000ull, "negro", true, true, false) ||
        p.marcos.tomar_reinicio(inicio + 699000000ull) ||
        p.marcos.tomar_reinicio(inicio + 700000000ull) ||
        p.marcos.tomar_reinicio(inicio + 5000000000ull) || !p.marcos.reiniciando() ||
        !capturar(inicio + 5000000000ull, "apagado_persistente", true, true, false))
      return fallo_monitor("No coinciden las fases del apagado o se enciende sin pulsar");
    const auto primero = inicio + 6000000000ull;
    if (!p.marcos.evento_boton(p.ventana, p.dibujante, clic, primero) ||
        !p.marcos.evento_boton(p.ventana, p.dibujante, clic, primero + 1500000000ull) ||
        p.marcos.destino_juego(primero + 1510000000ull).h != 0 ||
        p.marcos.tomar_reinicio(primero + 2300000000ull) ||
        !capturar(primero + 2300000000ull, "arranque_interrumpido", true, true, false))
      return fallo_monitor("Apagar durante BIOS deja ver la partida o vuelve a encenderse");
    const auto encendido = primero + 3000000000ull;
    if (!p.marcos.evento_boton(p.ventana, p.dibujante, clic, encendido) ||
        !capturar(encendido + 1200000000ull, "bios_memoria", false, false, false) ||
        !capturar(encendido + 2800000000ull, "bios_disco", false, false, false) ||
        !capturar(encendido + 4600000000ull, "dos", false, false, false) ||
        !capturar(encendido + 6200000000ull, "carga", false, false, false) ||
        p.marcos.tomar_reinicio(encendido + 6999000000ull) ||
        !p.marcos.tomar_reinicio(encendido + 7000000000ull) ||
        p.marcos.tomar_reinicio(encendido + 8000000000ull) || p.marcos.reiniciando())
      return fallo_monitor("El arranque no respeta BIOS, DOS y carga completos o se repite");
    if (!reiniciar_presentacion(carpeta, p, partidas) || partidas.abierta() ||
        std::string(SDL_GetWindowTitle(p.ventana)).find("Core") == std::string::npos ||
        !capturar(encendido + 7000000000ull, "reiniciado", false, false, false))
      return fallo_monitor("El reinicio no vuelve a Core y cierra el panel");
    SDL_FRect despues;
    if (!MarcosPantalla::area_juego(p.dibujante, despues) || despues.x != antes.x ||
        despues.y != antes.y || despues.w != antes.w || despues.h != antes.h)
      return fallo_monitor("El reinicio modifica el zoom elegido");
  }
  Audio prueba_audio;
  bool sonido = prueba_audio.abrir(carpeta) && prueba_audio.ajustar_volumen(-40) &&
                prueba_audio.alternar_dispositivo() && prueba_audio.alternar_silencio() &&
                prueba_audio.alternar_efectos();
  const auto perfil = prueba_audio.perfil_musical();
  sonido = sonido && prueba_audio.pausar_reinicio() && prueba_audio.reiniciar() &&
           prueba_audio.nivel_volumen() == 60 && prueba_audio.perfil_musical() == perfil;
  prueba_audio.cerrar();
  if (!sonido)
    return fallo_monitor("El reinicio no conserva la selección de audio o el volumen");
  registrar_accion(
      "Reinicio del monitor: tres formatos, clic izquierdo, piloto rojo/verde, línea CRT, pausa "
      "negra, una solicitud, vuelta a Core, cierre de panel y preferencias de audio comprobados");
  return true;
}

// Revisa los cuatro lados del bisel para detectar rendijas de habitación o restos de color.
bool comprobar_bisel_monitor(const SDL_Surface *captura, const SDL_FRect &juego) {
  const auto *rgba = static_cast<const unsigned char *>(captura->pixels);
  const auto negro = [&](int x, int y) {
    if (x < 0 || y < 0 || x >= captura->w || y >= captura->h)
      return true;
    const auto *p = rgba + y * captura->pitch + x * 4;
    return p[0] == 0 && p[1] == 0 && p[2] == 0;
  };
  const int izquierda = int(std::floor(juego.x)) - 1, derecha = int(std::ceil(juego.x + juego.w));
  const int arriba = int(std::floor(juego.y)) - 1, abajo = int(std::ceil(juego.y + juego.h));
  for (int y = arriba; y <= abajo; ++y)
    if (!negro(izquierda, y) || !negro(derecha, y))
      return false;
  for (int x = izquierda; x <= derecha; ++x)
    if (!negro(x, arriba) || !negro(x, abajo))
      return false;
  return true;
}

// Verifica tamaño máximo y píxeles del juego sin cambios, incluido 32:9 y ventana vertical.
bool comprobar_monitor(const std::string &carpeta, const std::string &prefijo) {
  Presentacion p;
  if (!SDL_Init(SDL_INIT_VIDEO) ||
      !SDL_CreateWindowAndRenderer("Monitor de PC 286", 1280, 720,
                                   SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE, &p.ventana,
                                   &p.dibujante))
    return fallo_monitor("No se ha podido comprobar el monitor");
  p.textura =
      SDL_CreateTexture(p.dibujante, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, 320, 200);
  if (!p.textura || !SDL_SetTextureScaleMode(p.textura, SDL_SCALEMODE_NEAREST) ||
      !cargar_imagen(carpeta, "TITLE"))
    return fallo_monitor("No se ha podido comprobar el monitor");
  for (unsigned int i = 0; i < 768; ++i)
    paleta_visible[i] = imagen.paleta[i];
  if (!actualizar_textura(p))
    return fallo_monitor("No se ha podido comprobar el monitor");
  constexpr Uint64 inicio = 1000000000ull;
  p.marcos.alternar(inicio);
  p.marcos.alternar(inicio + 300000000ull);
  const Uint64 ahora = inicio + 600000000ull;
  for (const auto &tamano : {std::pair<int, int>{1280, 720},
                             {720, 1280},
                             {960, 600},
                             {3440, 1440},
                             {3840, 1080},
                             {1279, 721}}) {
    for (const int zoom : {100, 70, 50}) {
      SDL_Event ajuste{};
      ajuste.type = SDL_EVENT_KEY_DOWN;
      ajuste.key.mod = SDL_KMOD_CTRL;
      ajuste.key.key = SDLK_0;
      if (!p.marcos.evento_zoom(ajuste))
        return fallo_monitor("No se reconoce Ctrl+0");
      ajuste.key.key = SDLK_MINUS;
      for (int paso = 100; paso > zoom; paso -= 10)
        if (!p.marcos.evento_zoom(ajuste))
          return fallo_monitor("No se reconoce Ctrl+menos");
      SDL_FRect encuadre_sin_marco;
      if (!SDL_SetWindowSize(p.ventana, tamano.first, tamano.second) ||
          !SDL_SyncWindow(p.ventana) ||
          !SDL_SetRenderLogicalPresentation(p.dibujante, 320, 200,
                                            SDL_LOGICAL_PRESENTATION_LETTERBOX) ||
          !SDL_GetRenderLogicalPresentationRect(p.dibujante, &encuadre_sin_marco) ||
          !p.marcos.preparar_encuadre(p.dibujante, ahora))
        return fallo_monitor("No se ha podido comprobar el monitor");
      SDL_FRect juego;
      if (!MarcosPantalla::area_juego(p.dibujante, juego))
        return false;
      if (std::abs(juego.w - encuadre_sin_marco.w * zoom / 100) > 1 ||
          std::abs(juego.h - encuadre_sin_marco.h * zoom / 100) > 1)
        return fallo_monitor("El monitor reduce la superficie jugable");
      if (juego.x < 0 || juego.y < 0 || juego.x + juego.w > tamano.first ||
          juego.y + juego.h > tamano.second || std::abs(juego.w / juego.h - 1.6f) > 0.01f)
        return fallo_monitor("El monitor recorta o deforma el juego");
      for (const auto &punto : {std::pair<float, float>{0, 0}, {160, 100}, {319, 199}}) {
        float wx = 0, wy = 0;
        if (!SDL_RenderCoordinatesToWindow(p.dibujante, punto.first, punto.second, &wx, &wy))
          return fallo_monitor("No se ha podido comprobar el monitor");
        SDL_Event evento{};
        evento.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
        evento.button.windowID = SDL_GetWindowID(p.ventana);
        evento.button.x = wx;
        evento.button.y = wy;
        const bool convertido = SDL_ConvertEventToRenderCoordinates(p.dibujante, &evento);
        if (!convertido || std::abs(evento.button.x - punto.first) > 0.001f ||
            std::abs(evento.button.y - punto.second) > 0.001f)
          return fallo_monitor("El ratón no coincide con la pantalla del monitor");
      }
      ContadorFps contador;
      if (!contador.dibujar(p.dibujante))
        return fallo_monitor("No se ha podido dibujar el contador sobre el monitor");
      SDL_FRect despues;
      if (!MarcosPantalla::area_juego(p.dibujante, despues) || despues.x != juego.x ||
          despues.y != juego.y || despues.w != juego.w || despues.h != juego.h)
        return fallo_monitor("El contador ha cambiado el encuadre del monitor");
      const SDL_FRect destino{0, 0, 320, 200};
      if (!SDL_SetRenderDrawColor(p.dibujante, 0, 0, 0, 255) || !SDL_RenderClear(p.dibujante) ||
          !SDL_RenderTexture(p.dibujante, p.textura, nullptr, &destino))
        return fallo_monitor("No se ha podido dibujar la referencia sin marco");
      SDL_Surface *original = capturar_monitor(p.dibujante);
      if (!original)
        return fallo_monitor("No se ha podido capturar la referencia sin marco");
      if (!p.marcos.dibujar(p.dibujante, "titulo", ahora)) {
        SDL_DestroySurface(original);
        return fallo_monitor("No se ha podido dibujar el monitor con habitación");
      }
      SDL_Surface *captura = capturar_monitor(p.dibujante);
      if (!captura) {
        SDL_DestroySurface(original);
        return fallo_monitor("No se ha podido comprobar el monitor");
      }
      bool coinciden = captura->w == original->w && captura->h == original->h;
      const auto *antes = static_cast<const unsigned char *>(original->pixels);
      const auto *despues_marco = static_cast<const unsigned char *>(captura->pixels);
      const int x0 = int(std::ceil(juego.x)), x1 = int(std::floor(juego.x + juego.w));
      const int y0 = int(std::ceil(juego.y)), y1 = int(std::floor(juego.y + juego.h));
      for (int y = y0; coinciden && y < y1; ++y)
        coinciden =
            std::equal(antes + y * original->pitch + x0 * 4, antes + y * original->pitch + x1 * 4,
                       despues_marco + y * captura->pitch + x0 * 4);
      SDL_DestroySurface(original);
      if (!coinciden) {
        SDL_DestroySurface(captura);
        return fallo_monitor("La habitación o el monitor altera píxeles del juego");
      }
      if (!comprobar_bisel_monitor(captura, juego)) {
        SDL_DestroySurface(captura);
        return fallo_monitor("Hay una rendija o un resto de color junto al bisel");
      }
      // La transparencia central debe permitir ver el título, sin incorporar gráficos DOS al PNG.
      Uint8 r = 0, g = 0, b = 0, a = 0;
      const bool centro = SDL_ReadSurfacePixel(captura, int(juego.x + juego.w / 2),
                                               int(juego.y + juego.h / 2), &r, &g, &b, &a);
      const std::string ruta = prefijo + "." + std::to_string(tamano.first) + "x" +
                               std::to_string(tamano.second) +
                               (zoom == 100 ? "" : ".zoom" + std::to_string(zoom)) + ".png";
      const bool guardada = SDL_SavePNG(captura, ruta.c_str());
      SDL_DestroySurface(captura);
      if (!centro || (!r && !g && !b) || !guardada || !SDL_RenderPresent(p.dibujante))
        return fallo_monitor("El centro del monitor tapa el juego o no se puede capturar");
    }
  }
  SDL_FRect al_cincuenta;
  if (!MarcosPantalla::area_juego(p.dibujante, al_cincuenta))
    return false;
  const auto modificadores = SDL_GetModState();
  SDL_SetModState(SDL_KMOD_CTRL);
  SDL_Event rueda{};
  rueda.type = SDL_EVENT_MOUSE_WHEEL;
  rueda.wheel.y = 1;
  const bool rueda_aceptada =
      p.marcos.evento_zoom(rueda) && p.marcos.preparar_encuadre(p.dibujante, ahora);
  SDL_FRect al_sesenta;
  const bool rueda_acerca = rueda_aceptada && MarcosPantalla::area_juego(p.dibujante, al_sesenta) &&
                            al_sesenta.w > al_cincuenta.w;
  rueda.wheel.direction = SDL_MOUSEWHEEL_FLIPPED;
  const bool rueda_inversa = p.marcos.evento_zoom(rueda);
  SDL_SetModState(modificadores);
  if (!rueda_acerca || !rueda_inversa)
    return fallo_monitor("No funciona Ctrl con rueda normal o invertida");
  SDL_Event ajuste{};
  ajuste.type = SDL_EVENT_KEY_DOWN;
  ajuste.key.mod = SDL_KMOD_CTRL;
  ajuste.key.key = SDLK_EQUALS;
  if (!p.marcos.evento_zoom(ajuste))
    return fallo_monitor("No se reconoce Ctrl+mas");
  ajuste.key.key = SDLK_MINUS;
  p.marcos.evento_zoom(ajuste);
  p.marcos.evento_zoom(ajuste);
  SDL_FRect limite;
  if (!p.marcos.preparar_encuadre(p.dibujante, ahora) ||
      !MarcosPantalla::area_juego(p.dibujante, limite) || limite.w != al_cincuenta.w)
    return fallo_monitor("El zoom no conserva su límite del 50 por ciento");
  p.marcos.alternar(ahora);
  if (!p.marcos.preparar_encuadre(p.dibujante, ahora + 300000000ull) ||
      SDL_RenderViewportSet(p.dibujante))
    return fallo_monitor("F1 no restaura la presentación sin marco");
  p.marcos.alternar(ahora + 400000000ull);
  p.marcos.alternar(ahora + 700000000ull);
  if (!p.marcos.preparar_encuadre(p.dibujante, ahora + 1000000000ull) ||
      !MarcosPantalla::area_juego(p.dibujante, limite) || limite.w != al_cincuenta.w)
    return fallo_monitor("Cambiar con F1 pierde el zoom elegido");
  ajuste.key.key = SDLK_0;
  p.marcos.evento_zoom(ajuste);
  ajuste.key.key = SDLK_PLUS;
  p.marcos.evento_zoom(ajuste);
  if (!p.marcos.preparar_encuadre(p.dibujante, ahora + 1000000000ull) ||
      SDL_RenderViewportSet(p.dibujante))
    return fallo_monitor("Ctrl+0 no restaura el máximo o Ctrl+mas supera el límite");
  registrar_accion("Monitor: seis proporciones a 100, 70 y 50%%; píxeles del juego intactos, bisel "
                   "sin rendijas, ratón, Ctrl+zoom y F1 comprobados");
  return true;
}

// Presenta los bloques reconstruidos y permite recorrerlos con los atajos de desarrollo.
int main(int argc, char *argv[]) {
  configurar_consola();
  if (argc != 2 && argc != 3 && argc != 4 && argc != 5) {
    std::fprintf(
        stderr,
        "Uso: curse-reborn <carpeta DATA> "
        "[--sin-sonido | --comprobar-escena <captura.bmp> | --comprobar-audio <efecto.wav>]\n");
    return 1;
  }
  const std::string opcion = argc >= 3 ? argv[2] : "";
  if (opcion == "--comprobar-tiempo" && argc == 4) {
    Presentacion limpieza;
    return SDL_Init(0) && comprobar_tiempo_juego(argv[3])
               ? 0
               : fallo("No coincide el contador de tiempo");
  }
  if (opcion == "--comprobar-reinicio" && argc == 4)
    return comprobar_reinicio(argv[1], argv[3]) ? 0
                                                : fallo("No se ha podido comprobar el reinicio");
  if (opcion == "--comprobar-monitor" && argc == 4)
    return comprobar_monitor(argv[1], argv[3]) ? 0 : fallo("No se ha podido comprobar el monitor");
  if (opcion == "--comprobar-cursor" && argc == 4)
    return comprobar_cursor(argv[1], argv[3]) ? 0 : fallo("No coincide el cursor original");
  if (opcion == "--comprobar-decorados" && argc == 4)
    return comprobar_decorados(argv[1], argv[3])
               ? 0
               : fallo("No coincide la composición de la decorado");
  if (argc == 3 && opcion != "--sin-sonido") {
    std::fprintf(stderr, "La opción de ejecución debe ser --sin-sonido.\n");
    return 1;
  }
  if (opcion == "--exportar-musica" && argc == 5) {
    unsigned int segundos = 0;
    for (const char *letra = argv[4]; *letra; ++letra) {
      if (*letra < '0' || *letra > '9' || segundos > 600)
        return fallo("La duración debe ser un entero entre 1 y 600 segundos");
      segundos = segundos * 10 + static_cast<unsigned int>(*letra - '0');
    }
    Presentacion limpieza;
    if (!SDL_Init(0) || !exportar_musica_wav(argv[1], argv[3], segundos))
      return fallo("No se ha podido exportar la música; revisa datos, ROM y destino");
    return 0;
  }
  if (argc == 5)
    return fallo("Opción no reconocida");
  if (opcion == "--comprobar-mt32" && argc == 4) {
    Presentacion limpieza;
    if (!SDL_Init(0) || !comprobar_musica_mt32(argv[1], argv[3]) ||
        !comprobar_salida_audio(argv[3]))
      return fallo("No se ha podido comprobar la música MT-32");
    return 0;
  }
  if ((opcion == "--comprobar-musica" || opcion == "--muestra-musica-0") && argc == 4) {
    Presentacion limpieza;
    if (!SDL_Init(0) ||
        !comprobar_musica_adlib(argv[1], argv[3], opcion == "--muestra-musica-0" ? 0 : 1) ||
        !comprobar_salida_audio(argv[3]))
      return fallo("No se ha podido comprobar la música AdLib");
    return 0;
  }
  if (opcion == "--comprobar-digital" && argc == 4) {
    Presentacion limpieza;
    if (!SDL_Init(0) || !comprobar_efectos_digitales(argv[1], argv[3]))
      return fallo("No se han podido comprobar los efectos digitales");
    return 0;
  }
  if (opcion == "--comprobar-audio" && argc == 4) {
    Presentacion limpieza;
    if (!SDL_Init(0) || !comprobar_salida_audio(argv[3]))
      return fallo("No se ha podido comprobar la salida de audio");
    return 0;
  }
  const bool comprobar_introduccion =
      opcion == "--comprobar-caldero" || opcion == "--comprobar-balcon" ||
      opcion == "--comprobar-beisbol" || opcion == "--comprobar-fin-intro" ||
      opcion == "--comprobar-salto-intro";
  const bool comprobar_fps = opcion == "--comprobar-fps";
  const bool comprobar_filtros = opcion == "--comprobar-filtros";
  const bool comprobar_animacion_marcos = opcion == "--comprobar-marcos-animacion";
  const bool comprobar_marcos =
      comprobar_animacion_marcos || opcion == "--comprobar-marcos" ||
      opcion == "--comprobar-marcos-vertical" || opcion == "--comprobar-marcos-ajustado" ||
      opcion == "--comprobar-marcos-impar" || opcion == "--comprobar-marcos-filtro" ||
      opcion == "--comprobar-marcos-corredor" || opcion == "--comprobar-marcos-lago" ||
      opcion == "--comprobar-marcos-mazmorra";
  const bool comprobar_vistas = opcion == "--comprobar-vistas";
  const bool comprobar_caida = opcion == "--comprobar-caida-16" ||
                               opcion == "--comprobar-caida-32" || opcion == "--comprobar-agua-24";
  const bool comprobar_final_cuevas = opcion.rfind("--comprobar-final-", 0) == 0;
  const bool comprobar_catapulta =
      comprobar_final_cuevas || opcion.rfind("--comprobar-catapulta-", 0) == 0;
  const bool comprobar_cuevas = comprobar_catapulta || opcion == "--comprobar-cuevas-piedras-uno" ||
                                opcion == "--comprobar-cuevas-primera-entrega" ||
                                opcion == "--comprobar-cuevas-piedras-tres" ||
                                opcion == "--comprobar-cuevas-cuerda";
  const bool comprobar_poza = comprobar_cuevas || opcion == "--comprobar-poza-algas" ||
                              opcion == "--comprobar-poza-puerta" ||
                              opcion == "--comprobar-cuevas-corredor-entrada" ||
                              opcion == "--comprobar-cuevas-corredor";
  const bool lago_tercera_parte =
      comprobar_poza || opcion == "--comprobar-lago-almeja" || opcion == "--comprobar-lago-tapon" ||
      opcion == "--comprobar-cueva-entrada" || opcion == "--comprobar-cueva-emergida";
  const bool lago_segunda_parte =
      lago_tercera_parte || opcion == "--comprobar-lago-concha" ||
      opcion == "--comprobar-lago-viaje" || opcion == "--comprobar-lago-tortuga" ||
      opcion == "--comprobar-lago-picana" || opcion == "--comprobar-lago-tiburon";
  const bool comprobar_lago = opcion == "--comprobar-lago" || opcion == "--comprobar-lago-pecera" ||
                              opcion == "--comprobar-lago-hablar" || lago_segunda_parte;
  const bool corredor_con_audio = opcion == "--comprobar-corredor-audio";
  const bool corredor_completo =
      opcion == "--comprobar-corredor-pecera" || opcion == "--comprobar-corredor-golpe" ||
      opcion == "--comprobar-corredor-caida" || opcion == "--comprobar-corredor-agua" ||
      corredor_con_audio || comprobar_lago;
  const bool comprobar_corredor = opcion == "--comprobar-corredor-entrada" ||
                                  opcion == "--comprobar-corredor-mecanismo" ||
                                  opcion == "--comprobar-corredor-regreso" || corredor_completo;
  const bool comprobar_jugabilidad = opcion == "--comprobar-liberacion" ||
                                     opcion == "--comprobar-movimiento" ||
                                     opcion == "--comprobar-puzle" || comprobar_corredor;
  const bool comprobar_informacion = opcion.rfind("--comprobar-info-", 0) == 0;
  const bool comprobar_menu = comprobar_informacion || opcion == "--comprobar-menu" ||
                              opcion == "--comprobar-orden" || opcion == "--comprobar-depuracion";
  const bool comprobar_animacion = argc == 4 && std::string(argv[2]) == "--comprobar-animacion";
  const bool comprobar_escena =
      argc == 4 && (std::string(argv[2]) == "--comprobar-escena" || comprobar_animacion ||
                    comprobar_menu || comprobar_jugabilidad || comprobar_caida || comprobar_vistas);
  const bool comprobar_core = argc == 4 && std::string(argv[2]) == "--comprobar-core";
  const bool comprobar_transicion = argc == 4 && std::string(argv[2]) == "--comprobar-transicion";
  const bool comprobar =
      argc == 4 && (std::string(argv[2]) == "--comprobar" || comprobar_escena || comprobar_core ||
                    comprobar_transicion || comprobar_introduccion || comprobar_filtros ||
                    comprobar_fps || comprobar_marcos);
  if (argc == 4 && !comprobar) {
    std::fprintf(stderr, "Opción desconocida de comprobación.\n");
    return 1;
  }
  const std::string carpeta = argv[1];
  Presentacion presentacion;
  ContadorFps contador_fps(!comprobar || comprobar_fps);
  if (!SDL_Init(SDL_INIT_VIDEO))
    return fallo("No se ha podido iniciar el vídeo");
  if (corredor_con_audio && !mazmorra.abrir_audio(carpeta))
    return fallo("No se ha podido abrir el audio para el recorrido del corredor");
  if (!comprobar && opcion != "--sin-sonido" && !mazmorra.abrir_audio(carpeta))
    std::fprintf(stderr, "Audio no disponible; se continúa sin sonido: %s\n", SDL_GetError());
  const SDL_WindowFlags opciones = SDL_WINDOW_RESIZABLE | (comprobar ? SDL_WINDOW_HIDDEN : 0);
  if (!SDL_CreateWindowAndRenderer(
          "Curse Reborn — Pantalla de título",
          comprobar_marcos ? (opcion == "--comprobar-marcos-vertical"   ? 720
                              : opcion == "--comprobar-marcos-ajustado" ? 960
                              : opcion == "--comprobar-marcos-impar"    ? 1279
                                                                        : 1280)
          : comprobar && !comprobar_filtros && !comprobar_fps ? 320
          : comprobar_fps                                     ? 1000
                                                              : 960,
          comprobar_marcos ? (opcion == "--comprobar-marcos-vertical"   ? 1280
                              : opcion == "--comprobar-marcos-ajustado" ? 600
                              : opcion == "--comprobar-marcos-impar"    ? 721
                                                                        : 720)
          : comprobar && !comprobar_filtros && !comprobar_fps ? 200
          : comprobar_fps                                     ? 700
                                                              : 600,
          opciones, &presentacion.ventana, &presentacion.dibujante)) {
    return fallo("No se ha podido crear la ventana");
  }
  if (!SDL_SetWindowMinimumSize(presentacion.ventana, 320, 200) ||
      !SDL_SetRenderLogicalPresentation(presentacion.dibujante, 320, 200,
                                        SDL_LOGICAL_PRESENTATION_LETTERBOX)) {
    return fallo("No se ha podido configurar la presentación");
  }
  presentacion.textura = SDL_CreateTexture(presentacion.dibujante, SDL_PIXELFORMAT_RGBA32,
                                           SDL_TEXTUREACCESS_STATIC, 320, 200);
  if (!presentacion.textura ||
      !SDL_SetTextureScaleMode(presentacion.textura, SDL_SCALEMODE_NEAREST) ||
      !cambiar_pantalla(carpeta,
                        comprobar_introduccion                                  ? 2
                        : comprobar && !comprobar_core && !comprobar_transicion ? 1
                                                                                : 0,
                        presentacion))
    return fallo("No se ha podido crear la imagen");
  if (comprobar_escena || comprobar_transicion) {
    SDL_Event avance{};
    avance.type = SDL_EVENT_KEY_DOWN;
    avance.key.key = comprobar_transicion ? SDLK_RETURN : SDLK_F6;
    if (comprobar_escena && !SDL_PushEvent(&avance))
      return fallo("No se ha podido preparar el segundo avance de desarrollo");
    if (!SDL_PushEvent(&avance))
      return fallo("No se ha podido comprobar el avance con F6");
    if (comprobar_caida && (!SDL_PushEvent(&avance) || !SDL_PushEvent(&avance)))
      return fallo("No se ha podido disponer la vista de caída con F6");
  }
  bool salir = false, capturada = false, fundiendo = false;
  bool animacion_preparada = false, introduccion_comprobada = false, menu_comprobado = false,
       jugabilidad_comprobada = false, caida_comprobada = false;
  bool entrada_introduccion = false;
  bool depuracion = false, cajas_objetos = true, cajas_interaccion = true, cajas_limites = true;
  unsigned short puntero_x = 152, puntero_y = 92;
  int pantalla = comprobar_introduccion                                  ? 2
                 : comprobar && !comprobar_core && !comprobar_transicion ? 1
                                                                         : 0;
  unsigned short paso_fundido = 0;
  Uint64 inicio_pantalla = SDL_GetTicksNS(), inicio_fundido = 0;
  Uint64 ultimo_paso_escena = inicio_pantalla;
  Uint64 siguiente_presentacion = inicio_pantalla;
  bool limitar_presentacion = true;
  bool pantalla_completa = false;
  if (comprobar_filtros) {
    SDL_Event alternar{};
    alternar.type = SDL_EVENT_KEY_DOWN;
    alternar.key.key = SDLK_RETURN;
    alternar.key.mod = SDL_KMOD_ALT;
    if (!SDL_PushEvent(&alternar) || !SDL_PushEvent(&alternar))
      return fallo("No se ha podido preparar Alt + Enter");
  }
  if (opcion == "--comprobar-marcos-filtro") {
    SDL_Event filtro{};
    filtro.type = SDL_EVENT_KEY_DOWN;
    filtro.key.key = SDLK_F11;
    for (unsigned int i = 0; i < 4; ++i)
      if (!SDL_PushEvent(&filtro))
        return fallo("No se ha podido preparar xBRZ con el marco");
  }
  unsigned int paso_vistas = 0, paso_aspecto = 0;
  PartidasSDL partidas;
  TiempoJuego tiempo_juego;
  if (!comprobar)
    tiempo_juego.iniciar();
  unsigned short escena_titulada = 0xffff;
  // Libera las flechas antes de pausar para que el personaje no siga caminando al cerrar el panel.
  const auto abrir_partidas = [&](unsigned int modo) {
    for (const auto bit : {1, 2, 4, 8})
      mazmorra.tecla_direccion(static_cast<unsigned short>(bit), false);
    partidas.abrir(modo, presentacion.ventana);
  };
  Uint64 reloj_marcos_prueba = 1000000000ull;
  while (!salir) {
    if (!presentacion.marcos.preparar_encuadre(
            presentacion.dibujante, comprobar_marcos ? reloj_marcos_prueba : SDL_GetTicksNS()))
      return fallo("No se ha podido preparar el encuadre del juego");
    if (!mazmorra.actualizar_audio())
      return fallo("No se ha podido actualizar el audio del juego");
    SDL_Event evento;
    while (SDL_PollEvent(&evento)) {
      // SDL puede omitir KEY_UP al cambiar de aplicación; no dejar una dirección retenida.
      if (evento.type == SDL_EVENT_WINDOW_FOCUS_LOST || evento.type == SDL_EVENT_WINDOW_MINIMIZED)
        for (const auto bit : {1, 2, 4, 8})
          mazmorra.tecla_direccion(static_cast<unsigned short>(bit), false);
      if (evento.type == SDL_EVENT_QUIT ||
          (evento.type == SDL_EVENT_KEY_DOWN && evento.key.key == SDLK_ESCAPE &&
           (pantalla < 3 || comprobar)))
        salir = true;
      if (!salir &&
          presentacion.marcos.evento_boton(presentacion.ventana, presentacion.dibujante, evento)) {
        if (!mazmorra.pausar_audio_reinicio())
          return fallo("No se ha podido pausar el sonido durante el apagado");
        continue;
      }
      if (!salir && presentacion.marcos.evento_zoom(evento)) {
        if (!presentacion.marcos.preparar_encuadre(presentacion.dibujante))
          return fallo("No se ha podido ajustar el zoom del monitor");
        continue;
      }
      if (!salir && presentacion.marcos.reiniciando())
        continue;
      const auto solicitud = mazmorra.tomar_solicitud_partidas();
      if (solicitud)
        abrir_partidas(solicitud);
      SDL_Event entrada_partidas = evento;
      if (partidas.abierta() &&
          (evento.type == SDL_EVENT_MOUSE_MOTION || evento.type == SDL_EVENT_MOUSE_BUTTON_DOWN) &&
          !SDL_ConvertEventToRenderCoordinates(presentacion.dibujante, &entrada_partidas))
        return fallo("No se ha podido adaptar el ratón del panel de partidas");
      bool redibujar_partida = false;
      if (!salir &&
          partidas.evento(entrada_partidas, presentacion.ventana, mazmorra, redibujar_partida)) {
        if (redibujar_partida) {
          if (!mazmorra.dibujar(imagen))
            return fallo("No se ha podido dibujar la partida cargada");
          for (unsigned int i = 0; i < 768; ++i)
            paleta_visible[i] = imagen.paleta[i];
          if (!actualizar_textura(presentacion))
            return fallo("No se ha podido presentar la partida cargada");
        }
        ultimo_paso_escena = SDL_GetTicksNS();
        continue;
      }
      if (!salir && pantalla >= 3 &&
          (evento.type == SDL_EVENT_KEY_DOWN || evento.type == SDL_EVENT_KEY_UP)) {
        const unsigned short bit = evento.key.key == SDLK_LEFT    ? 8
                                   : evento.key.key == SDLK_RIGHT ? 4
                                   : evento.key.key == SDLK_UP    ? 2
                                   : evento.key.key == SDLK_DOWN  ? 1
                                                                  : 0;
        if (bit && (!mazmorra.menu_abierto() || evento.type == SDL_EVENT_KEY_UP))
          mazmorra.tecla_direccion(bit, evento.type == SDL_EVENT_KEY_DOWN);
      }
      if (!salir && evento.type == SDL_EVENT_KEY_DOWN && !evento.key.repeat) {
        if (evento.key.key == SDLK_F1) {
          if (evento.key.mod & SDL_KMOD_SHIFT)
            presentacion.decorados.alternar();
          else
            presentacion.marcos.alternar(comprobar_marcos ? reloj_marcos_prueba : SDL_GetTicksNS());
          if (!presentacion.marcos.preparar_encuadre(presentacion.dibujante,
                                                     comprobar_marcos ? reloj_marcos_prueba
                                                                      : SDL_GetTicksNS()))
            return fallo("No se ha podido cambiar el encuadre del juego");
          continue;
        }
        if ((evento.key.key == SDLK_RETURN || evento.key.key == SDLK_KP_ENTER) &&
            (evento.key.mod & SDL_KMOD_ALT)) {
          const bool siguiente = !pantalla_completa;
          if (!SDL_SetWindowFullscreen(presentacion.ventana, siguiente))
            return fallo("No se ha podido alternar la pantalla completa");
          if (comprobar_filtros && (!SDL_SyncWindow(presentacion.ventana) ||
                                    bool(SDL_GetWindowFlags(presentacion.ventana) &
                                         SDL_WINDOW_FULLSCREEN) != siguiente ||
                                    SDL_GetWindowFullscreenMode(presentacion.ventana) != nullptr))
            return fallo("No coincide el cambio de pantalla completa sin bordes");
          pantalla_completa = siguiente;
          registrar_accion("Pantalla: %s", pantalla_completa ? "completa sin bordes" : "ventana");
          continue;
        }
        if (evento.key.key == SDLK_PLUS || evento.key.key == SDLK_EQUALS ||
            evento.key.key == SDLK_KP_PLUS || evento.key.key == SDLK_MINUS ||
            evento.key.key == SDLK_KP_MINUS) {
          const bool bajar = evento.key.key == SDLK_MINUS || evento.key.key == SDLK_KP_MINUS;
          if (!mazmorra.ajustar_volumen(bajar ? -10 : 10))
            return fallo("No se ha podido ajustar el volumen");
          continue;
        }
        if (evento.key.key == SDLK_F2) {
          limitar_presentacion = !limitar_presentacion;
          siguiente_presentacion = SDL_GetTicksNS();
          registrar_accion("Presentación: %s",
                           limitar_presentacion ? "límite de 70 FPS" : "sin límite de FPS");
          continue;
        }
        if (evento.key.key == SDLK_F12) {
          contador_fps.alternar();
          continue;
        }
        if (evento.key.key == SDLK_F11) {
          if (!presentacion.filtros.alternar(presentacion.dibujante, presentacion.textura, pixeles))
            return fallo("No se ha podido cambiar el filtro de pantalla");
          continue;
        }
        if (evento.key.key == SDLK_F3) {
          if (!mazmorra.alternar_efectos_audio())
            return fallo("No se ha podido cambiar el dispositivo de efectos");
          continue;
        }
        if (evento.key.key == SDLK_F4) {
          if (!mazmorra.alternar_dispositivo_audio())
            return fallo("No se ha podido cambiar la opción de audio");
          continue;
        }
        if (evento.key.key == SDLK_M) {
          if (!mazmorra.alternar_silencio())
            return fallo("No se ha podido alternar el silencio");
          continue;
        }
        if (pantalla >= 3) {
          const auto tecla = evento.key.key;
          if (tecla == SDLK_F7)
            depuracion = !depuracion;
          if (tecla == SDLK_F8) {
            cajas_objetos = !cajas_objetos;
            depuracion = true;
          }
          if (tecla == SDLK_F9) {
            cajas_interaccion = !cajas_interaccion;
            depuracion = true;
          }
          if (tecla == SDLK_F10) {
            cajas_limites = !cajas_limites;
            depuracion = true;
          }
          if (tecla == SDLK_F7 || tecla == SDLK_F8 || tecla == SDLK_F9 || tecla == SDLK_F10)
            registrar_accion("Depuración: %s; entidades=%s, proximidad=%s, límites=%s",
                             depuracion ? "activada" : "desactivada", cajas_objetos ? "sí" : "no",
                             cajas_interaccion ? "sí" : "no", cajas_limites ? "sí" : "no");
          if (mazmorra.menu_abierto()) {
            if (tecla == SDLK_LEFT)
              puntero_x = puntero_x >= 32 ? puntero_x - 32 : 16;
            if (tecla == SDLK_RIGHT)
              puntero_x = puntero_x < 288 ? puntero_x + 32 : 304;
          }
          if (tecla == SDLK_ESCAPE ||
              (mazmorra.menu_abierto() &&
               (tecla == SDLK_RETURN || tecla == SDLK_LEFT || tecla == SDLK_RIGHT))) {
            if (!mazmorra.entrada_menu(puntero_x, puntero_y, tecla == SDLK_RETURN,
                                       tecla == SDLK_ESCAPE) ||
                !mazmorra.dibujar(imagen,
                                  static_cast<unsigned int>(comprobar ? 0 : SDL_GetTicks() / 57u)))
              return fallo("No se ha podido actualizar el menú");
            for (unsigned int i = 0; i < 768; ++i)
              paleta_visible[i] = imagen.paleta[i];
            if (!actualizar_textura(presentacion))
              return fallo("No se ha podido dibujar el menú");
          }
        }
        const int vista_actual =
            pantalla >= 3 ? vista_de_escena(mazmorra.escena_actual()) : pantalla;
        const bool avanzar = vista_actual >= 0 &&
                             vista_actual < static_cast<int>(cantidad_vistas) + 2 &&
                             evento.key.key == SDLK_F6;
        const bool retroceder = vista_actual > 0 && evento.key.key == SDLK_F5;
        if (avanzar || retroceder) {
          pantalla = vista_actual + (avanzar ? 1 : -1);
          if (!cambiar_pantalla(carpeta, pantalla, presentacion)) {
            std::fprintf(
                stderr,
                "No se ha podido reconstruir la pantalla solicitada con sus datos originales.\n");
            return 1;
          }
          fundiendo = false;
          entrada_introduccion = false;
          inicio_pantalla = SDL_GetTicksNS();
          ultimo_paso_escena = inicio_pantalla;
        } else if (!fundiendo && pantalla == 1 && evento.key.key != SDLK_F5 &&
                   evento.key.key != SDLK_F6) {
          pantalla = 2;
          if (!cambiar_pantalla(carpeta, pantalla, presentacion))
            return fallo("No se ha podido cargar la introducción");
          inicio_pantalla = SDL_GetTicksNS();
        } else if (pantalla == 2 && evento.key.key != SDLK_F5 && evento.key.key != SDLK_F6) {
          entrada_introduccion = true;
        } else if (!fundiendo && pantalla == 0 && evento.key.key != SDLK_F5 &&
                   evento.key.key != SDLK_F6) {
          fundiendo = true;
          paso_fundido = 0;
          inicio_fundido = SDL_GetTicksNS();
        }
      }
      if (!salir && pantalla >= 3 &&
          (evento.type == SDL_EVENT_MOUSE_MOTION || evento.type == SDL_EVENT_MOUSE_BUTTON_DOWN)) {
        if (!SDL_ConvertEventToRenderCoordinates(presentacion.dibujante, &evento))
          return fallo("No se ha podido adaptar el ratón a la ventana");
        const float x = evento.type == SDL_EVENT_MOUSE_MOTION ? evento.motion.x : evento.button.x;
        const float y = evento.type == SDL_EVENT_MOUSE_MOTION ? evento.motion.y : evento.button.y;
        if (x >= 0 && x < 320 && y >= 0 && y < 200) {
          puntero_x = static_cast<unsigned short>(x);
          puntero_y = static_cast<unsigned short>(y);
          if (!mazmorra.entrada_menu(puntero_x, puntero_y,
                                     evento.type == SDL_EVENT_MOUSE_BUTTON_DOWN &&
                                         evento.button.button == SDL_BUTTON_LEFT,
                                     evento.type == SDL_EVENT_MOUSE_BUTTON_DOWN &&
                                         evento.button.button == SDL_BUTTON_RIGHT) ||
              !mazmorra.dibujar(imagen,
                                static_cast<unsigned int>(comprobar ? 0 : SDL_GetTicks() / 57u)))
            return fallo("No se ha podido procesar el menú con el ratón");
          for (unsigned int i = 0; i < 768; ++i)
            paleta_visible[i] = imagen.paleta[i];
          if (!actualizar_textura(presentacion))
            return fallo("No se ha podido actualizar la selección");
        }
      }
      if (!salir && !fundiendo && pantalla < 3 && evento.type == SDL_EVENT_MOUSE_BUTTON_DOWN &&
          evento.button.button == SDL_BUTTON_LEFT) {
        if (pantalla == 1) {
          pantalla = 2;
          if (!cambiar_pantalla(carpeta, pantalla, presentacion))
            return fallo("No se ha podido cargar la introducción");
          inicio_pantalla = SDL_GetTicksNS();
        } else if (pantalla == 2) {
          entrada_introduccion = true;
        } else {
          fundiendo = true;
          paso_fundido = 0;
          inicio_fundido = SDL_GetTicksNS();
        }
      }
    }
    if (salir)
      break;
    if (presentacion.marcos.tomar_reinicio()) {
      if (!reiniciar_presentacion(carpeta, presentacion, partidas))
        return fallo("No se ha podido reiniciar la presentación desde Core");
      pantalla = 0;
      fundiendo = false;
      paso_fundido = 0;
      entrada_introduccion = false;
      escena_titulada = 0xffff;
      puntero_x = 152;
      puntero_y = 92;
      inicio_fundido = 0;
      inicio_pantalla = ultimo_paso_escena = siguiente_presentacion = SDL_GetTicksNS();
    }
    const auto solicitud = mazmorra.tomar_solicitud_partidas();
    if (solicitud)
      abrir_partidas(solicitud);
    if (comprobar_vistas && !capturada) {
      const auto indice = paso_vistas < 2                      ? paso_vistas
                          : paso_vistas <= cantidad_vistas + 1 ? paso_vistas - 2
                                                               : cantidad_vistas * 2 - paso_vistas;
      if (mazmorra.escena_actual() != vistas_desarrollo[indice].escena)
        return fallo("F5/F6 no han respetado el orden de las escenas");
      if ((mazmorra.escena_actual() == 24 || mazmorra.escena_actual() == 25 ||
           mazmorra.escena_actual() == 26 || mazmorra.escena_actual() == 27 ||
           mazmorra.escena_actual() == 35) &&
          !comprobar_partida_tienda(mazmorra, argv[1]))
        return fallo("Guardar y cargar altera la imagen o la escena de una tienda");
      if (paso_vistas < cantidad_vistas * 2) {
        SDL_Event tecla{};
        tecla.type = SDL_EVENT_KEY_DOWN;
        tecla.key.key = paso_vistas == 1 || paso_vistas >= cantidad_vistas + 1 ? SDLK_F5 : SDLK_F6;
        ++paso_vistas;
        if (!SDL_PushEvent(&tecla))
          return fallo("No se ha podido comprobar la navegación de desarrollo");
        continue;
      }
      std::printf("F5/F6: %u cambios comprobados en las %u vistas jugables.\n", cantidad_vistas * 2,
                  cantidad_vistas);
    }
    if (comprobar_introduccion && !introduccion_comprobada) {
      const unsigned int limite = opcion == "--comprobar-caldero"   ? 33
                                  : opcion == "--comprobar-balcon"  ? 378
                                  : opcion == "--comprobar-beisbol" ? 708
                                                                    : 2000;
      unsigned int ultimo_tick = 0;
      for (unsigned int tick = 0; tick <= limite && !introduccion.terminada(); ++tick) {
        ultimo_tick = tick;
        if (!introduccion.avanzar(tick, opcion == "--comprobar-salto-intro" && tick == 34, imagen))
          return fallo("No se ha podido comprobar el recorrido de la introducción");
      }
      if (limite == 2000) {
        if (!introduccion.terminada())
          return fallo("La introducción no ha terminado");
        if (ultimo_tick != (opcion == "--comprobar-salto-intro" ? 73u : 1005u))
          return fallo("La introducción no ha terminado en el retrazo previsto");
        pantalla = 3;
        if (!cambiar_pantalla(carpeta, pantalla, presentacion))
          return fallo("La introducción no ha entrado en la mazmorra");
      } else {
        for (unsigned int i = 0; i < 768; ++i)
          paleta_visible[i] = imagen.paleta[i];
        if (!actualizar_textura(presentacion))
          return fallo("No se ha podido comprobar la imagen de la introducción");
      }
      introduccion_comprobada = true;
    }
    if (comprobar_caida && !caida_comprobada) {
      const unsigned int pasos = opcion == "--comprobar-caida-16" ? 16u : 32u;
      if (opcion == "--comprobar-agua-24") {
        for (unsigned int i = 0; i < 256 && mazmorra.escena_actual() == 2; ++i)
          if (!mazmorra.actualizar_audio() || !mazmorra.avanzar() ||
              !mazmorra.avanzar_presentacion(SDL_GetTicksNS() * 70u / 1000000000u + 100u))
            return fallo("No se ha podido seguir la salida automática de la caída");
        if (mazmorra.escena_actual() != 3)
          return fallo("La caída no ha llegado al agua");
      }
      for (unsigned int i = 0; i < (opcion == "--comprobar-agua-24" ? 24u : pasos); ++i)
        if (!mazmorra.avanzar())
          return fallo("No se ha podido avanzar la caída o la inmersión");
      if (!mazmorra.dibujar(imagen))
        return fallo("No se ha podido dibujar la caída");
      for (unsigned int i = 0; i < 768; ++i)
        paleta_visible[i] = imagen.paleta[i];
      if (!actualizar_textura(presentacion))
        return fallo("No se ha podido capturar la caída");
      caida_comprobada = true;
    }
    if (comprobar_jugabilidad && pantalla >= 3 && !jugabilidad_comprobada) {
      // Recorre los mismos clics del menú que el jugador, sin cambiar directamente su inventario.
      const auto elegir = [](unsigned short casilla) {
        return mazmorra.entrada_menu(static_cast<unsigned short>(casilla * 32 + 16), 92, true,
                                     false);
      };
      // Avanza los pasos de juego de forma determinista para comparar la imagen resultante.
      const auto avanzar = [comprobar_lago](unsigned int cantidad) {
        for (unsigned int i = 0; i < cantidad; ++i) {
          if (!mazmorra.actualizar_audio() || !mazmorra.avanzar())
            return false;
          const bool camara = comprobar_lago && mazmorra.camara_en_presentacion();
          if (!mazmorra.avanzar_presentacion(SDL_GetTicksNS() * 70u / 1000000000u + 100u))
            return false;
          // EAD5 termina el desplazamiento antes de ejecutar la iteración lógica del mismo ciclo.
          if (camara && (!mazmorra.avanzar() || !mazmorra.avanzar_presentacion(
                                                    SDL_GetTicksNS() * 70u / 1000000000u + 100u)))
            return false;
        }
        return true;
      };
      if (!avanzar(1) || !mazmorra.entrada_menu(152, 92, false, true) || !elegir(4) || !elegir(1) ||
          !avanzar(192) || !mazmorra.entrada_menu(152, 92, false, true) || !elegir(1) ||
          !elegir(0) || !avanzar(8) || !mazmorra.entrada_menu(152, 92, false, true))
        return fallo("No se ha podido preparar la liberación desde el menú");
      for (const auto casilla : {2, 0, 0, 0, 1})
        if (!elegir(static_cast<unsigned short>(casilla)))
          return fallo("No se ha podido desbloquear las esposas con la llave");
      if (mazmorra.menu_abierto() ||
          !avanzar(opcion == "--comprobar-puzle" || comprobar_corredor ? 192 : 80))
        return fallo("La liberación no ha terminado correctamente");
      if (opcion == "--comprobar-movimiento") {
        for (const auto bit : {4, 8, 2, 1}) {
          mazmorra.tecla_direccion(static_cast<unsigned short>(bit), true);
          if (!avanzar(8))
            return fallo("No se ha podido caminar con las flechas");
          mazmorra.tecla_direccion(static_cast<unsigned short>(bit), false);
        }
        if (!avanzar(8))
          return fallo("No se ha podido detener al personaje");
        const unsigned short destinos[][2] = {{180, 157}, {80, 160}, {150, 185}, {300, 180}};
        for (const auto &destino : destinos)
          if (!mazmorra.mover_a(destino[0], destino[1]) || !avanzar(48))
            return fallo("No se ha podido caminar hacia el destino del ratón");
      }
      if (opcion == "--comprobar-puzle" || comprobar_corredor) {
        if (!mazmorra.mover_a(210, 157) || !avanzar(96) ||
            !mazmorra.entrada_menu(152, 92, false, true) || !elegir(2) || !elegir(2) ||
            !elegir(1) || !avanzar(192) || !mazmorra.entrada_menu(152, 92, false, true) ||
            !elegir(1) || !elegir(0) || !avanzar(192) || !mazmorra.mover_a(55, 157) ||
            !avanzar(96) || !mazmorra.entrada_menu(152, 92, false, true))
          return fallo("No se ha podido completar la acción sobre la pared y recoger el clip");
        for (const auto casilla : {2, 0, 0, 1, 1})
          if (!elegir(static_cast<unsigned short>(casilla)))
            return fallo("No se ha podido usar el clip en la puerta");
        if (mazmorra.menu_abierto() || !avanzar(192))
          return fallo("No ha terminado la apertura de la puerta");
      }
      if (comprobar_corredor) {
        if (!mazmorra.mover_a(45, 152))
          return fallo("No se ha podido proponer la salida de la celda");
        for (unsigned int i = 0; i < 96 && mazmorra.escena_actual() == 0; ++i)
          if (!avanzar(1))
            return fallo("No se ha podido cruzar la puerta de la celda");
        if (mazmorra.escena_actual() != 1 || !avanzar(48))
          return fallo("No se ha alcanzado el corredor con su entrada original");
        if (corredor_completo) {
          if (!mazmorra.mover_a(267, 149) || !avanzar(96) ||
              !mazmorra.entrada_menu(152, 92, false, true) || !elegir(1) || !elegir(0) ||
              mazmorra.menu_abierto() || !avanzar(96) || !mazmorra.conserva_objeto(6) ||
              !mazmorra.conserva_objeto(7))
            return fallo("No se ha podido recoger la pecera con su agua desde el menú");
          if (opcion != "--comprobar-corredor-pecera") {
            mazmorra.tecla_direccion(8, true);
            bool golpe = false;
            for (unsigned int i = 0; i < 800 && mazmorra.escena_actual() == 1; ++i) {
              const bool anterior = mazmorra.golpe_estatua();
              if (!avanzar(1))
                return fallo("No se ha podido atravesar el corredor con sus guardias y estatuas");
              golpe = golpe || (!anterior && mazmorra.golpe_estatua());
              if (opcion == "--comprobar-corredor-golpe" && golpe)
                break;
            }
            mazmorra.tecla_direccion(8, false);
            if (!golpe)
              return fallo("No se ha alcanzado la secuencia original del golpe de estatua");
            if (opcion != "--comprobar-corredor-golpe") {
              if (mazmorra.escena_actual() != 2)
                return fallo("La puerta izquierda no ha conducido a DROP");
              if (opcion == "--comprobar-corredor-caida") {
                if (!avanzar(16))
                  return fallo("No se ha podido continuar la caída de la partida real");
              } else {
                for (unsigned int i = 0; i < 256 && mazmorra.escena_actual() == 2; ++i)
                  if (!avanzar(1))
                    return fallo("No se ha podido enlazar el corredor con el agua");
                if (mazmorra.escena_actual() != 3 || !avanzar(24) || !mazmorra.conserva_objeto(6) ||
                    !mazmorra.conserva_objeto(7) || mazmorra.conserva_objeto(1))
                  return fallo(
                      "La llegada al agua no conserva el inventario original del corredor");
              }
            }
          }
        }
        if (comprobar_lago) {
          for (unsigned int i = 0; i < 128 && mazmorra.secuencia_personaje_activa(); ++i)
            if (!avanzar(1))
              return fallo("No se ha podido terminar la inmersión");
          // Selecciona un identificador en el menú real y conserva sus submenús originales.
          const auto objeto = [&](unsigned short id) {
            const int casilla = mazmorra.casilla_objeto(id);
            return casilla >= 0 && elegir(static_cast<unsigned int>(casilla));
          };
          // Abre el menú común antes de cada acción del recorrido automático.
          const auto abrir = [&]() { return mazmorra.entrada_menu(152, 92, false, true); };
          // Busca el objeto por la misma cámara y los mismos controles de la partida normal.
          const auto acercar = [&](unsigned short id, short dx, short dy, bool suelo,
                                   unsigned short limite_derecha = 240) {
            unsigned short x = 0, y = 0, alto = 0;
            bool visible = false;
            for (unsigned int i = 0; i < 400; ++i) {
              if (!mazmorra.posicion_objeto(id, x, y, alto))
                return false;
              if (x >= 40 && x <= limite_derecha) {
                visible = true;
                break;
              }
              const unsigned short direccion = x >= 32768 || x < 40 ? 8 : 4;
              mazmorra.tecla_direccion(direccion, true);
              const bool correcto = avanzar(1);
              mazmorra.tecla_direccion(direccion, false);
              if (!correcto || mazmorra.escena_actual() != 3)
                return false;
            }
            return visible &&
                   mazmorra.mover_a(static_cast<unsigned short>(x + dx),
                                    static_cast<unsigned short>(y + dy + (suelo ? alto : 0))) &&
                   avanzar(96);
          };
          if (mazmorra.secuencia_personaje_activa() || !abrir() || !elegir(2) || !elegir(4) ||
              !objeto(6) || !avanzar(128) || mazmorra.conserva_objeto(6) ||
              mazmorra.conserva_objeto(7))
            return fallo("No se ha podido poner la pecera desde el inventario");
          if (opcion != "--comprobar-lago-pecera") {
            if (!acercar(15, -16, 24, false) || !abrir() || !elegir(2) || !elegir(2) ||
                !objeto(15) || !avanzar(96) || !acercar(26, 5, -2, true) || !abrir() ||
                !elegir(1) || !objeto(26) || !avanzar(96))
              return fallo("No se ha podido liberar al pez y recoger la moneda");
            unsigned short x = 0, y = 0, alto = 0;
            if (!mazmorra.posicion_objeto(0, x, y, alto) ||
                !mazmorra.mover_a(static_cast<unsigned short>(x + 16), 177) || !avanzar(96) ||
                !acercar(16, 4, 0, false) || !abrir() || !elegir(3) || !objeto(16) ||
                !avanzar(96) || !acercar(17, 5, -2, true) || !abrir() || !elegir(1) ||
                !objeto(17) || !avanzar(96) || !mazmorra.conserva_objeto(17) ||
                !acercar(20, 12, 52, false))
              return fallo("No se ha podido llegar a la tienda con el gusano");
            if (opcion == "--comprobar-lago-hablar") {
              if (!abrir() || !elegir(4) || !elegir(0) || !avanzar(8))
                return fallo("No se ha podido hablar con Mr. Fish desde su tema original");
            } else if (!abrir() || !elegir(2) || !elegir(6) || !objeto(17) || !objeto(20) ||
                       !avanzar(192) || mazmorra.conserva_objeto(17))
              return fallo("No se ha podido entregar el gusano a Mr. Fish");
          }
        }
        if (lago_segunda_parte) {
          // Ejecuta menús y localiza objetos por sus identificadores en las casillas originales.
          const auto objeto = [&](unsigned short id) {
            const int casilla = mazmorra.casilla_objeto(id);
            return casilla >= 0 && elegir(static_cast<unsigned int>(casilla));
          };
          // Inicia una acción de inventario con la misma apertura y casillas que usa el jugador.
          const auto abrir = [&]() { return mazmorra.entrada_menu(152, 92, false, true); };
          // Aproxima la concha hasta su borde derecho y utiliza los pies originales para el clic.
          const auto recoger_concha = [&]() {
            unsigned short x = 0, y = 0, alto = 0;
            for (unsigned int i = 0; i < 400; ++i) {
              if (!mazmorra.posicion_objeto(18, x, y, alto))
                return false;
              if (x >= 40 && x <= 280)
                return mazmorra.mover_a(static_cast<unsigned short>(x + 5),
                                        static_cast<unsigned short>(y + alto - 2)) &&
                       avanzar(96);
              const unsigned short direccion = x >= 32768 || x < 40 ? 8 : 4;
              mazmorra.tecla_direccion(direccion, true);
              const bool correcto = avanzar(1);
              mazmorra.tecla_direccion(direccion, false);
              if (!correcto)
                return false;
            }
            return false;
          };
          if (!recoger_concha() || !abrir() || !elegir(1) || !objeto(18) || !avanzar(96) ||
              !mazmorra.conserva_objeto(18))
            return fallo("No se ha podido recoger la concha del pez liberado");
          if (opcion != "--comprobar-lago-concha") {
            bool entregada = false;
            for (unsigned int i = 0; i < 500; ++i) {
              unsigned short x = 0, y = 0, alto = 0, brad = 0;
              if (!mazmorra.posicion_objeto(22, x, y, alto) ||
                  !mazmorra.posicion_objeto(0, brad, y, alto))
                return fallo("No se encuentra la tortuga durante la aproximación");
              const int delta = static_cast<int>(x) - brad;
              if (x < 32768 && delta > -24 && delta < 24) {
                if (!avanzar(8) || !abrir() || !elegir(2) || !elegir(6) || !objeto(18) ||
                    !objeto(22))
                  return fallo("No se ha podido entregar la concha a la tortuga");
                entregada = true;
                break;
              }
              mazmorra.tecla_direccion(8, true);
              const bool correcto = avanzar(1);
              mazmorra.tecla_direccion(8, false);
              if (!correcto)
                return fallo("No se ha podido aproximar Brad a la tortuga");
            }
            if (!entregada || mazmorra.conserva_objeto(18) ||
                !avanzar(opcion == "--comprobar-lago-viaje" ? 64 : 192))
              return fallo("No se ha podido completar el transporte de la tortuga");
            if (opcion != "--comprobar-lago-viaje" && opcion != "--comprobar-lago-tortuga") {
              unsigned short x = 0, y = 0, alto = 0;
              if (!mazmorra.posicion_objeto(23, x, y, alto) ||
                  !mazmorra.mover_a(static_cast<unsigned short>(x + 5),
                                    static_cast<unsigned short>(y + alto - 2)) ||
                  !avanzar(96) || !abrir() || !elegir(1) || !objeto(23) || !avanzar(96) ||
                  !mazmorra.conserva_objeto(23))
                return fallo("No se ha podido recoger la picana tras las anguilas");
              if (opcion != "--comprobar-lago-picana") {
                bool descarga = false;
                for (unsigned int i = 0; i < 350; ++i) {
                  unsigned short tiburon = 0, brad = 0;
                  if (mazmorra.tiburon_activo() &&
                      mazmorra.posicion_objeto(0x040c, tiburon, y, alto) &&
                      mazmorra.posicion_objeto(0, brad, y, alto) &&
                      static_cast<int>(brad) - static_cast<short>(tiburon) <= 200) {
                    if (!abrir() || !elegir(5) || !objeto(23))
                      return fallo("No se ha podido usar la picana");
                    descarga = true;
                    break;
                  }
                  mazmorra.tecla_direccion(8, true);
                  const bool correcto = avanzar(1);
                  mazmorra.tecla_direccion(8, false);
                  if (!correcto)
                    return fallo("No se ha podido alcanzar la aparición del tiburón");
                }
                for (unsigned int i = 0; i < 48 && descarga && !mazmorra.tiburon_aturdido(); ++i)
                  if (!avanzar(1))
                    return fallo("No se ha podido reproducir la descarga de la picana");
                if (!descarga || !mazmorra.tiburon_aturdido())
                  return fallo("La picana no ha aturdido al tiburón en su intervalo original");
                if (lago_tercera_parte) {
                  bool alcanzada = false;
                  for (unsigned int i = 0; i < 500; ++i) {
                    if (!mazmorra.posicion_objeto(24, x, y, alto))
                      return fallo("No se encuentra la almeja");
                    if (x > 30 && x < 250) {
                      alcanzada = true;
                      break;
                    }
                    unsigned short tiburon = 0, brad = 0;
                    if (mazmorra.tiburon_activo() && !mazmorra.tiburon_aturdido() &&
                        !mazmorra.secuencia_personaje_activa() &&
                        mazmorra.posicion_objeto(0x040c, tiburon, y, alto) &&
                        mazmorra.posicion_objeto(0, brad, y, alto) &&
                        static_cast<int>(brad) - static_cast<short>(tiburon) <= 200) {
                      if (!abrir() || !elegir(5) || !objeto(23))
                        return fallo("No se ha podido repetir la descarga ante el tiburón");
                    }
                    mazmorra.tecla_direccion(8, true);
                    const bool correcto = avanzar(1);
                    mazmorra.tecla_direccion(8, false);
                    if (!correcto)
                      return fallo("No se ha podido cruzar el tramo del tiburón");
                  }
                  if (!alcanzada || !mazmorra.posicion_objeto(24, x, y, alto) ||
                      !mazmorra.mover_a(static_cast<unsigned short>(x + 61),
                                        static_cast<unsigned short>(y + 21)) ||
                      !avanzar(64))
                    return fallo("No se ha podido preparar el salto de la almeja");
                  for (unsigned int i = 0; i < 120 && !mazmorra.almeja_cerrada(); ++i)
                    if (!avanzar(1))
                      return fallo("No se ha podido esperar el cierre de la almeja");
                  if (!mazmorra.almeja_cerrada() || !abrir() || !elegir(6) || !objeto(24) ||
                      !avanzar(96))
                    return fallo("No se ha podido saltar la almeja cerrada");
                  if (opcion != "--comprobar-lago-almeja") {
                    bool tapon_visible = false;
                    for (unsigned int i = 0; i < 400; ++i) {
                      if (!mazmorra.posicion_objeto(25, x, y, alto))
                        return fallo("No se encuentra el tapón");
                      if (x >= 40 && x <= 240) {
                        tapon_visible = true;
                        break;
                      }
                      const unsigned short direccion = x >= 32768 || x < 40 ? 8 : 4;
                      mazmorra.tecla_direccion(direccion, true);
                      const bool correcto = avanzar(1);
                      mazmorra.tecla_direccion(direccion, false);
                      if (!correcto)
                        return fallo("No se ha podido alcanzar el tapón");
                    }
                    if (!tapon_visible ||
                        !mazmorra.mover_a(static_cast<unsigned short>(x + 68),
                                          static_cast<unsigned short>(y + 30)) ||
                        !avanzar(96) || !abrir() || !elegir(2) || !elegir(1) || !objeto(23) ||
                        !objeto(25) || !avanzar(17))
                      return fallo("No se ha podido insertar la picana en el tapón");
                    if (opcion != "--comprobar-lago-tapon") {
                      for (unsigned int i = 0; i < 200 && mazmorra.escena_actual() == 3; ++i)
                        if (!avanzar(1))
                          return fallo("No se ha podido seguir el desagüe hacia las cuevas");
                      if (mazmorra.escena_actual() != 8 || mazmorra.conserva_objeto(23) ||
                          ((opcion == "--comprobar-cueva-emergida" || comprobar_poza) &&
                           !avanzar(96)))
                        return fallo("No se ha podido completar la llegada a las cuevas");
                      if (comprobar_poza) {
                        for (const auto destino :
                             {std::pair{280, 180}, std::pair{305, 195}, std::pair{180, 100}})
                          if (!mazmorra.mover_a(static_cast<unsigned short>(destino.first),
                                                static_cast<unsigned short>(destino.second)) ||
                              !avanzar(96))
                            return fallo("No se ha podido alcanzar las algas de la poza");
                        if (!abrir() || !elegir(1) || !objeto(49) || !avanzar(96) ||
                            !mazmorra.conserva_objeto(49))
                          return fallo("No se han podido recoger las algas");
                        if (opcion != "--comprobar-poza-algas") {
                          if (!abrir() || !elegir(3) || !objeto(50) ||
                              !mazmorra.entrada_menu(152, 92, false, true) ||
                              !mazmorra.posicion_objeto(48, x, y, alto) ||
                              !mazmorra.mover_a(static_cast<unsigned short>(x - 15),
                                                static_cast<unsigned short>(y + 33)) ||
                              !avanzar(96) || !abrir() || !elegir(2) || !elegir(2) || !objeto(48) ||
                              !avanzar(96))
                            return fallo("No se ha podido pulsar el botón y abrir la puerta");
                          if (opcion != "--comprobar-poza-puerta") {
                            if (!mazmorra.mover_a(90, 110) || !avanzar(96) ||
                                !mazmorra.mover_a(80, 80))
                              return fallo("No se ha podido alcanzar la salida secreta");
                            for (unsigned int i = 0; i < 96 && mazmorra.escena_actual() == 8; ++i)
                              if (!avanzar(1))
                                return fallo("No se ha podido atravesar la puerta secreta");
                            if (mazmorra.escena_actual() != 4 || !mazmorra.conserva_objeto(49) ||
                                ((opcion == "--comprobar-cuevas-corredor" || comprobar_cuevas) &&
                                 !avanzar(96)))
                              return fallo(
                                  "No se ha podido completar la entrada al corredor de las cuevas");
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
        if (!corredor_completo && opcion != "--comprobar-corredor-entrada") {
          if (!mazmorra.mover_a(210, 185) || !avanzar(96) || !mazmorra.mover_a(210, 160) ||
              !avanzar(96) || !mazmorra.entrada_menu(152, 92, false, true) || !elegir(3) ||
              !elegir(0) || !avanzar(192))
            return fallo("No se ha podido accionar el mecanismo del corredor");
        }
        if (opcion == "--comprobar-corredor-regreso") {
          if (!mazmorra.mover_a(210, 185) || !avanzar(96))
            return fallo("No se ha podido volver al suelo del corredor");
          mazmorra.tecla_direccion(8, true);
          if (!avanzar(80))
            return fallo("No se ha podido contrastar la barrera del corredor");
          mazmorra.tecla_direccion(8, false);
          mazmorra.tecla_direccion(4, true);
          for (unsigned int i = 0; i < 160 && mazmorra.escena_actual() == 1; ++i)
            if (!avanzar(1))
              return fallo("No se ha podido regresar hacia la celda");
          mazmorra.tecla_direccion(4, false);
          if (mazmorra.escena_actual() != 0 || !avanzar(48))
            return fallo("No se ha completado el regreso a la mazmorra");
        }
      }
      if (comprobar_cuevas) {
        // Abre el menú sin cambiar la partida ni resolver acciones mediante estados de prueba.
        const auto abrir = [&]() { return mazmorra.entrada_menu(152, 92, false, true); };
        // Escoge el identificador de la misma lista visible para el jugador.
        const auto objeto = [&](unsigned short id) {
          const int casilla = mazmorra.casilla_objeto(id);
          return casilla >= 0 && elegir(static_cast<unsigned int>(casilla));
        };
        if (!comprobar_recorrido_cuevas(mazmorra, opcion, avanzar, abrir, elegir, objeto))
          return fallo("No coincide el recorrido SDL de las piedras y el picapedrero");
        if (comprobar_catapulta &&
            !comprobar_recorrido_catapulta(mazmorra, opcion, avanzar, abrir, elegir, objeto))
          return fallo("No coincide el recorrido SDL de catapulta, imán y alambre");
        if (comprobar_final_cuevas &&
            !comprobar_recorrido_final_cuevas(mazmorra, opcion, avanzar, abrir, elegir, objeto))
          return fallo("No coincide el recorrido SDL hasta salir de las cuevas");
        if ((opcion.rfind("--comprobar-final-campo-", 0) == 0 ||
             opcion.rfind("--comprobar-final-asaltante-", 0) == 0 ||
             opcion.rfind("--comprobar-final-poblacion-", 0) == 0) &&
            !comprobar_recorrido_campo(mazmorra, opcion, avanzar))
          return fallo("No coincide el recorrido SDL de la salida al campo");
        if ((opcion.rfind("--comprobar-final-asaltante-", 0) == 0 ||
             opcion.rfind("--comprobar-final-poblacion-", 0) == 0) &&
            !comprobar_recorrido_asaltante(mazmorra, opcion, avanzar, abrir, elegir, objeto))
          return fallo("No coincide el recorrido SDL del camino del asaltante");
        if (opcion.rfind("--comprobar-final-poblacion-", 0) == 0 &&
            !comprobar_recorrido_poblacion(mazmorra, opcion, avanzar, abrir, elegir, objeto))
          return fallo("No coincide el recorrido SDL de la población");
        if ((opcion == "--comprobar-final-poblacion-borde_puente" ||
             opcion == "--comprobar-final-poblacion-borde_loro_entrada" ||
             opcion == "--comprobar-final-poblacion-borde_loro_regreso" ||
             opcion == "--comprobar-final-poblacion-borde_valle_post_entrada") &&
            !comprobar_partida_tienda(mazmorra, argv[1]))
          return fallo("No coincide la restauración DOS en EDGE o durante la visita al loro");
        if (opcion == "--comprobar-final-partidas" &&
            !comprobar_partidas_sdl(partidas, presentacion.ventana, mazmorra, argv[1], imagen))
          return fallo("No coinciden las ranuras y la restauración SDL de partidas");
      }
      if (!mazmorra.dibujar(imagen))
        return fallo("No se ha podido dibujar el estado jugable");
      for (unsigned int i = 0; i < 768; ++i)
        paleta_visible[i] = imagen.paleta[i];
      if (!actualizar_textura(presentacion))
        return fallo("No se ha podido presentar el movimiento");
      jugabilidad_comprobada = true;
    }
    if (comprobar_menu && pantalla >= 3 && !menu_comprobado) {
      if (!mazmorra.avanzar())
        return fallo("No se ha podido habilitar el menú inicial");
      if (opcion == "--comprobar-depuracion") {
        depuracion = true;
      } else {
        if (!mazmorra.entrada_menu(152, 92, false, true) || !mazmorra.menu_abierto())
          return fallo("No se ha abierto el menú");
        if (opcion == "--comprobar-orden") {
          if (!mazmorra.entrada_menu(144, 92, true, false) ||
              !mazmorra.entrada_menu(48, 92, true, false) || mazmorra.menu_abierto())
            return fallo("No se ha podido ejecutar la primera orden del menú");
          for (unsigned int paso = 0; paso < 32; ++paso)
            if (!mazmorra.avanzar())
              return fallo("No se ha podido avanzar la interacción");
        }
      }
      if (!mazmorra.dibujar(imagen))
        return fallo("No se ha podido comprobar el menú");
      for (unsigned int i = 0; i < 768; ++i)
        paleta_visible[i] = imagen.paleta[i];
      if (!actualizar_textura(presentacion))
        return fallo("No se ha podido presentar la interacción");
      if (comprobar_informacion &&
          !comprobar_informacion_sdl(partidas, presentacion.ventana, presentacion.dibujante,
                                     mazmorra, static_cast<unsigned int>(opcion.back() - '0')))
        return fallo("No se ha podido comprobar el último icono de información");
      menu_comprobado = true;
    }
    if (comprobar_animacion && pantalla >= 3 && !animacion_preparada) {
      for (unsigned int paso = 0; paso < 32; ++paso) {
        if (!mazmorra.avanzar() ||
            !mazmorra.dibujar(imagen,
                              static_cast<unsigned int>(comprobar ? 0 : SDL_GetTicks() / 57u)))
          return fallo("No se ha podido comprobar la animación de los objetos");
      }
      if (!actualizar_textura(presentacion))
        return fallo("No se ha podido presentar la animación comprobada");
      animacion_preparada = true;
    }
    const auto ahora = SDL_GetTicksNS();
    if (!comprobar)
      tiempo_juego.actualizar(
          TiempoJuego::debe_contar(pantalla >= 3, SDL_GetWindowFlags(presentacion.ventana),
                                   partidas.abierta(), presentacion.marcos.reiniciando()),
          ahora);
    // 0516 carga 2100 esperas de retrazo. La plataforma usa una referencia nominal de 70 Hz.
    if (!comprobar && !presentacion.marcos.reiniciando() && pantalla == 0 && !fundiendo &&
        (ahora - inicio_pantalla) * 70u / 1000000000u >= 2100u) {
      fundiendo = true;
      paso_fundido = 0;
      inicio_fundido = ahora;
    }
    if (fundiendo && !presentacion.marcos.reiniciando()) {
      const auto paso = (ahora - inicio_fundido) * 70u / 1000000000u;
      if (paso > 32u) {
        ++pantalla;
        if (!cambiar_pantalla(carpeta, pantalla, presentacion))
          return fallo("No se ha podido cargar la pantalla tras el fundido");
        fundiendo = false;
        inicio_pantalla = ahora;
        ultimo_paso_escena = ahora;
      } else if (paso != paso_fundido) {
        paso_fundido = static_cast<unsigned short>(paso);
        if (!motor::calcular_fundido(imagen.paleta, 256, 32, paso_fundido, paleta_visible) ||
            !actualizar_textura(presentacion))
          return fallo("No se ha podido avanzar el fundido original");
      }
    }
    if (!comprobar && !presentacion.marcos.reiniciando() && pantalla == 2) {
      if (!introduccion.avanzar((ahora - inicio_pantalla) * 70u / 1000000000u, entrada_introduccion,
                                imagen))
        return fallo("No se ha podido avanzar la introducción original");
      entrada_introduccion = false;
      for (unsigned int i = 0; i < 768; ++i)
        paleta_visible[i] = imagen.paleta[i];
      if (introduccion.terminada()) {
        pantalla = 3;
        if (!cambiar_pantalla(carpeta, pantalla, presentacion))
          return fallo("No se ha podido entrar en la mazmorra");
        ultimo_paso_escena = ahora;
      } else if (!actualizar_textura(presentacion))
        return fallo("No se ha podido dibujar la introducción");
    }
    if (!comprobar && !presentacion.marcos.reiniciando() && pantalla >= 3 &&
        mazmorra.en_presentacion()) {
      if (!mazmorra.avanzar_presentacion(ahora * 70u / 1000000000u) || !mazmorra.dibujar(imagen))
        return fallo("No se ha podido avanzar el cambio de habitación o la cámara");
      for (unsigned int i = 0; i < 768; ++i)
        paleta_visible[i] = imagen.paleta[i];
      if (!actualizar_textura(presentacion))
        return fallo("No se ha podido presentar el cambio de habitación o la cámara");
      ultimo_paso_escena = ahora;
    }
    // EAD5 espera cinco ticks del temporizador. Mantiene la referencia nominal de 70 Hz de esta
    // plataforma.
    if (partidas.abierta())
      ultimo_paso_escena = ahora;
    if (!comprobar && !presentacion.marcos.reiniciando() && !partidas.abierta() && pantalla >= 3 &&
        ahora - ultimo_paso_escena >= 5000000000ull / 70u) {
      if (!mazmorra.avanzar() ||
          !mazmorra.dibujar(imagen, static_cast<unsigned int>(SDL_GetTicks() / 57u))) {
        std::fprintf(stderr, "No se ha podido avanzar la escena con sus datos originales.\n");
        return 1;
      }
      for (unsigned int i = 0; i < 768; ++i)
        paleta_visible[i] = imagen.paleta[i];
      if (!actualizar_textura(presentacion))
        return fallo("No se ha podido actualizar la paleta de la escena");
      ultimo_paso_escena = ahora;
    }
    // Refleja también las salidas normales y las cargas, sin consultar la vista F5/F6 anterior.
    if (pantalla >= 3) {
      const auto actual = mazmorra.escena_actual();
      if (actual != escena_titulada) {
        const std::string titulo = std::string("Curse Reborn — ") + nombre_numerado_escena(actual);
        if (!SDL_SetWindowTitle(presentacion.ventana, titulo.c_str()))
          return fallo("No se ha podido mostrar el nombre de la escena");
        escena_titulada = actual;
      }
    } else
      escena_titulada = 0xffff;
    if (pantalla >= 3 && DecoradosAmpliados::buscar(mazmorra.escena_actual()) &&
        !actualizar_textura(presentacion, true))
      return fallo("No se ha podido componer el decorado ampliado");
    if (comprobar_marcos && (!comprobar_animacion_marcos || paso_aspecto > 0))
      reloj_marcos_prueba += comprobar_animacion_marcos ? 125000000ull : 300000000ull;
    if (!presentacion.marcos.preparar_encuadre(
            presentacion.dibujante, comprobar_marcos ? reloj_marcos_prueba : SDL_GetTicksNS()))
      return fallo("No se ha podido actualizar el encuadre del juego");
    const auto area_imagen = presentacion.marcos.destino_juego();
    if (!SDL_SetRenderDrawColor(presentacion.dibujante, 0, 0, 0, 255) ||
        !SDL_RenderClear(presentacion.dibujante) ||
        (area_imagen.h > 0 &&
         !SDL_RenderTexture(presentacion.dibujante,
                            presentacion.filtros.textura_visible(presentacion.textura), nullptr,
                            &area_imagen))) {
      return fallo("No se ha podido dibujar el título");
    }
    const std::string contexto_marco = opcion == "--comprobar-marcos-mazmorra"   ? "escena_0"
                                       : opcion == "--comprobar-marcos-corredor" ? "escena_1"
                                       : opcion == "--comprobar-marcos-lago"     ? "escena_3"
                                       : pantalla == 0                           ? "core"
                                       : pantalla == 1                           ? "titulo"
                                       : pantalla == 2
                                           ? "introduccion"
                                           : "escena_" + std::to_string(mazmorra.escena_actual());
    if (!presentacion.marcos.dibujar(presentacion.dibujante, contexto_marco.c_str(),
                                     comprobar_marcos ? reloj_marcos_prueba : SDL_GetTicksNS()))
      return fallo("No se ha podido dibujar el marco de pantalla");
    if (!presentacion.marcos.reiniciando() && pantalla >= 3 && depuracion &&
        !mazmorra.en_presentacion() &&
        !mazmorra.depurar(presentacion.dibujante, cajas_objetos, cajas_interaccion, cajas_limites))
      return fallo("No se han podido dibujar las cajas de depuración");
    if (!presentacion.marcos.reiniciando() && !contador_fps.dibujar(presentacion.dibujante))
      return fallo("No se ha podido dibujar el contador FPS");
    if (!presentacion.marcos.reiniciando() && !partidas.dibujar(presentacion.dibujante, mazmorra))
      return fallo("No se ha podido dibujar el panel de partidas");
    if (!comprobar && !presentacion.cursor.actualizar(
                          presentacion.ventana, presentacion.dibujante, mazmorra,
                          pantalla >= 3 && !presentacion.marcos.reiniciando(),
                          partidas.incorpora_cursor() && !presentacion.marcos.reiniciando()))
      return fallo("No se ha podido actualizar el cursor original");
    if (comprobar && !capturada && (!comprobar_transicion || pantalla == 1)) {
      // La prueba del HUD incluye las bandas físicas donde se coloca el contador al maximizar.
      if ((comprobar_fps || comprobar_marcos) &&
          (!SDL_SetRenderViewport(presentacion.dibujante, nullptr) ||
           !SDL_SetRenderScale(presentacion.dibujante, 1, 1) ||
           !SDL_SetRenderLogicalPresentation(presentacion.dibujante, 0, 0,
                                             SDL_LOGICAL_PRESENTATION_DISABLED)))
        return fallo("No se ha podido preparar la captura completa del contador");
      SDL_Surface *captura = SDL_RenderReadPixels(presentacion.dibujante, nullptr);
      if ((comprobar_fps || comprobar_marcos) &&
          !SDL_SetRenderLogicalPresentation(presentacion.dibujante, 320, 200,
                                            SDL_LOGICAL_PRESENTATION_LETTERBOX)) {
        SDL_DestroySurface(captura);
        return fallo("No se ha podido recuperar el encuadre después de capturar el contador");
      }
      if (!captura)
        return fallo("No se ha podido capturar el fotograma");
      SDL_Surface *rgb = SDL_ConvertSurface(captura, SDL_PIXELFORMAT_RGB24);
      SDL_DestroySurface(captura);
      if (!rgb)
        return fallo("No se ha podido convertir la captura");
      const std::string ruta_captura =
          comprobar_filtros || comprobar_fps || comprobar_marcos
              ? std::string(argv[3]) + "." + std::to_string(paso_aspecto) + ".bmp"
              : argv[3];
      const bool guardado = SDL_SaveBMP(rgb, ruta_captura.c_str());
      SDL_DestroySurface(rgb);
      if (!guardado)
        return fallo("No se ha podido guardar el fotograma");
      if (comprobar_lago && !mazmorra.guardar_estado_diagnostico(std::string(argv[3]) + ".ds.bin"))
        return fallo("No se ha podido guardar el estado diagnóstico de la captura");
      if ((comprobar_filtros && paso_aspecto < 6) ||
          (comprobar_animacion_marcos && paso_aspecto < 4) ||
          ((comprobar_fps && paso_aspecto < 2) ||
           (comprobar_marcos && !comprobar_animacion_marcos && paso_aspecto < 3))) {
        const bool pulsar = !comprobar_animacion_marcos || paso_aspecto == 0 || paso_aspecto == 2;
        ++paso_aspecto;
        SDL_Event filtro{};
        filtro.type = SDL_EVENT_KEY_DOWN;
        filtro.key.key = comprobar_marcos ? SDLK_F1 : comprobar_fps ? SDLK_F12 : SDLK_F11;
        if (pulsar && !SDL_PushEvent(&filtro))
          return fallo("No se ha podido comprobar el atajo de presentación");
        // Dos pulsaciones rápidas saltan el monitor y verifican la salida animada del clásico.
        if (comprobar_animacion_marcos && paso_aspecto == 3 && !SDL_PushEvent(&filtro))
          return fallo("No se ha podido completar el ciclo animado de F1");
      } else {
        capturada = true;
        // La comprobación termina por el mismo tratamiento de Escape que la ventana normal.
        SDL_Event cierre{};
        cierre.type = SDL_EVENT_KEY_DOWN;
        cierre.key.key = SDLK_ESCAPE;
        if (!SDL_PushEvent(&cierre))
          return fallo("No se ha podido comprobar el cierre con Escape");
      }
    }
    if (!SDL_RenderPresent(presentacion.dibujante))
      return fallo("No se ha podido presentar el título");
    if (!salir && !comprobar && limitar_presentacion) {
      siguiente_presentacion += intervalo_presentacion_ns;
      const Uint64 fin = SDL_GetTicksNS();
      if (siguiente_presentacion > fin)
        SDL_DelayPrecise(siguiente_presentacion - fin);
      else
        // Tras una carga o pausa, reanuda sin acumular fotogramas atrasados.
        siguiente_presentacion = fin;
    }
  }
  return 0;
}
