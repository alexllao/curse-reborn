#include "audio.hpp"
#include <cstdio>
#include <vector>

// Abre dos flujos del mismo dispositivo: música autónoma y efectos atendidos por el juego.
bool Audio::abrir(const std::string &carpeta) {
  if (flujo)
    return true;
  if (!SDL_InitSubSystem(SDL_INIT_AUDIO))
    return false;
  const SDL_AudioSpec formato{SDL_AUDIO_S16, 2, 48000};
  const SDL_AudioSpec formato_efectos{SDL_AUDIO_S16, 1, 48000};
  dispositivo = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &formato);
  if (!dispositivo)
    return false;
  flujo = SDL_CreateAudioStream(&formato_efectos, &formato);
  if (!flujo || !SDL_BindAudioStream(dispositivo, flujo)) {
    cerrar();
    return false;
  }
  size_t longitud = 0;
  void *archivo = SDL_LoadFile((carpeta + "/../CURSE.EXE").c_str(), &longitud);
  const bool preparada = archivo && longitud <= 0xffffffffu &&
                         musica.iniciar(static_cast<const unsigned char *>(archivo),
                                        static_cast<unsigned int>(longitud));
  if (preparada && mt32.iniciar(carpeta, static_cast<const unsigned char *>(archivo),
                                static_cast<unsigned int>(longitud)))
    perfil = 2;
  else {
    perfil = 1;
    std::printf(
        "MT-32 no disponible: revisa MT32_CONTROL.ROM y MT32_PCM.ROM en datos_originales/ROMS.\n");
  }
  SDL_free(archivo);
  if (!preparada) {
    cerrar();
    return false;
  }
  flujo_musical = SDL_CreateAudioStream(&formato, &formato);
  if (!flujo_musical || !SDL_SetAudioStreamGetCallback(flujo_musical, alimentar_musica, this) ||
      !SDL_BindAudioStream(dispositivo, flujo_musical)) {
    cerrar();
    return false;
  }
  ultimo = SDL_GetTicksNS();
  resto = 0;
  fallo_musical.store(false);
  muestras_producidas.store(0);
  muestras_con_senal.store(0);
  if (!SDL_SetAudioDeviceGain(dispositivo, silencio ? 0.0f : volumen / 100.0f) ||
      !SDL_ResumeAudioDevice(dispositivo)) {
    cerrar();
    return false;
  }
  std::printf("Audio: %s + PC Speaker (efectos). F4 cambia la opción; M silencia.\nSB16 digital: "
              "pendiente.\n",
              perfil == 2 ? "MT-32 (música por escena)" : "AdLib (tema continuo original)");
  std::fflush(stdout);
  registrar_volumen();
  return true;
}

// SDL espera al callback antes de liberar su sintetizador y el dispositivo compartido.
void Audio::cerrar() {
  if (flujo_musical)
    SDL_DestroyAudioStream(flujo_musical);
  flujo_musical = nullptr;
  if (flujo)
    SDL_DestroyAudioStream(flujo);
  flujo = nullptr;
  datos = nullptr;
  if (dispositivo)
    SDL_CloseAudioDevice(dispositivo);
  dispositivo = 0;
  mt32.cerrar();
}

// El callback tiene el bloqueo del flujo: los cambios de banco y perfil usan el mismo bloqueo.
void Audio::alimentar_musica(void *contexto, SDL_AudioStream *salida, int adicional, int) {
  auto &audio = *static_cast<Audio *>(contexto);
  int pendientes = (adicional + 3) / 4;
  short fm[512], roland[1024], mezcla[1024];
  while (pendientes > 0) {
    const auto cantidad = static_cast<unsigned int>(pendientes > 512 ? 512 : pendientes);
    bool correcto = audio.musica.generar(fm, cantidad);
    if (audio.mt32.disponible())
      correcto = correcto && audio.mt32.generar(roland, cantidad);
    unsigned int con_senal = 0;
    for (unsigned int i = 0; i < cantidad * 2; ++i) {
      mezcla[i] = !correcto || audio.silencio || audio.perfil == 0 ? 0
                  : audio.perfil == 2                              ? roland[i]
                                                                   : fm[i / 2];
      con_senal += mezcla[i] > 64 || mezcla[i] < -64 ? 1u : 0u;
    }
    audio.muestras_con_senal.fetch_add(con_senal);
    if (!correcto || !SDL_PutAudioStreamData(salida, mezcla, static_cast<int>(cantidad * 4))) {
      audio.fallo_musical.store(true);
      return;
    }
    audio.muestras_producidas.fetch_add(cantidad);
    pendientes -= static_cast<int>(cantidad);
  }
}

