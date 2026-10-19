#include "actualizar.hpp"
#include "../estado/segmento.hpp"

namespace motor {
// Traslada la lista de 8AA6: reserva los registros iniciales y distingue actores de
// superposiciones.
bool cargar_entidades_escena(unsigned char *datos, EstadoAleatorio &estado,
                             const RecursosEntidades &recursos) {
  const auto escena = palabra(datos, 0x8082);
  if (escena >= 78)
    return false;
  for (unsigned int i = 0; i < 68; ++i)
    datos[0x2c4c + i * 33] = 0;
  datos[0xadce] = datos[0xadcf] = 1;
  guardar(datos, 0x3588, 2);
  guardar(datos, 0x358a, 0);
  guardar(datos, 0x99b4, 0x2c4c);
  guardar(datos, 0x99b6, 0x3510);
  guardar(datos, 0x99b8, static_cast<Palabra>(datos[0x3597] << 5));
  auto descriptor = palabra(datos, static_cast<Palabra>(0x53b5 + escena * 2));
  for (unsigned int iteracion = 0; iteracion < 4096; ++iteracion) {
    const auto banderas = datos[descriptor];
    if (banderas == 0xff) {
      guardar(datos, 0x99b8, 0);
      return true;
    }
    if (banderas == 0) {
      const auto cantidad = palabra(datos, 0x358a);
      if (cantidad >= 10 || !recursos.parrafo || !recursos.dimensiones)
        return false;
      const auto indice = datos[static_cast<Palabra>(descriptor + 2)];
      Palabra ancho = 0, alto = 0, parrafo = 0;
      if (!recursos.dimensiones(recursos.contexto, indice, ancho, alto) ||
          !recursos.parrafo(recursos.contexto, indice, parrafo))
        return false;
      const auto destino = palabra(datos, 0x99b6);
      guardar(datos, 0x358a, static_cast<Palabra>(cantidad + 1));
      guardar(datos, 0x99b6, static_cast<Palabra>(destino + 12));
      guardar(datos, destino,
              static_cast<Palabra>(palabra(datos, static_cast<Palabra>(descriptor + 3)) -
                                   palabra(datos, 0x99b8)));
      guardar(datos, static_cast<Palabra>(destino + 2),
              palabra(datos, static_cast<Palabra>(descriptor + 5)));
      guardar(datos, static_cast<Palabra>(destino + 4), ancho);
      guardar(datos, static_cast<Palabra>(destino + 6), alto);
      guardar(datos, static_cast<Palabra>(destino + 8),
              static_cast<Palabra>(recursos.segmento_original + parrafo));
      guardar(datos, static_cast<Palabra>(destino + 10), parrafo);
    } else if (datos[0x0ad6] != 0 || (banderas & 0x80) == 0) {
      const auto reservadas = palabra(datos, 0x3588);
      if (reservadas >= 70)
        return false;
      // 8B6F y la carga fija comparten 8BA0; la reserva aún apunta al siguiente registro libre.
      const auto resultado = crear_entidad(datos, estado, recursos, descriptor);
      if (!resultado.valido || !resultado.creada || resultado.posicion != palabra(datos, 0x99b4))
        return false;
      guardar(datos, 0x3588, static_cast<Palabra>(reservadas + 1));
      guardar(datos, 0x99b4, static_cast<Palabra>(resultado.posicion + 33));
    }
    descriptor = static_cast<Palabra>(descriptor + 9);
  }
  return false;
}
} // namespace motor
