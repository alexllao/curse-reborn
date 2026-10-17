#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <cstdio>
#include <string>
#include "../../motor/recursos/titulo.hpp"

namespace {
motor::ImagenTitulo imagen;
unsigned char planos[262144];
unsigned char pixeles[64000 * 4];

struct Presentacion {
  SDL_Window* ventana = nullptr;
  SDL_Renderer* dibujante = nullptr;
  SDL_Texture* textura = nullptr;
  ~Presentacion() {
    if (textura) SDL_DestroyTexture(textura);
    if (dibujante) SDL_DestroyRenderer(dibujante);
    if (ventana) SDL_DestroyWindow(ventana);
    SDL_Quit();
  }
};

int fallo(const char* mensaje) {
  std::fprintf(stderr, "%s: %s\n", mensaje, SDL_GetError());
  return 1;
}
}

int main(int argc, char* argv[]) {
  if (argc != 2 && argc != 4) {
    std::fprintf(stderr, "Uso: curse-reborn <carpeta DATA> [--comprobar <captura.bmp>]\n");
    return 1;
  }
  const bool comprobar = argc == 4 && std::string(argv[2]) == "--comprobar";
  if (argc == 4 && !comprobar) {
    std::fprintf(stderr, "Opción desconocida. Usa --comprobar para guardar un fotograma.\n");
    return 1;
  }
  const std::string carpeta = argv[1];
  size_t longitud = 0, longitud_paleta = 0;
  void* comprimido = SDL_LoadFile((carpeta + "/TITLE.DAT").c_str(), &longitud);
  if (!comprimido) return fallo("No se ha podido leer TITLE.DAT");
  void* paleta = SDL_LoadFile((carpeta + "/TITLE.PAL").c_str(), &longitud_paleta);
  if (!paleta) { SDL_free(comprimido); return fallo("No se ha podido leer TITLE.PAL"); }
  const bool preparado = longitud <= 0xffffffffu && longitud_paleta <= 0xffffffffu &&
    motor::preparar_titulo(static_cast<const unsigned char*>(comprimido), static_cast<unsigned int>(longitud),
      static_cast<const unsigned char*>(paleta), static_cast<unsigned int>(longitud_paleta), imagen, planos);
  SDL_free(paleta); SDL_free(comprimido);
  if (!preparado) {
    std::fprintf(stderr, "No se ha podido reconstruir el título a partir de los datos originales.\n");
    return 1;
  }
  // Adaptación de color de seis bits a RGBA; no sustituye la lógica original del juego.
  for (unsigned int i = 0; i < 64000; ++i) {
    const auto color = imagen.indices[i] * 3u;
    for (unsigned int componente = 0; componente < 3; ++componente) {
      pixeles[i * 4 + componente] = static_cast<unsigned char>(imagen.paleta[color + componente] * 255u / 63u);
    }
    pixeles[i * 4 + 3] = 255;
  }
  Presentacion presentacion;
  if (!SDL_Init(SDL_INIT_VIDEO)) return fallo("No se ha podido iniciar el vídeo");
  const SDL_WindowFlags opciones = SDL_WINDOW_RESIZABLE | (comprobar ? SDL_WINDOW_HIDDEN : 0);
  if (!SDL_CreateWindowAndRenderer("Curse Reborn — Pantalla de título", comprobar ? 320 : 960,
      comprobar ? 200 : 600, opciones, &presentacion.ventana, &presentacion.dibujante)) {
    return fallo("No se ha podido crear la ventana");
  }
  if (!SDL_SetWindowMinimumSize(presentacion.ventana, 320, 200) ||
      !SDL_SetRenderLogicalPresentation(presentacion.dibujante, 320, 200, SDL_LOGICAL_PRESENTATION_INTEGER_SCALE)) {
    return fallo("No se ha podido configurar la presentación");
  }
  presentacion.textura = SDL_CreateTexture(presentacion.dibujante, SDL_PIXELFORMAT_RGBA32,
                                          SDL_TEXTUREACCESS_STATIC, 320, 200);
  if (!presentacion.textura || !SDL_SetTextureScaleMode(presentacion.textura, SDL_SCALEMODE_NEAREST) ||
      !SDL_UpdateTexture(presentacion.textura, nullptr, pixeles, 320 * 4)) return fallo("No se ha podido crear la imagen");
  bool salir = false, capturada = false;
  while (!salir) {
    SDL_Event evento;
    while (SDL_PollEvent(&evento)) {
      if (evento.type == SDL_EVENT_QUIT ||
          (evento.type == SDL_EVENT_KEY_DOWN && evento.key.key == SDLK_ESCAPE)) salir = true;
    }
    if (salir) break;
    if (!SDL_SetRenderDrawColor(presentacion.dibujante, 0, 0, 0, 255) ||
        !SDL_RenderClear(presentacion.dibujante) ||
        !SDL_RenderTexture(presentacion.dibujante, presentacion.textura, nullptr, nullptr)) {
      return fallo("No se ha podido dibujar el título");
    }
    if (comprobar && !capturada) {
      SDL_Surface* captura = SDL_RenderReadPixels(presentacion.dibujante, nullptr);
      if (!captura) return fallo("No se ha podido capturar el fotograma");
      SDL_Surface* rgb = SDL_ConvertSurface(captura, SDL_PIXELFORMAT_RGB24);
      SDL_DestroySurface(captura);
      if (!rgb) return fallo("No se ha podido convertir la captura");
      const bool guardado = SDL_SaveBMP(rgb, argv[3]);
      SDL_DestroySurface(rgb);
      if (!guardado) return fallo("No se ha podido guardar el fotograma");
      capturada = true;
      // La comprobación termina por el mismo tratamiento de Escape que la ventana normal.
      SDL_Event cierre{};
      cierre.type = SDL_EVENT_KEY_DOWN;
      cierre.key.key = SDLK_ESCAPE;
      if (!SDL_PushEvent(&cierre)) return fallo("No se ha podido comprobar el cierre con Escape");
    }
    if (!SDL_RenderPresent(presentacion.dibujante)) return fallo("No se ha podido presentar el título");
    if (!salir) SDL_Delay(16);
  }
  return 0;
}
