#include "audio.hpp"
#include <cstdio>

// El flujo empieza pausado y se activa solo al tener una sesión de juego conectada.
bool Audio::abrir() {
  if (flujo)
    return true;
  if (!SDL_InitSubSystem(SDL_INIT_AUDIO))
    return false;
  const SDL_AudioSpec formato{SDL_AUDIO_S16, 1, static_cast<int>(motor::AltavozPcm::frecuencia)};
  flujo = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &formato, nullptr, nullptr);
  return flujo != nullptr;
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
  if (!segmento || !SDL_ClearAudioStream(flujo))
    return false;
  datos = segmento;
  altavoz.reiniciar();
  if (!altavoz.detener(datos))
    return false;
  datos[0x9924] = 0;
  datos[0x0ad4] = 1;
  ultimo = SDL_GetTicksNS();
  resto = 0;
  return SDL_ResumeAudioStreamDevice(flujo);
}

// Las vistas de desarrollo reinician la sesión; no deben dejar notas de otra vista sonando.
void Audio::desconectar() {
  if (datos) {
    altavoz.detener(datos);
    datos[0x0ad4] = 0;
  }
  datos = nullptr;
  if (flujo) {
    SDL_ClearAudioStream(flujo);
    SDL_PauseAudioStreamDevice(flujo);
  }
}

// Limita la cola a unos cien milisegundos sin ejecutar el motor desde el hilo de audio de SDL.
bool Audio::actualizar() {
  if (!flujo || !datos)
    return true;
  const auto ahora = SDL_GetTicksNS();
  auto transcurrido = ahora - ultimo;
  ultimo = ahora;
  // Tras una suspensión prolongada se descarta el efecto que habría terminado fuera de pantalla.
  if (transcurrido > 5000000000ull) {
    if (!altavoz.detener(datos) || !SDL_ClearAudioStream(flujo))
      return false;
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
  short muestras[1024];
  while (pendientes) {
    const auto cantidad = static_cast<unsigned int>(pendientes > 1024 ? 1024 : pendientes);
    if (!altavoz.generar(datos, muestras, cantidad))
      return false;
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
  return !flujo || SDL_ClearAudioStream(flujo);
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
