#include "rnc.hpp"

namespace motor {
namespace {
struct Flujo {
  const unsigned char *datos;
  unsigned int longitud;
  unsigned int posicion = 18;
  unsigned int reserva = 0, disponibles = 0;
  bool valido = true;

  // Lee un literal o un byte de bits del mismo flujo, rechazando la entrada truncada.
  unsigned int byte() {
    if (posicion >= longitud) {
      valido = false;
      return 0;
    }
    return datos[posicion++];
  }

  // 6862–6868 y recargas: sustituye el centinela de AL por un contador explícito.
  unsigned int bit() {
    if (disponibles == 0) {
      reserva = byte();
      disponibles = 8;
    }
    return (reserva >> --disponibles) & 1u;
  }
};

// Interpreta los tamaños de la cabecera RNC en orden de mayor a menor peso.
unsigned int grande(const unsigned char *datos) {
  return (static_cast<unsigned int>(datos[0]) << 24) | (static_cast<unsigned int>(datos[1]) << 16) |
         (static_cast<unsigned int>(datos[2]) << 8) | datos[3];
}

// 68BF–68E5 y 6925–6934: reconstruye la distancia, incluido el desplazamiento de uno.
unsigned int distancia(Flujo &flujo) {
  unsigned int alto = 0;
  if (flujo.bit()) {
    alto = flujo.bit();
    if (flujo.bit()) {
      alto = ((alto << 1) | flujo.bit()) | 4u;
      if (!flujo.bit())
        alto = (alto << 1) | flujo.bit();
    } else if (alto == 0) {
      alto = 2u | flujo.bit();
    }
  }
  return ((alto << 8) | flujo.byte()) + 1u;
}
} // namespace

// Traslada 685D–6989 con comprobaciones de entrada, capacidad y referencias anteriores.
// 6802–685C solo reubica la entrada solapada en DOS; esta API exige búferes separados.
ResultadoRnc descomprimir_rnc2(const unsigned char *origen, unsigned int longitud,
                               unsigned char *destino, unsigned int capacidad) {
  if (longitud < 4 || origen[0] != 'R' || origen[1] != 'N' || origen[2] != 'C')
    return {EstadoRnc::sin_compresion, 0};
  if (origen[3] != 2)
    return {EstadoRnc::metodo_no_admitido, 0};
  if (longitud < 18)
    return {EstadoRnc::entrada_invalida, 0};
  const auto esperados = grande(origen + 4), empaquetados = grande(origen + 8);
  if (empaquetados == 0 || empaquetados > longitud - 18 || esperados > capacidad)
    return {EstadoRnc::entrada_invalida, 0};
  Flujo flujo{origen, empaquetados + 18};
  flujo.bit();
  flujo.bit();
  unsigned int escritos = 0;
  while (flujo.valido) {
    unsigned int cantidad = 1, desplazamiento = 0;
    bool literal = true;
    if (flujo.bit()) {
      literal = false;
      cantidad = 2;
      if (!flujo.bit()) {
        // 68A7–68BD: tamaños 4–8 o escape de literales agrupados.
        cantidad = 4u | flujo.bit();
        if (flujo.bit()) {
          cantidad = ((cantidad - 1u) << 1) | flujo.bit();
          if (cantidad == 9) {
            unsigned int grupos = 0;
            for (unsigned int i = 0; i < 4; ++i)
              grupos = (grupos << 1) | flujo.bit();
            cantidad = (grupos + 3u) * 4u;
            literal = true;
          }
        }
        if (!literal)
          desplazamiento = distancia(flujo);
      } else if (!flujo.bit()) {
        // 6910: coincidencia de dos bytes con distancia limitada a un byte.
        desplazamiento = flujo.byte() + 1u;
      } else {
        cantidad = 3;
        if (flujo.bit()) {
          // 6919–6923 y 694F–697D: longitud larga o fin/continuación de bloque.
          cantidad = flujo.byte();
          if (cantidad == 0) {
            const bool continuar = flujo.bit() != 0;
            if (!flujo.valido)
              break;
            if (continuar)
              continue;
            return {escritos == esperados ? EstadoRnc::correcto : EstadoRnc::entrada_invalida,
                    escritos};
          }
          cantidad += 8;
        }
        desplazamiento = distancia(flujo);
      }
    }
    if (!flujo.valido || cantidad > esperados - escritos || (!literal && desplazamiento > escritos))
      break;
    if (literal) {
      if (cantidad > flujo.longitud - flujo.posicion)
        break;
      for (unsigned int i = 0; i < cantidad; ++i)
        destino[escritos++] = static_cast<unsigned char>(flujo.byte());
    } else {
      // 68E8: copia hacia delante; una referencia puede solaparse consigo misma.
      for (unsigned int i = 0; i < cantidad; ++i) {
        destino[escritos] = destino[escritos - desplazamiento];
        ++escritos;
      }
    }
  }
  return {EstadoRnc::entrada_invalida, escritos};
}
} // namespace motor
