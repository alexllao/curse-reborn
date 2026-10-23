#include <SDL3/SDL.h>
#include <CoreGraphics/CoreGraphics.h>
#include <ImageIO/ImageIO.h>
#include <string>

// ImageIO decodifica los JPEG de la entrega; SDL sigue cargando los maestros PNG y las molduras.
SDL_Surface *cargar_jpeg_ios(const std::string &ruta) {
  CFURLRef url = CFURLCreateFromFileSystemRepresentation(
      kCFAllocatorDefault, reinterpret_cast<const UInt8 *>(ruta.data()), ruta.size(), false);
  CGImageSourceRef fuente = url ? CGImageSourceCreateWithURL(url, nullptr) : nullptr;
  CGImageRef imagen = fuente ? CGImageSourceCreateImageAtIndex(fuente, 0, nullptr) : nullptr;
  SDL_Surface *salida = nullptr;
  if (imagen) {
    const size_t ancho = CGImageGetWidth(imagen), alto = CGImageGetHeight(imagen);
    if (ancho > 0 && alto > 0 && ancho <= 16384 && alto <= 16384) {
      salida = SDL_CreateSurface(int(ancho), int(alto), SDL_PIXELFORMAT_RGBA32);
      CGColorSpaceRef color = CGColorSpaceCreateDeviceRGB();
      CGContextRef contexto =
          salida && color
              ? CGBitmapContextCreate(salida->pixels, ancho, alto, 8, salida->pitch, color,
                                      kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big)
              : nullptr;
      if (contexto) {
        CGContextDrawImage(contexto, CGRectMake(0, 0, ancho, alto), imagen);
        CGContextRelease(contexto);
      } else {
        SDL_DestroySurface(salida);
        salida = nullptr;
      }
      if (color)
        CGColorSpaceRelease(color);
    }
    CGImageRelease(imagen);
  }
  if (fuente)
    CFRelease(fuente);
  if (url)
    CFRelease(url);
  if (!salida)
    SDL_SetError("No se puede decodificar el fondo JPEG en iOS: %s", ruta.c_str());
  return salida;
}
