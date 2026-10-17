#pragma once

namespace motor {
// 4C43–4D40 y 5145: entradas 0–3; STD conserva 32x48 y PLUNGE usa dimensiones del banco cargado.
// El adaptador carga PLUNGE.BRD para la salida tres antes de invocar este bloque de estado.
bool preparar_transicion_escena(unsigned char *datos, unsigned short salida,
                                unsigned short referencia_estandar,
                                unsigned short referencia_modelo, unsigned short ancho,
                                unsigned short alto);
// 4B05–4B0E, 4C43–4C98, 4D2B–4D40 y 5145–5181: entradas 0/2 entre celda y pasillo.
// Ambas usan STD.BRD; la carga de bancos, fondo, geometría y entidades pertenece al llamador.
bool preparar_transicion_mazmorra(unsigned char *datos, unsigned short salida,
                                  unsigned short referencia_estandar);
// Lee únicamente el segmento de datos de la versión MZ estudiada; no interpreta instrucciones.
// La herramienta de lanzamiento debe comprobar antes la huella del EXE. Destino: 65.536 bytes.
bool leer_datos_iniciales(const unsigned char *ejecutable, unsigned int longitud,
                          unsigned char *datos);

// 4C43–4C8A y 5145–517D: prepara el personaje y controles del primer acceso a la mazmorra.
// Los recursos se identifican con referencias de datos; no son segmentos accesibles del anfitrión.
void preparar_entrada_mazmorra(unsigned char *datos, unsigned short modelo, unsigned short ancho,
                               unsigned short alto);
} // namespace motor
