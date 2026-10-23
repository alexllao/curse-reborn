#pragma once
#include <SDL3/SDL.h>
#include <string>
#include <vector>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <wincodec.h>

namespace fondos_windows {
// Libera las interfaces antes de cerrar COM y su biblioteca.
template <class T> struct Interfaz {
  T *p = nullptr;
  ~Interfaz() {
    if (p)
      p->Release();
  }
};

// WIC pertenece a Windows: el paquete no necesita otra DLL ni un decodificador descargado.
inline SDL_Surface *cargar_jpeg(const std::string &ruta) {
  HMODULE ole = LoadLibraryExW(L"ole32.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
  if (!ole)
    return nullptr;
  const auto iniciar =
      reinterpret_cast<decltype(&CoInitializeEx)>(GetProcAddress(ole, "CoInitializeEx"));
  const auto terminar =
      reinterpret_cast<decltype(&CoUninitialize)>(GetProcAddress(ole, "CoUninitialize"));
  const auto crear =
      reinterpret_cast<decltype(&CoCreateInstance)>(GetProcAddress(ole, "CoCreateInstance"));
  SDL_Surface *salida = nullptr;
  if (iniciar && terminar && crear) {
    const HRESULT inicio = iniciar(nullptr, COINIT_MULTITHREADED);
    if (SUCCEEDED(inicio) || inicio == RPC_E_CHANGED_MODE) {
      {
        Interfaz<IWICImagingFactory> fabrica;
        Interfaz<IWICBitmapDecoder> decodificador;
        Interfaz<IWICBitmapFrameDecode> imagen;
        Interfaz<IWICFormatConverter> conversor;
        const GUID clase{
            0xcacaf262, 0x9370, 0x4615, {0xa1, 0x3b, 0x9f, 0x55, 0x39, 0xda, 0x4c, 0x0a}};
        const GUID bgra{
            0x6fddc324, 0x4e03, 0x4bfe, {0xb1, 0x85, 0x3d, 0x77, 0x76, 0x8d, 0xc9, 0x0f}};
        const int n =
            MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, ruta.c_str(), -1, nullptr, 0);
        std::vector<wchar_t> nombre(n > 0 ? n : 0);
        UINT ancho = 0, alto = 0;
        if (n > 0 &&
            MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, ruta.c_str(), -1, nombre.data(),
                                n) &&
            SUCCEEDED(crear(clase, nullptr, CLSCTX_INPROC_SERVER, __uuidof(IWICImagingFactory),
                            reinterpret_cast<void **>(&fabrica.p))) &&
            SUCCEEDED(fabrica.p->CreateDecoderFromFilename(nombre.data(), nullptr, GENERIC_READ,
                                                           WICDecodeMetadataCacheOnLoad,
                                                           &decodificador.p)) &&
            SUCCEEDED(decodificador.p->GetFrame(0, &imagen.p)) &&
            SUCCEEDED(imagen.p->GetSize(&ancho, &alto)) && ancho > 0 && alto > 0 &&
            ancho <= 16384 && alto <= 16384 &&
            SUCCEEDED(fabrica.p->CreateFormatConverter(&conversor.p)) &&
            SUCCEEDED(conversor.p->Initialize(imagen.p, bgra, WICBitmapDitherTypeNone, nullptr, 0,
                                              WICBitmapPaletteTypeCustom))) {
          salida = SDL_CreateSurface(int(ancho), int(alto), SDL_PIXELFORMAT_BGRA32);
          if (salida && FAILED(conversor.p->CopyPixels(nullptr, UINT(salida->pitch),
                                                       UINT(salida->pitch) * alto,
                                                       static_cast<BYTE *>(salida->pixels)))) {
            SDL_DestroySurface(salida);
            salida = nullptr;
          }
        }
      }
      if (SUCCEEDED(inicio))
        terminar();
    }
  }
  FreeLibrary(ole);
  if (!salida)
    SDL_SetError("No se puede decodificar el fondo JPEG: %s", ruta.c_str());
  return salida;
}
} // namespace fondos_windows
#endif

#ifdef SDL_PLATFORM_IOS
SDL_Surface *cargar_jpeg_ios(const std::string &ruta);
#endif

// El catálogo conserva las rutas maestras. Solo la copia empaquetada sustituye PNG por JPG.
inline SDL_Surface *cargar_fondo(const std::string &ruta) {
  SDL_PathInfo info;
  if (SDL_GetPathInfo(ruta.c_str(), &info))
    return SDL_LoadPNG(ruta.c_str());
  if (ruta.size() < 4 || ruta.substr(ruta.size() - 4) != ".png")
    return SDL_LoadPNG(ruta.c_str());
  const std::string jpeg = ruta.substr(0, ruta.size() - 4) + ".jpg";
#ifdef _WIN32
  return fondos_windows::cargar_jpeg(jpeg);
#elif defined(SDL_PLATFORM_IOS)
  return cargar_jpeg_ios(jpeg);
#else
  SDL_SetError("Esta plataforma no dispone todavía de cargador JPEG: %s", jpeg.c_str());
  return nullptr;
#endif
}
