#pragma once
#include <cstdlib>
#include <cstdio>
#include <set>

// Presenta la ruta real con cinco ticks de70Hz por paso, menus legibles y captura aislada.
class RecorridoGrabado {
public:
  // Conserva el canal anonimo heredado para video y dirige los printf a un archivo propio.
  static bool preparar_consola() {
    const char *ruta = SDL_getenv("CURSE_REBORN_GRABAR_RUTA");
    if (!ruta || !*ruta)
      return true;
#ifdef _WIN32
    HANDLE copia = nullptr;
    if (!DuplicateHandle(GetCurrentProcess(), GetStdHandle(STD_OUTPUT_HANDLE), GetCurrentProcess(),
                         &copia, 0, FALSE, DUPLICATE_SAME_ACCESS))
      return false;
    canal_grabacion = copia;
    std::FILE *archivo = nullptr;
    return freopen_s(&archivo, (std::string(ruta) + ".consola.log").c_str(), "wb", stdout) == 0;
#else
    return false;
#endif
  }
  // Lee el destino opcional; sin el no instala observadores ni altera las comprobaciones.
  explicit RecorridoGrabado(Presentacion &presentacion) : p(presentacion) {
    const char *ruta = SDL_getenv("CURSE_REBORN_GRABAR_RUTA");
    if (ruta)
      prefijo = ruta;
  }
  // Desactiva los observadores antes de cerrar la mezcla y los archivos, tambien al fallar la ruta.
  ~RecorridoGrabado() {
    if (prefijo.empty())
      return;
    mazmorra.observar_entradas(nullptr, nullptr);
    mazmorra.terminar_audio_grabado();
    if (video) {
#ifdef _WIN32
      CloseHandle(static_cast<HANDLE>(video));
#else
      SDL_CloseIO(static_cast<SDL_IOStream *>(video));
#endif
    }
    const auto duracion = inicio ? (SDL_GetTicksNS() - inicio) / 1000000ull : 0;
    const std::string informe = "{\"completo\":" + std::string(completo ? "true" : "false") +
                                ",\"fotogramas\":" + std::to_string(fotogramas) +
                                ",\"duracion_ms\":" + std::to_string(duracion) +
                                ",\"escenas\":" + std::to_string(escenas.size()) +
                                ",\"ticks_por_paso\":5,\"frecuencia_logica_hz\":14" +
                                ",\"resolucion\":[1920,1080],\"fps\":30,\"filtro\":\"scanlines\","
                                "\"marco\":\"Cielo encantado\"}\n";
    SDL_SaveFile((prefijo + ".json").c_str(), informe.data(), informe.size());
  }
  // Permite conservar el camino determinista anterior cuando no se solicita una grabacion.
  bool activa() const {
    return !prefijo.empty();
  }
  // No usa preferencias ni partidas del perfil; el audio ya esta conectado al inicio de la
  // mazmorra.
  bool preparar() {
    if (!activa())
      return true;
    const char *muestra = SDL_getenv("CURSE_REBORN_GRABAR_SEGUNDOS");
    limite = muestra ? std::strtoul(muestra, nullptr, 10) : 0;
    if (!SDL_SetWindowBordered(p.ventana, false) || !SDL_SetWindowSize(p.ventana, 1920, 1080) ||
        !SDL_SyncWindow(p.ventana))
      return false;
    video = canal_grabacion;
    canal_grabacion = nullptr;
    if (!video)
      return false;
    for (unsigned int i = 0; i < 2; ++i)
      if (!p.filtros.alternar(p.dibujante, p.textura, pixeles))
        return false;
    p.filtros.suspender(false);
    inicio = siguiente = SDL_GetTicksNS();
    p.marcos.alternar(inicio);
    if (!mazmorra.grabar_audio((prefijo + ".wav").c_str()))
      return false;
    mazmorra.observar_entradas(this, entrada);
    return cuadro();
  }
  // Conserva los32 retrazos del fundido y los ocho de camara, sin adelantar el reloj de
  // presentacion.
  bool presentacion() {
    while (mazmorra.en_presentacion())
      if (!mazmorra.avanzar_presentacion(SDL_GetTicksNS() * 70ull / 1000000000ull) || !cuadro())
        return false;
    return true;
  }
  // Cada iteracion sigue la misma ruta logica del comparador, mostrando todas las transiciones.
  bool avanzar(unsigned int cantidad, bool comprobar_lago) {
    for (unsigned int i = 0; i < cantidad; ++i) {
      if (!presentacion() || !mazmorra.actualizar_audio() || !mazmorra.avanzar())
        return false;
      const bool camara = comprobar_lago && mazmorra.camara_en_presentacion();
      if (!presentacion())
        return false;
      if (camara && (!esperar_paso() || !mazmorra.avanzar() || !presentacion()))
        return false;
      if (!esperar_paso())
        return false;
    }
    return true;
  }
  // Mantiene el desenlace visible cinco segundos y marca completa solo la llegada real a73.
  bool terminar() {
    if (!activa())
      return true;
    if (mazmorra.escena_actual() != 73 || !avanzar(70, true))
      return false;
    completo = true;
    return true;
  }

private:
  static inline void *canal_grabacion = nullptr;
  Presentacion &p;
  std::string prefijo;
  void *video = nullptr;
  Uint64 inicio = 0, siguiente = 0, fotogramas = 0;
  unsigned long limite = 0;
  bool completo = false;
  float cursor_x = 152, cursor_y = 92, destino_x = 152, destino_y = 92;
  std::set<unsigned short> escenas;
  unsigned short escena_anterior = 65535;
  std::string cronologia;
  // EAD5 y el bucle interactivo esperan cinco ticks; los70Hz son del reloj, no de la logica.
  bool esperar_paso() {
    for (unsigned int tick = 0; tick < 5; ++tick)
      if (!cuadro())
        return false;
    return true;
  }
  // Escribe la tuberia existente por bloques y propaga cierres o escrituras incompletas.
  bool enviar(const void *datos, size_t bytes) {
#ifdef _WIN32
    const auto *origen = static_cast<const unsigned char *>(datos);
    while (bytes) {
      DWORD escritos = 0;
      const DWORD cantidad = static_cast<DWORD>(std::min(bytes, size_t(1048576)));
      if (!WriteFile(static_cast<HANDLE>(video), origen, cantidad, &escritos, nullptr) || !escritos)
        return false;
      bytes -= escritos;
      origen += escritos;
    }
    return true;
#else
    return SDL_WriteIO(static_cast<SDL_IOStream *>(video), datos, bytes) == bytes;
#endif
  }
  // El menu original pausa la logica mientras se elige; la musica y el cursor siguen visibles.
  static bool entrada(void *contexto, unsigned short x, unsigned short y, bool menu) {
    auto &r = *static_cast<RecorridoGrabado *>(contexto);
    r.destino_x = x;
    r.destino_y = menu ? static_cast<float>(mazmorra.franja_menu() + 16) : y;
    if (menu) {
      // La entrada de prueba usa y92; la mano debe senalar la casilla realmente dibujada.
      if (!mazmorra.entrada_menu(x, static_cast<unsigned short>(r.destino_y), false, false))
        return false;
      for (unsigned int i = 0; i < 35; ++i)
        if (!r.cuadro())
          return false;
    }
    return true;
  }
  // Compone el mismo juego, mano, filtro y marco; envia RGBA a FFmpeg sin capturar el escritorio.
  bool cuadro() {
    const auto ahora = SDL_GetTicksNS();
    if (limite && ahora - inicio >= limite * 1000000000ull)
      return SDL_SetError("Muestra de grabacion terminada en su limite solicitado");
    if (siguiente > ahora)
      SDL_DelayNS(siguiente - ahora);
    siguiente += 1000000000ull / 70;
    SDL_PumpEvents();
    if (!mazmorra.actualizar_audio())
      return false;
    const auto necesarios = (SDL_GetTicksNS() - inicio) * 30 / 1000000000ull + 1;
    if (fotogramas >= necesarios)
      return true;
    if (!mazmorra.dibujar(imagen))
      return false;
    for (unsigned int i = 0; i < 768; ++i)
      paleta_visible[i] = imagen.paleta[i];
    if (!actualizar_textura(p))
      return false;
    cursor_x += (destino_x - cursor_x) * 0.25f;
    cursor_y += (destino_y - cursor_y) * 0.25f;
    unsigned char mano[16 * 16 * 4];
    if (!mazmorra.preparar_cursor_rgba(mano))
      return false;
    const int cx = static_cast<int>(cursor_x), cy = static_cast<int>(cursor_y);
    for (int y = 0; y < 16; ++y)
      for (int x = 0; x < 16; ++x) {
        if (cx + x < 0 || cx + x >= 320 || cy + y < 0 || cy + y >= 200)
          continue;
        const auto origen = (y * 16 + x) * 4;
        const auto destino = ((cy + y) * 320 + cx + x) * 4;
        const unsigned int alfa = mano[origen + 3];
        for (unsigned int c = 0; c < 3; ++c)
          pixeles[destino + c] = static_cast<unsigned char>(
              (mano[origen + c] * alfa + pixeles[destino + c] * (255 - alfa)) / 255);
      }
    if (!p.filtros.actualizar(p.textura, pixeles) || !p.marcos.preparar_encuadre(p.dibujante) ||
        !SDL_SetRenderDrawColor(p.dibujante, 0, 0, 0, 255) || !SDL_RenderClear(p.dibujante) ||
        !SDL_RenderTexture(p.dibujante, p.filtros.textura_visible(p.textura), nullptr, nullptr) ||
        !p.marcos.dibujar(p.dibujante, "escena"))
      return false;
    if (fotogramas < necesarios) {
      SDL_Surface *rgba = capturar_monitor(p.dibujante);
      if (!rgba || rgba->w != 1920 || rgba->h != 1080) {
        SDL_SetError("Captura de video de %dx%d; se esperaba1920x1080", rgba ? rgba->w : 0,
                     rgba ? rgba->h : 0);
        SDL_DestroySurface(rgba);
        return false;
      }
      bool correcto = true;
      while (fotogramas < necesarios && correcto) {
        if (rgba->pitch == 1920 * 4) {
          constexpr size_t bytes = 1920u * 1080u * 4u;
          correcto = enviar(rgba->pixels, bytes);
        } else {
          for (int y = 0; y < rgba->h && correcto; ++y)
            correcto =
                enviar(static_cast<unsigned char *>(rgba->pixels) + y * rgba->pitch, 1920 * 4);
        }
        ++fotogramas;
      }
      SDL_DestroySurface(rgba);
      if (!correcto)
        return false;
    }
    escenas.insert(mazmorra.escena_actual());
    if (escena_anterior != mazmorra.escena_actual()) {
      escena_anterior = mazmorra.escena_actual();
      cronologia += std::to_string((SDL_GetTicksNS() - inicio) / 1000000ull) + '\t' +
                    std::to_string(escena_anterior) + '\n';
      if (!SDL_SaveFile((prefijo + ".escenas.tsv").c_str(), cronologia.data(), cronologia.size()))
        return false;
      registrar_accion("Grabacion: escena %u, segundo %llu", mazmorra.escena_actual(),
                       (SDL_GetTicksNS() - inicio) / 1000000000ull);
    }
    return SDL_RenderPresent(p.dibujante);
  }
};