// Prepara efectos de una sesión nueva sin reiniciar ni vaciar la música en curso.
bool Audio::conectar(unsigned char *segmento) {
  if (!flujo)
    return true;
  if (!segmento || !actualizar())
    return false;
  datos = segmento;
  altavoz.reiniciar();
  if (!altavoz.detener(datos))
    return false;
  datos[0x9924] = 0;
  datos[0x0ad4] = 1;
  return cambiar_escena(static_cast<unsigned short>(datos[0x8082] | (datos[0x8083] << 8)));
}

// Desconecta solo los efectos: la música mantiene su búfer y su reloj durante la carga de imágenes.
void Audio::desconectar() {
  if (datos) {
    altavoz.detener(datos);
    datos[0x0ad4] = datos[0x0ad3] = 0;
  }
  datos = nullptr;
}

// Toda escena consulta las mismas tablas; los bancos repetidos no se reinician.
bool Audio::cambiar_escena(unsigned short escena) {
  if (!flujo_musical)
    return true;
  if (!SDL_LockAudioStream(flujo_musical))
    return false;
  const bool correcto = !mt32.disponible() || mt32.cambiar_escena(escena);
  if (datos)
    datos[0x0ad3] = perfil == 2 ? 4 : perfil == 1 ? 2 : 0;
  SDL_UnlockAudioStream(flujo_musical);
  return correcto;
}

// Solo los efectos acceden a DS desde el hilo principal; las imágenes no detienen el flujo musical.
bool Audio::actualizar() {
  if (!flujo)
    return true;
  if (fallo_musical.load())
    return false;
  const auto ahora = SDL_GetTicksNS();
  auto transcurrido = ahora - ultimo;
  ultimo = ahora;
  if (transcurrido > 5000000000ull) {
    if (datos) {
      altavoz.detener(datos);
      datos[0x9924] = 0;
    }
    transcurrido = resto = 0;
  }
  const auto reloj = transcurrido * 48000u + resto;
  auto pendientes = reloj / 1000000000u;
  resto = reloj % 1000000000u;
  if (SDL_GetAudioStreamQueued(flujo) > 9600 && !SDL_ClearAudioStream(flujo))
    return false;
  short muestras[1024];
  while (pendientes) {
    const auto cantidad = static_cast<unsigned int>(pendientes > 1024 ? 1024 : pendientes);
    if (datos && !altavoz.generar(datos, muestras, cantidad))
      return false;
    if (!datos || silencio)
      for (unsigned int i = 0; i < cantidad; ++i)
        muestras[i] = 0;
    pendientes -= cantidad;
    if (pendientes < 2400 &&
        !SDL_PutAudioStreamData(flujo, muestras, static_cast<int>(cantidad * 2)))
      return false;
  }
  return true;
}

// El segmento debe pertenecer a la sesión conectada para no mezclar partidas de desarrollo.
bool Audio::reproducir(unsigned char *segmento, unsigned short entidad,
                       unsigned char identificador) {
  if (segmento[0x0ad4] == 0)
    return true;
  return datos == segmento && actualizar() && altavoz.reproducir(datos, entidad, identificador);
}

// El cambio de ganancia evita vaciar muestras y mantiene la continuidad del dispositivo.
bool Audio::alternar_silencio() {
  if (flujo_musical && !SDL_LockAudioStream(flujo_musical))
    return false;
  silencio = !silencio;
  if (flujo_musical)
    SDL_UnlockAudioStream(flujo_musical);
  std::printf("Audio: %s.\n", silencio ? "silenciado" : "reactivado");
  std::fflush(stdout);
  registrar_volumen();
  return !dispositivo || SDL_SetAudioDeviceGain(dispositivo, silencio ? 0.0f : volumen / 100.0f);
}

