#include "audio.hpp"
#include <cstdio>
#include <vector>

// El tema arranca antes de Core, como 048A; SDL adapta el formato al dispositivo real.
bool Audio::abrir(const std::string &carpeta) {
  if (flujo)
    return true;
  if (!SDL_InitSubSystem(SDL_INIT_AUDIO))
    return false;
  const SDL_AudioSpec formato{SDL_AUDIO_S16, 1, static_cast<int>(motor::AltavozPcm::frecuencia)};
  flujo = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &formato, nullptr, nullptr);
  if (!flujo)
    return false;
  size_t longitud = 0;
  void *archivo = SDL_LoadFile((carpeta + "/../CURSE.EXE").c_str(), &longitud);
  const bool preparada = archivo && longitud <= 0xffffffffu &&
                         musica.iniciar(static_cast<const unsigned char *>(archivo),
                                        static_cast<unsigned int>(longitud));
  SDL_free(archivo);
  ultimo = SDL_GetTicksNS();
  resto = 0;
  if (!preparada || !SDL_ResumeAudioStreamDevice(flujo)) {
    cerrar();
    return false;
  }
  std::printf("Audio: AdLib (música) + PC Speaker (efectos). F4 cambia la opción; M silencia.\n"
              "SB16 y MT-32: pendientes de integrar.\n");
  std::fflush(stdout);
  return true;
}

// El flujo posee su dispositivo, por lo que destruirlo libera también la salida asociada.
void Audio::cerrar() {
  if (flujo)
    SDL_DestroyAudioStream(flujo);
  flujo = nullptr;
  datos = nullptr;
}

// Las comparaciones gráficas conservan la modalidad cero al no abrir un dispositivo.
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
  datos[0x0ad3] = musica_audible ? 2 : 0;
  return SDL_ResumeAudioStreamDevice(flujo);
}

// Las vistas de desarrollo reinician la sesión; no deben dejar notas de otra vista sonando.
void Audio::desconectar() {
  if (datos) {
    altavoz.detener(datos);
    datos[0x0ad4] = 0;
    datos[0x0ad3] = 0;
  }
  datos = nullptr;
  if (flujo)
    SDL_ClearAudioStream(flujo);
}

// Limita la cola a unos cien milisegundos sin ejecutar el motor desde el hilo de audio de SDL.
bool Audio::actualizar() {
  if (!flujo)
    return true;
  const auto ahora = SDL_GetTicksNS();
  auto transcurrido = ahora - ultimo;
  ultimo = ahora;
  // Tras una suspensión prolongada se descarta el efecto que habría terminado fuera de pantalla.
  if (transcurrido > 5000000000ull) {
    if ((datos && !altavoz.detener(datos)) || !SDL_ClearAudioStream(flujo))
      return false;
    if (datos)
      datos[0x9924] = 0;
    transcurrido = resto = 0;
  }
  const auto reloj = transcurrido * motor::AltavozPcm::frecuencia + resto;
  auto pendientes = reloj / 1000000000u;
  resto = reloj % 1000000000u;
  const auto en_cola = SDL_GetAudioStreamQueued(flujo);
  if (en_cola < 0)
    return false;
  if (en_cola > 9600 && !SDL_ClearAudioStream(flujo))
    return false;
  short muestras[1024], melodia[1024];
  while (pendientes) {
    const auto cantidad = static_cast<unsigned int>(pendientes > 1024 ? 1024 : pendientes);
    if (!musica.generar(melodia, cantidad) ||
        (datos && !altavoz.generar(datos, muestras, cantidad)))
      return false;
    for (unsigned int i = 0; i < cantidad; ++i) {
      const int mezcla = (musica_audible ? melodia[i] : 0) + (datos ? muestras[i] : 0);
      muestras[i] = static_cast<short>(mezcla < -32768 ? -32768 : mezcla > 32767 ? 32767 : mezcla);
    }
    pendientes -= cantidad;
    if (silencio)
      for (unsigned int i = 0; i < cantidad; ++i)
        muestras[i] = 0;
    // Al recuperar un cuadro lento solo se presenta su final, pero todas las IRQ se consumen.
    if (pendientes < 2400 &&
        !SDL_PutAudioStreamData(flujo, muestras, static_cast<int>(cantidad * 2)))
      return false;
  }
  return true;
}

// El segmento debe pertenecer a la sesión conectada, para no mezclar partidas de desarrollo.
bool Audio::reproducir(unsigned char *segmento, unsigned short entidad,
                       unsigned char identificador) {
  if (segmento[0x0ad4] == 0)
    return true;
  return datos == segmento && actualizar() && altavoz.reproducir(datos, entidad, identificador);
}

// M es un control de plataforma; no cambia la modalidad DOS ni el resultado de los guiones.
bool Audio::alternar_silencio() {
  silencio = !silencio;
  std::printf("Audio: %s.\n", silencio ? "silenciado" : "reactivado");
  std::fflush(stdout);
  return !flujo || SDL_ClearAudioStream(flujo);
}

// Mantiene el tema en curso: cambiar de opción modifica la mezcla, sin reiniciar canales.
bool Audio::alternar_dispositivo() {
  if (!flujo) {
    std::printf("Audio desactivado o sin dispositivo; inicia sin --sin-sonido para cambiarlo.\n");
    std::fflush(stdout);
    return true;
  }
  if (!actualizar() || !SDL_ClearAudioStream(flujo))
    return false;
  musica_audible = !musica_audible;
  if (datos)
    datos[0x0ad3] = musica_audible ? 2 : 0;
  std::printf("Audio cambiado a %s%s.\n",
              musica_audible ? "AdLib (música) + PC Speaker (efectos)"
                             : "PC Speaker (efectos; sin música)",
              silencio ? " [silenciado con M]" : "");
  std::fflush(stdout);
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
bool comprobar_musica_adlib(const std::string &carpeta, const char *ruta) {
  size_t longitud = 0;
  void *archivo = SDL_LoadFile((carpeta + "/../CURSE.EXE").c_str(), &longitud);
  if (!archivo || longitud > 0xffffffffu) {
    SDL_free(archivo);
    return false;
  }
  static MusicaAdlib continua, fragmentada;
  const bool preparadas = continua.iniciar(static_cast<const unsigned char *>(archivo),
                                           static_cast<unsigned int>(longitud)) &&
                          fragmentada.iniciar(static_cast<const unsigned char *>(archivo),
                                              static_cast<unsigned int>(longitud));
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
  bool integrada = audio.actualizar() && audio.conectar(segmento) && segmento[0x0ad3] == 2 &&
                   audio.alternar_dispositivo() && segmento[0x0ad3] == 0 &&
                   audio.alternar_silencio();
  SDL_Delay(25);
  integrada =
      integrada && audio.actualizar() && audio.alternar_dispositivo() && segmento[0x0ad3] == 2;
  audio.desconectar();
  SDL_Delay(25);
  integrada = integrada && audio.actualizar() && audio.alternar_silencio();
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
                        formato.channels == 1 && formato.freq == 48000 &&
                        SDL_PutAudioStreamData(flujo, muestras, static_cast<int>(longitud)) &&
                        SDL_GetAudioStreamQueued(flujo) == static_cast<int>(longitud) &&
                        SDL_ResumeAudioStreamDevice(flujo);
  SDL_free(muestras);
  if (flujo)
    SDL_DestroyAudioStream(flujo);
  if (correcto)
    std::printf("Salida SDL: PCM mono de 48.000 Hz aceptado por el dispositivo.\n");
  return correcto;
}
