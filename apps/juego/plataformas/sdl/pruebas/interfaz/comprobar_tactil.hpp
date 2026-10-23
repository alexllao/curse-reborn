#pragma once
#include "../../interfaz/entrada_tactil.hpp"

// Eventos reproducibles sin dispositivo táctil; la validación UIKit queda para el iPad real.
inline bool comprobar_entrada_tactil() {
  EntradaTactil tactil;
  const auto dedo = [](Uint32 tipo, SDL_FingerID id, Uint64 tiempo) {
    SDL_Event e{};
    e.type = tipo;
    e.tfinger.fingerID = id;
    e.tfinger.timestamp = tiempo;
    e.tfinger.x = 0.5f;
    e.tfinger.y = 0.25f;
    return e;
  };
  auto e = dedo(SDL_EVENT_FINGER_DOWN, 1, 1000000000);
  if (!tactil.traducir(e, 1001, 801) || e.type != SDL_EVENT_MOUSE_MOTION)
    return false;
  e = dedo(SDL_EVENT_FINGER_UP, 1, 1100000000);
  if (!tactil.traducir(e, 1001, 801) || e.type != SDL_EVENT_MOUSE_BUTTON_DOWN ||
      e.button.button != SDL_BUTTON_LEFT || e.button.x != 500 || e.button.y != 200)
    return false;
  e = dedo(SDL_EVENT_FINGER_DOWN, 1, 2000000000);
  tactil.traducir(e, 1001, 801);
  e = dedo(SDL_EVENT_FINGER_UP, 1, 2600000000);
  if (!tactil.traducir(e, 1001, 801) || e.button.button != SDL_BUTTON_RIGHT)
    return false;
  e = dedo(SDL_EVENT_FINGER_DOWN, 1, 3000000000);
  tactil.traducir(e, 1001, 801);
  e = dedo(SDL_EVENT_FINGER_DOWN, 2, 3100000000);
  if (!tactil.traducir(e, 1001, 801) || e.type != SDL_EVENT_KEY_DOWN || e.key.key != SDLK_ESCAPE ||
      !(e.key.mod & SDL_KMOD_SHIFT))
    return false;
  for (SDL_FingerID id : {1, 2}) {
    e = dedo(SDL_EVENT_FINGER_UP, id, 3200000000);
    if (tactil.traducir(e, 1001, 801))
      return false;
  }
  e = dedo(SDL_EVENT_FINGER_DOWN, 1, 4000000000);
  tactil.traducir(e, 1001, 801);
  e = dedo(SDL_EVENT_FINGER_CANCELED, 1, 4100000000);
  if (tactil.traducir(e, 1001, 801))
    return false;
  e = dedo(SDL_EVENT_FINGER_UP, 1, 4200000000);
  if (tactil.traducir(e, 1001, 801))
    return false;
  e = {};
  e.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
  e.button.which = 7;
  return tactil.traducir(e, 1001, 801) && e.button.which == 7;
}
