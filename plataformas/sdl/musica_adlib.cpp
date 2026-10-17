#include "musica_adlib.hpp"
#include "../../motor/sonido/musica.hpp"
#include "../../motor/estado/segmento.hpp"

// El emulador solo recibe registros FM y nunca ve los datos ni el código del juego.
void MusicaAdlib::escribir(void *contexto, unsigned short registro, unsigned char valor) {
  auto &estado = *static_cast<MusicaAdlib *>(contexto);
  OPL3_WriteRegBuffered(&estado.chip, registro, valor);
}

// La copia incluye tablas y patrones originales; su uso se limita a lecturas y al estado musical.
bool MusicaAdlib::iniciar(const unsigned char *ejecutable, unsigned int longitud) {
  preparada = false;
  if (!ejecutable || longitud < 512 + 65536 || ejecutable[0] != 'M' || ejecutable[1] != 'Z' ||
      motor::palabra(ejecutable, 8) != 32)
    return false;
  for (unsigned int i = 0; i < 65536; ++i)
    codigo[i] = ejecutable[512 + i];
  OPL3_Reset(&chip, 48000);
  fraccion = 0;
  hasta_interrupcion = 0x4250;
  interrupciones = 0;
  preparada =
      motor::inicializar_chip_adlib(codigo, {this, escribir}) && motor::iniciar_musica(codigo, 1);
  return preparada;
}

// Las órdenes que cruzan una muestra se aplican al terminarla: precisión de una muestra de PCM.
bool MusicaAdlib::generar(short *salida, unsigned int cantidad) {
  if (!preparada || (!salida && cantidad))
    return false;
  for (unsigned int i = 0; i < cantidad; ++i) {
    short canales[2];
    OPL3_GenerateResampled(&chip, canales);
    salida[i] = static_cast<short>((static_cast<int>(canales[0]) + canales[1]) / 4);
    fraccion += 14318180;
    const auto ciclos = fraccion / 576000u;
    fraccion %= 576000u;
    if (ciclos >= hasta_interrupcion) {
      hasta_interrupcion += 0x4250;
      if (!motor::avanzar_musica(codigo, {this, escribir}))
        return false;
      ++interrupciones;
    }
    hasta_interrupcion -= ciclos;
  }
  return true;
}
