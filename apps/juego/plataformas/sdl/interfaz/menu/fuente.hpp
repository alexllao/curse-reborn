#pragma once
#include <SDL3/SDL.h>
#include <map>
#include <string>
#include <vector>
#include "../texto_hud.hpp"
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

// Texto nítido del sistema en Windows, sin distribuir fuentes ni añadir DLL obligatorias.
// Las demás plataformas conservan la fuente SDL hasta incorporar su adaptador tipográfico.
class FuenteMenu {
public:
  ~FuenteMenu() {
    cerrar();
  }
  void cerrar() {
    for (auto &p : cache)
      SDL_DestroyTexture(p.second.textura);
    cache.clear();
  }
  bool texto(SDL_Renderer *r, const std::string &s, float x, float y, float alto, SDL_Color color,
             bool centrado = false) {
#ifdef _WIN32
    const auto clave = std::to_string(int(alto)) + ":" + s;
    auto encontrado = cache.find(clave);
    if (encontrado == cache.end()) {
      Letra l;
      if (crear(r, s, int(alto * 2), l))
        encontrado = cache.emplace(clave, l).first;
    }
    if (encontrado != cache.end()) {
      const auto &l = encontrado->second;
      const SDL_FRect destino{x - (centrado ? l.ancho / 4.0f : 0), y, l.ancho / 2.0f,
                              l.alto / 2.0f};
      return SDL_SetTextureColorMod(l.textura, color.r, color.g, color.b) &&
             SDL_SetTextureAlphaMod(l.textura, color.a) &&
             SDL_RenderTexture(r, l.textura, nullptr, &destino);
    }
#endif
    // El mismo tamaño lógico, sin hacer depender la navegación de la tipografía disponible.
    const float escala = alto / 12;
    float sx = 1, sy = 1;
    if (!SDL_GetRenderScale(r, &sx, &sy) || !SDL_SetRenderScale(r, sx * escala, sy * escala))
      return false;
    const bool ok = SDL_SetRenderDrawColor(r, color.r, color.g, color.b, color.a) &&
                    dibujar_texto_hud(r, x / escala - (centrado ? s.size() * 4 : 0), y / escala, s);
    return SDL_SetRenderScale(r, sx, sy) && ok;
  }

private:
  struct Letra {
    SDL_Texture *textura = nullptr;
    int ancho = 0, alto = 0;
  };
  std::map<std::string, Letra> cache;
#ifdef _WIN32
  bool crear(SDL_Renderer *r, const std::string &s, int tamano, Letra &l) {
    // Se resuelve GDI dinámicamente: el motor y las compilaciones no necesitan otra biblioteca.
    static HMODULE gdi = LoadLibraryW(L"gdi32.dll");
    if (!gdi)
      return false;
#define FUNCION_GDI(nombre)                                                                        \
  const auto f_##nombre = reinterpret_cast<decltype(&nombre)>(GetProcAddress(gdi, #nombre));       \
  if (!f_##nombre)                                                                                 \
  return false
    FUNCION_GDI(CreateCompatibleDC);
    FUNCION_GDI(CreateFontW);
    FUNCION_GDI(SelectObject);
    FUNCION_GDI(GetTextExtentPoint32W);
    FUNCION_GDI(CreateDIBSection);
    FUNCION_GDI(SetTextColor);
    FUNCION_GDI(SetBkColor);
    FUNCION_GDI(TextOutW);
    FUNCION_GDI(DeleteObject);
    FUNCION_GDI(DeleteDC);
    FUNCION_GDI(GdiFlush);
#undef FUNCION_GDI
    const int n = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
    if (n <= 1)
      return false;
    std::vector<wchar_t> texto(n);
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, texto.data(), n);
    HDC dc = f_CreateCompatibleDC(nullptr);
    HFONT fuente = f_CreateFontW(-tamano, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                 OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
                                 DEFAULT_PITCH, L"Georgia");
    if (!dc || !fuente) {
      if (fuente)
        f_DeleteObject(fuente);
      if (dc)
        f_DeleteDC(dc);
      return false;
    }
    const auto anterior = f_SelectObject(dc, fuente);
    SIZE medida{};
    const bool medido = f_GetTextExtentPoint32W(dc, texto.data(), n - 1, &medida);
    const int ancho = int(medida.cx) + 8, alto = tamano * 2;
    BITMAPINFO info{};
    info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biWidth = ancho;
    info.bmiHeader.biHeight = -alto;
    info.bmiHeader.biPlanes = 1;
    info.bmiHeader.biBitCount = 32;
    info.bmiHeader.biCompression = BI_RGB;
    void *pixeles = nullptr;
    HBITMAP bitmap =
        medido ? f_CreateDIBSection(dc, &info, DIB_RGB_COLORS, &pixeles, nullptr, 0) : nullptr;
    HGDIOBJ viejo_bitmap = nullptr;
    bool ok = bitmap && pixeles;
    if (ok) {
      viejo_bitmap = f_SelectObject(dc, bitmap);
      SDL_memset(pixeles, 0, size_t(ancho) * alto * 4);
      f_SetTextColor(dc, RGB(255, 255, 255));
      f_SetBkColor(dc, RGB(0, 0, 0));
      ok = f_TextOutW(dc, 4, 0, texto.data(), n - 1) && f_GdiFlush();
      std::vector<unsigned char> rgba(size_t(ancho) * alto * 4);
      const auto *p = static_cast<const unsigned char *>(pixeles);
      for (size_t i = 0; i < rgba.size(); i += 4) {
        rgba[i] = rgba[i + 1] = rgba[i + 2] = 255;
        rgba[i + 3] = p[i];
      }
      SDL_Surface *srf =
          SDL_CreateSurfaceFrom(ancho, alto, SDL_PIXELFORMAT_RGBA32, rgba.data(), ancho * 4);
      l.textura = ok && srf ? SDL_CreateTextureFromSurface(r, srf) : nullptr;
      SDL_DestroySurface(srf);
      l.ancho = ancho;
      l.alto = alto;
      ok = l.textura && SDL_SetTextureBlendMode(l.textura, SDL_BLENDMODE_BLEND) &&
           SDL_SetTextureScaleMode(l.textura, SDL_SCALEMODE_LINEAR);
      if (!ok && l.textura) {
        SDL_DestroyTexture(l.textura);
        l.textura = nullptr;
      }
    }
    if (viejo_bitmap)
      f_SelectObject(dc, viejo_bitmap);
    if (bitmap)
      f_DeleteObject(bitmap);
    f_SelectObject(dc, anterior);
    f_DeleteObject(fuente);
    f_DeleteDC(dc);
    return ok;
  }
#endif
};
