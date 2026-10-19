#pragma once
namespace motor {
// E89D/D87A y E46D–E496: placa original de 128×100, ocho nombres y teclado de trece caracteres.
// modo1=cargar,2=guardar,3=borrar. Conserva fuera de la placa el destino de320×200.
bool componer_panel_partidas(const unsigned char *datos, const unsigned char *fuente,
                             unsigned int longitud_fuente, const unsigned char *archivo,
                             unsigned int longitud_archivo, unsigned int modo, bool editando,
                             const unsigned char *nombre, unsigned int letras,
                             unsigned char *destino);
} // namespace motor
