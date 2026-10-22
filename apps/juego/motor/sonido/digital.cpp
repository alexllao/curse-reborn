#include "digital.hpp"
#include "../estado/segmento.hpp"

namespace motor {
// 7FC1–8016: conserva incluso la cuenta FFFF de una petición de longitud cero del original.
BloqueDma preparar_dma(unsigned short segmento, unsigned short posicion, unsigned int longitud) {
  const auto lineal = static_cast<unsigned int>(segmento) * 16 + posicion;
  const auto direccion = static_cast<unsigned short>(lineal);
  auto cantidad =
      longitud > 65535u || direccion + longitud > 65535u ? 65536u - direccion : longitud;
  if (!cantidad)
    cantidad = 65536;
  return {direccion, static_cast<unsigned short>(cantidad - 1),
          static_cast<unsigned char>(lineal >> 16), longitud - cantidad};
}
// El tercer campo de cada registro DS:784E es el SFX; cero significa que no hay banco de escena.
bool nombre_banco_sonido(const unsigned char *datos, unsigned short escena, char *nombre) {
  if (!datos || !nombre || (escena != 0xffff && escena >= 78))
    return false;
  const auto referencia =
      escena == 0xffff ? 0x0e11 : palabra(datos, static_cast<Palabra>(0x784e + escena * 8 + 4));
  nombre[0] = 0;
  if (!referencia)
    return true;
  for (unsigned int i = 0; i < 13; ++i) {
    if (referencia + i >= 65536)
      return false;
    const auto c = datos[referencia + i];
    if (!c) {
      nombre[i] = 0;
      return i != 0;
    }
    if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' ||
          c == '.'))
      return false;
    nombre[i] = static_cast<char>(c);
  }
  return false;
}
// Los bancos siguen siendo propiedad del adaptador; iniciar copia solo la muestra que va a sonar.
void DigitalPcm::conectar(const unsigned char *general, unsigned int longitud_general,
                          const unsigned char *escena, unsigned int longitud_escena) {
  detener();
  bancos[0] = general;
  bancos[1] = escena;
  longitudes[0] = longitud_general;
  longitudes[1] = longitud_escena;
  estado = {};
  estado.recurso_general = general ? 1 : 0;
  estado.recurso_escena = escena ? 2 : 0;
}
// Cada acceso de 5288 debe caber completo; los bancos no admiten envoltura del segmento DOS.
bool DigitalPcm::leer(void *contexto, unsigned short recurso, unsigned short posicion,
                      unsigned short &valor) {
  auto &audio = *static_cast<DigitalPcm *>(contexto);
  if (recurso < 1 || recurso > 2 || !audio.bancos[recurso - 1] ||
      static_cast<unsigned int>(posicion) + 2 > audio.longitudes[recurso - 1])
    return false;
  valor = palabra(audio.bancos[recurso - 1], posicion);
  return true;
}
// Las órdenes omitidas no detienen una voz en curso ni crean colas ajenas al original.
bool DigitalPcm::reproducir(unsigned char *datos, unsigned short entidad,
                            unsigned char identificador, bool *iniciada) {
  if (iniciada)
    *iniciada = false;
  const auto orden = seleccionar_sonido(datos, estado, {this, leer}, entidad, identificador);
  const bool correcto = orden.valida && (orden.rutina == 0 || iniciar(orden));
  if (iniciada)
    *iniciada = correcto && orden.rutina != 0;
  return correcto;
}
// 7FB3: D0 interrumpe la muestra previa; 40 fija el reloj y 14 inicia PCM de ocho bits sin signo.
bool DigitalPcm::iniciar(const OrdenSonido &orden) {
  if (!orden.valida || orden.rutina != 0x7fb3 || orden.referencia < 1 || orden.referencia > 2)
    return false;
  const auto indice = orden.referencia - 1;
  if (!bancos[indice] || !orden.longitud ||
      static_cast<unsigned int>(orden.posicion) + orden.longitud > longitudes[indice])
    return false;
  longitud_muestra = orden.longitud;
  for (unsigned int i = 0; i < longitud_muestra; ++i)
    muestra[i] = bancos[indice][orden.posicion + i];
  reloj = 0;
  periodo = 48000u * (256u - orden.canal);
  estado.muestra_activa = 1;
  return true;
}
// La posición se calcula desde tiempo absoluto para que el tamaño de los bloques no cambie la
// señal.
void DigitalPcm::generar(short *salida, unsigned int cantidad) {
  for (unsigned int i = 0; i < cantidad; ++i) {
    salida[i] = 0;
    if (!activa())
      continue;
    const auto posicion = reloj / periodo;
    salida[i] = static_cast<short>(
        (static_cast<int>(muestra[static_cast<unsigned int>(posicion)]) - 128) * 256);
    reloj += 1000000;
    if (reloj >= static_cast<unsigned long long>(longitud_muestra) * periodo)
      estado.muestra_activa = 0;
  }
}
// 82E5: la última interrupción libera la prioridad al finalizar la muestra.
void DigitalPcm::detener() {
  estado.muestra_activa = 0;
  longitud_muestra = 0;
  reloj = 0;
}
} // namespace motor
