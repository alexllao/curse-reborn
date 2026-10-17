#pragma once

namespace motor {

// Cuatro palabras almacenadas originalmente en CS:78DE, 78E0, 78E2 y 78E4.
struct EstadoAleatorio {
  unsigned short primera;
  unsigned short segunda;
  unsigned short tercera;
  unsigned short cuarta;
};

// Traslado de 0000:788E–78DD. Devuelve un valor entre cero y limite-1.
// Un límite cero se rechaza sin modificar estado ni resultado; en DOS provoca DIV/0.
bool generar_aleatorio(EstadoAleatorio& estado, unsigned short limite,
                      unsigned short& resultado);

}
