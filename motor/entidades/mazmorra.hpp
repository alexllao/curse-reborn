#pragma once

namespace motor {
struct EstadoAleatorio;
struct RecursosEntidades;
// C093–C4DB: obstáculos, rescate y creaciones del borde del mundo.
bool actualizar_borde(unsigned char *datos, EstadoAleatorio &azar,
                      const RecursosEntidades &recursos);
// C5E0–C793: proximidad de CELESTE33 y reacción de REMOTE34 sin consumir azar.
bool actualizar_ramas_valle(unsigned char *datos);
// C794–C8B0: encuentro del poste y partículas; conserva inventario y azar originales.
bool actualizar_poste(unsigned char *datos, EstadoAleatorio &azar,
                      const RecursosEntidades &recursos);
// BEFD–C092: añade el fondo del lago, sus burbujas y entidades, con el generador original.
bool actualizar_rutina_escena(unsigned char *datos, EstadoAleatorio &azar,
                              const RecursosEntidades &recursos);
// 183F–1904: reacción particular de la escena cinco, después de actualizar las entidades.
void actualizar_escena_cinco(unsigned char *datos);
// 1791–17A2: despacha solo las escenas con rutina integrada; la escena cero no tiene manejador.
bool actualizar_rutina_escena(unsigned char *datos);
} // namespace motor
