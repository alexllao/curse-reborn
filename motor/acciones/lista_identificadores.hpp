#pragma once

namespace motor {

// Lista de diez palabras en DS:5225. true equivale a acarreo despejado.
bool contiene_identificador(const unsigned char* datos, unsigned short identificador);
bool insertar_identificador(unsigned char* datos, unsigned short identificador);
bool retirar_identificador(unsigned char* datos, unsigned short identificador);

}
