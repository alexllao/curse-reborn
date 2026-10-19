#pragma once

#include "imagen.hpp"

namespace motor {
enum class OperacionIntroduccion : unsigned short {
  entrada,
  fundido,
  cargar,
  seleccionar,
  paleta,
  esperar,
  limpiar,
  terminar
};
struct EventoIntroduccion {
  OperacionIntroduccion operacion;
  unsigned short valor;
  unsigned short pagina;
};

// 28B2–2F4F: devuelve el evento solicitado del recorrido original sin entrada del usuario.
bool obtener_evento_introduccion(unsigned int indice, EventoIntroduccion &evento);
// Nombres originales de las dieciséis imágenes y tres paletas; un índice ajeno devuelve nulo.
const char *recurso_introduccion(bool paleta, unsigned short indice);
// 2F50–2F88: RNC2 y subida con paso de 80 bytes por plano a una de las cuatro páginas VGA.
bool cargar_pagina_introduccion(const unsigned char *datos, unsigned int longitud,
                                unsigned short pagina, unsigned char *planos,
                                unsigned char *temporal);
// Recupera la página seleccionada por el CRTC, conservando la paleta administrada por el llamador.
bool mostrar_pagina_introduccion(const unsigned char *planos, unsigned short pagina,
                                 ImagenIndexada &imagen);
} // namespace motor