// El volumen se aplica al dispositivo compartido, incluyendo música y efectos de cualquier perfil.
bool Audio::ajustar_volumen(int cambio) {
  int siguiente = static_cast<int>(volumen) + cambio;
  if (siguiente < 0)
    siguiente = 0;
  if (siguiente > 100)
    siguiente = 100;
  if (dispositivo && !SDL_SetAudioDeviceGain(dispositivo, silencio ? 0.0f : siguiente / 100.0f))
    return false;
  volumen = static_cast<unsigned int>(siguiente);
  registrar_volumen();
  return true;
}

// El silencio muestra cero efectivo y permite recordar el porcentaje que se recuperará con M.
void Audio::registrar_volumen() const {
  if (silencio)
    std::printf("[partida] Volumen: 0%% (silenciado con M; ajuste guardado: %u%%).\n", volumen);
  else
    std::printf("[partida] Volumen: %u%%.\n", volumen);
  std::fflush(stdout);
}

// F4 solo ofrece dispositivos preparados; cada cambio se informa por consola.
bool Audio::alternar_dispositivo() {
  if (!flujo_musical) {
    std::printf("Audio desactivado o sin dispositivo.\n");
    return true;
  }
  if (!SDL_LockAudioStream(flujo_musical))
    return false;
  perfil = (perfil + 1) % (mt32.disponible() ? 3u : 2u);
  if (datos)
    datos[0x0ad3] = perfil == 2 ? 4 : perfil == 1 ? 2 : 0;
  std::printf("Audio cambiado a %s%s.\n",
              perfil == 2   ? "MT-32 (música por escena) + PC Speaker"
              : perfil == 1 ? "AdLib (tema continuo original) + PC Speaker"
                            : "PC Speaker (efectos; sin música)",
              silencio ? " [silenciado con M]" : "");
  std::fflush(stdout);
  SDL_UnlockAudioStream(flujo_musical);
  return true;
}

namespace {
// Escribe los campos RIFF en orden little endian, independientemente del ordenador anfitrión.
void campo_wav(unsigned char *cabecera, unsigned int posicion, unsigned int valor,
               unsigned int bytes) {
  for (unsigned int i = 0; i < bytes; ++i)
    cabecera[posicion + i] = static_cast<unsigned char>(valor >> (i * 8));
}
} // namespace

