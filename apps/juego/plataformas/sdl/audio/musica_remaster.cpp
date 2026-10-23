#include "musica_remaster.hpp"
#include "../interfaz/configuracion.hpp"
#include <algorithm>
#include <limits>
#define DR_MP3_IMPLEMENTATION
#define DR_MP3_NO_STDIO
#include "../../../terceros/dr_mp3/dr_mp3.h"

bool MusicaRemaster::iniciar(const std::string &catalogo) {
  preparada = false;
  const auto json = leer_json_recurso(catalogo);
  const auto carpeta = catalogo.substr(0, catalogo.find_last_of('/') + 1);
  for (unsigned int i = 0; i < pistas.size(); ++i) {
    auto &p = pistas[i];
    p.muestras.clear();
    const auto entrada = json["rs" + std::to_string(i + 1)];
    p.titulo = entrada["titulo"].string_value();
    const auto archivo = entrada["archivo"].string_value();
    if (p.titulo.empty() || !ruta_recurso_valida(archivo))
      return false;
    const auto ruta = std::string(SDL_GetBasePath()) + carpeta + archivo;
    SDL_AudioSpec origen{};
    Uint8 *pcm = nullptr;
    Uint32 bytes = 0;
    const bool mp3 = archivo.size() > 4 && archivo.substr(archivo.size() - 4) == ".mp3";
    if (mp3) {
      size_t longitud = 0;
      void *comprimido = SDL_LoadFile(ruta.c_str(), &longitud);
      drmp3_config formato{};
      drmp3_uint64 fotogramas = 0;
      auto *decodificado = comprimido ? drmp3_open_memory_and_read_pcm_frames_s16(
                                            comprimido, longitud, &formato, &fotogramas, nullptr)
                                      : nullptr;
      SDL_free(comprimido);
      const auto cantidad = fotogramas * formato.channels * sizeof(short);
      if (!decodificado || !cantidad || cantidad > 256u * 1024u * 1024u) {
        drmp3_free(decodificado, nullptr);
        return false;
      }
      origen = {SDL_AUDIO_S16, static_cast<int>(formato.channels),
                static_cast<int>(formato.sampleRate)};
      bytes = static_cast<Uint32>(cantidad);
      pcm = static_cast<Uint8 *>(SDL_malloc(bytes));
      if (pcm)
        SDL_memcpy(pcm, decodificado, bytes);
      drmp3_free(decodificado, nullptr);
    } else if (!SDL_LoadWAV(ruta.c_str(), &origen, &pcm, &bytes)) {
      SDL_Log("No se puede leer la canción %s: %s", ruta.c_str(), SDL_GetError());
      return false;
    }
    const SDL_AudioSpec destino{SDL_AUDIO_S16, 2, 48000};
    Uint8 *convertido = nullptr;
    int longitud = 0;
    const bool ok = pcm && bytes > 0 && bytes <= 256u * 1024u * 1024u &&
                    SDL_ConvertAudioSamples(&origen, pcm, static_cast<int>(bytes), &destino,
                                            &convertido, &longitud);
    SDL_free(pcm);
    if (!ok || longitud < 4 || longitud % 4) {
      SDL_free(convertido);
      return false;
    }
    auto *muestras = reinterpret_cast<short *>(convertido);
    p.muestras.assign(muestras, muestras + longitud / 2);
    SDL_free(convertido);
    // Repetición de la canción completa; 20 ms en cada extremo evitan clics eléctricos.
    // No se atribuyen a estas versiones los puntos de bucle de los originales.
    const size_t borde = std::min<size_t>(960, p.muestras.size() / 4);
    for (size_t f = 0; f < borde; ++f)
      for (size_t c = 0; c < 2; ++c) {
        p.muestras[f * 2 + c] = static_cast<short>(p.muestras[f * 2 + c] * static_cast<int>(f) /
                                                   static_cast<int>(borde));
        const auto final = p.muestras.size() - 2 * (f + 1) + c;
        p.muestras[final] =
            static_cast<short>(p.muestras[final] * static_cast<int>(f) / static_cast<int>(borde));
      }
  }
  banco = 4;
  cursor = 0;
  preparada = true;
  return true;
}

void MusicaRemaster::cambiar_banco(unsigned int siguiente, bool reiniciar) {
  if (siguiente < pistas.size() && (reiniciar || siguiente != banco)) {
    banco = siguiente;
    cursor = 0;
  }
}

void MusicaRemaster::generar(short *salida, unsigned int fotogramas) {
  if (!preparada) {
    std::fill(salida, salida + fotogramas * 2, 0);
    return;
  }
  const auto &muestras = pistas[banco].muestras;
  for (unsigned int i = 0; i < fotogramas * 2; ++i) {
    salida[i] = muestras[cursor++];
    if (cursor == muestras.size())
      cursor = 0;
  }
}
