#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <cstdio>
#include <string>
#include "../../motor/recursos/titulo.hpp"
#include "../../motor/recursos/escena.hpp"
#include "../../motor/video/sprites.hpp"
#include "../../motor/video/fundido.hpp"
#include "mazmorra.hpp"
#include "introduccion.hpp"

namespace {
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

// 06B2 y 50D2: prepara el fondo y delega la entrada, los objetos y sus guiones en el adaptador.
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
  return preparada && mazmorra.iniciar(carpeta, imagen) && mazmorra.dibujar(imagen);
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

// Recorre las vistas de desarrollo; cada entrada a la mazmorra prepara una sesión nueva sin
// guardar.
bool cambiar_pantalla(const std::string &carpeta, int pantalla, Presentacion &presentacion) {
  const bool cargada =
      pantalla == 3   ? cargar_mazmorra(carpeta)
      : pantalla == 2 ? (cargar_imagen(carpeta, "TITLE") && introduccion.iniciar(carpeta, imagen))
                      : cargar_imagen(carpeta, pantalla == 0 ? "CORE" : "TITLE");
  if (!cargada)
    return false;
  for (unsigned int i = 0; i < 768; ++i)
    paleta_visible[i] = imagen.paleta[i];
  return actualizar_textura(presentacion.textura) &&
         SDL_SetWindowTitle(presentacion.ventana,
                            pantalla == 0   ? "Curse Reborn — Core Design"
                            : pantalla == 1 ? "Curse Reborn — Pantalla de título"
                            : pantalla == 2 ? "Curse Reborn — Introducción original"
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
  const std::string opcion = argc == 4 ? argv[2] : "";
  const bool comprobar_introduccion =
      opcion == "--comprobar-caldero" || opcion == "--comprobar-balcon" ||
      opcion == "--comprobar-beisbol" || opcion == "--comprobar-fin-intro" ||
      opcion == "--comprobar-salto-intro";
  const bool comprobar_jugabilidad =
      opcion == "--comprobar-liberacion" || opcion == "--comprobar-movimiento";
  const bool comprobar_menu = opcion == "--comprobar-menu" || opcion == "--comprobar-orden" ||
                              opcion == "--comprobar-depuracion";
  const bool comprobar_animacion = argc == 4 && std::string(argv[2]) == "--comprobar-animacion";
  const bool comprobar_escena =
      argc == 4 && (std::string(argv[2]) == "--comprobar-escena" || comprobar_animacion ||
                    comprobar_menu || comprobar_jugabilidad);
  const bool comprobar_core = argc == 4 && std::string(argv[2]) == "--comprobar-core";
  const bool comprobar_transicion = argc == 4 && std::string(argv[2]) == "--comprobar-transicion";
  const bool comprobar =
      argc == 4 && (std::string(argv[2]) == "--comprobar" || comprobar_escena || comprobar_core ||
                    comprobar_transicion || comprobar_introduccion);
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
  }
  bool salir = false, capturada = false, fundiendo = false;
  bool animacion_preparada = false, introduccion_comprobada = false, menu_comprobado = false,
       jugabilidad_comprobada = false;
  bool entrada_introduccion = false;
  bool depuracion = false, cajas_objetos = true, cajas_interaccion = true, cajas_limites = true;
  unsigned short puntero_x = 152, puntero_y = 92;
  int pantalla = comprobar_introduccion                                  ? 2
                 : comprobar && !comprobar_core && !comprobar_transicion ? 1
                                                                         : 0;
  unsigned short paso_fundido = 0;
  Uint64 inicio_pantalla = SDL_GetTicksNS(), inicio_fundido = 0;
  Uint64 ultimo_paso_escena = inicio_pantalla;
  while (!salir) {
    SDL_Event evento;
    while (SDL_PollEvent(&evento)) {
      if (evento.type == SDL_EVENT_QUIT ||
          (evento.type == SDL_EVENT_KEY_DOWN && evento.key.key == SDLK_ESCAPE &&
           (pantalla != 3 || comprobar)))
        salir = true;
      if (!salir && pantalla == 3 &&
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
        if (pantalla == 3) {
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
            if (!actualizar_textura(presentacion.textura))
              return fallo("No se ha podido dibujar el menú");
          }
        }
        const bool avanzar = pantalla < 3 && evento.key.key == SDLK_F6;
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
      if (!salir && pantalla == 3 &&
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
          if (!actualizar_textura(presentacion.textura))
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
        if (!actualizar_textura(presentacion.textura))
          return fallo("No se ha podido comprobar la imagen de la introducción");
      }
      introduccion_comprobada = true;
    }
    if (comprobar_jugabilidad && pantalla == 3 && !jugabilidad_comprobada) {
      // Recorre los mismos clics del menú que el jugador, sin cambiar directamente su inventario.
      const auto elegir = [](unsigned short casilla) {
        return mazmorra.entrada_menu(static_cast<unsigned short>(casilla * 32 + 16), 92, true,
                                     false);
      };
      // Avanza los pasos de juego de forma determinista para comparar la imagen resultante.
      const auto avanzar = [](unsigned int cantidad) {
        for (unsigned int i = 0; i < cantidad; ++i)
          if (!mazmorra.avanzar())
            return false;
        return true;
      };
      if (!avanzar(1) || !mazmorra.entrada_menu(152, 92, false, true) || !elegir(4) || !elegir(1) ||
          !avanzar(192) || !mazmorra.entrada_menu(152, 92, false, true) || !elegir(1) ||
          !elegir(0) || !avanzar(8) || !mazmorra.entrada_menu(152, 92, false, true))
        return fallo("No se ha podido preparar la liberación desde el menú");
      for (const auto casilla : {2, 0, 0, 0, 1})
        if (!elegir(static_cast<unsigned short>(casilla)))
          return fallo("No se ha podido combinar la llave y las esposas");
      if (mazmorra.menu_abierto() || !avanzar(80))
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
      if (!mazmorra.dibujar(imagen))
        return fallo("No se ha podido dibujar el estado jugable");
      for (unsigned int i = 0; i < 768; ++i)
        paleta_visible[i] = imagen.paleta[i];
      if (!actualizar_textura(presentacion.textura))
        return fallo("No se ha podido presentar el movimiento");
      jugabilidad_comprobada = true;
    }
    if (comprobar_menu && pantalla == 3 && !menu_comprobado) {
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
      if (!actualizar_textura(presentacion.textura))
        return fallo("No se ha podido presentar la interacción");
      menu_comprobado = true;
    }
    if (comprobar_animacion && pantalla == 3 && !animacion_preparada) {
      for (unsigned int paso = 0; paso < 32; ++paso) {
        if (!mazmorra.avanzar() ||
            !mazmorra.dibujar(imagen,
                              static_cast<unsigned int>(comprobar ? 0 : SDL_GetTicks() / 57u)))
          return fallo("No se ha podido comprobar la animación de los objetos");
      }
      if (!actualizar_textura(presentacion.textura))
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
            !actualizar_textura(presentacion.textura))
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
      } else if (!actualizar_textura(presentacion.textura))
        return fallo("No se ha podido dibujar la introducción");
    }
    // EAD5 espera cinco ticks del temporizador. Mantiene la referencia nominal de 70 Hz de esta
    // plataforma.
    if (!comprobar && pantalla == 3 && ahora - ultimo_paso_escena >= 5000000000ull / 70u) {
      if (!mazmorra.avanzar() ||
          !mazmorra.dibujar(imagen,
                            static_cast<unsigned int>(comprobar ? 0 : SDL_GetTicks() / 57u)) ||
          !actualizar_textura(presentacion.textura)) {
        std::fprintf(stderr, "No se ha podido avanzar la escena con sus datos originales.\n");
        return 1;
      }
      ultimo_paso_escena = ahora;
    }
    if (!SDL_SetRenderDrawColor(presentacion.dibujante, 0, 0, 0, 255) ||
        !SDL_RenderClear(presentacion.dibujante) ||
        !SDL_RenderTexture(presentacion.dibujante, presentacion.textura, nullptr, nullptr)) {
      return fallo("No se ha podido dibujar el título");
    }
    if (pantalla == 3 && depuracion &&
        !mazmorra.depurar(presentacion.dibujante, cajas_objetos, cajas_interaccion, cajas_limites))
      return fallo("No se han podido dibujar las cajas de depuración");
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
