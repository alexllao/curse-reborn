#pragma once
#include "fuente.hpp"
#include "../idioma.hpp"
#include "../creditos_port.hpp"
#include <algorithm>
#include <cmath>
#include <vector>

enum class AccionMenuPort {
  ninguna,
  jugar,
  cargar,
  salir,
  volumen,
  volumen_musica,
  volumen_efectos,
  silencio,
  musica,
  efectos,
  estereo,
  filtro,
  marco,
  completa,
  habitacion,
  depuracion,
  entidades,
  interacciones,
  limites
};
struct EstadoMenuPort {
  bool sesion = false, silencio = false, estereo = true, marco = false, completa = false;
  bool depuracion = false, entidades = true, interacciones = true, limites = true;
  unsigned int volumen = 100, volumen_musica = 100, volumen_efectos = 100, musica = 4, filtro = 2;
  std::string efectos = "Sound Blaster", habitacion;
};
struct PeticionMenuPort {
  AccionMenuPort accion = AccionMenuPort::ninguna;
  int sentido = 1;
};

// Interfaz física del port: nunca escribe DS ni interpreta iconos del menú DOS.
class MenuPort {
public:
  bool abierto() const {
    return visible;
  }
  void abrir() {
    visible = true;
    pagina = Pagina::principal;
    seleccion = 0;
  }
  void cerrar() {
    visible = false;
  }
  ~MenuPort() {
    liberar();
  }
  void liberar() {
    fuente.cerrar();
    if (logo)
      SDL_DestroyTexture(logo);
    logo = nullptr;
    if (capa)
      SDL_DestroyTexture(capa);
    capa = nullptr;
  }
  PeticionMenuPort evento(const SDL_Event &e, int ancho_ventana, int alto_ventana,
                          const EstadoMenuPort &estado) {
    if (!visible)
      return {};
    const auto botones = filas(estado);
    if (e.type == SDL_EVENT_MOUSE_MOTION || e.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
      const float x = (e.type == SDL_EVENT_MOUSE_MOTION ? e.motion.x : e.button.x) * 900 /
                      std::max(alto_ventana, 1);
      const float y = (e.type == SDL_EVENT_MOUSE_MOTION ? e.motion.y : e.button.y) * 900 /
                      std::max(alto_ventana, 1);
      const float centro = float(ancho_ventana) / std::max(alto_ventana, 1) * 450;
      for (size_t i = 0; i < botones.size(); ++i) {
        const auto r = rectangulo(centro, i);
        if (x >= r.x && x < r.x + r.w && y >= r.y && y < r.y + r.h) {
          seleccion = int(i);
          if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN && e.button.button == SDL_BUTTON_LEFT)
            return aceptar(botones[i], x < centro ? -1 : 1);
        }
      }
    }
    if (e.type != SDL_EVENT_KEY_DOWN)
      return {};
    if (e.key.key == SDLK_ESCAPE && !e.key.repeat) {
      if (pagina != Pagina::principal) {
        pagina = padre();
        seleccion = 0;
      } else if (estado.sesion)
        return {AccionMenuPort::jugar, 1};
      return {};
    }
    if (e.key.key == SDLK_UP || (e.key.key == SDLK_TAB && (e.key.mod & SDL_KMOD_SHIFT)))
      seleccion = (seleccion + int(botones.size()) - 1) % int(botones.size());
    else if (e.key.key == SDLK_DOWN || e.key.key == SDLK_TAB)
      seleccion = (seleccion + 1) % int(botones.size());
    else if ((e.key.key == SDLK_RETURN || e.key.key == SDLK_KP_ENTER || e.key.key == SDLK_SPACE) &&
             !e.key.repeat)
      return aceptar(botones[seleccion], 1);
    else if ((e.key.key == SDLK_LEFT || e.key.key == SDLK_RIGHT) && botones[seleccion].ajustable)
      return aceptar(botones[seleccion], e.key.key == SDLK_LEFT ? -1 : 1);
    return {};
  }

  bool dibujar(SDL_Renderer *r, const EstadoMenuPort &estado) {
    if (!visible)
      return true;
    SDL_Rect vp;
    const bool con_vp = SDL_RenderViewportSet(r);
    float sx = 1, sy = 1;
    int lw = 0, lh = 0, w = 0, h = 0;
    SDL_RendererLogicalPresentation lp;
    SDL_BlendMode bm;
    SDL_Color color;
    if (!SDL_GetRenderViewport(r, &vp) || !SDL_GetRenderScale(r, &sx, &sy) ||
        !SDL_GetRenderLogicalPresentation(r, &lw, &lh, &lp) ||
        !SDL_GetRenderOutputSize(r, &w, &h) || !SDL_GetRenderDrawBlendMode(r, &bm) ||
        !SDL_GetRenderDrawColor(r, &color.r, &color.g, &color.b, &color.a) || h <= 0)
      return false;
    // Componer a 900 líneas evita que el renderer software elimine trazos de menos de un píxel.
    const int cw = std::max(1, int(float(w) / h * 900));
    if (!capa || capa_ancho != cw) {
      if (capa)
        SDL_DestroyTexture(capa);
      capa = SDL_CreateTexture(r, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, cw, 900);
      capa_ancho = cw;
      if (!capa || !SDL_SetTextureBlendMode(capa, SDL_BLENDMODE_BLEND) ||
          !SDL_SetTextureScaleMode(capa, SDL_SCALEMODE_LINEAR))
        return false;
    }
    SDL_Texture *anterior = SDL_GetRenderTarget(r);
    if (!SDL_SetRenderTarget(r, capa) ||
        !SDL_SetRenderLogicalPresentation(r, 0, 0, SDL_LOGICAL_PRESENTATION_DISABLED) ||
        !SDL_SetRenderViewport(r, nullptr) || !SDL_SetRenderScale(r, 1, 1) ||
        !SDL_SetRenderDrawColor(r, 0, 0, 0, 0) || !SDL_RenderClear(r))
      return false;
    const float ancho = float(cw);
    bool ok = contenido(r, estado, ancho);
    ok = SDL_SetRenderTarget(r, anterior) && ok;
    ok = SDL_SetRenderLogicalPresentation(r, 0, 0, SDL_LOGICAL_PRESENTATION_DISABLED) && ok;
    ok = SDL_SetRenderViewport(r, nullptr) && ok;
    ok = SDL_SetRenderScale(r, 1, 1) && ok;
    const SDL_FRect destino{0, 0, float(w), float(h)};
    ok = SDL_RenderTexture(r, capa, nullptr, &destino) && ok;
    ok = SDL_SetRenderLogicalPresentation(r, lw, lh, lp) && ok;
    ok = SDL_SetRenderViewport(r, con_vp ? &vp : nullptr) && ok;
    ok = SDL_SetRenderScale(r, sx, sy) && ok;
    ok = SDL_SetRenderDrawBlendMode(r, bm) && ok;
    return SDL_SetRenderDrawColor(r, color.r, color.g, color.b, color.a) && ok;
  }

