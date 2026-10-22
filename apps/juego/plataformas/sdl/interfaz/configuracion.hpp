#pragma once
#include <SDL3/SDL.h>
#include "../../../terceros/json11/json11.hpp"
#include <string>

// Rutas de recursos propias, relativas al ejecutable; nunca se interpretan como órdenes.
inline bool ruta_recurso_valida(const std::string &ruta) {
  return !ruta.empty() && ruta.front() != '/' && ruta.find("..") == std::string::npos &&
         ruta.find(':') == std::string::npos && ruta.find('\\') == std::string::npos;
}

inline json11::Json leer_json_recurso(const std::string &ruta) {
  if (!ruta_recurso_valida(ruta))
    return {};
  size_t longitud = 0;
  auto *bytes =
      static_cast<char *>(SDL_LoadFile((std::string(SDL_GetBasePath()) + ruta).c_str(), &longitud));
  std::string error;
  const auto json = bytes && longitud < 65536
                        ? json11::Json::parse(std::string(bytes, longitud), error)
                        : json11::Json();
  SDL_free(bytes);
  if (!error.empty())
    SDL_Log("Configuración JSON inválida: %s: %s", ruta.c_str(), error.c_str());
  return json;
}

inline json11::Json configuracion_inicial() {
  auto json = leer_json_recurso("configuracion.json");
  return json["version"].int_value() == 1 ? json : json11::Json();
}
