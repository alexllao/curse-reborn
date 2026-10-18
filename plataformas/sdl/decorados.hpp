#pragma once
#include <SDL3/SDL.h>
#include <algorithm>
#include <string>
#include "registro.hpp"

struct DefinicionDecorado {
  unsigned short escena;
  const char *archivo;
  SDL_Rect reserva, vano;
  int ancho, alto;
  bool ajustar_silueta;
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
    visible = !visible;
    registrar_accion("Decorados ampliados: %s", visible ? "activados" : "desactivados");
  }
  // Permite liberar los recursos antes de SDL_Quit y repetir el cierre sin efectos.
  void cerrar() {
    SDL_DestroySurface(imagen);
    imagen = nullptr;
    actual = nullptr;
  }
  // Las escenas sin ilustración conservan su vacío original, sin heredar la del lugar anterior.
  static const DefinicionDecorado *buscar(unsigned short escena) {
    for (const auto &decorado : catalogo_decorados)
      if (decorado.escena == escena)
        return &decorado;
    return nullptr;
  }
  // Sustituye únicamente el vacío exterior; conserva escena, cursor y menú originales.
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
    }
    const auto &reserva = actual->reserva;
    const auto *fuente = static_cast<const unsigned char *>(imagen->pixels);
    for (int y = 0; y < 200; ++y) {
      if (menu_y >= 0 && y >= menu_y && y < menu_y + 32)
        continue;
      int izquierda = reserva.x, derecha = reserva.x + reserva.w;
      if (actual->ajustar_silueta) {
        int primera = derecha, ultima = izquierda - 1;
        for (int x = izquierda; x < derecha; ++x) {
          const auto *color = rgba + (y * 320 + x) * 4;
          if (color[0] || color[1] || color[2]) {
            primera = std::min(primera, x);
            ultima = x;
          }
        }
        if (primera <= ultima) {
          izquierda = primera;
          derecha = ultima + 1;
        }
      }
      const int sy = std::min(
          proyectar(y, reserva.y, reserva.h, 200, actual->vano.y, actual->vano.h, imagen->h),
          imagen->h - 1);
      for (int x = 0; x < 320; ++x) {
        if (x >= izquierda && x < derecha && y >= reserva.y && y < reserva.y + reserva.h)
          continue;
        auto *destino = rgba + (y * 320 + x) * 4;
        if (destino[0] || destino[1] || destino[2])
          continue;
        const int sx = std::min(proyectar(x, izquierda, derecha - izquierda, 320, actual->vano.x,
                                          actual->vano.w, imagen->w),
                                imagen->w - 1);
        const auto *color = fuente + sy * imagen->pitch + sx * 4;
        for (int c = 0; c < 3; ++c)
          destino[c] = static_cast<unsigned char>(color[c] * color[3] * luz / (255u * 63u));
      }
    }
    return true;
  }

private:
  // Alinea por separado cada lado y el vano sin estrechar ni desplazar la escena DOS.
  static int proyectar(int p, int inicio, int largo, int total, int vano, int hueco, int salida) {
    if (p < inicio)
      return p * vano / inicio;
    if (p < inicio + largo)
      return vano + (p - inicio) * hueco / largo;
    return vano + hueco + (p - inicio - largo) * (salida - vano - hueco) / (total - inicio - largo);
  }
  bool visible = false;
  SDL_Surface *imagen = nullptr;
  const DefinicionDecorado *actual = nullptr;
};
