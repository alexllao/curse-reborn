#pragma once

namespace motor {
// Lee únicamente el segmento de datos de la versión MZ estudiada; no interpreta instrucciones.
// La herramienta de lanzamiento debe comprobar antes la huella del EXE. Destino: 65.536 bytes.
bool leer_datos_iniciales(const unsigned char *ejecutable, unsigned int longitud,
                          unsigned char *datos);

// 4C43–4C8A y 5145–517D: prepara el personaje y controles del primer acceso a la mazmorra.
// Los recursos se identifican con referencias de datos; no son segmentos accesibles del anfitrión.
void preparar_entrada_mazmorra(unsigned char *datos, unsigned short modelo, unsigned short ancho,
                               unsigned short alto);
} // namespace motor
