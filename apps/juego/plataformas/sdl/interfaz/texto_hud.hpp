#pragma once
#include <SDL3/SDL.h>
#include <string>

// La fuente de depuración solo tiene ASCII: completar las tildes de títulos y avisos.
inline bool dibujar_texto_hud(SDL_Renderer *r, float x, float y, const std::string &texto) {
  for (size_t i = 0; i < texto.size();) {
    unsigned int codigo = static_cast<unsigned char>(texto[i++]);
    if (codigo >= 0xc0 && codigo < 0xe0 && i < texto.size()) {
      codigo = ((codigo & 31u) << 6) | (static_cast<unsigned char>(texto[i++]) & 63u);
    } else if (codigo >= 0xe0 && codigo < 0xf0 && i + 1 < texto.size()) {
      const auto segundo = static_cast<unsigned char>(texto[i++]);
      const auto tercero = static_cast<unsigned char>(texto[i++]);
      codigo = ((codigo & 15u) << 12) | ((segundo & 63u) << 6) | (tercero & 63u);
    }
    char letra = codigo < 128 ? static_cast<char>(codigo) : '?';
    unsigned int acento = 0;
    switch (codigo) {
    case 0xc0:
      letra = 'A';
      acento = 2;
      break;
    case 0xc1:
      letra = 'A';
      acento = 1;
      break;
    case 0xc9:
      letra = 'E';
      acento = 1;
      break;
    case 0xe1:
      letra = 'a';
      acento = 1;
      break;
    case 0xe8:
      letra = 'e';
      acento = 2;
      break;
    case 0xe9:
      letra = 'e';
      acento = 1;
      break;
    case 0xea:
      letra = 'e';
      acento = 3;
      break;
    case 0xed:
      letra = 'i';
      acento = 1;
      break;
    case 0xf3:
      letra = 'o';
      acento = 1;
      break;
    case 0xfa:
      letra = 'u';
      acento = 1;
      break;
    case 0xf1:
      letra = 'n';
      acento = 4;
      break;
    case 0xb7:
      letra = '.';
      break;
    case 0x2019:
      letra = '\'';
      break;
    }
    const char caracter[] = {letra, 0};
    if (!SDL_RenderDebugText(r, x, y, caracter))
      return false;
    if (acento == 1 && !SDL_RenderLine(r, x + 3, y - 1, x + 5, y - 3))
      return false;
    if (acento == 2 && !SDL_RenderLine(r, x + 3, y - 3, x + 5, y - 1))
      return false;
    if (acento == 3 && (!SDL_RenderLine(r, x + 2, y - 1, x + 4, y - 3) ||
                        !SDL_RenderLine(r, x + 4, y - 3, x + 6, y - 1)))
      return false;
    if (acento == 4 && !SDL_RenderLine(r, x + 2, y - 2, x + 6, y - 2))
      return false;
    x += SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE;
  }
  return true;
}