// Contrasta ocho segundos de música real en un bloque y en fragmentos de tamaños distintos.
bool comprobar_musica_adlib(const std::string &carpeta, const char *ruta, unsigned short tema) {
  size_t longitud = 0;
  void *archivo = SDL_LoadFile((carpeta + "/../CURSE.EXE").c_str(), &longitud);
  if (!archivo || longitud > 0xffffffffu) {
    SDL_free(archivo);
    return false;
  }
  static MusicaAdlib continua, fragmentada;
  const bool preparadas = continua.iniciar(static_cast<const unsigned char *>(archivo),
                                           static_cast<unsigned int>(longitud), tema) &&
                          fragmentada.iniciar(static_cast<const unsigned char *>(archivo),
                                              static_cast<unsigned int>(longitud), tema);
  SDL_free(archivo);
  if (!preparadas)
    return false;
  constexpr unsigned int cantidad = 48000 * 8;
  std::vector<short> primera(cantidad), segunda(cantidad);
  if (!continua.generar(primera.data(), cantidad))
    return false;
  constexpr unsigned int tamanos[] = {1, 682, 683, 1024, 4096};
  unsigned int posicion = 0, bloque = 0;
  while (posicion < cantidad) {
    auto tamano = tamanos[bloque++ % 5];
    if (tamano > cantidad - posicion)
      tamano = cantidad - posicion;
    if (!fragmentada.generar(segunda.data() + posicion, tamano))
      return false;
    posicion += tamano;
  }
  unsigned int no_nulas = 0;
  for (unsigned int i = 0; i < cantidad; ++i) {
    if (primera[i] != segunda[i])
      return false;
    no_nulas += primera[i] != 0 ? 1u : 0u;
  }
  constexpr auto pulsos = static_cast<unsigned long long>(cantidad) * 14318180 / 576000 / 0x4250;
  if (no_nulas < cantidad / 4 || continua.pulsos() != pulsos || fragmentada.pulsos() != pulsos)
    return false;
  unsigned char cabecera[44] = {
      'R', 'I', 'F', 'F', 0, 0, 0, 0, 'W', 'A', 'V', 'E', 'f', 'm', 't', ' ', 16,  0,   0, 0, 1, 0,
      1,   0,   0,   0,   0, 0, 0, 0, 0,   0,   2,   0,   16,  0,   'd', 'a', 't', 'a', 0, 0, 0, 0};
  campo_wav(cabecera, 4, cantidad * 2 + 36, 4);
  campo_wav(cabecera, 24, 48000, 4);
  campo_wav(cabecera, 28, 96000, 4);
  campo_wav(cabecera, 40, cantidad * 2, 4);
  std::vector<unsigned char> bytes(cantidad * 2);
  for (unsigned int i = 0; i < cantidad; ++i)
    campo_wav(bytes.data(), i * 2, static_cast<unsigned short>(primera[i]), 2);
  auto *salida = SDL_IOFromFile(ruta, "wb");
  if (!salida)
    return false;
  const bool guardado = SDL_WriteIO(salida, cabecera, 44) == 44 &&
                        SDL_WriteIO(salida, bytes.data(), bytes.size()) == bytes.size();
  const bool cerrado = SDL_CloseIO(salida);
  // Ejercita la salida común, el cambio de opción y su continuidad al desconectar una escena.
  Audio audio;
  static unsigned char segmento[65536]{};
  if (!guardado || !cerrado || !audio.abrir(carpeta))
    return false;
  SDL_Delay(25);
  bool integrada = audio.actualizar() && audio.conectar(segmento);
  const bool con_mt32 = segmento[0x0ad3] == 4;
  integrada = integrada && (segmento[0x0ad3] == 2 || con_mt32) && audio.alternar_dispositivo() &&
              segmento[0x0ad3] == 0 && audio.alternar_silencio();
  SDL_Delay(25);
  integrada =
      integrada && audio.actualizar() && audio.alternar_dispositivo() && segmento[0x0ad3] == 2;
  integrada = integrada && audio.alternar_silencio();
  const auto senal_anterior = audio.muestras_audibles();
  SDL_Delay(300);
  integrada = integrada && audio.muestras_audibles() > senal_anterior;
  if (con_mt32)
    integrada = integrada && audio.alternar_dispositivo() && segmento[0x0ad3] == 4;
  audio.desconectar();
  const auto antes_de_carga = audio.muestras_musicales();
  SDL_Delay(150);
  integrada = integrada && audio.muestras_musicales() > antes_de_carga;
  SDL_Delay(25);
  integrada = integrada && audio.actualizar();
  audio.cerrar();
  if (!integrada)
    return false;
  if (guardado && cerrado)
    std::printf("Música AdLib: %u muestras idénticas entre bloques, %llu IRQ; WAV: %s\n", cantidad,
                pulsos, ruta);
  return guardado && cerrado;
}

// Valida formato, señal no vacía, alimentación y arranque usando el mismo API que la partida.
bool comprobar_salida_audio(const char *ruta) {
  SDL_AudioSpec formato{};
  Uint8 *muestras = nullptr;
  Uint32 longitud = 0;
  if (!SDL_InitSubSystem(SDL_INIT_AUDIO) || !SDL_LoadWAV(ruta, &formato, &muestras, &longitud))
    return false;
  bool audible = false;
  for (Uint32 i = 0; i < longitud; ++i)
    audible = audible || muestras[i] != 0;
  auto *flujo =
      SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &formato, nullptr, nullptr);
  const bool correcto = flujo && audible && formato.format == SDL_AUDIO_S16 &&
                        (formato.channels == 1 || formato.channels == 2) && formato.freq == 48000 &&
                        SDL_PutAudioStreamData(flujo, muestras, static_cast<int>(longitud)) &&
                        SDL_GetAudioStreamQueued(flujo) == static_cast<int>(longitud) &&
                        SDL_ResumeAudioStreamDevice(flujo);
  SDL_free(muestras);
  if (flujo)
    SDL_DestroyAudioStream(flujo);
  if (correcto)
    std::printf("Salida SDL: PCM de 48.000 Hz aceptado por el dispositivo.\n");
  return correcto;
}

