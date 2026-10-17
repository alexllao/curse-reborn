#pragma once

namespace motor {
// 8BC5–8BD7: consulta la referencia en párrafos y el tamaño de un fotograma sin usar segmentos.
bool consultar_fotograma(const unsigned char *banco, unsigned int longitud, unsigned short indice,
                         unsigned short &parrafo, unsigned short &ancho, unsigned short &alto);
} // namespace motor
