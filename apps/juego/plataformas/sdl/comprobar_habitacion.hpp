#pragma once
#include <cmath>

// Comprueba el arranque, los dos botones, las variantes y el zoom con ventanas ocultas.
bool comprobar_habitacion(const std::string &carpeta, const std::string &prefijo) {
  Presentacion p;
  if (!SDL_Init(SDL_INIT_VIDEO) ||
      !SDL_CreateWindowAndRenderer("Habitaciones", 1280, 720,
                                   SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE, &p.ventana,
                                   &p.dibujante))
    return false;
  p.textura =
      SDL_CreateTexture(p.dibujante, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, 320, 200);
  if (!p.textura || !SDL_SetTextureScaleMode(p.textura, SDL_SCALEMODE_NEAREST) ||
      !cambiar_pantalla(carpeta, 0, p))
    return false;
  p.habitacion.iniciar();
  Uint64 reloj = 1000000000ull;
  unsigned int capturas = 0;
  // Captura desde el mismo orden de composición que el bucle normal.
  const auto capturar = [&](const std::string &nombre, bool negra) {
    if (!p.marcos.preparar_encuadre(p.dibujante, reloj) ||
        !p.habitacion.preparar_encuadre(p.dibujante, reloj) ||
        !SDL_SetRenderDrawColor(p.dibujante, 0, 0, 0, 255) || !SDL_RenderClear(p.dibujante))
      return false;
    const auto destino = p.habitacion.destino_juego(reloj);
    if ((destino.h > 0 && !SDL_RenderTexture(p.dibujante, p.textura, nullptr, &destino)) ||
        !p.habitacion.dibujar(p.dibujante, reloj))
      return false;
    SDL_FRect imagen, pantalla, juego;
    if (!p.habitacion.geometria(p.dibujante, imagen, pantalla, juego, reloj))
      return false;
    SDL_Surface *s = capturar_monitor(p.dibujante);
    if (!s)
      return false;
    Uint8 r = 0, g = 0, b = 0, a = 0;
    bool bien = SDL_ReadSurfacePixel(s, int(juego.x + juego.w / 2), int(juego.y + juego.h / 2), &r,
                                     &g, &b, &a) &&
                (!negra || (r == 0 && g == 0 && b == 0));
    // El cristal libre por encima del juego debe ser negro, sin la fotografía gris del PNG.
    if (!p.habitacion.ampliada(reloj)) {
      bien =
          bien &&
          SDL_ReadSurfacePixel(s, int(juego.x + juego.w / 2), int(juego.y - 2), &r, &g, &b, &a) &&
          r == 0 && g == 0 && b == 0;
    }
    bien = bien && SDL_SavePNG(s, (prefijo + "." + nombre + ".png").c_str());
    SDL_DestroySurface(s);
    ++capturas;
    return bien;
  };
  // Convierte una posición física al espacio de la ventana, igual que los clics reales.
  const auto pulsar = [&](const SDL_FRect &zona, Uint8 boton = SDL_BUTTON_LEFT) {
    int w = 0, h = 0, vw = 0, vh = 0;
    if (!SDL_GetRenderOutputSize(p.dibujante, &w, &h) || !SDL_GetWindowSize(p.ventana, &vw, &vh))
      return false;
    SDL_Event e{};
    e.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
    e.button.button = boton;
    e.button.x = (zona.x + zona.w / 2) * vw / w;
    e.button.y = (zona.y + zona.h / 2) * vh / h;
    return p.habitacion.evento(p.ventana, p.dibujante, e, reloj);
  };
  // Comprueba fundidos y variantes con reloj simulado, sin esperar ni abrir ventanas visibles.
  p.banner.iniciar(reloj);
  if (BannerLanzador::duracion() != 9200000000ull || !p.banner.activo(reloj) ||
      p.banner.opacidad(reloj) != 0 || p.banner.opacidad(reloj + 1500000000ull) < 126 ||
      p.banner.opacidad(reloj + 1500000000ull) > 129 ||
      p.banner.opacidad(reloj + 3000000000ull) != 255 ||
      p.banner.opacidad(reloj + 8000000000ull) != 255 ||
      p.banner.opacidad(reloj + 8600000000ull) < 126 ||
      p.banner.opacidad(reloj + 8600000000ull) > 129 ||
      p.banner.activo(reloj + BannerLanzador::duracion()))
    return fallo_monitor("El banner no respeta los fundidos y sus9,2 segundos");
  for (const auto &tamano : {std::pair<int, int>{1280, 720},
                             {1280, 800},
                             {1920, 720},
                             {2560, 720},
                             {960, 720},
                             {720, 1280}}) {
    if (!SDL_SetWindowSize(p.ventana, tamano.first, tamano.second) || !SDL_SyncWindow(p.ventana) ||
        !p.banner.dibujar(p.dibujante, reloj))
      return false;
    SDL_Surface *negro = SDL_RenderReadPixels(p.dibujante, nullptr);
    Uint8 rojo = 0, verde = 0, azul = 0, alfa = 0;
    const bool apagado = negro &&
                         SDL_ReadSurfacePixel(negro, tamano.first / 2, tamano.second / 2, &rojo,
                                              &verde, &azul, &alfa) &&
                         rojo == 0 && verde == 0 && azul == 0;
    SDL_DestroySurface(negro);
    if (!apagado || !p.banner.dibujar(p.dibujante, reloj + 4000000000ull))
      return false;
    const auto &v = BannerLanzador::variante(tamano.first, tamano.second);
    if (tamano.first != 720 &&
        std::abs(float(v.ancho) / v.alto - float(tamano.first) / tamano.second) > 0.001f)
      return fallo_monitor("Se selecciona un banner de proporción incorrecta");
    SDL_Surface *banner = SDL_RenderReadPixels(p.dibujante, nullptr);
    const bool guardado =
        banner && SDL_SavePNG(banner, (prefijo + ".banner_" + std::to_string(tamano.first) + "x" +
                                       std::to_string(tamano.second) + ".png")
                                          .c_str());
    SDL_DestroySurface(banner);
    if (!guardado)
      return false;
  }
  p.banner.cerrar();
  // El encendido automático conserva el sonido, los siete segundos completos y una sola solicitud.
  p.habitacion.programar_encendido(reloj + 2000000000ull);
  if (p.habitacion.tomar_reinicio(reloj + 1999999999ull) || p.habitacion.tomar_sonido() != 0 ||
      p.habitacion.tomar_reinicio(reloj + 2000000000ull) || p.habitacion.tomar_sonido() != 1 ||
      p.habitacion.tomar_reinicio(reloj + 8999999999ull) ||
      !p.habitacion.tomar_reinicio(reloj + 9000000000ull) ||
      p.habitacion.tomar_reinicio(reloj + 10000000000ull))
    return fallo_monitor("El encendido automático acorta o repite BIOS/DOS");
  p.habitacion.iniciar();
  // Un clic manual anula el plazo: apagar durante el arranque debe seguir dejando el PC apagado.
  SDL_FRect pc_auto, monitor_auto;
  p.habitacion.programar_encendido(reloj + 2000000000ull);
  if (!p.habitacion.controles(p.dibujante, pc_auto, monitor_auto, reloj) || !pulsar(pc_auto) ||
      p.habitacion.tomar_sonido() != 1 || !pulsar(pc_auto) || p.habitacion.tomar_sonido() != 2)
    return false;
  p.habitacion.tomar_reinicio(reloj + 3000000000ull);
  if (p.habitacion.tomar_sonido() != 0 || !p.habitacion.reiniciando())
    return fallo_monitor("El plazo automático vuelve a encender un PC apagado manualmente");
  p.habitacion.iniciar();
  for (unsigned int tema = 0; tema < 5; ++tema) {
    for (const auto &tamano : {std::pair<int, int>{1280, 720},
                               {1280, 800},
                               {1280, 960},
                               {2400, 900},
                               {3200, 900},
                               {600, 1000}}) {
      if (!SDL_SetWindowSize(p.ventana, tamano.first, tamano.second) ||
          !SDL_SyncWindow(p.ventana) || !p.marcos.preparar_encuadre(p.dibujante, reloj) ||
          !p.habitacion.preparar_encuadre(p.dibujante, reloj))
        return false;
      SDL_FRect imagen, pantalla, juego, pc, monitor;
      if (!p.habitacion.geometria(p.dibujante, imagen, pantalla, juego, reloj) ||
          !p.habitacion.controles(p.dibujante, pc, monitor, reloj) ||
          std::abs(juego.w / juego.h - 1.6f) > 0.0001f || juego.x < pantalla.x - 1 ||
          juego.y < pantalla.y - 1 || juego.x + juego.w > pantalla.x + pantalla.w + 1 ||
          juego.y + juego.h > pantalla.y + pantalla.h + 1)
        return fallo_monitor("Geometría de habitación inválida");
      const auto nombre = std::to_string(tema) + "." + std::to_string(tamano.first) + "x" +
                          std::to_string(tamano.second);
      if (!p.habitacion.reiniciando() || !capturar(nombre + ".apagado", true) ||
          !pulsar(pc, SDL_BUTTON_RIGHT) || !p.habitacion.reiniciando() || !pulsar(pc))
        return fallo_monitor("No empieza apagado o no responde el PC");
      if (p.habitacion.tomar_reinicio(reloj + 6999000000ull))
        return fallo_monitor("BIOS/DOS se han saltado");
      if (p.habitacion.tomar_sonido() != 1 || p.habitacion.tomar_sonido() != 0)
        return fallo_monitor("El encendido no solicita una única secuencia sonora");
      if (tema == 0 && tamano.first == 1280 && tamano.second == 720) {
        const auto inicio = reloj;
        for (const auto &fase : {std::pair<Uint64, const char *>{1200000000ull, "bios"},
                                 {4600000000ull, "dos"},
                                 {6200000000ull, "carga"}}) {
          reloj = inicio + fase.first;
          if (!capturar(nombre + "." + fase.second, false))
            return false;
        }
        reloj = inicio;
      }
      reloj += 7000000000ull;
      if (!p.habitacion.tomar_reinicio(reloj) || p.habitacion.tomar_reinicio(reloj) ||
          p.habitacion.reiniciando())
        return fallo_monitor("El arranque no entrega una única solicitud");
      if (pulsar(juego) || p.habitacion.ampliada(reloj))
        return fallo_monitor("El clic del juego pequeño se intercepta o amplía la pantalla");
      if (p.habitacion.tomar_sonido() != 0)
        return fallo_monitor("Un clic del juego produce sonido de botón del PC");
      if (tema == 0 && !capturar(nombre + ".juego", false))
        return false;
      if (tema == 0 && tamano.first == 1280 && tamano.second == 720) {
        if (!cambiar_pantalla(carpeta, 3, p) || !capturar(nombre + ".mazmorra", false) ||
            !cambiar_pantalla(carpeta, 0, p))
          return false;
      }
      // Compara todos los píxeles realmente rasterizados antes y después de superponer el PNG.
      if (!SDL_SetRenderDrawColor(p.dibujante, 210, 40, 200, 255) ||
          !SDL_RenderFillRect(p.dibujante, nullptr))
        return false;
      SDL_Surface *antes = capturar_monitor(p.dibujante);
      if (!antes)
        return false;
      if (!p.habitacion.dibujar(p.dibujante, reloj)) {
        SDL_DestroySurface(antes);
        return false;
      }
      SDL_Surface *colores = capturar_monitor(p.dibujante);
      SDL_FRect zona_juego;
      bool completo = colores && antes->w == colores->w && antes->h == colores->h &&
                      MarcosPantalla::area_juego(p.dibujante, zona_juego);
      unsigned int comprobados = 0;
      for (int y = std::max(0, int(std::floor(zona_juego.y)));
           completo && y < std::min(antes->h, int(std::ceil(zona_juego.y + zona_juego.h))); ++y) {
        const auto *a = static_cast<const Uint8 *>(antes->pixels) + y * antes->pitch;
        const auto *b = static_cast<const Uint8 *>(colores->pixels) + y * colores->pitch;
        for (int x = std::max(0, int(std::floor(zona_juego.x)));
             completo && x < std::min(antes->w, int(std::ceil(zona_juego.x + zona_juego.w))); ++x) {
          if (a[x * 4] == 210 && a[x * 4 + 1] == 40 && a[x * 4 + 2] == 200) {
            ++comprobados;
            completo = b[x * 4] == 210 && b[x * 4 + 1] == 40 && b[x * 4 + 2] == 200;
            if (!completo)
              std::fprintf(stderr, "Recorte %s: pixel %d,%d: %u,%u,%u; zona %.2f,%.2f %.2fx%.2f\n",
                           nombre.c_str(), x, y, b[x * 4], b[x * 4 + 1], b[x * 4 + 2], zona_juego.x,
                           zona_juego.y, zona_juego.w, zona_juego.h);
          }
        }
      }
      if (!completo && colores)
        SDL_SavePNG(colores, (prefijo + ".bordes.png").c_str());
      SDL_DestroySurface(antes);
      SDL_DestroySurface(colores);
      if (!completo || comprobados == 0)
        return fallo_monitor("La habitación recorta píxeles de la imagen del juego");
      if (!pulsar(monitor))
        return fallo_monitor("No responde el botón del monitor");
      if (p.habitacion.tomar_sonido() != 3)
        return fallo_monitor("El monitor no solicita el clic de su botón");
      reloj += 650000000ull;
      if (!p.habitacion.preparar_encuadre(p.dibujante, reloj) ||
          !p.habitacion.geometria(p.dibujante, imagen, pantalla, juego, reloj) ||
          !p.habitacion.ampliada(reloj) ||
          std::abs(juego.w -
                   std::min(float(tamano.first) / 320, float(tamano.second) / 200) * 320) > 1)
        return fallo_monitor("El acercamiento no alcanza el máximo disponible");
      SDL_FPoint centro;
      if (!SDL_RenderCoordinatesToWindow(p.dibujante, 160, 100, &centro.x, &centro.y))
        return false;
      float x = 0, y = 0;
      if (!SDL_RenderCoordinatesFromWindow(p.dibujante, centro.x, centro.y, &x, &y) ||
          std::abs(x - 160) > 0.01 || std::abs(y - 100) > 0.01)
        return fallo_monitor("El ratón se desplaza durante el zoom");
      if (tema == 0 && !capturar(nombre + ".ampliado", false))
        return false;
      SDL_Event escape{};
      escape.type = SDL_EVENT_KEY_DOWN;
      escape.key.key = SDLK_ESCAPE;
      if (p.habitacion.evento(p.ventana, p.dibujante, escape, reloj) ||
          !p.habitacion.ampliada(reloj))
        return fallo_monitor("La habitación intercepta Escape del juego");
      SDL_Event salir{};
      salir.type = SDL_EVENT_KEY_DOWN;
      salir.key.key = SDLK_R;
      salir.key.mod = SDL_KMOD_CTRL;
      if (!p.habitacion.evento(p.ventana, p.dibujante, salir, reloj))
        return false;
      reloj += 650000000ull;
      if (!p.habitacion.preparar_encuadre(p.dibujante, reloj) ||
          !p.habitacion.controles(p.dibujante, pc, monitor, reloj) || !pulsar(pc))
        return false;
      reloj += 800000000ull;
      if (p.habitacion.tomar_sonido() != 2)
        return fallo_monitor("El apagado no sustituye el arranque por el clic");
      p.habitacion.tomar_reinicio(reloj);
      if (!p.habitacion.reiniciando())
        return fallo_monitor("Apagar no conserva el PC apagado");
    }
    SDL_Event siguiente{};
    siguiente.type = SDL_EVENT_KEY_DOWN;
    siguiente.key.mod = SDL_KMOD_CTRL;
    siguiente.key.key = SDLK_H;
    if (!p.habitacion.evento(p.ventana, p.dibujante, siguiente, reloj))
      return false;
  }
  Audio audio;
  if (!audio.abrir(carpeta, false, true))
    return fallo_monitor("No se abre el audio inicialmente pausado");
  SDL_Delay(30);
  if (audio.muestras_musicales() != 0 || !audio.ajustar_volumen(-40) || !audio.alternar_silencio())
    return fallo_monitor("Se produce música antes de encender el PC");
  const auto perfil = audio.perfil_musical();
  if (!audio.reiniciar() || !audio.silenciado() || audio.nivel_volumen() != 60 ||
      audio.perfil_musical() != perfil)
    return fallo_monitor("El encendido no conserva silencio, volumen o perfil");
  audio.cerrar();
  if (!comprobar_sonidos_pc())
    return fallo_monitor("Sonidos del PC: cola, mute, volumen o parada incorrectos");
  registrar_accion("Habitaciones: cinco diseños, cinco proporciones y ventana vertical; PC "
                   "apagado, BIOS/DOS completos, botones, zoom máximo y ratón. %u capturas",
                   capturas);
  return true;
}
