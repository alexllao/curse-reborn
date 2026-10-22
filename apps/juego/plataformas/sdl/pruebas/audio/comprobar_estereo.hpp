#pragma once
#include "../../audio/estereo_posicional.hpp"
#include "../../interfaz/configuracion.hpp"
#include "../../../../motor/estado/inicializar.hpp"
#include "../../../../motor/sonido/digital.hpp"
#include <array>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <vector>

// PCM real de STD.SFX, sin abrir dispositivo, ventana ni reproductor audible.
inline bool comprobar_estereo_posicional(const std::string &carpeta, const char *informe) {
  std::array<unsigned char, 65536> datos{};
  size_t longitud = 0, longitud_banco = 0;
  auto *exe =
      static_cast<unsigned char *>(SDL_LoadFile((carpeta + "/../CURSE.EXE").c_str(), &longitud));
  auto *banco =
      static_cast<unsigned char *>(SDL_LoadFile((carpeta + "/STD.SFX").c_str(), &longitud_banco));
  const bool preparado =
      exe && banco && longitud <= 0xffffffffu && longitud_banco <= 65536 &&
      motor::leer_datos_iniciales(exe, static_cast<unsigned int>(longitud), datos.data());
  SDL_free(exe);
  if (!preparado) {
    SDL_free(banco);
    return false;
  }
  constexpr unsigned short brad = 0x2c2b, objeto = 0x2c4c;
  const auto colocar = [&](unsigned short entidad, short x) {
    motor::guardar(datos.data(), entidad + 2, static_cast<unsigned short>(x));
    motor::guardar(datos.data(), entidad + 4, 50);
    motor::guardar(datos.data(), entidad + 6, 32);
    motor::guardar(datos.data(), entidad + 8, 48);
  };
  colocar(brad, 0);
  colocar(objeto, 288);
  datos[0x0ad4] = 3;
  motor::guardar(datos.data(), 0x8082, 1);
  motor::DigitalPcm digital;
  digital.conectar(banco, static_cast<unsigned int>(longitud_banco), nullptr, 0);
  bool iniciada = false;
  bool correcto = digital.reproducir(datos.data(), brad, 0, &iniciada) && iniciada;
  EstereoPosicional espacial;
  espacial.iniciar(datos.data(), brad);
  // Una petición rechazada por prioridad no debe mover el efecto de Brad al objeto derecho.
  bool rechazada = true;
  correcto = correcto && digital.reproducir(datos.data(), objeto, 0, &rechazada) && !rechazada;
  std::vector<short> mono(32000), salida(mono.size() * 2), concatenada;
  digital.generar(mono.data(), static_cast<unsigned int>(mono.size()));
  SDL_free(banco);
  if (!correcto || !std::any_of(mono.begin(), mono.end(), [](short v) { return v != 0; }))
    return false;
  json11::Json::array casos;
  for (const auto x : {short(-32), short(0), short(144), short(288), short(320)}) {
    colocar(brad, x);
    espacial.iniciar(datos.data(), brad);
    const auto antes = datos;
    espacial.procesar(datos.data(), mono.data(), salida.data(),
                      static_cast<unsigned int>(mono.size()));
    if (datos != antes)
      return false;
    double izquierda = 0, derecha = 0;
    for (size_t i = 0; i < mono.size(); ++i) {
      izquierda += double(salida[i * 2]) * salida[i * 2];
      derecha += double(salida[i * 2 + 1]) * salida[i * 2 + 1];
      if (std::abs(int(salida[i * 2])) > std::abs(int(mono[i])) ||
          std::abs(int(salida[i * 2 + 1])) > std::abs(int(mono[i])))
        return false;
    }
    if ((x < 144 && izquierda <= derecha) || (x > 144 && izquierda >= derecha) ||
        (x == 144 && izquierda != derecha))
      return false;
    casos.emplace_back(
        json11::Json::object{{"x", int(x)}, {"energia_L", izquierda}, {"energia_R", derecha}});
    if (x == 0 || x == 144 || x == 288) {
      concatenada.insert(concatenada.end(), salida.begin(), salida.begin() + 24744 * 2);
      concatenada.insert(concatenada.end(), 12000, 0);
    }
  }
  espacial.configurar(false);
  espacial.iniciar(datos.data(), brad);
  espacial.procesar(datos.data(), mono.data(), salida.data(),
                    static_cast<unsigned int>(mono.size()));
  for (size_t i = 0; i < mono.size(); ++i)
    if (salida[i * 2] != mono[i] || salida[i * 2 + 1] != mono[i])
      return false;
  espacial.configurar(true);
  espacial.iniciar(datos.data(), 0xffff);
  espacial.procesar(datos.data(), mono.data(), salida.data(),
                    static_cast<unsigned int>(mono.size()));
  for (size_t i = 0; i < mono.size(); ++i)
    if (salida[i * 2] != mono[i] || salida[i * 2 + 1] != mono[i])
      return false;
  colocar(brad, 0);
  EstereoPosicional completo, dividido;
  completo.iniciar(datos.data(), brad);
  dividido.iniciar(datos.data(), brad);
  colocar(brad, 288);
  motor::guardar(datos.data(), 0x3591, 640); // El origen del mundo no se resta otra vez.
  std::vector<short> bloques(salida.size());
  completo.procesar(datos.data(), mono.data(), salida.data(),
                    static_cast<unsigned int>(mono.size()));
  for (size_t i = 0; i < mono.size();) {
    const auto n = std::min<size_t>(137, mono.size() - i);
    dividido.procesar(datos.data(), mono.data() + i, bloques.data() + i * 2,
                      static_cast<unsigned int>(n));
    i += n;
  }
  if (salida != bloques)
    return false;
  const auto balance = balance_sonido(datos.data(), brad, true);
  motor::guardar(datos.data(), 0x3591, 0);
  if (balance.derecha != balance_sonido(datos.data(), brad, true).derecha ||
      balance.izquierda != balance_sonido(datos.data(), brad, true).izquierda)
    return false;
  std::vector<unsigned char> wav(44 + concatenada.size() * 2);
  const auto campo = [&](size_t p, unsigned int v, unsigned int n) {
    for (unsigned int i = 0; i < n; ++i)
      wav[p + i] = static_cast<unsigned char>(v >> (8 * i));
  };
  std::memcpy(wav.data(), "RIFF", 4);
  campo(4, static_cast<unsigned int>(wav.size() - 8), 4);
  std::memcpy(wav.data() + 8, "WAVEfmt ", 8);
  campo(16, 16, 4);
  campo(20, 1, 2);
  campo(22, 2, 2);
  campo(24, 48000, 4);
  campo(28, 192000, 4);
  campo(32, 4, 2);
  campo(34, 16, 2);
  std::memcpy(wav.data() + 36, "data", 4);
  campo(40, static_cast<unsigned int>(wav.size() - 44), 4);
  for (size_t i = 0; i < concatenada.size(); ++i)
    campo(44 + i * 2, static_cast<unsigned short>(concatenada[i]), 2);
  const auto texto = json11::Json(json11::Json::object{{"casos", casos},
                                                       {"mono_exacto_desactivado", true},
                                                       {"origen_desconocido_centrado", true},
                                                       {"particiones_y_rampa", true},
                                                       {"sin_doble_desplazamiento_camara", true},
                                                       {"prioridad_original", true}})
                         .dump() +
                     "\n";
  return SDL_SaveFile(informe, texto.data(), texto.size()) &&
         SDL_SaveFile((std::string(informe) + ".wav").c_str(), wav.data(), wav.size());
}