// Recorre 32 segundos de cada banco, incluida la percusión posterior al arranque de RS1.
bool comprobar_musica_mt32(const std::string &carpeta, const char *ruta) {
  size_t longitud = 0;
  void *archivo = SDL_LoadFile((carpeta + "/../CURSE.EXE").c_str(), &longitud);
  static MusicaMt32 sintetizador;
  const bool preparado = archivo && longitud <= 0xffffffffu &&
                         sintetizador.iniciar(carpeta, static_cast<const unsigned char *>(archivo),
                                              static_cast<unsigned int>(longitud));
  SDL_free(archivo);
  if (!preparado)
    return false;
  constexpr unsigned short escenas[] = {0, 64, 3, 15, 0xffff};
  constexpr unsigned int cantidad = 48000 * 32;
  std::vector<short> muestras(cantidad * 2);
  bool guardado = false;
  for (unsigned int banco = 0; banco < 5; ++banco) {
    if (!sintetizador.cambiar_escena(escenas[banco]) || sintetizador.banco() != banco ||
        !sintetizador.generar(muestras.data(), cantidad)) {
      sintetizador.cerrar();
      return false;
    }
    unsigned int audibles = 0, limitadas = 0, pico = 0;
    for (const auto valor : muestras) {
      audibles += valor > 64 || valor < -64 ? 1u : 0u;
      const auto amplitud = static_cast<unsigned int>(valor < 0 ? -static_cast<int>(valor) : valor);
      if (amplitud > pico)
        pico = amplitud;
      limitadas += amplitud == 28000 ? 1u : 0u;
    }
    if (audibles < 100) {
      sintetizador.cerrar();
      return false;
    }
    std::printf("MT-32: CURSE.RS%u genera %u muestras con señal; pico %u; muestras limitadas %u.\n",
                banco + 1, audibles, pico, limitadas);
    if (banco == 0) {
      if (!sintetizador.cambiar_escena(1) || sintetizador.banco() != 0 ||
          !sintetizador.cambiar_escena(2) || sintetizador.banco() != 0) {
        sintetizador.cerrar();
        return false;
      }
      unsigned char cabecera[44] = {'R', 'I', 'F', 'F', 0,   0,   0,   0, 'W', 'A', 'V',
                                    'E', 'f', 'm', 't', ' ', 16,  0,   0, 0,   1,   0,
                                    1,   0,   0,   0,   0,   0,   0,   0, 0,   0,   2,
                                    0,   16,  0,   'd', 'a', 't', 'a', 0, 0,   0,   0};
      campo_wav(cabecera, 4, cantidad * 4 + 36, 4);
      campo_wav(cabecera, 22, 2, 2);
      campo_wav(cabecera, 32, 4, 2);
      campo_wav(cabecera, 24, 48000, 4);
      campo_wav(cabecera, 28, 192000, 4);
      campo_wav(cabecera, 40, cantidad * 4, 4);
      std::vector<unsigned char> bytes(cantidad * 4);
      for (unsigned int i = 0; i < cantidad * 2; ++i)
        campo_wav(bytes.data(), i * 2, static_cast<unsigned short>(muestras[i]), 2);
      auto *salida = SDL_IOFromFile(ruta, "wb");
      if (!salida) {
        sintetizador.cerrar();
        return false;
      }
      guardado = SDL_WriteIO(salida, cabecera, 44) == 44 &&
                 SDL_WriteIO(salida, bytes.data(), bytes.size()) == bytes.size();
      guardado = SDL_CloseIO(salida) && guardado;
    }
  }
  sintetizador.cerrar();
  return guardado;
}
