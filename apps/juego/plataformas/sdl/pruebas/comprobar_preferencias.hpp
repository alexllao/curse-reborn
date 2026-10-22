#pragma once
#include "../interfaz/preferencias.hpp"

// Perfiles aislados: reconstruir el lector simula cada arranque sin tocar el perfil del usuario.
inline bool comprobar_preferencias(const std::string &ruta) {
  const auto ruta_prueba = ruta + ".json";
  SDL_RemovePath(ruta_prueba.c_str());
  PreferenciasUsuario inicial;
  inicial.cargar(ruta_prueba);
  if (inicial.volumen != 100 || inicial.silencio || inicial.marco || inicial.filtro != 2)
    return false;
  unsigned int casos = 0;
  for (const auto volumen : {0u, 100u, 300u, 400u})
    for (unsigned int silencio = 0; silencio < 2; ++silencio)
      for (unsigned int marco = 0; marco < 2; ++marco)
        for (unsigned int filtro = 0; filtro < 6; ++filtro) {
          if (!inicial.guardar(volumen, silencio != 0, marco != 0, filtro))
            return false;
          PreferenciasUsuario siguiente;
          siguiente.cargar(ruta_prueba);
          if (siguiente.volumen != volumen || siguiente.silencio != silencio ||
              siguiente.marco != marco || siguiente.filtro != filtro)
            return false;
          ++casos;
        }
  for (const std::string texto :
       {"{\"version\":2,\"volumen\":100,\"silencio\":0,\"marco\":0,\"filtro\":2}",
        "{\"version\":1,\"volumen\":401,\"silencio\":0,\"marco\":0,\"filtro\":2}",
        "{\"version\":1,\"volumen\":100,\"silencio\":0,\"marco\":0,\"filtro\":6}",
        "{\"version\":1,\"volumen\":99999999999999999999999999999999}", "{\"version\":1"}) {
    if (!SDL_SaveFile(ruta_prueba.c_str(), texto.data(), texto.size()))
      return false;
    PreferenciasUsuario ajena;
    ajena.cargar(ruta_prueba);
    if (ajena.guardar(100, false, false, 2))
      return false;
    size_t longitud = 0;
    auto *datos = static_cast<char *>(SDL_LoadFile(ruta_prueba.c_str(), &longitud));
    const bool intacto = datos && std::string(datos, longitud) == texto;
    SDL_free(datos);
    if (!intacto)
      return false;
    ++casos;
  }
  SDL_RemovePath(ruta_prueba.c_str());
  inicial.cargar(ruta_prueba);
  for (unsigned int perfil = 0; perfil < 5; ++perfil) {
    if (!inicial.guardar(100, false, false, 2, perfil))
      return false;
    PreferenciasUsuario siguiente;
    siguiente.cargar(ruta_prueba);
    if (siguiente.musica != perfil)
      return false;
    ++casos;
  }
  SDL_RemovePath(ruta_prueba.c_str());
  registrar_accion(
      "Preferencias: %u casos; valores iniciales, recuperación y archivos ajenos intactos", casos);
  return true;
}
