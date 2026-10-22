#include "pcm.hpp"

namespace motor {

// El cristal de 14.318.180 Hz dividido entre doce determina el reloj del PIT del PC.
void AltavozPcm::reiniciar() {
  seleccion = {};
  divisor = 65536;
  fase = fraccion = pendiente = control = 0;
  hasta_interrupcion = 0x4250;
  segundo_byte = programado = false;
}

// No se añade ningún efecto: una orden sin secuencia permanece silenciosa.
bool AltavozPcm::reproducir(unsigned char *datos, unsigned short entidad,
                            unsigned char identificador, bool *iniciada) {
  if (iniciada)
    *iniciada = false;
  if (!datos)
    return false;
  const auto orden =
      seleccionar_sonido(datos, seleccion, {nullptr, nullptr}, entidad, identificador);
  const bool correcto = atender_orden_altavoz(datos, orden);
  if (iniciada)
    *iniciada = correcto && orden.rutina != 0;
  return correcto;
}

// El original solo programa modo tres, con byte bajo seguido de byte alto.
void AltavozPcm::escribir(unsigned short puerto, unsigned char valor) {
  if (puerto == 0x43 && valor == 0xb6) {
    segundo_byte = false;
    programado = true;
  } else if (puerto == 0x42 && programado) {
    if (!segundo_byte) {
      pendiente = valor;
      segundo_byte = true;
    } else {
      pendiente = static_cast<unsigned short>(pendiente | (valor << 8));
      divisor = pendiente ? pendiente : 65536;
      fase = 0;
      segundo_byte = false;
    }
  } else if (puerto == 0x61) {
    if ((valor & 1) && !(control & 1))
      fase = 0;
    control = valor;
  }
}

// Los demás puertos no se leen desde 7A61 ni 7AC6.
unsigned char AltavozPcm::leer(unsigned short puerto) const {
  return puerto == 0x61 ? control : 0;
}

// Convierte las entradas de la rutina original a operaciones sobre este sintetizador.
unsigned char AltavozPcm::leer_puerto(void *contexto, unsigned short puerto) {
  return static_cast<AltavozPcm *>(contexto)->leer(puerto);
}

// Conserva el orden bajo/alto y la compuerta, incluida la pausa de una nota en curso.
void AltavozPcm::escribir_puerto(void *contexto, unsigned short puerto, unsigned char valor) {
  static_cast<AltavozPcm *>(contexto)->escribir(puerto, valor);
}

// Reutiliza 7A61 en lugar de borrar sus campos con un estado inventado.
bool AltavozPcm::detener(unsigned char *datos) {
  return datos && detener_altavoz(datos, {this, leer_puerto, escribir_puerto});
}

// Integra la señal lógica por muestra; mantiene el resto racional entre llamadas y bloques.
bool AltavozPcm::generar(unsigned char *datos, short *salida, unsigned int cantidad) {
  if (!datos || (!salida && cantidad))
    return false;
  const PuertosAltavoz puertos{this, leer_puerto, escribir_puerto};
  for (unsigned int i = 0; i < cantidad; ++i) {
    fraccion += 14318180;
    const auto ciclos = fraccion / (12u * frecuencia);
    fraccion %= 12u * frecuencia;
    int suma = 0;
    for (unsigned int ciclo = 0; ciclo < ciclos; ++ciclo) {
      if (programado && (control & 3) == 3)
        suma += fase < (divisor + 1) / 2 ? 1 : -1;
      if (control & 1)
        fase = (fase + 1) % divisor;
      if (--hasta_interrupcion == 0) {
        hasta_interrupcion = 0x4250;
        if (datos[0x0ad4] == 1 && !avanzar_altavoz(datos, puertos))
          return false;
      }
    }
    salida[i] = static_cast<short>(suma * 4096 / static_cast<int>(ciclos));
  }
  return true;
}

} // namespace motor
