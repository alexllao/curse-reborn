#include "tiempo_juego.hpp"
#include "registro.hpp"
#include <charconv>
#include <cctype>
#include <filesystem>
#include <limits>
#include <string_view>

namespace {
constexpr Uint64 ns_por_ms = 1000000ull;
constexpr Uint64 intervalo_guardado = 30000000000ull;
constexpr Uint64 intervalo_maximo = 5000000000ull;

// Lee el formato propio versionado, ignorando espacios pero nunca números inválidos o campos extra.
bool leer_tiempo(const char *texto, size_t longitud, Uint64 &milisegundos) {
  std::string compacto;
  for (size_t i = 0; i < longitud; ++i)
    if (!std::isspace(static_cast<unsigned char>(texto[i])))
      compacto += texto[i];
  constexpr std::string_view prefijo = "{\"version\":1,\"milisegundos_jugados\":";
  if (compacto.size() <= prefijo.size() + 1 || compacto.compare(0, prefijo.size(), prefijo) ||
      compacto.back() != '}')
    return false;
  const char *inicio = compacto.data() + prefijo.size(),
             *fin = compacto.data() + compacto.size() - 1;
  const auto resultado = std::from_chars(inicio, fin, milisegundos);
  return resultado.ec == std::errc{} && resultado.ptr == fin &&
         milisegundos <= std::numeric_limits<Uint64>::max() / ns_por_ms;
}
} // namespace

// La duración de la sesión no depende del momento en que se escriba el archivo.
TiempoJuego::~TiempoJuego() {
  cerrar();
}

// Los fallos o versiones ajenas dejan la estadística en memoria y el archivo original intacto.
void TiempoJuego::iniciar(const std::string &archivo) {
  if (iniciado)
    return;
  total_ns = sesion_ns = muestra_anterior = ultimo_guardado = 0;
  persistente = contando = pendiente = aviso_guardado = false;
  primera_muestra = true;
  iniciado = true;
  ruta = archivo;
  if (ruta.empty()) {
    const char *alternativa = SDL_getenv("CURSE_REBORN_TIEMPO");
    if (alternativa && *alternativa)
      ruta = alternativa;
    else {
      char *carpeta = SDL_GetPrefPath("AlexLlao", "CurseReborn");
      if (carpeta) {
        ruta = std::string(carpeta) + "tiempo_juego.json";
        SDL_free(carpeta);
      }
    }
  }
  if (ruta.empty()) {
    registrar_accion("Tiempo de juego: no se puede abrir el perfil; contador solo en memoria");
    return;
  }
  std::error_code error;
  const bool existe = std::filesystem::exists(std::filesystem::u8path(ruta), error);
  if (error) {
    registrar_accion("Tiempo de juego: no se puede consultar %s; se conserva sin modificar",
                     ruta.c_str());
    return;
  }
  if (existe) {
    SDL_PathInfo info{};
    if (!SDL_GetPathInfo(ruta.c_str(), &info) || info.type != SDL_PATHTYPE_FILE ||
        info.size > 256) {
      registrar_accion("Tiempo de juego: archivo no reconocido; se conserva sin modificar: %s",
                       ruta.c_str());
      return;
    }
    size_t longitud = 0;
    auto *texto = static_cast<char *>(SDL_LoadFile(ruta.c_str(), &longitud));
    Uint64 milisegundos = 0;
    const bool valido = texto && leer_tiempo(texto, longitud, milisegundos);
    SDL_free(texto);
    if (!valido) {
      registrar_accion("Tiempo de juego: formato no reconocido; se conserva sin modificar: %s",
                       ruta.c_str());
      return;
    }
    total_ns = milisegundos * ns_por_ms;
  }
  persistente = true;
  pendiente = !existe;
  registrar_accion("Tiempo efectivo acumulado: %s. Archivo: %s",
                   formatear(milisegundos_totales()).c_str(), ruta.c_str());
}

