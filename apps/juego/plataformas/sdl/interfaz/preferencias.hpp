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
  unsigned int musica = 5, volumen_musica = 100, volumen_efectos = 100;
  unsigned int estereo_posicional = 1, efectos_digitales = 1, pantalla_completa = 0;
  std::string habitacion;
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
    unsigned int perfil = 5, estereo = estereo_posicional;
    unsigned int efectos = efectos_digitales, completa = pantalla_completa;
    std::string cuarto = habitacion;
    unsigned int vm = volumen_musica, ve = volumen_efectos;
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
    bool correcto = literal("{\"version\":1,\"volumen\":") && numero(v) &&
                    literal(",\"silencio\":") && numero(s) && literal(",\"marco\":") && numero(m) &&
                    literal(",\"filtro\":") && numero(f);
    if (correcto && resto.substr(0, 10) == ",\"musica\":")
      correcto = literal(",\"musica\":") && numero(perfil) && perfil <= 4;
    constexpr std::string_view clave_estereo = ",\"estereo_posicional\":";
    if (correcto && resto.substr(0, clave_estereo.size()) == clave_estereo)
      correcto = literal(clave_estereo) && numero(estereo) && estereo <= 1;
    const auto opcional = [&](std::string_view clave, unsigned int &valor) {
      if (correcto && resto.substr(0, clave.size()) == clave)
        correcto = literal(clave) && numero(valor) && valor <= 1;
    };
    opcional(",\"efectos_digitales\":", efectos);
    opcional(",\"pantalla_completa\":", completa);
    constexpr std::string_view clave_cuarto = ",\"habitacion\":\"";
    if (correcto && resto.substr(0, clave_cuarto.size()) == clave_cuarto) {
      correcto = literal(clave_cuarto);
      const auto fin = resto.find('"');
      correcto = correcto && fin != std::string_view::npos && fin <= 64;
      if (correcto) {
        cuarto = std::string(resto.substr(0, fin));
        correcto =
            cuarto.find_first_not_of("abcdefghijklmnopqrstuvwxyz0123456789-_") == std::string::npos;
        resto.remove_prefix(fin + 1);
      }
    }
    const auto volumen_opcional = [&](std::string_view clave, unsigned int &valor) {
      if (correcto && resto.substr(0, clave.size()) == clave)
        correcto = literal(clave) && numero(valor) && valor <= 400 && valor % 10 == 0;
    };
    volumen_opcional(",\"volumen_musica\":", vm);
    volumen_opcional(",\"volumen_efectos\":", ve);
    if (!correcto || !literal("}") || !resto.empty() || v > 400 || v % 10 || s > 1 || m > 1 ||
        f > 5) {
      registrar_accion("Preferencias: formato no reconocido; se conserva el archivo sin modificar");
      return;
    }
    volumen = v;
    silencio = s;
    marco = m;
    filtro = f;
    musica = perfil;
    estereo_posicional = estereo;
    efectos_digitales = efectos;
    pantalla_completa = completa;
    habitacion = cuarto;
    volumen_musica = vm;
    volumen_efectos = ve;
    persistente = true;
    registrar_accion("Preferencias recuperadas: volumen %u%%, silencio %u, marco %u, filtro %u",
                     volumen, silencio, marco, filtro);
  }
  bool guardar(unsigned int v, bool s, bool m, unsigned int f, unsigned int perfil = 5,
               unsigned int estereo = 2, unsigned int efectos = 2, unsigned int completa = 2,
               const std::string &cuarto = "", unsigned int vm = 500, unsigned int ve = 500) {
    if ((vm != 500 && (vm > 400 || vm % 10)) || (ve != 500 && (ve > 400 || ve % 10)))
      return false;
    if (!persistente || v > 400 || v % 10 || f > 5 || perfil > 5 || estereo > 2 || efectos > 2 ||
        completa > 2 || cuarto.size() > 64 ||
        cuarto.find_first_not_of("abcdefghijklmnopqrstuvwxyz0123456789-_") != std::string::npos)
      return false;
    const std::string contenido =
        "{\"version\":1,\"volumen\":" + std::to_string(v) + ",\"silencio\":" + std::to_string(s) +
        ",\"marco\":" + std::to_string(m) + ",\"filtro\":" + std::to_string(f) +
        (perfil < 5 ? ",\"musica\":" + std::to_string(perfil) : "") +
        (estereo < 2 ? ",\"estereo_posicional\":" + std::to_string(estereo) : "") +
        (efectos < 2 ? ",\"efectos_digitales\":" + std::to_string(efectos) : "") +
        (completa < 2 ? ",\"pantalla_completa\":" + std::to_string(completa) : "") +
        (!cuarto.empty() ? ",\"habitacion\":\"" + cuarto + "\"" : "") +
        (vm <= 400 ? ",\"volumen_musica\":" + std::to_string(vm) : "") +
        (ve <= 400 ? ",\"volumen_efectos\":" + std::to_string(ve) : "") + "}\n";
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
