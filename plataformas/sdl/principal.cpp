#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <cstdio>
#include <string>
#include "../../motor/recursos/titulo.hpp"
#include "../../motor/recursos/escena.hpp"
#include "../../motor/video/sprites.hpp"
#include "../../motor/video/fundido.hpp"

namespace {
motor::ImagenTitulo imagen;
unsigned char planos[262144];
unsigned char pixeles[64000 * 4];
unsigned char tira[6400];
unsigned char paleta_visible[768];

struct Presentacion {
  SDL_Window *ventana = nullptr;
  SDL_Renderer *dibujante = nullptr;
  SDL_Texture *textura = nullptr;
  // Libera los objetos de la plataforma al cerrar o al fallar la inicialización.
  ~Presentacion() {
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

// 06B2, DS:859E y 50D2: vista inicial de la escena cero, antes de avanzar sus guiones.
// El descriptor original selecciona MANA.BRD, imagen cero, en (125, 98).
bool cargar_mazmorra(const std::string &carpeta) {
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
  size_t longitud = 0, longitud_personaje = 0;
  void *mapa = SDL_LoadFile((carpeta + "/DUNGEON.MAP").c_str(), &longitud);
  if (!mapa)
    return false;
  void *personaje = SDL_LoadFile((carpeta + "/MANA.BRD").c_str(), &longitud_personaje);
  if (!personaje) {
    SDL_free(mapa);
    return false;
  }
  // DS:7648 contiene diez tiras consecutivas para la mazmorra.
  const unsigned char seleccion[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  const bool preparada = longitud <= 0xffffffffu && longitud_personaje <= 0xffffffffu &&
                         motor::preparar_fondo_mapa(static_cast<const unsigned char *>(mapa),
                                                    static_cast<unsigned int>(longitud), seleccion,
                                                    imagen, planos, tira) &&
                         motor::dibujar_fotograma(static_cast<const unsigned char *>(personaje),
                                                  static_cast<unsigned int>(longitud_personaje), 0,
                                                  imagen.indices, 125, 98, false);
  SDL_free(personaje);
  SDL_free(mapa);
  return preparada;
}

// Adapta la paleta VGA de seis bits a RGBA y actualiza la textura sin filtros adicionales.
bool actualizar_textura(SDL_Texture *textura) {
  // Adaptación de color de seis bits a RGBA; no sustituye la lógica original del juego.
  for (unsigned int i = 0; i < 64000; ++i) {
    const auto color = imagen.indices[i] * 3u;
    for (unsigned int componente = 0; componente < 3; ++componente) {
      pixeles[i * 4 + componente] =
          static_cast<unsigned char>(paleta_visible[color + componente] * 255u / 63u);
    }
    pixeles[i * 4 + 3] = 255;
  }
  return SDL_UpdateTexture(textura, nullptr, pixeles, 320 * 4);
}

// Cambia solo la presentación de desarrollo: no altera estados de puzles ni partidas guardadas.
bool cambiar_pantalla(const std::string &carpeta, int pantalla, Presentacion &presentacion) {
  const bool cargada = pantalla == 2 ? cargar_mazmorra(carpeta)
                                     : cargar_imagen(carpeta, pantalla == 0 ? "CORE" : "TITLE");
  if (!cargada)
    return false;
  for (unsigned int i = 0; i < 768; ++i)
    paleta_visible[i] = imagen.paleta[i];
  return actualizar_textura(presentacion.textura) &&
         SDL_SetWindowTitle(presentacion.ventana,
                            pantalla == 0   ? "Curse Reborn — Core Design"
                            : pantalla == 1 ? "Curse Reborn — Pantalla de título"
                                            : "Curse Reborn — Mazmorra inicial (reconstrucción)");
}

// Presenta los bloques reconstruidos y permite recorrerlos con los atajos de desarrollo.
int main(int argc, char *argv[]) {
  if (argc != 2 && argc != 4) {
    std::fprintf(
        stderr,
        "Uso: curse-reborn <carpeta DATA> "
        "[--comprobar|--comprobar-core|--comprobar-transicion|--comprobar-escena <captura.bmp>]\n");
    return 1;
  }
  const bool comprobar_escena = argc == 4 && std::string(argv[2]) == "--comprobar-escena";
  const bool comprobar_core = argc == 4 && std::string(argv[2]) == "--comprobar-core";
  const bool comprobar_transicion = argc == 4 && std::string(argv[2]) == "--comprobar-transicion";
  const bool comprobar = argc == 4 && (std::string(argv[2]) == "--comprobar" || comprobar_escena ||
                                       comprobar_core || comprobar_transicion);
  if (argc == 4 && !comprobar) {
    std::fprintf(stderr, "Opción desconocida de comprobación.\n");
    return 1;
  }
  const std::string carpeta = argv[1];
  Presentacion presentacion;
  if (!SDL_Init(SDL_INIT_VIDEO))
    return fallo("No se ha podido iniciar el vídeo");
  const SDL_WindowFlags opciones = SDL_WINDOW_RESIZABLE | (comprobar ? SDL_WINDOW_HIDDEN : 0);
  if (!SDL_CreateWindowAndRenderer("Curse Reborn — Pantalla de título", comprobar ? 320 : 960,
                                   comprobar ? 200 : 600, opciones, &presentacion.ventana,
                                   &presentacion.dibujante)) {
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
      !cambiar_pantalla(carpeta, comprobar && !comprobar_core && !comprobar_transicion ? 1 : 0,
                        presentacion))
    return fallo("No se ha podido crear la imagen");
  if (comprobar_escena || comprobar_transicion) {
    SDL_Event avance{};
    avance.type = SDL_EVENT_KEY_DOWN;
    avance.key.key = comprobar_transicion ? SDLK_RETURN : SDLK_F6;
    if (!SDL_PushEvent(&avance))
      return fallo("No se ha podido comprobar el avance con F6");
  }
  bool salir = false, capturada = false, fundiendo = false;
  int pantalla = comprobar && !comprobar_core && !comprobar_transicion ? 1 : 0;
  unsigned short paso_fundido = 0;
  Uint64 inicio_pantalla = SDL_GetTicksNS(), inicio_fundido = 0;
  while (!salir) {
    SDL_Event evento;
    while (SDL_PollEvent(&evento)) {
      if (evento.type == SDL_EVENT_QUIT ||
          (evento.type == SDL_EVENT_KEY_DOWN && evento.key.key == SDLK_ESCAPE))
        salir = true;
      if (!salir && evento.type == SDL_EVENT_KEY_DOWN && !evento.key.repeat) {
        const bool avanzar = pantalla < 2 && evento.key.key == SDLK_F6;
        const bool retroceder = pantalla > 0 && evento.key.key == SDLK_F5;
        if (avanzar || retroceder) {
          pantalla += avanzar ? 1 : -1;
          if (!cambiar_pantalla(carpeta, pantalla, presentacion)) {
            std::fprintf(
                stderr,
                "No se ha podido reconstruir la pantalla solicitada con sus datos originales.\n");
            return 1;
          }
          fundiendo = false;
          inicio_pantalla = SDL_GetTicksNS();
        } else if (!fundiendo && pantalla < 2 && evento.key.key != SDLK_F5 &&
                   evento.key.key != SDLK_F6) {
          fundiendo = true;
          paso_fundido = 0;
          inicio_fundido = SDL_GetTicksNS();
        }
      }
      if (!salir && !fundiendo && pantalla < 2 && evento.type == SDL_EVENT_MOUSE_BUTTON_DOWN &&
          evento.button.button == SDL_BUTTON_LEFT) {
        fundiendo = true;
        paso_fundido = 0;
        inicio_fundido = SDL_GetTicksNS();
      }
    }
    if (salir)
      break;
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
      } else if (paso != paso_fundido) {
        paso_fundido = static_cast<unsigned short>(paso);
        if (!motor::calcular_fundido(imagen.paleta, 256, 32, paso_fundido, paleta_visible) ||
            !actualizar_textura(presentacion.textura))
          return fallo("No se ha podido avanzar el fundido original");
      }
    }
    if (!SDL_SetRenderDrawColor(presentacion.dibujante, 0, 0, 0, 255) ||
        !SDL_RenderClear(presentacion.dibujante) ||
        !SDL_RenderTexture(presentacion.dibujante, presentacion.textura, nullptr, nullptr)) {
      return fallo("No se ha podido dibujar el título");
    }
    if (comprobar && !capturada && (!comprobar_transicion || pantalla == 1)) {
      SDL_Surface *captura = SDL_RenderReadPixels(presentacion.dibujante, nullptr);
      if (!captura)
        return fallo("No se ha podido capturar el fotograma");
      SDL_Surface *rgb = SDL_ConvertSurface(captura, SDL_PIXELFORMAT_RGB24);
      SDL_DestroySurface(captura);
      if (!rgb)
        return fallo("No se ha podido convertir la captura");
      const bool guardado = SDL_SaveBMP(rgb, argv[3]);
      SDL_DestroySurface(rgb);
      if (!guardado)
        return fallo("No se ha podido guardar el fotograma");
      capturada = true;
      // La comprobación termina por el mismo tratamiento de Escape que la ventana normal.
      SDL_Event cierre{};
      cierre.type = SDL_EVENT_KEY_DOWN;
      cierre.key.key = SDLK_ESCAPE;
      if (!SDL_PushEvent(&cierre))
        return fallo("No se ha podido comprobar el cierre con Escape");
    }
    if (!SDL_RenderPresent(presentacion.dibujante))
      return fallo("No se ha podido presentar el título");
    if (!salir)
      SDL_Delay(8);
  }
  return 0;
}
