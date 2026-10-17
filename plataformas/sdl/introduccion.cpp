#include "introduccion.hpp"
#include <SDL3/SDL.h>
#include "../../motor/video/fundido.hpp"

namespace {
// Lee un recurso local y libera siempre el búfer de SDL, incluso ante una longitud inválida.
bool leer_recurso(const std::string &ruta, std::vector<unsigned char> &destino) {
  size_t longitud = 0;
  void *datos = SDL_LoadFile(ruta.c_str(), &longitud);
  if (!datos)
    return false;
  const bool valido = longitud > 0 && longitud <= 65536;
  if (valido) {
    const auto *bytes = static_cast<const unsigned char *>(datos);
    destino.assign(bytes, bytes + longitud);
  }
  SDL_free(datos);
  return valido;
}
} // namespace

// 28B2: reserva páginas y precarga los recursos para que las lecturas no bloqueen la reproducción.
bool Introduccion::iniciar(const std::string &carpeta, motor::ImagenIndexada &imagen) {
  for (unsigned short i = 0; i < 16; ++i)
    if (!leer_recurso(carpeta + "/" + motor::recurso_introduccion(false, i), recursos_[i]))
      return false;
  std::vector<unsigned char> colores;
  for (unsigned short p = 0; p < 3; ++p) {
    if (!leer_recurso(carpeta + "/" + motor::recurso_introduccion(true, p), colores) ||
        colores.size() != 768)
      return false;
    for (unsigned int i = 0; i < 768; ++i) {
      if (colores[i] > 63)
        return false;
      paletas_[p][i] = colores[i];
    }
  }
  for (unsigned int i = 0; i < 768; ++i)
    paleta_[i] = imagen.paleta[i];
  for (auto &valor : planos_)
    valor = 0;
  indice_ = 0;
  pagina_ = 0;
  hasta_ = inicio_fundido_ = 0;
  fundiendo_ = omitir_ = salida_anticipada_ = terminada_ = false;
  return avanzar(0, false, imagen);
}

// Sustituye únicamente las esperas de retrazo, los archivos y el CRTC por servicios de plataforma.
bool Introduccion::avanzar(unsigned long long retrazos, bool entrada,
                           motor::ImagenIndexada &imagen) {
  using O = motor::OperacionIntroduccion;
  omitir_ = omitir_ || entrada;
  while (!terminada_) {
    if (fundiendo_) {
      const auto transcurridos = retrazos - inicio_fundido_;
      const auto paso = static_cast<unsigned short>(transcurridos < 32 ? transcurridos : 32);
      if (!motor::calcular_fundido(paleta_, 256, 32, paso, imagen.paleta))
        return false;
      if (transcurridos < 32)
        return true;
      fundiendo_ = false;
      if (salida_anticipada_) {
        terminada_ = true;
        return true;
      }
    }
    if (retrazos < hasta_)
      return true;
    motor::EventoIntroduccion evento{};
    if (!motor::obtener_evento_introduccion(indice_++, evento))
      return false;
    switch (evento.operacion) {
    case O::entrada:
      if (omitir_) {
        salida_anticipada_ = fundiendo_ = true;
        inicio_fundido_ = hasta_;
      }
      break;
    case O::fundido:
      fundiendo_ = true;
      inicio_fundido_ = hasta_;
      hasta_ += 32;
      break;
    case O::cargar: {
      const auto &recurso = recursos_[evento.valor];
      if (!motor::cargar_pagina_introduccion(recurso.data(),
                                             static_cast<unsigned int>(recurso.size()),
                                             evento.pagina, planos_, temporal_))
        return false;
      // Una carga puede sobrescribir la página visible, tal como ocurre en las últimas escenas.
      if (!motor::mostrar_pagina_introduccion(planos_, pagina_, imagen))
        return false;
      break;
    }
    case O::seleccionar:
      pagina_ = evento.valor;
      if (!motor::mostrar_pagina_introduccion(planos_, pagina_, imagen))
        return false;
      break;
    case O::paleta:
      for (unsigned int i = 0; i < 768; ++i)
        imagen.paleta[i] = paleta_[i] = paletas_[evento.valor][i];
      break;
    case O::esperar:
      hasta_ += evento.valor;
      break;
    case O::limpiar:
      for (auto &valor : planos_)
        valor = 0;
      for (auto &valor : imagen.indices)
        valor = 0;
      break;
    case O::terminar:
      terminada_ = true;
      break;
    }
  }
  return true;
}
