#pragma once
#include <SDL3/SDL.h>
#include <cstdio>
#include <string>
#include <utility>
#include <vector>

// Sonido de la presentación: dispositivo lógico independiente del audio DOS pausado.
class SonidosPC {
public:
  ~SonidosPC() {
    cerrar();
  }
  void configurar(bool permitidos) {
    habilitado = permitidos;
  }
  void ajustar(bool silencio, unsigned int volumen) {
    ganancia = silencio ? 0.0f : volumen / 100.0f;
    if (flujo)
      SDL_SetAudioStreamGain(flujo, ganancia);
  }
  // 1: encendido completo; 2: apagado; 3: botón del monitor.
  bool reproducir(unsigned int accion) {
    if (!habilitado || !accion)
      return true;
    if (!abrir())
      return false;
    const auto &muestras = accion == 1 ? arranque : boton;
    return SDL_ClearAudioStream(flujo) &&
           SDL_PutAudioStreamData(flujo, muestras.data(), static_cast<int>(muestras.size())) &&
           SDL_FlushAudioStream(flujo);
  }
  void parar() {
    if (flujo)
      SDL_ClearAudioStream(flujo);
  }
  void cerrar() {
    if (flujo)
      SDL_DestroyAudioStream(flujo);
    flujo = nullptr;
    arranque.clear();
    boton.clear();
  }

private:
  friend bool comprobar_sonidos_pc();
  bool habilitado = false;
  float ganancia = 1;
  SDL_AudioStream *flujo = nullptr;
  std::vector<Uint8> arranque, boton;
  bool cargar(const char *nombre, std::vector<Uint8> &datos) {
    SDL_AudioSpec formato{};
    Uint8 *muestras = nullptr;
    Uint32 cantidad = 0;
    const std::string ruta =
        std::string(SDL_GetBasePath()) + "assets/lanzador/sonidos_pc/" + nombre;
    if (!SDL_LoadWAV(ruta.c_str(), &formato, &muestras, &cantidad))
      return false;
    const bool valido = formato.format == SDL_AUDIO_S16LE && formato.channels == 1 &&
                        formato.freq == 24000 && cantidad > 0 && cantidad <= 336000;
    if (valido)
      datos.assign(muestras, muestras + cantidad);
    SDL_free(muestras);
    return valido || SDL_SetError("Formato de sonido del PC incorrecto");
  }
  bool abrir() {
    if (flujo)
      return true;
    if (!SDL_InitSubSystem(SDL_INIT_AUDIO) || !cargar("arranque.wav", arranque) ||
        !cargar("boton.wav", boton))
      return false;
    const SDL_AudioSpec formato{SDL_AUDIO_S16LE, 1, 24000};
    flujo =
        SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &formato, nullptr, nullptr);
    if (!flujo || !SDL_SetAudioStreamGain(flujo, ganancia) || !SDL_ResumeAudioStreamDevice(flujo)) {
      cerrar();
      return false;
    }
    return true;
  }
};

// Comprueba el dispositivo real en silencio: mute conserva la cola y el final la vacía.
inline bool comprobar_sonidos_pc() {
  SonidosPC sonido;
  if (!sonido.reproducir(1) || sonido.flujo)
    return false;
  sonido.configurar(true);
  sonido.ajustar(true, 60);
  if (!sonido.abrir() || !SDL_PauseAudioStreamDevice(sonido.flujo) || !sonido.reproducir(1) ||
      SDL_GetAudioStreamQueued(sonido.flujo) != 336000 || SDL_GetAudioStreamGain(sonido.flujo) != 0)
    return false;
  sonido.ajustar(false, 60);
  if (SDL_GetAudioStreamGain(sonido.flujo) != 0.6f ||
      SDL_GetAudioStreamQueued(sonido.flujo) != 336000)
    return false;
  sonido.ajustar(false, 200);
  if (SDL_GetAudioStreamGain(sonido.flujo) != 2 || !sonido.reproducir(2) ||
      SDL_GetAudioStreamQueued(sonido.flujo) != 3840)
    return false;
  sonido.parar();
  if (SDL_GetAudioStreamQueued(sonido.flujo) != 0)
    return false;
  if (!sonido.reproducir(3) || SDL_GetAudioStreamQueued(sonido.flujo) != 3840)
    return false;
  sonido.cerrar();
  return !sonido.flujo;
}
