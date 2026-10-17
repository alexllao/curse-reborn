#include "introduccion.hpp"
#include "planos.hpp"
#include "../recursos/rnc.hpp"

namespace motor {
namespace {
struct Recorrido {
  unsigned int buscado;
  unsigned int actual = 0;
  EventoIntroduccion &salida;
  bool encontrado = false;
  // Conserva únicamente el evento pedido; la enumeración no necesita reservas ni estado global.
  void emitir(OperacionIntroduccion operacion, unsigned short valor = 0,
              unsigned short pagina = 0) {
    if (actual++ == buscado) {
      salida = {operacion, valor, pagina};
      encontrado = true;
    }
  }
};
// Representa la selección de plano en las matrices de memoria, sin acceso a puertos reales.
void escribir_plano(void *, unsigned short, unsigned short, unsigned char) {}
} // namespace

// 28B2–2F4F: traduce las cargas, cambios CRTC, controles y esperas; no cuenta el tiempo de E/S DOS.
bool obtener_evento_introduccion(unsigned int indice, EventoIntroduccion &evento) {
  using O = OperacionIntroduccion;
  Recorrido r{indice, 0, evento, false};
  r.emitir(O::entrada);
  r.emitir(O::fundido, 32);
  for (unsigned short i = 0; i < 4; ++i)
    r.emitir(O::cargar, i, i);
  r.emitir(O::seleccionar, 0);
  r.emitir(O::esperar, 1);
  r.emitir(O::paleta, 0);
  // 2951–2A24: 39 selecciones, páginas 1/2/3/0 y ocho retrazos por selección.
  for (unsigned short i = 0; i < 39; ++i) {
    r.emitir(O::entrada);
    r.emitir(O::seleccionar, (i + 1) % 4);
    r.emitir(O::esperar, 8);
  }
  for (unsigned short i = 0; i < 3; ++i)
    r.emitir(O::cargar, i + 4, i);
  r.emitir(O::fundido, 32);
  r.emitir(O::seleccionar, 0);
  r.emitir(O::esperar, 1);
  r.emitir(O::paleta, 1);
  // 2A98–2B9E: 29 selecciones; el índice vuelve a cero al alcanzar la cuarta página.
  for (unsigned short i = 0; i < 29; ++i) {
    r.emitir(O::entrada);
    r.emitir(O::seleccionar, (i + 1) % 3);
    r.emitir(O::esperar, 10);
  }
  r.emitir(O::cargar, 7, 0);
  r.emitir(O::cargar, 8, 1);
  r.emitir(O::seleccionar, 0);
  r.emitir(O::esperar, 8);
  r.emitir(O::seleccionar, 1);
  r.emitir(O::entrada);
  r.emitir(O::cargar, 9, 2);
  r.emitir(O::fundido, 32);
  r.emitir(O::seleccionar, 2);
  r.emitir(O::paleta, 2);
  r.emitir(O::entrada);
  r.emitir(O::cargar, 10, 3);
  r.emitir(O::cargar, 11, 0);
  r.emitir(O::cargar, 12, 1);
  r.emitir(O::esperar, 70);
  r.emitir(O::seleccionar, 3);
  r.emitir(O::esperar, 9);
  r.emitir(O::seleccionar, 0);
  r.emitir(O::esperar, 1);
  r.emitir(O::entrada);
  r.emitir(O::esperar, 8);
  r.emitir(O::seleccionar, 1);
  r.emitir(O::esperar, 9);
  r.emitir(O::entrada);
  r.emitir(O::cargar, 13, 2);
  r.emitir(O::cargar, 14, 3);
  r.emitir(O::cargar, 15, 0);
  r.emitir(O::entrada);
  r.emitir(O::seleccionar, 2);
  r.emitir(O::esperar, 9);
  r.emitir(O::seleccionar, 1);
  r.emitir(O::esperar, 9);
  r.emitir(O::seleccionar, 3);
  r.emitir(O::esperar, 9);
  r.emitir(O::seleccionar, 0);
  r.emitir(O::esperar, 141);
  r.emitir(O::fundido, 32);
  r.emitir(O::limpiar);
  r.emitir(O::terminar);
  return r.encontrado;
}

// DS:1938–1A2C: conserva los nombres DOS; no se incluyen recursos comerciales en el código.
const char *recurso_introduccion(bool paleta, unsigned short indice) {
  static const char *const imagenes[] = {
      "CAULDRN1.DAT", "CAULDRN2.DAT", "CAULDRN3.DAT", "CAULDRN4.DAT",
      "BALCONY1.DAT", "BALCONY2.DAT", "BALCONY3.DAT", "BALCONY4.DAT",
      "BALCONY5.DAT", "BASEBAL1.DAT", "BASEBAL2.DAT", "BASEBAL3.DAT",
      "BASEBAL4.DAT", "BASEBAL5.DAT", "BASEBAL6.DAT", "BASEBAT.DAT"};
  static const char *const paletas[] = {"CAULDRN.PAL", "BALCONY.PAL", "BASEBALL.PAL"};
  return paleta ? (indice < 3 ? paletas[indice] : nullptr)
                : (indice < 16 ? imagenes[indice] : nullptr);
}

// 2F50–2F88 llama al método dos y a 7636 con CX=320: el paso por plano es 80, no 112.
bool cargar_pagina_introduccion(const unsigned char *datos, unsigned int longitud,
                                unsigned short pagina, unsigned char *planos,
                                unsigned char *temporal) {
  if (pagina > 3)
    return false;
  const auto resultado = descomprimir_rnc2(datos, longitud, temporal, 64000);
  return resultado.estado == EstadoRnc::correcto && resultado.escritos == 64000 &&
         subir_imagen(temporal, planos, {nullptr, nullptr, escribir_plano}, 320, 200, 80,
                      static_cast<unsigned short>(pagina * 0x4000u));
}

// Las selecciones CRTC de la introducción usan páginas separadas por 4000h bytes por plano.
bool mostrar_pagina_introduccion(const unsigned char *planos, unsigned short pagina,
                                 ImagenIndexada &imagen) {
  return pagina < 4 &&
         recuperar_imagen(planos, imagen.indices, {nullptr, nullptr, escribir_plano}, 320, 200, 80,
                          static_cast<unsigned short>(pagina * 0x4000u));
}
} // namespace motor