// Las transiciones de actividad no atribuyen el intervalo anterior a una actividad diferente.
void TiempoJuego::actualizar(bool activo, Uint64 ahora) {
  if (!iniciado)
    return;
  if (primera_muestra) {
    primera_muestra = false;
    muestra_anterior = ultimo_guardado = ahora;
    contando = activo;
    return;
  }
  const Uint64 intervalo = ahora >= muestra_anterior ? ahora - muestra_anterior : 0;
  if (activo && contando && intervalo <= intervalo_maximo &&
      intervalo <= std::numeric_limits<Uint64>::max() - total_ns &&
      intervalo <= std::numeric_limits<Uint64>::max() - sesion_ns) {
    total_ns += intervalo;
    sesion_ns += intervalo;
    pendiente = pendiente || intervalo != 0;
  }
  const bool pausa = contando && !activo;
  contando = activo;
  muestra_anterior = ahora;
  if (pausa || (ahora >= ultimo_guardado && ahora - ultimo_guardado >= intervalo_guardado)) {
    guardar();
    ultimo_guardado = ahora;
  }
}

// Sustituye el JSON solo cuando existe una copia temporal completa y validada por escritura.
bool TiempoJuego::guardar() {
  if (!persistente)
    return false;
  if (!pendiente)
    return true;
  const std::string contenido = "{\n  \"version\": 1,\n  \"milisegundos_jugados\": " +
                                std::to_string(milisegundos_totales()) + "\n}\n";
  const auto temporal = ruta + ".temporal";
  const bool correcto = SDL_SaveFile(temporal.c_str(), contenido.data(), contenido.size()) &&
                        SDL_RenamePath(temporal.c_str(), ruta.c_str());
  if (correcto) {
    pendiente = false;
    aviso_guardado = false;
  } else if (!aviso_guardado) {
    registrar_accion(
        "Tiempo de juego: no se ha podido guardar; se conserva el contador en memoria: %s",
        SDL_GetError());
    aviso_guardado = true;
  }
  return correcto;
}

// El cierre es idempotente y no suma tiempo de limpieza, menús o presentación.
void TiempoJuego::cerrar() {
  if (!iniciado)
    return;
  guardar();
  registrar_accion("Tiempo efectivo de la sesión: %s; acumulado del perfil: %s",
                   formatear(milisegundos_sesion()).c_str(),
                   formatear(milisegundos_totales()).c_str());
  iniciado = false;
}

// La persistencia conserva milisegundos; las fracciones menores viven durante la sesión.
Uint64 TiempoJuego::milisegundos_totales() const {
  return total_ns / ns_por_ms;
}

// Esta cifra empieza en cero incluso si el perfil ya tenía horas acumuladas.
Uint64 TiempoJuego::milisegundos_sesion() const {
  return sesion_ns / ns_por_ms;
}

// Permite comprobar los fallos de perfil sin confundirlos con errores del motor.
bool TiempoJuego::admite_guardado() const {
  return persistente;
}

// Pensar e inventario cuentan; paneles, segundo plano y alimentación apagada no cuentan.
bool TiempoJuego::debe_contar(bool jugable, SDL_WindowFlags ventana, bool panel, bool apagado) {
  return jugable && !panel && !apagado && (ventana & SDL_WINDOW_INPUT_FOCUS) &&
         !(ventana & (SDL_WINDOW_HIDDEN | SDL_WINDOW_MINIMIZED));
}

// Evita reiniciar las horas al superar un día y no necesita la hora civil del sistema.
std::string TiempoJuego::formatear(Uint64 milisegundos) {
  const auto segundos = milisegundos / 1000;
  const auto horas = segundos / 3600;
  char resto[16]{};
  SDL_snprintf(resto, sizeof(resto), ":%02u:%02u", unsigned(segundos / 60 % 60),
               unsigned(segundos % 60));
  return (horas < 10 ? "0" : "") + std::to_string(horas) + resto;
}
