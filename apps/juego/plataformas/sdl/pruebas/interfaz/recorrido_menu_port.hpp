#pragma once
#include "../../interfaz/menu/menu_port.hpp"
#include "../../lanzador/habitacion.hpp"
#include "../../mazmorra.hpp"
#include "../../audio/laboratorio.hpp"
#include <vector>

// Recorre el bucle real con eventos SDL, en una ventana oculta y un perfil deshabilitado.
class RecorridoMenuPort {
public:
  int avanzar(MenuPort &menu, Habitacion &habitacion, Mazmorra &partida, int pantalla,
              const std::string &prefijo, bool cargar, bool panel_abierto, LaboratorioAudio &lab) {
    const auto ahora = SDL_GetTicks();
    if (!inicio)
      inicio = ahora;
    if (ahora - inicio > 45000)
      return -1;
    ++fotogramas;
    switch (fase) {
    case 0:
      if (fotogramas < 4)
        break;
      if (!menu.abierto() || !habitacion.reiniciando())
        return -1;
      if (cargar)
        pulsar(SDLK_DOWN);
      pulsar(SDLK_RETURN);
      fase = 1;
      break;
    case 1:
      if (habitacion.reiniciando())
        break;
      if (menu.abierto())
        return -1;
      if (cargar) {
        if (pantalla != 3 || !panel_abierto)
          return -1;
        pulsar(SDLK_ESCAPE);
      } else {
        if (pantalla != 0)
          return -1;
        pulsar(SDLK_F6);
        pulsar(SDLK_F6);
        pulsar(SDLK_F6);
      }
      fase = 2;
      break;
    case 2:
      if (pantalla < 3)
        break;
      pulsar(SDLK_ESCAPE, SDL_KMOD_SHIFT);
      fase = 3;
      break;
    case 3:
      if (!menu.abierto())
        return -1;
      estado = leer(partida, prefijo + ".antes.ds");
      muestras = partida.muestras_musicales();
      marca = ahora;
      fase = 4;
      break;
    case 4:
      if (ahora - marca < 500)
        break;
      if (!menu.abierto() || estado.empty() || estado != leer(partida, prefijo + ".despues.ds") ||
          muestras != partida.muestras_musicales())
        return -1;
      // Ajustar volumen con teclado por el mismo panel que utiliza el usuario.
      pulsar(SDLK_DOWN);
      pulsar(SDLK_DOWN);
      pulsar(SDLK_RETURN);
      pulsar(SDLK_RETURN);
      pulsar(SDLK_RIGHT);
      fase = 5;
      break;
    case 5:
      if (partida.nivel_volumen_musica() != 110 || partida.nivel_volumen_efectos() != 100 ||
          partida.nivel_volumen() != 100)
        return -1;
      pulsar(SDLK_LEFT);
      for (int i = 0; i < 4; ++i)
        pulsar(SDLK_DOWN);
      pulsar(SDLK_RETURN); // Música y efectos.
      for (int i = 0; i < 3; ++i)
        pulsar(SDLK_DOWN);
      pulsar(SDLK_RETURN); // Pruebas.
      pulsar(SDLK_RETURN); // Reproductor.
      pulsar(SDLK_DOWN);
      pulsar(SDLK_DOWN);
      pulsar(SDLK_RETURN);
      fase = 9;
      break;
    case 9:
      if (!lab.reproduciendo() || !menu.abierto() || menu.pagina_audio() != 1 ||
          estado != leer(partida, prefijo + ".laboratorio.ds") ||
          muestras != partida.muestras_musicales())
        return -1;
      pulsar(SDLK_ESCAPE); // Laboratorio.
      pulsar(SDLK_DOWN);
      pulsar(SDLK_RETURN); // Estéreo.
      for (SDL_Keycode key = SDLK_1; key <= SDLK_5; ++key)
        pulsar(key);
      fase = 10;
      break;
    case 10:
      if (!lab.reproduciendo() || lab.posicion() != 4 || menu.pagina_audio() != 2 ||
          estado != leer(partida, prefijo + ".laboratorio_fx.ds") ||
          muestras != partida.muestras_musicales())
        return -1;
      pulsar(SDLK_ESCAPE, SDL_KMOD_SHIFT);
      marca = ahora;
      fase = 6;
      break;
    case 6:
      if (lab.reproduciendo() || menu.abierto() || partida.nivel_volumen_musica() != 100 ||
          partida.nivel_volumen_efectos() != 100 || partida.nivel_volumen() != 100)
        return -1;
      if (ahora - marca < 200)
        break;
      if (estado == leer(partida, prefijo + ".reanudada.ds") ||
          muestras == partida.muestras_musicales())
        return -1;
      pulsar(SDLK_ESCAPE); // Esc conserva el menú DOS.
      fase = 7;
      break;
    case 7:
      if (menu.abierto() || !partida.menu_abierto())
        return -1;
      pulsar(SDLK_ESCAPE);
      fase = 8;
      break;
    default:
      if (menu.abierto() || partida.menu_abierto())
        return -1;
      registrar_accion("Menú del port integrado: espera inicial, encendido, pausa DS/audio, "
                       "volumen, laboratorio aislado, reanudación y menú DOS comprobados");
      return 1;
    }
    return 0;
  }

private:
  unsigned int fase = 0, fotogramas = 0;
  Uint64 inicio = 0, marca = 0;
  unsigned long long muestras = 0;
  std::vector<unsigned char> estado;
  void pulsar(SDL_Keycode key, SDL_Keymod mod = SDL_KMOD_NONE) {
    SDL_Event e{};
    e.type = SDL_EVENT_KEY_DOWN;
    e.key.key = key;
    e.key.mod = mod;
    SDL_PushEvent(&e);
  }
  std::vector<unsigned char> leer(Mazmorra &partida, const std::string &ruta) {
    if (!partida.guardar_estado_diagnostico(ruta))
      return {};
    size_t n = 0;
    auto *p = static_cast<unsigned char *>(SDL_LoadFile(ruta.c_str(), &n));
    std::vector<unsigned char> salida;
    if (p)
      salida.assign(p, p + n);
    SDL_free(p);
    return salida;
  }
};
