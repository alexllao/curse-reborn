#include "altavoz.hpp"
#include "../estado/segmento.hpp"

namespace motor {

void iniciar_altavoz(unsigned char* datos, unsigned short secuencia) {
  guardar(datos, 0x9920, secuencia);
  guardar(datos, 0x9922, 1);
  datos[0x9924] = 1;
}

bool atender_orden_altavoz(unsigned char* datos, const OrdenSonido& orden) {
  if (!orden.valida) return false;
  if (orden.rutina == 0) return true;
  if (orden.rutina != 0x7a46) return false;
  iniciar_altavoz(datos, orden.posicion);
  return true;
}

bool detener_altavoz(unsigned char* datos, const PuertosAltavoz& puertos) {
  if (!puertos.leer || !puertos.escribir) return false;
  guardar(datos, 0x9920, 0);
  const auto control = puertos.leer(puertos.contexto, 0x61);
  puertos.escribir(puertos.contexto, 0x61, static_cast<unsigned char>(control & 0xfc));
  return true;
}

bool avanzar_altavoz(unsigned char* datos, const PuertosAltavoz& puertos) {
  if (!puertos.leer || !puertos.escribir) return false;
  const auto posicion = palabra(datos, 0x9920);
  if (posicion == 0) return true;
  const auto contador = static_cast<Palabra>(palabra(datos, 0x9922) - 1);
  guardar(datos, 0x9922, contador);
  if (contador != 0) return true;
  const auto duracion = palabra(datos, posicion);
  if (duracion == 0) {
    datos[0x9924] = 0;
    guardar(datos, 0x9920, 0);
    const auto control = puertos.leer(puertos.contexto, 0x61);
    puertos.escribir(puertos.contexto, 0x61, static_cast<unsigned char>(control & 0xfc));
    return true;
  }
  guardar(datos, 0x9922, duracion);
  puertos.escribir(puertos.contexto, 0x43, 0xb6);
  puertos.escribir(puertos.contexto, 0x42, datos[static_cast<Palabra>(posicion + 2)]);
  puertos.escribir(puertos.contexto, 0x42, datos[static_cast<Palabra>(posicion + 3)]);
  guardar(datos, 0x9920, static_cast<Palabra>(posicion + 4));
  const auto control = puertos.leer(puertos.contexto, 0x61);
  puertos.escribir(puertos.contexto, 0x61, static_cast<unsigned char>(control | 3));
  return true;
}

}
