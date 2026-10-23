#pragma once
#include "../../interfaz/menu/menu_port.hpp"
#include "../../lanzador/habitacion.hpp"
#include "../../interfaz/preferencias.hpp"
#include <array>
#include <filesystem>

// Comprueba entrada y composición sin ventanas visibles, audio ni perfiles del usuario.
inline bool comprobar_menu_port(const std::string &carpeta) {
  std::filesystem::create_directories(std::filesystem::u8path(carpeta));
  const auto idioma_anterior = idioma_interfaz;
  EstadoMenuPort estado;
  estado.habitacion = "Buhardilla";
  MenuPort navegacion;
  navegacion.abrir();
  const auto tecla = [&](SDL_Keycode key, SDL_Keymod mod = SDL_KMOD_NONE) {
    SDL_Event e{};
    e.type = SDL_EVENT_KEY_DOWN;
    e.key.key = key;
    e.key.mod = mod;
    return navegacion.evento(e, 960, 600, estado);
  };
  using A = AccionMenuPort;
  bool ok = tecla(SDLK_ESCAPE).accion == A::ninguna && navegacion.abierto() &&
            tecla(SDLK_RETURN).accion == A::jugar;
  tecla(SDLK_DOWN);
  ok = ok && tecla(SDLK_RETURN).accion == A::cargar;
  tecla(SDLK_DOWN);
  tecla(SDLK_RETURN); // Opciones.
  tecla(SDLK_RETURN); // Sonido.
  ok = ok && tecla(SDLK_LEFT).accion == A::volumen_musica && tecla(SDLK_LEFT).sentido == -1;
  tecla(SDLK_DOWN);
  ok = ok && tecla(SDLK_RETURN).accion == A::volumen_efectos;
  tecla(SDLK_DOWN);
  ok = ok && tecla(SDLK_RIGHT).accion == A::volumen;
  tecla(SDLK_DOWN);
  ok = ok && tecla(SDLK_RETURN).accion == A::silencio;
  tecla(SDLK_DOWN);
  tecla(SDLK_RETURN); // Perfiles, efectos y estéreo en su propio apartado.
  ok = ok && tecla(SDLK_RIGHT).accion == A::musica;
  tecla(SDLK_DOWN);
  ok = ok && tecla(SDLK_RETURN).accion == A::efectos;
  tecla(SDLK_DOWN);
  ok = ok && tecla(SDLK_RETURN).accion == A::estereo;
  navegacion.abrir();
  tecla(SDLK_TAB, SDL_KMOD_SHIFT);
  tecla(SDLK_RETURN); // Confirmación de salida, no salir por una flecha accidental.
  ok = ok && tecla(SDLK_RETURN).accion == A::salir;
  navegacion.abrir();
  estado.sesion = true;
  ok = ok && tecla(SDLK_ESCAPE).accion == A::jugar;
  SDL_Event clic{};
  clic.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
  clic.button.button = SDL_BUTTON_LEFT;
  clic.button.x = 480;
  clic.button.y = 240;
  ok = ok && navegacion.evento(clic, 960, 600, estado).accion == A::jugar;
  clic.button.y = 287;
  ok = ok && navegacion.evento(clic, 960, 600, estado).accion == A::cargar;
  clic.button.x = 1;
  ok = ok && navegacion.evento(clic, 960, 600, estado).accion == A::ninguna;
  const std::string perfil = carpeta + "/preferencias.json";
  SDL_RemovePath(perfil.c_str());
  PreferenciasUsuario prefs;
  prefs.cargar(perfil);
  for (unsigned int efectos = 0; efectos < 2; ++efectos)
    for (unsigned int completa = 0; completa < 2; ++completa) {
      ok = ok &&
           prefs.guardar(130, true, true, 3, 4, 0, efectos, completa, "02-buhardilla", 60, 140);
      PreferenciasUsuario copia;
      copia.cargar(perfil);
      ok = ok && copia.efectos_digitales == efectos && copia.pantalla_completa == completa &&
           copia.habitacion == "02-buhardilla" && copia.volumen == 130 &&
           copia.estereo_posicional == 0 && copia.volumen_musica == 60 &&
           copia.volumen_efectos == 140;
    }
  SDL_RemovePath(perfil.c_str());
  prefs.cargar(perfil);
  ok = ok && !prefs.guardar(100, false, false, 2, 4, 1, 1, 0, "../fuera");
  ok = ok && prefs.guardar(100, false, false, 2);
  PreferenciasUsuario antigua;
  antigua.cargar(perfil);
  ok = ok && antigua.volumen_musica == 100 && antigua.volumen_efectos == 100;
  SDL_RemovePath(perfil.c_str());
  estado.sesion = false;
  const std::array<std::array<int, 2>, 6> medidas{
      {{1280, 360}, {1280, 480}, {960, 540}, {960, 600}, {800, 600}, {540, 960}}};
  unsigned int capturas = 0;
  for (int lengua = 0; lengua < 2 && ok; ++lengua) {
    idioma_interfaz = lengua ? IdiomaInterfaz::castellano : IdiomaInterfaz::ingles;
    for (const auto &m : medidas) {
      SDL_Surface *s = SDL_CreateSurface(m[0], m[1], SDL_PIXELFORMAT_RGBA32);
      SDL_Renderer *r = s ? SDL_CreateSoftwareRenderer(s) : nullptr;
      if (!r) {
        SDL_DestroySurface(s);
        idioma_interfaz = idioma_anterior;
        return false;
      }
      Habitacion habitacion;
      habitacion.iniciar("02-buhardilla");
      MenuPort menu;
      menu.abrir();
      const SDL_Rect vp{3, 4, 300, 180};
      ok = SDL_SetRenderDrawColor(r, 0, 0, 0, 255) && SDL_RenderClear(r) && habitacion.dibujar(r) &&
           SDL_SetRenderLogicalPresentation(r, 320, 200, SDL_LOGICAL_PRESENTATION_LETTERBOX) &&
           SDL_SetRenderViewport(r, &vp) && SDL_SetRenderScale(r, 1.2f, 1.3f) &&
           menu.dibujar(r, estado) && SDL_FlushRenderer(r);
      SDL_Rect vp_despues{};
      float sx = 0, sy = 0;
      int lw = 0, lh = 0;
      SDL_RendererLogicalPresentation lp;
      ok = ok && SDL_GetRenderViewport(r, &vp_despues) && vp.x == vp_despues.x &&
           vp.y == vp_despues.y && vp.w == vp_despues.w && vp.h == vp_despues.h &&
           SDL_GetRenderScale(r, &sx, &sy) && sx == 1.2f && sy == 1.3f &&
           SDL_GetRenderLogicalPresentation(r, &lw, &lh, &lp) && lw == 320 && lh == 200 &&
           lp == SDL_LOGICAL_PRESENTATION_LETTERBOX && habitacion.reiniciando() && menu.abierto();
      const auto nombre = carpeta + "/menu_" + std::to_string(m[0]) + "x" + std::to_string(m[1]) +
                          (lengua ? "_es.png" : "_en.png");
      ok = ok && SDL_SavePNG(s, nombre.c_str());
      if (ok)
        ++capturas;
      // También se dibujan las seis páginas internas con sus valores y textos completos.
      if (ok && lengua && m[0] == 960 && m[1] == 600) {
        for (int pagina = 0; pagina < 7 && ok; ++pagina) {
          menu.abrir();
          const auto pulsar = [&](SDL_Keycode key) {
            SDL_Event e{};
            e.type = SDL_EVENT_KEY_DOWN;
            e.key.key = key;
            return menu.evento(e, m[0], m[1], estado);
          };
          const int indice = pagina < 4 || pagina == 6 ? 2 : pagina == 4 ? 3 : 4;
          for (int j = 0; j < indice; ++j)
            pulsar(SDLK_DOWN);
          pulsar(SDLK_RETURN);
          if (pagina == 6) {
            pulsar(SDLK_RETURN);
            for (int j = 0; j < 4; ++j)
              pulsar(SDLK_DOWN);
            pulsar(SDLK_RETURN);
          } else if (pagina < 4) {
            for (int j = 0; j < pagina; ++j)
              pulsar(SDLK_DOWN);
            pulsar(SDLK_RETURN);
          }
          ok = SDL_SetRenderLogicalPresentation(r, 0, 0, SDL_LOGICAL_PRESENTATION_DISABLED) &&
               SDL_SetRenderViewport(r, nullptr) && SDL_SetRenderScale(r, 1, 1) &&
               SDL_SetRenderDrawColor(r, 0, 0, 0, 255) && SDL_RenderClear(r) &&
               habitacion.dibujar(r) && menu.dibujar(r, estado) && SDL_FlushRenderer(r) &&
               SDL_SavePNG(s, (carpeta + "/pagina_" + std::to_string(pagina) + ".png").c_str());
          if (ok)
            ++capturas;
        }
      }
      menu.liberar();
      habitacion.cerrar();
      SDL_DestroyRenderer(r);
      SDL_DestroySurface(s);
      if (!ok)
        break;
    }
  }
  idioma_interfaz = idioma_anterior;
  registrar_accion("Menú del port: teclado, ratón, preferencias y %u capturas en seis "
                   "proporciones; PC apagado y encuadre conservado",
                   capturas);
  return ok;
}
