#pragma once
#include "../../audio/audio.hpp"
#include "../../interfaz/configuracion.hpp"
#include <algorithm>
#include <vector>

// Verifica WAV/MP3 reales sin ventana ni salida física: ejecutar con SDL_AUDIODRIVER=dummy.
inline bool comprobar_musica_remaster(const std::string &carpeta, const char *informe) {
  MusicaRemaster a, b;
  const auto config = configuracion_inicial();
  const auto catalogo = config["musica"]["catalogo"].string_value();
  const auto titulos = leer_json_recurso(catalogo);
  if (!a.iniciar(catalogo) || !b.iniciar(catalogo))
    return false;
  json11::Json::array pistas;
  for (unsigned int banco = 0; banco < 5; ++banco) {
    a.cambiar_banco(banco);
    b.cambiar_banco(banco);
    const auto longitud = a.fotogramas(banco);
    if (!longitud ||
        a.titulo() != titulos["rs" + std::to_string(banco + 1)]["titulo"].string_value())
      return false;
    const size_t cantidad = longitud + 2048;
    std::vector<short> continua(cantidad * 2), dividida(cantidad * 2);
    a.generar(continua.data(), static_cast<unsigned int>(cantidad));
    for (size_t i = 0; i < cantidad;) {
      const auto n = std::min<size_t>(137, cantidad - i);
      b.generar(dividida.data() + i * 2, static_cast<unsigned int>(n));
      i += n;
    }
    if (continua != dividida ||
        !std::any_of(continua.begin(), continua.end(), [](short m) { return m > 100 || m < -100; }))
      return false;
    const auto posicion = a.posicion();
    a.cambiar_banco(banco);
    if (a.posicion() != posicion)
      return false;
    pistas.emplace_back(json11::Json::object{{"banco", static_cast<int>(banco + 1)},
                                             {"titulo", a.titulo()},
                                             {"fotogramas", static_cast<double>(longitud)},
                                             {"repeticion_y_particiones", true}});
  }
  Audio audio;
  audio.configurar_musica();
  audio.preferir_musica(4);
  if (!audio.abrir(carpeta) || audio.perfil_musical() != 4 || !audio.cambiar_escena(1) ||
      audio.descripcion_musica().find("Épopée des Rois") == std::string::npos)
    return false;
  bool perfiles[5]{};
  for (unsigned int n = 0; n < 6; ++n) {
    perfiles[audio.perfil_musical()] = true;
    if (!audio.alternar_dispositivo())
      return false;
  }
  if (!perfiles[0] || !perfiles[1] || !perfiles[2] || !perfiles[4] || !audio.ajustar_volumen(100) ||
      !audio.alternar_silencio() || !audio.silenciado())
    return false;
  audio.cerrar();
  const auto contenido =
      json11::Json(json11::Json::object{{"pistas", pistas}, {"f4_volumen_silencio", true}}).dump() +
      "\n";
  return SDL_SaveFile(informe, contenido.data(), contenido.size());
}
