#pragma once
namespace motor {
// DEFB–DFCF y DEBE–DEEB: dibuja los diez iconos y sus máscaras sobre MENU.DAT ya descomprimido.
// Destino 320×200; las 32 filas superiores contienen el panel, sin el marco animado de selección.
bool dibujar_menu(const unsigned char *datos, const unsigned char *fondo,
                  const unsigned char *iconos, unsigned int longitud_iconos,
                  unsigned char *destino);
// DB99–DBF5: copia el panel arriba/abajo y dibuja el marco del icono señalado con color 255.
void componer_menu(const unsigned char *datos, const unsigned char *panel, unsigned short casilla,
                   unsigned char *destino);
} // namespace motor
