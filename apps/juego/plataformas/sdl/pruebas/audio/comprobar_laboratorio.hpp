#pragma once
#include "../../audio/laboratorio.hpp"
#include <cmath>
#include <filesystem>

// Renderiza los bancos en pausa; el fin de voz se comprueba con el dispositivo dummy, sin perfiles.
inline bool comprobar_laboratorio_audio(const std::string &carpeta, const std::string &salida) {
  std::filesystem::create_directories(std::filesystem::u8path(salida));
  LaboratorioAudio lab;
  if (!lab.preparar(carpeta))
    return false;
  const auto perfiles = lab.audio->perfiles_disponibles();
  json11::Json::array temas, posiciones;
  constexpr unsigned int cantidad = 48000 * 2;
  std::vector<short> pcm(cantidad * 2), fm(cantidad), opl2(cantidad);
  for (unsigned int modo = 1; modo <= 4; ++modo) {
    if (!(perfiles & (1u << modo)))
      continue;
    lab.modo = modo;
    for (unsigned int pista = 0; pista < lab.cantidad_pistas(); ++pista) {
      lab.tema = pista;
      if (!lab.reproducir_musica(carpeta, perfiles, false) ||
          !SDL_AudioDevicePaused(lab.audio->dispositivo))
        return false;
      if (modo < 3) {
        if (!lab.audio->musica.generar(fm.data(), cantidad, opl2.data()))
          return false;
        for (unsigned int i = 0; i < cantidad * 2; ++i)
          pcm[i] = modo == 1 ? opl2[i / 2] : fm[i / 2];
      } else if (modo == 3) {
        if (lab.audio->mt32.banco() != pista || !lab.audio->mt32.generar(pcm.data(), cantidad))
          return false;
      } else {
        if (lab.audio->remaster.posicion() != 0)
          return false;
        lab.audio->remaster.generar(pcm.data(), cantidad);
        lab.audio->remaster.cambiar_banco(pista, true);
        if (lab.audio->remaster.posicion() != 0)
          return false;
      }
      unsigned int no_cero = 0, pico = 0;
      for (const auto v : pcm) {
        no_cero += v != 0;
        pico = std::max(pico, static_cast<unsigned int>(std::abs(int(v))));
      }
      if (!no_cero)
        return false;
      temas.emplace_back(json11::Json::object{{"version", lab.nombre_version()},
                                              {"pista", int(pista)},
                                              {"muestras_con_senal", int(no_cero)},
                                              {"pico", int(pico)}});
      lab.detener();
      if (lab.reproduciendo() || SDL_GetAudioStreamQueued(lab.audio->flujo_musical) != 0)
        return false;
    }
  }
  std::vector<short> mono(32000), estereo(mono.size() * 2), referencia;
  for (unsigned int voz = 0; voz < 2; ++voz) {
    lab.identificador = voz;
    for (unsigned int punto = 0; punto < 5; ++punto) {
      lab.posicional = true;
      if (!lab.reproducir_voz(carpeta, punto, perfiles, false))
        return false;
      lab.audio->digital.generar(mono.data(), static_cast<unsigned int>(mono.size()));
      lab.audio->espacial.procesar(lab.datos.data(), mono.data(), estereo.data(),
                                   static_cast<unsigned int>(mono.size()));
      if (punto == 0)
        referencia = mono;
      if (mono != referencia)
        return false; // Misma voz y duración en las cinco posiciones.
      double l = 0, r = 0;
      for (size_t i = 0; i < mono.size(); ++i) {
        l += double(estereo[i * 2]) * estereo[i * 2];
        r += double(estereo[i * 2 + 1]) * estereo[i * 2 + 1];
      }
      if (!l || !r || (punto < 2 && l <= r) || (punto > 2 && l >= r) || (punto == 2 && l != r))
        return false;
      posiciones.emplace_back(json11::Json::object{
          {"voz_STD", int(voz)}, {"posicion", int(punto)}, {"energia_L", l}, {"energia_R", r}});
      lab.posicional = false;
      if (!lab.reproducir_voz(carpeta, punto, perfiles, false))
        return false;
      lab.audio->digital.generar(mono.data(), static_cast<unsigned int>(mono.size()));
      lab.audio->espacial.procesar(lab.datos.data(), mono.data(), estereo.data(),
                                   static_cast<unsigned int>(mono.size()));
      if (mono != referencia)
        return false;
      for (size_t i = 0; i < mono.size(); ++i)
        if (estereo[i * 2] != mono[i] || estereo[i * 2 + 1] != mono[i])
          return false;
    }
  }
  lab.posicional = true;
  if (!lab.reproducir_voz(carpeta, 2, perfiles))
    return false;
  const auto inicio = SDL_GetTicks();
  while (lab.reproduciendo() && SDL_GetTicks() - inicio < 2000) {
    SDL_Delay(8);
    if (!lab.actualizar())
      return false;
  }
  if (lab.reproduciendo())
    return false;
  lab.cerrar();
  if (lab.audio || lab.reproduciendo())
    return false;
  const auto json = json11::Json(json11::Json::object{{"temas", temas},
                                                      {"posiciones", posiciones},
                                                      {"mono_exacto", true},
                                                      {"fin_automatico_voz", true},
                                                      {"perfiles_disponibles", int(perfiles)}})
                        .dump() +
                    "\n";
  const bool ok = SDL_SaveFile((salida + "/resultado.json").c_str(), json.data(), json.size());
  if (ok)
    std::printf("Laboratorio de audio: %zu temas/versiones, dos voces en cinco posiciones, "
                "repetición exacta, comparación mono y parada comprobados.\n",
                temas.size());
  return ok;
}