private:
  enum class Pagina {
    principal,
    opciones,
    sonido,
    reproduccion,
    pantalla,
    habitacion,
    avanzado,
    controles,
    creditos,
    salir
  };
  struct Fila {
    std::string texto, valor;
    AccionMenuPort accion = AccionMenuPort::ninguna;
    Pagina destino = Pagina::principal;
    bool ajustable = false;
  };
  bool visible = false;
  Pagina pagina = Pagina::principal;
  int seleccion = 0;
  FuenteMenu fuente;
  SDL_Texture *logo = nullptr, *capa = nullptr;
  int capa_ancho = 0;
  bool logo_buscado = false;
  bool es() const {
    return idioma_interfaz == IdiomaInterfaz::castellano;
  }
  std::string t(const char *castellano, const char *ingles) const {
    return es() ? castellano : ingles;
  }
  std::string si(bool valor) const {
    return valor ? t("Activado", "On") : t("Desactivado", "Off");
  }
  static SDL_FRect rectangulo(float c, size_t i) {
    const auto ancho = std::min(412.0f, std::max(100.0f, c * 2 - 64));
    return {c - ancho / 2, 332 + float(i) * 70, ancho, 57};
  }
  std::vector<Fila> filas(const EstadoMenuPort &s) const {
    using A = AccionMenuPort;
    const Fila atras{t("Volver", "Back"), "", A::ninguna, padre()};
    const auto ajuste = [](std::string texto, std::string valor, A a) {
      return Fila{texto, valor, a, Pagina::principal, true};
    };
    switch (pagina) {
    case Pagina::principal:
      return {{s.sesion ? t("Continuar", "Continue") : t("Jugar", "Play"), "", A::jugar},
              {t("Cargar partida", "Load game"), "", A::cargar},
              {t("Opciones", "Options"), "", A::ninguna, Pagina::opciones},
              {t("Controles", "Controls"), "", A::ninguna, Pagina::controles},
              {t("Créditos", "Credits"), "", A::ninguna, Pagina::creditos},
              {t("Salir", "Quit"), "", A::ninguna, Pagina::salir}};
    case Pagina::opciones:
      return {{t("Sonido", "Sound"), "", A::ninguna, Pagina::sonido},
              {t("Pantalla", "Display"), "", A::ninguna, Pagina::pantalla},
              {t("Habitación", "Room"), "", A::ninguna, Pagina::habitacion},
              {t("Avanzado", "Advanced"), "", A::ninguna, Pagina::avanzado},
              atras};
    case Pagina::sonido:
      return {ajuste(t("Volumen de música", "Music volume"),
                     std::to_string(s.volumen_musica) + " %", A::volumen_musica),
              ajuste(t("Volumen de efectos", "Effects volume"),
                     std::to_string(s.volumen_efectos) + " %", A::volumen_efectos),
              ajuste(t("Volumen general", "Master volume"), std::to_string(s.volumen) + " %",
                     A::volumen),
              ajuste(t("Silencio", "Mute"), si(s.silencio), A::silencio),
              {t("Música y efectos", "Music and effects"), "", A::ninguna, Pagina::reproduccion},
              atras};
    case Pagina::reproduccion: {
      const char *musicas[] = {es() ? "Sin música" : "No music", "AdLib OPL2", "Sound Blaster OPL3",
                               "Roland MT-32", "Remaster"};
      return {ajuste(t("Música", "Music"), musicas[std::min(s.musica, 4u)], A::musica),
              ajuste(t("Efectos", "Sound effects"), s.efectos, A::efectos),
              ajuste(t("Estéreo posicional", "Positional stereo"), si(s.estereo), A::estereo),
              atras};
    }
    case Pagina::pantalla: {
      const char *filtros[] = {"Original",  es() ? "Suavizado" : "Smooth",
                               "Scanlines", es() ? "CRT suave" : "Soft CRT",
                               "xBRZ 2x",   "2xSaI"};
      return {
          ajuste(t("Pantalla completa", "Fullscreen"), si(s.completa), A::completa),
          ajuste(t("Filtro del juego", "Game filter"), filtros[std::min(s.filtro, 5u)], A::filtro),
          ajuste(t("Marco", "Frame"), s.marco ? t("Cielo encantado", "Enchanted sky") : si(false),
                 A::marco),
          atras};
    }
    case Pagina::habitacion:
      return {ajuste(t("Diseño de habitación", "Room design"), s.habitacion, A::habitacion), atras};
    case Pagina::avanzado:
      return {ajuste(t("Depuración", "Debug overlay"), si(s.depuracion), A::depuracion),
              ajuste(t("Entidades", "Entities"), si(s.entidades), A::entidades),
              ajuste(t("Interacciones", "Interactions"), si(s.interacciones), A::interacciones),
              ajuste(t("Límites", "Boundaries"), si(s.limites), A::limites), atras};
    case Pagina::salir:
      return {{t("Salir del juego", "Quit game"), "", A::salir}, atras};
    default:
      return {atras};
    }
  }
  Pagina padre() const {
    if (pagina == Pagina::reproduccion)
      return Pagina::sonido;
    return pagina == Pagina::sonido || pagina == Pagina::pantalla || pagina == Pagina::habitacion ||
                   pagina == Pagina::avanzado
               ? Pagina::opciones
               : Pagina::principal;
  }
  PeticionMenuPort aceptar(const Fila &f, int sentido) {
    if (f.accion != AccionMenuPort::ninguna)
      return {f.accion, f.ajustable ? sentido : 1};
    pagina = f.destino;
    seleccion = 0;
    return {};
  }
  bool bloque(SDL_Renderer *r, SDL_FRect rect, SDL_Color color) {
    return SDL_SetRenderDrawColor(r, color.r, color.g, color.b, color.a) &&
           SDL_RenderFillRect(r, &rect);
  }
  bool borde(SDL_Renderer *r, SDL_FRect rect, SDL_Color color) {
    return SDL_SetRenderDrawColor(r, color.r, color.g, color.b, color.a) &&
           SDL_RenderRect(r, &rect);
  }
  bool ornamento(SDL_Renderer *r, float x, float y, float dx, float dy) {
    if (!SDL_SetRenderDrawColor(r, 215, 173, 88, 255))
      return false;
    for (int j = 0; j < 2; ++j) {
      SDL_FPoint p[25];
      for (int i = 0; i < 25; ++i) {
        const float a = i * .22f, radio = 15 - i * .48f;
        const float u = 18 + std::cos(a) * radio, v = 18 + std::sin(a) * radio;
        p[i] = {x + dx * (j ? v : u), y + dy * (j ? u : v)};
      }
      if (!SDL_RenderLines(r, p, 25))
        return false;
    }
    SDL_FPoint diamante[] = {{x, y + dy * 38},
                             {x + dx * 4, y + dy * 44},
                             {x, y + dy * 50},
                             {x - dx * 4, y + dy * 44},
                             {x, y + dy * 38}};
    return SDL_RenderLines(r, diamante, 5);
  }
  bool contenido(SDL_Renderer *r, const EstadoMenuPort &s, float ancho) {
    const float c = ancho / 2;
    const SDL_Color oro{228, 193, 116, 255}, crema{246, 232, 194, 255};
    if (!SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND) ||
        !bloque(r, {0, 0, ancho, 900}, {5, 8, 18, 142}))
      return false;
    if (!logo_buscado) {
      logo_buscado = true;
      const std::string ruta = std::string(SDL_GetBasePath()) +
                               "assets/lanzador/presentacion/curs-of-enchantia-reborn-logo.png";
      SDL_Surface *imagen = SDL_LoadPNG(ruta.c_str());
      if (!imagen)
        imagen =
            SDL_LoadPNG("packages/assets/lanzador/presentacion/curs-of-enchantia-reborn-logo.png");
      if (imagen) {
        logo = SDL_CreateTextureFromSurface(r, imagen);
        SDL_DestroySurface(imagen);
      }
      if (logo && (!SDL_SetTextureScaleMode(logo, SDL_SCALEMODE_LINEAR) ||
                   !SDL_SetTextureBlendMode(logo, SDL_BLENDMODE_BLEND)))
        return false;
    }
    const float logo_ancho = std::min(700.0f, ancho - 32);
    const SDL_FRect logotipo{c - logo_ancho / 2, 36, logo_ancho, logo_ancho * 750 / 2098};
    if (logo) {
      if (!SDL_RenderTexture(r, logo, nullptr, &logotipo))
        return false;
    } else if (!fuente.texto(r, "Curse of Enchantia: Reborn", c, 150, 32, oro, true))
      return false;
    const float panel_ancho = std::min(500.0f, ancho - 20);
    if (!bloque(r, {c - panel_ancho / 2 + 10, 300, panel_ancho, 540}, {0, 0, 0, 110}) ||
        !bloque(r, {c - panel_ancho / 2, 288, panel_ancho, 540}, {10, 18, 37, 247}) ||
        !borde(r, {c - panel_ancho / 2 + 5, 293, panel_ancho - 10, 530}, {177, 128, 53, 255}) ||
        !borde(r, {c - panel_ancho / 2 + 10, 298, panel_ancho - 20, 520}, {67, 59, 48, 255}))
      return false;
    for (int ix : {-1, 1})
      for (int iy : {-1, 1})
        if (!ornamento(r, c + ix * (panel_ancho / 2 - 8), iy < 0 ? 296 : 820, float(-ix),
                       float(-iy)))
          return false;
    const auto botones = filas(s);
    seleccion = std::clamp(seleccion, 0, int(botones.size()) - 1);
    for (size_t i = 0; i < botones.size(); ++i) {
      auto rect = rectangulo(c, i);
      if (ancho < 450) {
        rect.x = c - (ancho - 64) / 2;
        rect.w = ancho - 64;
      }
      const bool foco = int(i) == seleccion;
      for (int y = 0; y < 57; ++y) {
        const Uint8 rojo = Uint8(foco ? 159 - y : 25 - y / 5);
        const Uint8 verde = Uint8(foco ? 111 - y : 38 - y / 4);
        const Uint8 azul = Uint8(foco ? 37 - y / 2 : 62 - y / 3);
        if (!bloque(r, {rect.x, rect.y + y, rect.w, 1}, {rojo, verde, azul, 255}))
          return false;
      }
      if (!borde(r, rect, foco ? oro : SDL_Color{127, 97, 49, 255}) ||
          !borde(r, {rect.x + 3, rect.y + 3, rect.w - 6, rect.h - 6},
                 foco ? SDL_Color{244, 190, 75, 255} : SDL_Color{35, 53, 74, 255}))
        return false;
      const auto &f = botones[i];
      if (!fuente.texto(r, f.texto, c, rect.y + (f.valor.empty() ? 13 : 2),
                        f.valor.empty() ? 25 : 17, crema, true))
        return false;
      if (!f.valor.empty() && (!fuente.texto(r, f.valor, c, rect.y + 24, 22, oro, true) ||
                               !fuente.texto(r, "<", rect.x + 13, rect.y + 17, 24, oro) ||
                               !fuente.texto(r, ">", rect.x + rect.w - 30, rect.y + 17, 24, oro)))
        return false;
    }
    if (pagina == Pagina::creditos || pagina == Pagina::controles || pagina == Pagina::salir) {
      const std::vector<std::string> lineas =
          pagina == Pagina::creditos
              ? std::vector<std::string>{"Curse of Enchantia: Reborn",
                                         t("Port por Àlex Llaó", "Port by Àlex Llaó"),
                                         std::string(t("Versión ", "Version ")) +
                                             CURSE_REBORN_VERSION,
                                         fecha_compilacion_port(),
                                         "Curse of Enchantia · Core Design",
                                         "1992"}
          : pagina == Pagina::controles
              ? std::vector<std::string>{t("Ratón: caminar e interactuar",
                                           "Mouse: walk and interact"),
                                         t("Esc / clic derecho: menú original",
                                           "Esc / right click: original menu"),
                                         t("Mayús + Esc: menú del port", "Shift + Esc: port menu"),
                                         t("F1: marco    F11: filtro", "F1: frame    F11: filter"),
                                         t("F3: efectos    F4: música", "F3: effects    F4: music"),
                                         t("P: estéreo    M: silencio", "P: stereo    M: mute"),
                                         t("Ctrl + R: habitación", "Ctrl + R: room")}
              : std::vector<std::string>{
                    t("¿Quieres salir del juego?", "Quit the game?"),
                    t("Los cambios sin guardar se perderán.", "Unsaved progress will be lost.")};
      for (size_t i = 0; i < lineas.size(); ++i)
        if (!fuente.texto(r, lineas[i], c, 495 + float(i) * 34, 20, crema, true))
          return false;
    }
    if (!fuente.texto(r, CURSE_REBORN_VERSION, c, 790, 16, oro, true))
      return false;
    return fuente.texto(r, t("Flechas · Intro · Ratón", "Arrows · Enter · Mouse"), c, 852, 17,
                        {198, 194, 180, 255}, true);
  }
};
