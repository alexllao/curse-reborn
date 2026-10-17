#include "planos.hpp"

namespace motor {
namespace {
bool geometria(unsigned short ancho, unsigned short alto, unsigned short paso, unsigned short inicio) {
  const unsigned int grupos = ancho >> 2;
  return grupos != 0 && alto != 0 && paso >= grupos &&
    grupos * 4u * alto <= 65536u && inicio + (alto - 1u) * paso + grupos <= 65536u;
}
}

bool subir_imagen(const unsigned char* origen, unsigned char* planos, const PuertosVideo& puertos,
    unsigned short ancho, unsigned short alto, unsigned short paso, unsigned short destino) {
  if (!puertos.escribir || !geometria(ancho, alto, paso, destino)) return false;
  const unsigned int grupos = ancho >> 2;
  puertos.escribir(puertos.contexto, 0x3c4, 2, 1);
  unsigned int si = 0;
  for (unsigned int y = 0; y < alto; ++y) {
    for (unsigned int x = 0; x < grupos; ++x) {
      const auto di = destino + y * paso + x;
      for (unsigned int plano = 0; plano < 4; ++plano) {
        puertos.escribir(puertos.contexto, 0x3c5, static_cast<unsigned short>(1u << plano), 1);
        planos[plano * 65536u + di] = origen[si++];
      }
    }
  }
  return true;
}

bool recuperar_imagen(const unsigned char* planos, unsigned char* destino, const PuertosVideo& puertos,
    unsigned short ancho, unsigned short alto, unsigned short paso, unsigned short origen) {
  if (!puertos.escribir || !geometria(ancho, alto, paso, origen)) return false;
  const unsigned int grupos = ancho >> 2;
  puertos.escribir(puertos.contexto, 0x3ce, 4, 1);
  unsigned int di = 0;
  for (unsigned int y = 0; y < alto; ++y) {
    for (unsigned int x = 0; x < grupos; ++x) {
      const auto si = origen + y * paso + x;
      for (unsigned int plano = 0; plano < 4; ++plano) {
        puertos.escribir(puertos.contexto, 0x3cf, static_cast<unsigned short>(plano), 1);
        destino[di++] = planos[plano * 65536u + si];
      }
    }
  }
  return true;
}

bool copiar_planos(unsigned char* planos, const PuertosVideo& puertos, unsigned short ancho,
    unsigned short alto, unsigned short paso_origen, unsigned short paso_destino,
    unsigned short origen, unsigned short destino) {
  if (!puertos.leer || !puertos.escribir || !geometria(ancho, alto, paso_origen, origen) ||
      !geometria(ancho, alto, paso_destino, destino)) return false;
  const unsigned int grupos = ancho >> 2;
  puertos.escribir(puertos.contexto, 0x3c4, 0x0f02, 2);
  puertos.escribir(puertos.contexto, 0x3ce, 5, 1);
  auto control = puertos.leer(puertos.contexto, 0x3cf);
  puertos.escribir(puertos.contexto, 0x3cf, static_cast<unsigned char>((control & 0xfc) | 1), 1);
  for (unsigned int y = 0; y < alto; ++y) {
    for (unsigned int x = 0; x < grupos; ++x) {
      const auto si = origen + y * paso_origen + x;
      const auto di = destino + y * paso_destino + x;
      // Una lectura VGA carga simultáneamente los cuatro cerrojos antes de la escritura.
      unsigned char valores[4];
      for (unsigned int plano = 0; plano < 4; ++plano) valores[plano] = planos[plano * 65536u + si];
      for (unsigned int plano = 0; plano < 4; ++plano) planos[plano * 65536u + di] = valores[plano];
    }
  }
  puertos.escribir(puertos.contexto, 0x3ce, 5, 1);
  control = puertos.leer(puertos.contexto, 0x3cf);
  puertos.escribir(puertos.contexto, 0x3cf, static_cast<unsigned char>(control & 0xfc), 1);
  return true;
}

bool enviar_paleta(const unsigned char* datos, const PuertosVideo& puertos,
    unsigned short posicion, unsigned char primer_color, unsigned short cantidad) {
  if (!puertos.escribir || cantidad == 0 || cantidad > 256) return false;
  puertos.escribir(puertos.contexto, 0x3c8, primer_color, 1);
  for (unsigned int i = 0; i < cantidad * 3u; ++i) {
    puertos.escribir(puertos.contexto, 0x3c9, datos[static_cast<unsigned short>(posicion + i)], 1);
  }
  return true;
}

}
