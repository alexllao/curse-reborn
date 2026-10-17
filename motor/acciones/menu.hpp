#pragma once
namespace motor {
enum class ResultadoMenu { actualizado, accion, pendiente, error };
// DE96–DEF4: diez acciones iniciales y bloqueo de recoger/temas mientras Brad tiene una secuencia.
void preparar_menu_principal(unsigned char *datos);
// DB02–DB4B: inicializa los campos del menú y selecciona su posición superior o inferior.
void abrir_menu(unsigned char *datos, unsigned short x, unsigned short y);
// DE10 y DFD3–E15F: despacha una casilla; los manejadores aún sin traducir devuelven pendiente.
ResultadoMenu seleccionar_menu(unsigned char *datos, unsigned short casilla);
// 32DE–337A, 567E–56C8 y 989E–98FF: primeras acciones verificadas de la mazmorra.
ResultadoMenu ejecutar_interaccion_inicial(unsigned char *datos, unsigned short manejador,
                                           unsigned short identificador);
// 096D–0A58: recorre nodos de acciones y activa sus canales al alcanzar el destino.
bool avanzar_accion_pendiente(unsigned char *datos);
} // namespace motor
