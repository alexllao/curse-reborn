#pragma once
#include <SDL3/SDL.h>
#include "../registro.hpp"
#include <charconv>
#include <cctype>
#include <filesystem>
#include <string>
#include <string_view>

// Preferencias del usuario; independientes de CURSE.SAV y de las instantáneas DOS.
class PreferenciasUsuario {
public:
  unsigned int volumen = 100, silencio = 0, marco = 0, filtro = 2;
  unsigned int musica = 5;
  void cargar(const std::string &archivo = "") {
    ruta = archivo;
    persistente = false;
    if (ruta.empty()) {
      const char *alternativa = SDL_getenv("CURSE_REBORN_PREFERENCIAS");
      if (alternativa && *alternativa)
        ruta = alternativa;
      else {
        char *carpeta = SDL_GetPrefPath("AlexLlao", "CurseReborn");
        if (carpeta) {
          ruta = std::string(carpeta) + "preferencias.json";
          SDL_free(carpeta);
        }
      }
    }
    if (ruta.empty())
      return;
    std::error_code error;
    const bool existe = std::filesystem::exists(std::filesystem::u8path(ruta), error);
    if (error)
      return;
    if (!existe) {
      persistente = true;
      return;
    }
    SDL_PathInfo info{};
    if (!SDL_GetPathInfo(ruta.c_str(), &info) || info.type != SDL_PATHTYPE_FILE || info.size > 512)
      return;
    size_t longitud = 0;
    auto *datos = static_cast<char *>(SDL_LoadFile(ruta.c_str(), &longitud));
    std::string compacto;
    if (datos)
      for (size_t i = 0; i < longitud; ++i)
        if (!std::isspace(static_cast<unsigned char>(datos[i])))
          compacto += datos[i];
    SDL_free(datos);
    unsigned int v = 0, s = 0, m = 0, f = 0;
    unsigned int perfil = 5;
    std::string_view resto(compacto);
    const auto literal = [&](std::string_view esperado) {
      if (resto.substr(0, esperado.size()) != esperado)
        return false;
      resto.remove_prefix(esperado.size());
      return true;
    };
    const auto numero = [&](unsigned int &destino) {
      const auto r = std::from_chars(resto.data(), resto.data() + resto.size(), destino);
      if (r.ec != std::errc{})
        return false;
      resto.remove_prefix(static_cast<size_t>(r.ptr - resto.data()));
      return true;
    };
    if (!literal("{\"version\":1,\"volumen\":") || !numero(v) || !literal(",\"silencio\":") ||
        !numero(s) || !literal(",\"marco\":") || !numero(m) || !literal(",\"filtro\":") ||
        !numero(f) ||
        (!resto.empty() && resto.front() == ',' &&
         (!literal(",\"musica\":") || !numero(perfil) || perfil > 4)) ||
        !literal("}") || !resto.empty() || v > 400 || v % 10 || s > 1 || m > 1 || f > 5) {
      registrar_accion("Preferencias: formato no reconocido; se conserva el archivo sin modificar");
      return;
    }
    volumen = v;
    silencio = s;
    marco = m;
    filtro = f;
    musica = perfil;
    persistente = true;
    registrar_accion("Preferencias recuperadas: volumen %u%%, silencio %u, marco %u, filtro %u",
                     volumen, silencio, marco, filtro);
  }
  bool guardar(unsigned int v, bool s, bool m, unsigned int f, unsigned int perfil = 5) {
    if (!persistente || v > 400 || v % 10 || f > 5 || perfil > 5)
      return false;
    const std::string contenido =
        "{\"version\":1,\"volumen\":" + std::to_string(v) + ",\"silencio\":" + std::to_string(s) +
        ",\"marco\":" + std::to_string(m) + ",\"filtro\":" + std::to_string(f) +
        (perfil < 5 ? ",\"musica\":" + std::to_string(perfil) : "") + "}\n";
    const auto temporal = ruta + ".temporal";
    const bool ok = SDL_SaveFile(temporal.c_str(), contenido.data(), contenido.size()) &&
                    SDL_RenamePath(temporal.c_str(), ruta.c_str());
    if (!ok)
      registrar_accion("No se han podido guardar las preferencias: %s", SDL_GetError());
    return ok;
  }

private:
  std::string ruta;
  bool persistente = false;
};
