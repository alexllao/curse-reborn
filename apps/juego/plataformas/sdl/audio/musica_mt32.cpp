#include "musica_mt32.hpp"
#include "../../../motor/estado/inicializar.hpp"
#include "../registro.hpp"

namespace {
// Resuelve el API C tipado para permitir sustituir la biblioteca compartida sin volver a enlazar.
template <typename Funcion>
bool cargar_funcion(SDL_SharedObject *biblioteca, const char *nombre, Funcion &destino) {
  destino = reinterpret_cast<Funcion>(SDL_LoadFunction(biblioteca, nombre));
  return destino != nullptr;
}
} // namespace

// Munt identifica las ROM por su contenido; no se distribuyen ni se modifican estos archivos.
bool MusicaMt32::iniciar(const std::string &carpeta, const unsigned char *ejecutable,
                         unsigned int longitud, unsigned short escena_inicial) {
  cerrar();
  if (!motor::leer_datos_iniciales(ejecutable, longitud, datos))
    return false;
#ifdef _WIN32
  const auto ruta = std::string(SDL_GetBasePath()) + "mt32emu.dll";
#elif defined(__APPLE__)
  const auto ruta = std::string(SDL_GetBasePath()) + "libmt32emu.dylib";
#else
  const auto ruta = std::string(SDL_GetBasePath()) + "libmt32emu.so";
#endif
  biblioteca = SDL_LoadObject(ruta.c_str());
  if (!biblioteca || !cargar_funcion(biblioteca, "mt32emu_create_context", crear) ||
      !cargar_funcion(biblioteca, "mt32emu_free_context", liberar) ||
      !cargar_funcion(biblioteca, "mt32emu_add_rom_file", agregar_rom) ||
      !cargar_funcion(biblioteca, "mt32emu_get_rom_info", consultar_rom) ||
      !cargar_funcion(biblioteca, "mt32emu_set_analog_output_mode", fijar_modo_analogico) ||
      !cargar_funcion(biblioteca, "mt32emu_set_stereo_output_samplerate", fijar_frecuencia) ||
      !cargar_funcion(biblioteca, "mt32emu_open_synth", abrir_sintesis) ||
      !cargar_funcion(biblioteca, "mt32emu_play_msg", enviar) ||
      !cargar_funcion(biblioteca, "mt32emu_render_bit16s", sintetizar)) {
    cerrar();
    return false;
  }
  contexto = crear({nullptr}, nullptr);
  if (!contexto)
    return false;
  const auto roms = carpeta + "/../../ROMS/";
  if (agregar_rom(contexto, (roms + "MT32_CONTROL.ROM").c_str()) < 0 ||
      agregar_rom(contexto, (roms + "MT32_PCM.ROM").c_str()) < 0) {
    cerrar();
    return false;
  }
  mt32emu_rom_info informacion{};
  consultar_rom(contexto, &informacion);
  registrar_accion("ROM Roland: %s; PCM: %s", informacion.control_rom_description,
                   informacion.pcm_rom_id);
  fijar_modo_analogico(contexto, MT32EMU_AOM_ACCURATE);
  fijar_frecuencia(contexto, 48000);
  if (abrir_sintesis(contexto) != MT32EMU_RC_OK) {
    cerrar();
    return false;
  }
  for (unsigned int i = 0; i < 5; ++i) {
    size_t tamano = 0;
    auto *archivo = SDL_LoadFile((carpeta + "/CURSE.RS" + std::to_string(i + 1)).c_str(), &tamano);
    if (!archivo || tamano > 65536) {
      SDL_free(archivo);
      cerrar();
      return false;
    }
    const auto *bytes = static_cast<const unsigned char *>(archivo);
    bancos[i].assign(bytes, bytes + tamano);
    SDL_free(archivo);
  }
  banco_actual = 255;
  return cambiar_escena(escena_inicial);
}

// Las llamadas se serializan con el flujo musical y nunca acceden al DS de la partida.
void MusicaMt32::cerrar() {
  if (contexto && liberar)
    liberar(contexto);
  contexto = nullptr;
  if (biblioteca)
    SDL_UnloadObject(biblioteca);
  biblioteca = nullptr;
}

// Sustituye 4DF0/7D5D y la parada MPU 05 por fin de notas sin rampa de volumen; conserva la cola
// del sintetizador.
bool MusicaMt32::cambiar_escena(unsigned short escena) {
  const auto referencia = motor::seleccionar_musica_rs(datos, escena);
  if (!contexto || !referencia.valida)
    return false;
  if (banco_actual == referencia.banco)
    return true;
  for (unsigned int canal = 0; canal < 16; ++canal)
    if (enviar(contexto, (0xb0u | canal) | (123u << 8)) != MT32EMU_RC_OK)
      return false;
  banco_actual = referencia.banco;
  tempo = referencia.tempo;
  estado = 0;
  posicion = espera = 0;
  fraccion = 0;
  registrar_accion("Música MT-32: CURSE.RS%u, tempo %u, contexto %s", banco_actual + 1, tempo,
                   escena == 0xffff ? "presentación" : "escena del juego");
  return preparar_evento();
}

// Las marcas F9 no son notas; la cola Munt conserva órdenes simultáneas durante la liberación de
// voces.
bool MusicaMt32::preparar_evento() {
  const auto &banco = bancos[banco_actual];
  for (unsigned int i = 0; i < 4096; ++i) {
    if (!motor::leer_evento_musica_rs(banco.data(), static_cast<unsigned int>(banco.size()),
                                      posicion, estado, evento))
      return false;
    espera = evento.tiempo;
    if (espera)
      return true;
    if (evento.mensaje && enviar(contexto, evento.mensaje) != MT32EMU_RC_OK)
      return false;
  }
  return false;
}

// Agrupa muestras hasta el siguiente pulso para conservar tiempos sin llamar al sintetizador por
// byte.
bool MusicaMt32::generar(short *salida, unsigned int cantidad) {
  if (!contexto || (!salida && cantidad) || !tempo)
    return false;
  unsigned int escritos = 0;
  const auto incremento = static_cast<unsigned int>(tempo) * 120u;
  while (escritos < cantidad) {
    const auto hasta_pulso = (2880000u - fraccion + incremento - 1) / incremento;
    auto bloque = cantidad - escritos;
    if (bloque > 512)
      bloque = 512;
    if (bloque > hasta_pulso)
      bloque = hasta_pulso;
    short estereo[1024];
    sintetizar(contexto, estereo, bloque);
    // Conserva los dos canales de Munt; la ganancia por canal mantiene el nivel de la mezcla
    // anterior.
    for (unsigned int i = 0; i < bloque * 2; ++i) {
      auto valor = static_cast<int>(estereo[i]) * 2;
      if (valor > 28000)
        valor = 28000;
      if (valor < -28000)
        valor = -28000;
      salida[escritos * 2 + i] = static_cast<short>(valor);
    }
    escritos += bloque;
    fraccion += bloque * incremento;
    if (fraccion >= 2880000u) {
      fraccion -= 2880000u;
      if (espera && --espera == 0) {
        if (evento.mensaje && enviar(contexto, evento.mensaje) != MT32EMU_RC_OK)
          return false;
        if (!preparar_evento())
          return false;
      }
    }
  }
  return true;
}
