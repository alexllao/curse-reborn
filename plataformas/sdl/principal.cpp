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
#include "vistas.hpp"
#include "comprobar_cuevas.hpp"
#include "comprobar_catapulta.hpp"
#include "comprobar_final_cuevas.hpp"
#include "comprobar_campo.hpp"
#include "partidas.hpp"
#include "comprobar_partidas.hpp"
#include "marcos.hpp"

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
  // Libera los objetos de la plataforma al cerrar o al fallar la inicialización.
  ~Presentacion() {
    mazmorra.cerrar_audio();
    filtros.cerrar();
    marcos.cerrar();
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
bool actualizar_textura(Presentacion &presentacion) {
  // Adaptación de color de seis bits a RGBA; no sustituye la lógica original del juego.
  for (unsigned int i = 0; i < 64000; ++i) {
    const auto color = imagen.indices[i] * 3u;
    for (unsigned int componente = 0; componente < 3; ++componente) {
      pixeles[i * 4 + componente] =
          static_cast<unsigned char>(paleta_visible[color + componente] * 255u / 63u);
    }
    pixeles[i * 4 + 3] = 255;
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
      std::string("Curse Reborn — ") + nombre + (pantalla >= 3 ? " (vista de desarrollo)" : "");
  return actualizar_textura(presentacion) &&
         SDL_SetWindowTitle(presentacion.ventana, titulo.c_str());
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
  unsigned short escena_titulada = 0xffff;
  // Libera las flechas antes de pausar para que el personaje no siga caminando al cerrar el panel.
  const auto abrir_partidas = [&](unsigned int modo) {
    for (const auto bit : {1, 2, 4, 8})
      mazmorra.tecla_direccion(static_cast<unsigned short>(bit), false);
    partidas.abrir(modo, presentacion.ventana);
  };
  Uint64 reloj_marcos_prueba = 1000000000ull;
  while (!salir) {
    if (!mazmorra.actualizar_audio())
      return fallo("No se ha podido actualizar el audio del juego");
    SDL_Event evento;
    while (SDL_PollEvent(&evento)) {
      if (evento.type == SDL_EVENT_QUIT ||
          (evento.type == SDL_EVENT_KEY_DOWN && evento.key.key == SDLK_ESCAPE &&
           (pantalla < 3 || comprobar)))
        salir = true;
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
          presentacion.marcos.alternar(comprobar_marcos ? reloj_marcos_prueba : SDL_GetTicksNS());
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
    const auto solicitud = mazmorra.tomar_solicitud_partidas();
    if (solicitud)
      abrir_partidas(solicitud);
    if (comprobar_vistas && !capturada) {
      const auto indice = paso_vistas < 2                      ? paso_vistas
                          : paso_vistas <= cantidad_vistas + 1 ? paso_vistas - 2
                                                               : cantidad_vistas * 2 - paso_vistas;
      if (mazmorra.escena_actual() != vistas_desarrollo[indice].escena)
        return fallo("F5/F6 no han respetado el orden de las escenas");
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
        if (opcion.rfind("--comprobar-final-campo-", 0) == 0 &&
            !comprobar_recorrido_campo(mazmorra, opcion, avanzar))
          return fallo("No coincide el recorrido SDL de la salida al campo");
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
    // 0516 carga 2100 esperas de retrazo. La plataforma usa una referencia nominal de 70 Hz.
    if (!comprobar && pantalla == 0 && !fundiendo &&
        (ahora - inicio_pantalla) * 70u / 1000000000u >= 2100u) {
      fundiendo = true;
      paso_fundido = 0;
      inicio_fundido = ahora;
    }
    if (fundiendo) {
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
    if (!comprobar && pantalla == 2) {
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
    if (!comprobar && pantalla >= 3 && mazmorra.en_presentacion()) {
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
    if (!comprobar && !partidas.abierta() && pantalla >= 3 &&
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
        const std::string titulo = std::string("Curse Reborn — ") + nombre_escena(actual);
        if (!SDL_SetWindowTitle(presentacion.ventana, titulo.c_str()))
          return fallo("No se ha podido mostrar el nombre de la escena");
        escena_titulada = actual;
      }
    } else
      escena_titulada = 0xffff;
    if (!SDL_SetRenderDrawColor(presentacion.dibujante, 0, 0, 0, 255) ||
        !SDL_RenderClear(presentacion.dibujante) ||
        !SDL_RenderTexture(presentacion.dibujante,
                           presentacion.filtros.textura_visible(presentacion.textura), nullptr,
                           nullptr)) {
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
    if (comprobar_marcos && (!comprobar_animacion_marcos || paso_aspecto > 0))
      reloj_marcos_prueba += comprobar_animacion_marcos ? 125000000ull : 300000000ull;
    if (!presentacion.marcos.dibujar(presentacion.dibujante, contexto_marco.c_str(),
                                     comprobar_marcos ? reloj_marcos_prueba : SDL_GetTicksNS()))
      return fallo("No se ha podido dibujar el marco de pantalla");
    if (pantalla >= 3 && depuracion && !mazmorra.en_presentacion() &&
        !mazmorra.depurar(presentacion.dibujante, cajas_objetos, cajas_interaccion, cajas_limites))
      return fallo("No se han podido dibujar las cajas de depuración");
    if (!contador_fps.dibujar(presentacion.dibujante))
      return fallo("No se ha podido dibujar el contador FPS");
    if (!partidas.dibujar(presentacion.dibujante, mazmorra))
      return fallo("No se ha podido dibujar el panel de partidas");
    if (comprobar && !capturada && (!comprobar_transicion || pantalla == 1)) {
      // La prueba del HUD incluye las bandas físicas donde se coloca el contador al maximizar.
      if ((comprobar_fps || comprobar_marcos) &&
          !SDL_SetRenderLogicalPresentation(presentacion.dibujante, 0, 0,
                                            SDL_LOGICAL_PRESENTATION_DISABLED))
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
          ((comprobar_fps || (comprobar_marcos && !comprobar_animacion_marcos)) &&
           paso_aspecto < 2)) {
        const bool pulsar = !comprobar_animacion_marcos || paso_aspecto == 0 || paso_aspecto == 2;
        ++paso_aspecto;
        SDL_Event filtro{};
        filtro.type = SDL_EVENT_KEY_DOWN;
        filtro.key.key = comprobar_marcos ? SDLK_F1 : comprobar_fps ? SDLK_F12 : SDLK_F11;
        if (pulsar && !SDL_PushEvent(&filtro))
          return fallo("No se ha podido comprobar el atajo de presentación");
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
