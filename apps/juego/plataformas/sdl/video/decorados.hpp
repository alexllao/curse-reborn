#pragma once
#include <SDL3/SDL.h>
#include <algorithm>
#include <string>
#include <vector>
#include "../registro.hpp"

struct DefinicionDecorado {
  unsigned short escena;
  const char *archivo;
  SDL_Rect reserva;
  int ancho, alto;
  bool muestreo_area;
  int solape;
};
#include "catalogo_decorados.hpp"

// Decoración opcional dentro de los 320×200 originales, antes de todos los filtros.
class DecoradosAmpliados {
public:
  // La superficie propia no comparte recursos ni memoria con el motor DOS.
  ~DecoradosAmpliados() {
    cerrar();
  }
  // Mayús+F1 controla esta opción independientemente de los marcos de las bandas.
  void alternar() {
    if (catalogo_decorados.empty()) {
      registrar_accion("Decorados ampliados: aparcados; se conserva la imagen original");
      return;
    }
    visible = !visible;
    registrar_accion("Decorados ampliados: %s", visible ? "activados" : "desactivados");
  }
  // Permite liberar los recursos antes de SDL_Quit y repetir el cierre sin efectos.
  void cerrar() {
    SDL_DestroySurface(imagen);
    imagen = nullptr;
    actual = nullptr;
    integral.clear();
  }
  // Las escenas sin ilustración conservan su vacío original, sin heredar la del lugar anterior.
  static const DefinicionDecorado *buscar(unsigned short escena) {
    for (const auto &decorado : catalogo_decorados)
      if (decorado.escena == escena)
        return &decorado;
    return nullptr;
  }
  // Superpone el PNG completo: su alfa abre el vano y sus bordes quedan delante de la escena.
  bool componer(unsigned char *rgba, unsigned short escena, int menu_y, unsigned int luz) {
    const auto *seleccion = buscar(escena);
    if (!visible || !seleccion)
      return true;
    if (!imagen || actual != seleccion) {
      cerrar();
      const char *base = SDL_GetBasePath();
      const std::string ruta = std::string(base ? base : "") + seleccion->archivo;
      SDL_Surface *origen = SDL_LoadPNG(ruta.c_str());
      if (!origen)
        return false;
      imagen = SDL_ConvertSurface(origen, SDL_PIXELFORMAT_RGBA32);
      SDL_DestroySurface(origen);
      if (!imagen)
        return false;
      actual = seleccion;
      if (actual->muestreo_area)
        preparar_reduccion();
    }
    const auto *fuente = static_cast<const unsigned char *>(imagen->pixels);
    for (int y = 0; y < 200; ++y) {
      if (menu_y >= 0 && y >= menu_y && y < menu_y + 32)
        continue;
      const int sy = y * imagen->h / 200;
      for (int x = 0; x < 320; ++x) {
        auto *destino = rgba + (y * 320 + x) * 4;
        const int sx = x * imagen->w / 320;
        unsigned int color[4]{};
        if (actual->muestreo_area) {
          const int fin_x = std::max(sx + 1, (x + 1) * imagen->w / 320);
          const int fin_y = std::max(sy + 1, (y + 1) * imagen->h / 200);
          reducir_area(sx, sy, fin_x, fin_y, color);
        } else {
          const auto *muestra = fuente + sy * imagen->pitch + sx * 4;
          for (int c = 0; c < 3; ++c)
            color[c] = muestra[c] * muestra[3] / 255u;
          color[3] = muestra[3];
        }
        for (int c = 0; c < 3; ++c)
          destino[c] = static_cast<unsigned char>(color[c] * luz / 63u +
                                                  destino[c] * (255u - color[3]) / 255u);
      }
    }
    return true;
  }

private:
  // Suma una vez los colores premultiplicados para reducir sin ruido ni halos de transparencia.
  void preparar_reduccion() {
    const auto paso = static_cast<size_t>(imagen->w + 1) * 4u;
    integral.assign(paso * static_cast<size_t>(imagen->h + 1), 0);
    const auto *fuente = static_cast<const unsigned char *>(imagen->pixels);
    for (int y = 0; y < imagen->h; ++y) {
      Uint32 fila[4]{};
      for (int x = 0; x < imagen->w; ++x) {
        const auto *color = fuente + y * imagen->pitch + x * 4;
        for (int c = 0; c < 4; ++c) {
          fila[c] += c == 3 ? color[3] : color[c] * color[3] / 255u;
          const auto p = static_cast<size_t>(y + 1) * paso + static_cast<size_t>(x + 1) * 4u + c;
          integral[p] = integral[p - paso] + fila[c];
        }
      }
    }
  }
  // Promedia la superficie que representa cada píxel lógico en vez de escoger un punto aislado.
  void reducir_area(int x, int y, int derecha, int abajo, unsigned int *color) const {
    const auto paso = static_cast<size_t>(imagen->w + 1) * 4u;
    const unsigned int superficie = static_cast<unsigned int>((derecha - x) * (abajo - y));
    for (int c = 0; c < 4; ++c)
      color[c] = (integral[static_cast<size_t>(abajo) * paso + derecha * 4u + c] -
                  integral[static_cast<size_t>(y) * paso + derecha * 4u + c] -
                  integral[static_cast<size_t>(abajo) * paso + x * 4u + c] +
                  integral[static_cast<size_t>(y) * paso + x * 4u + c]) /
                 superficie;
  }
  bool visible = false;
  SDL_Surface *imagen = nullptr;
  const DefinicionDecorado *actual = nullptr;
  std::vector<Uint32> integral;
};
