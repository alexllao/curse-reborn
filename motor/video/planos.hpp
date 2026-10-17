#pragma once

namespace motor {

struct PuertosVideo {
  void* contexto;
  unsigned char (*leer)(void*, unsigned short puerto);
  void (*escribir)(void*, unsigned short puerto, unsigned short valor, unsigned char tamano);
};

// Cuatro planos contiguos de 65.536 bytes. Los pasos y posiciones se expresan en bytes por plano.
// 7636–7674: imagen lineal a memoria por planos.
bool subir_imagen(const unsigned char* origen, unsigned char* planos, const PuertosVideo& puertos,
    unsigned short ancho, unsigned short alto, unsigned short paso, unsigned short destino);
// 7679–76B7: memoria por planos a imagen lineal.
bool recuperar_imagen(const unsigned char* planos, unsigned char* destino, const PuertosVideo& puertos,
    unsigned short ancho, unsigned short alto, unsigned short paso, unsigned short origen);
// 76BC–770D: copia con los cuatro cerrojos VGA en modo de escritura uno.
bool copiar_planos(unsigned char* planos, const PuertosVideo& puertos, unsigned short ancho,
    unsigned short alto, unsigned short paso_origen, unsigned short paso_destino,
    unsigned short origen, unsigned short destino);
// 787E–788D: envía RGB consecutivo; no altera la precisión de los componentes originales.
bool enviar_paleta(const unsigned char* datos, const PuertosVideo& puertos,
    unsigned short posicion, unsigned char primer_color, unsigned short cantidad);

}
