#pragma once
#include <SDL3/SDL.h>
#include <algorithm>
#include <set>

// Traduce gestos al mismo recorrido de eventos que usa el ratón; no modifica el motor.
class EntradaTactil {
public:
  void reiniciar() {
    dedos.clear();
    bloqueado = false;
  }
  // Un toque acepta al soltar, mantener500ms equivale al botón derecho y dos dedos abren el port.
  bool traducir(SDL_Event &e, int ancho, int alto) {
    if (e.type != SDL_EVENT_FINGER_DOWN && e.type != SDL_EVENT_FINGER_UP &&
        e.type != SDL_EVENT_FINGER_MOTION && e.type != SDL_EVENT_FINGER_CANCELED)
      return true;
    const auto dedo = e.tfinger;
    if (e.type == SDL_EVENT_FINGER_CANCELED) {
      reiniciar();
      return false;
    }
    if (e.type != SDL_EVENT_FINGER_DOWN && !dedos.count(dedo.fingerID))
      return false;
    if (e.type == SDL_EVENT_FINGER_DOWN) {
      if (dedos.empty()) {
        principal = dedo.fingerID;
        inicio = dedo.timestamp;
      }
      dedos.insert(dedo.fingerID);
      if (dedos.size() > 1 && !bloqueado) {
        bloqueado = true;
        e = {};
        e.type = SDL_EVENT_KEY_DOWN;
        e.key.windowID = dedo.windowID;
        e.key.key = SDLK_ESCAPE;
        e.key.mod = SDL_KMOD_SHIFT;
        return true;
      }
    }
    const bool soltar = e.type == SDL_EVENT_FINGER_UP;
    if (soltar)
      dedos.erase(dedo.fingerID);
    if (bloqueado || dedo.fingerID != principal) {
      if (dedos.empty())
        bloqueado = false;
      return false;
    }
    // Una cancelación anterior no debe producir un clic al llegar un UP tardío.
    if (!soltar && dedos.empty())
      return false;
    if (soltar && dedo.timestamp < inicio)
      return false;
    const float x = std::clamp(dedo.x, 0.0f, 1.0f) * std::max(ancho - 1, 0);
    const float y = std::clamp(dedo.y, 0.0f, 1.0f) * std::max(alto - 1, 0);
    e = {};
    if (soltar) {
      e.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
      e.button.windowID = dedo.windowID;
      e.button.which = SDL_TOUCH_MOUSEID;
      e.button.button =
          dedo.timestamp - inicio >= 500000000ull ? SDL_BUTTON_RIGHT : SDL_BUTTON_LEFT;
      e.button.down = true;
      e.button.clicks = 1;
      e.button.x = x;
      e.button.y = y;
    } else {
      e.type = SDL_EVENT_MOUSE_MOTION;
      e.motion.windowID = dedo.windowID;
      e.motion.which = SDL_TOUCH_MOUSEID;
      e.motion.x = x;
      e.motion.y = y;
    }
    return true;
  }

private:
  std::set<SDL_FingerID> dedos;
  SDL_FingerID principal = 0;
  Uint64 inicio = 0;
  bool bloqueado = false;
};
