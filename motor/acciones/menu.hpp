#pragma once
#include "../sonido/servicio.hpp"
namespace motor {
enum class ResultadoMenu { actualizado, accion, pendiente, error };
// DE96–DEF4: diez acciones iniciales y bloqueo de recoger/temas mientras Brad tiene una secuencia.
void preparar_menu_principal(unsigned char *datos);
// DB02–DB4B: inicializa los campos del menú y selecciona su posición superior o inferior.
void abrir_menu(unsigned char *datos, unsigned short x, unsigned short y);
// DE10 y DFD3–E15F: despacha una casilla; los manejadores aún sin traducir devuelven pendiente.
ResultadoMenu seleccionar_menu(unsigned char *datos, unsigned short casilla,
                               const ServicioSonido &sonido = {});
// 32DE–337A, 563A–56DB, 615C–61A6 y 989E–9993: acciones y apertura de la mazmorra.
ResultadoMenu ejecutar_interaccion_inicial(unsigned char *datos, unsigned short manejador,
                                           unsigned short identificador,
                                           const ServicioSonido &sonido = {});
// 096D–0A58: recorre nodos de acciones y activa sus canales al alcanzar el destino.
bool avanzar_accion_pendiente(unsigned char *datos);
} // namespace motor
