#include "audio.hpp"
#include "../../../motor/estado/inicializar.hpp"
#include <cstdio>
#include <vector>
#include <unordered_set>
#include "../../../motor/sonido/musica.hpp"
#include "../../../motor/estado/segmento.hpp"

// Abre dos flujos del mismo dispositivo: música autónoma y efectos atendidos por el juego.
bool Audio::abrir(const std::string &carpeta, bool conservar_seleccion, bool iniciar_pausado) {
  if (flujo)
    return true;
  if (!SDL_InitSubSystem(SDL_INIT_AUDIO))
    return false;
  const SDL_AudioSpec formato{SDL_AUDIO_S16, 2, 48000};
  const SDL_AudioSpec formato_efectos{SDL_AUDIO_S16, 1, 48000};
  dispositivo = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &formato);
  if (!dispositivo)
    return false;
  grabacion_silenciosa = SDL_getenv("CURSE_REBORN_GRABAR_RUTA") != nullptr;
  if (grabacion_silenciosa && !SDL_SetAudioPostmixCallback(dispositivo, registrar_mezcla, this)) {
    cerrar();
    return false;
  }
  if (iniciar_pausado && !SDL_PauseAudioDevice(dispositivo)) {
    cerrar();
    return false;
  }
  flujo = SDL_CreateAudioStream(&formato_efectos, &formato);
  if (!flujo || !SDL_BindAudioStream(dispositivo, flujo)) {
    cerrar();
    return false;
  }
  size_t longitud = 0;
  void *archivo = SDL_LoadFile((carpeta + "/../CURSE.EXE").c_str(), &longitud);
  carpeta_datos = carpeta;
  const bool preparada = archivo && longitud <= 0xffffffffu &&
                         musica.iniciar(static_cast<const unsigned char *>(archivo),
                                        static_cast<unsigned int>(longitud));
  const bool efectos_preparados =
      preparada &&
      motor::leer_datos_iniciales(static_cast<const unsigned char *>(archivo),
                                  static_cast<unsigned int>(longitud), referencias) &&
      cargar_efectos(0xffff);
  if (preparada && mt32.iniciar(carpeta, static_cast<const unsigned char *>(archivo),
                                static_cast<unsigned int>(longitud)))
    perfil = conservar_seleccion ? perfil : 3;
  else {
    perfil = conservar_seleccion && perfil != 3 ? perfil : 1;
    std::printf("MT-32 no disponible: revisa MT32_CONTROL.ROM y MT32_PCM.ROM en "
                "apps/juego/datos_originales/ROMS.\n");
  }
  SDL_free(archivo);
  if (!preparada || !efectos_preparados) {
    cerrar();
    return false;
  }
  flujo_musical = SDL_CreateAudioStream(&formato, &formato);
  if (!flujo_musical || !SDL_SetAudioStreamGetCallback(flujo_musical, alimentar_musica, this) ||
      !SDL_BindAudioStream(dispositivo, flujo_musical)) {
    cerrar();
    return false;
  }
  ultimo = SDL_GetTicksNS();
  resto = 0;
  fallo_musical.store(false);
  muestras_producidas.store(0);
  muestras_con_senal.store(0);
  if (!SDL_SetAudioDeviceGain(dispositivo, silencio ? 0.0f : volumen / 100.0f) ||
      (!iniciar_pausado && !SDL_ResumeAudioDevice(dispositivo))) {
    cerrar();
    return false;
  }
  constexpr const char *nombres[] = {"sin música", "AdLib OPL2", "Sound Blaster 16 OPL3", "MT-32"};
  std::printf("Audio: %s + %s (efectos). F4 cambia música; F3 cambia efectos; M silencia.\n",
              nombres[perfil], efectos_digitales ? "Sound Blaster digital" : "PC Speaker");
  std::fflush(stdout);
  registrar_volumen();
  return true;
}

// La pausa afecta a los dos flujos del dispositivo sin cambiar el ajuste de M ni el volumen.
bool Audio::pausar_reinicio() {
  return !dispositivo || SDL_PauseAudioDevice(dispositivo);
}

// Reabre los sintetizadores para reiniciar sus secuencias y conserva las preferencias de sesión.
bool Audio::reiniciar() {
  if (!flujo)
    return true;
  const auto carpeta = carpeta_datos;
  cerrar();
  return abrir(carpeta, true);
}

// SDL espera al callback antes de liberar su sintetizador y el dispositivo compartido.
void Audio::cerrar() {
  if (dispositivo && grabacion_silenciosa)
    SDL_PauseAudioDevice(dispositivo);
  terminar_grabacion();
  if (flujo_musical)
    SDL_DestroyAudioStream(flujo_musical);
  flujo_musical = nullptr;
  if (flujo)
    SDL_DestroyAudioStream(flujo);
  flujo = nullptr;
  datos = nullptr;
  digital.conectar(nullptr, 0, nullptr, 0);
  banco_general.clear();
  banco_escena.clear();
  if (dispositivo)
    SDL_CloseAudioDevice(dispositivo);
  dispositivo = 0;
  mt32.cerrar();
}

// El callback tiene el bloqueo del flujo: los cambios de banco y perfil usan el mismo bloqueo.
void Audio::alimentar_musica(void *contexto, SDL_AudioStream *salida, int adicional, int) {
  auto &audio = *static_cast<Audio *>(contexto);
  int pendientes = (adicional + 3) / 4;
  short fm[512], fm_opl2[512], roland[1024], mezcla[1024];
  while (pendientes > 0) {
    const auto cantidad = static_cast<unsigned int>(pendientes > 512 ? 512 : pendientes);
    bool correcto = audio.musica.generar(fm, cantidad, fm_opl2);
    if (audio.mt32.disponible())
      correcto = correcto && audio.mt32.generar(roland, cantidad);
    unsigned int con_senal = 0;
    for (unsigned int i = 0; i < cantidad * 2; ++i) {
      mezcla[i] = !correcto || audio.silencio || audio.perfil == 0 ? 0
                  : audio.perfil == 3                              ? roland[i]
                  : audio.perfil == 1                              ? fm_opl2[i / 2]
                                                                   : fm[i / 2];
      con_senal += mezcla[i] > 64 || mezcla[i] < -64 ? 1u : 0u;
    }
    audio.muestras_con_senal.fetch_add(con_senal);
    if (!correcto || !SDL_PutAudioStreamData(salida, mezcla, static_cast<int>(cantidad * 4))) {
      audio.fallo_musical.store(true);
      return;
    }
    audio.muestras_producidas.fetch_add(cantidad);
    pendientes -= static_cast<int>(cantidad);
  }
}

// Prepara efectos de una sesión nueva sin reiniciar ni vaciar la música en curso.
bool Audio::conectar(unsigned char *segmento) {
  if (!flujo)
    return true;
  if (!segmento || !actualizar())
    return false;
  datos = segmento;
  altavoz.reiniciar();
  if (!altavoz.detener(datos))
    return false;
  datos[0x9924] = 0;
  datos[0x0ad4] = efectos_digitales ? 3 : 1;
  return cambiar_escena(static_cast<unsigned short>(datos[0x8082] | (datos[0x8083] << 8)));
}

// Desconecta solo los efectos: la música mantiene su búfer y su reloj durante la carga de imágenes.
void Audio::desconectar() {
  digital.detener();
  if (datos) {
    altavoz.detener(datos);
    datos[0x0ad4] = datos[0x0ad3] = 0;
  }
  datos = nullptr;
}

// Toda escena consulta las mismas tablas; los bancos repetidos no se reinician.
bool Audio::cambiar_escena(unsigned short escena) {
  if (!flujo_musical)
    return true;
  if (!cargar_efectos(escena) || !SDL_LockAudioStream(flujo_musical))
    return false;
  const bool correcto = !mt32.disponible() || mt32.cambiar_escena(escena);
  if (datos)
    datos[0x0ad3] = perfil == 3 ? 4 : perfil != 0 ? 2 : 0;
  SDL_UnlockAudioStream(flujo_musical);
  return correcto;
}

// Solo los efectos acceden a DS desde el hilo principal; las imágenes no detienen el flujo musical.
bool Audio::actualizar() {
  if (!flujo)
    return true;
  if (fallo_musical.load() || fallo_grabacion.load())
    return false;
  const auto ahora = SDL_GetTicksNS();
  auto transcurrido = ahora - ultimo;
  ultimo = ahora;
  if (transcurrido > 5000000000ull) {
    if (datos) {
      altavoz.detener(datos);
      datos[0x9924] = 0;
    }
    digital.detener();
    transcurrido = resto = 0;
  }
  const auto reloj = transcurrido * 48000u + resto;
  auto pendientes = reloj / 1000000000u;
  resto = reloj % 1000000000u;
  if (SDL_GetAudioStreamQueued(flujo) > 9600 && !SDL_ClearAudioStream(flujo))
    return false;
  short muestras[1024];
  while (pendientes) {
    const auto cantidad = static_cast<unsigned int>(pendientes > 1024 ? 1024 : pendientes);
    if (datos) {
      if (efectos_digitales)
        digital.generar(muestras, cantidad);
      else if (!altavoz.generar(datos, muestras, cantidad))
        return false;
    }
    if (!datos || silencio)
      for (unsigned int i = 0; i < cantidad; ++i)
        muestras[i] = 0;
    pendientes -= cantidad;
    if (pendientes < 2400 &&
        !SDL_PutAudioStreamData(flujo, muestras, static_cast<int>(cantidad * 2)))
      return false;
  }
  return true;
}

// El segmento debe pertenecer a la sesión conectada para no mezclar partidas de desarrollo.
bool Audio::reproducir(unsigned char *segmento, unsigned short entidad,
                       unsigned char identificador) {
  if (segmento[0x0ad4] == 0)
    return true;
  if (datos != segmento || !actualizar())
    return false;
  return efectos_digitales ? digital.reproducir(datos, entidad, identificador)
                           : altavoz.reproducir(datos, entidad, identificador);
}

// El cambio de ganancia evita vaciar muestras y mantiene la continuidad del dispositivo.
bool Audio::alternar_silencio() {
  if (flujo_musical && !SDL_LockAudioStream(flujo_musical))
    return false;
  silencio = !silencio;
  if (flujo_musical)
    SDL_UnlockAudioStream(flujo_musical);
  std::printf("Audio: %s.\n", silencio ? "silenciado" : "reactivado");
  std::fflush(stdout);
  registrar_volumen();
  return !dispositivo || SDL_SetAudioDeviceGain(dispositivo, silencio ? 0.0f : volumen / 100.0f);
}

// El volumen se aplica al dispositivo compartido, incluyendo música y efectos de cualquier perfil.
bool Audio::ajustar_volumen(int cambio) {
  int siguiente = static_cast<int>(volumen) + cambio;
  if (siguiente < 0)
    siguiente = 0;
  if (siguiente > 400)
    siguiente = 400;
  if (dispositivo && !SDL_SetAudioDeviceGain(dispositivo, silencio ? 0.0f : siguiente / 100.0f))
    return false;
  volumen = static_cast<unsigned int>(siguiente);
  registrar_volumen();
  return true;
}

// El silencio muestra cero efectivo y permite recordar el porcentaje que se recuperará con M.
void Audio::registrar_volumen() const {
  if (silencio)
    std::printf("[partida] Volumen: 0%% (silenciado con M; ajuste guardado: %u%%).\n", volumen);
  else
    std::printf("[partida] Volumen: %u%%.\n", volumen);
  std::fflush(stdout);
}

// F4 solo ofrece dispositivos preparados; cada cambio se informa por consola.
bool Audio::alternar_dispositivo() {
  if (!flujo_musical) {
    std::printf("Audio desactivado o sin dispositivo.\n");
    return true;
  }
  if (!SDL_LockAudioStream(flujo_musical))
    return false;
  perfil = (perfil + 1) % (mt32.disponible() ? 4u : 3u);
  if (datos)
    datos[0x0ad3] = perfil == 3 ? 4 : perfil != 0 ? 2 : 0;
  std::printf("Audio cambiado a %s; efectos: %s%s.\n",
              perfil == 3   ? "MT-32 (música por escena)"
              : perfil == 2 ? "Sound Blaster 16 OPL3 (tema continuo original)"
              : perfil == 1 ? "AdLib OPL2 (tema continuo original)"
                            : "sin música",
              efectos_digitales ? "Sound Blaster digital" : "PC Speaker",
              silencio ? " [silenciado con M]" : "");
  std::fflush(stdout);
  SDL_UnlockAudioStream(flujo_musical);
  return true;
}

// La tecla F3 solo afecta al dispositivo de efectos y conserva el perfil musical y su reloj.
bool Audio::alternar_efectos() {
  if (!flujo)
    return true;
  if (!actualizar())
    return false;
  if (datos && !altavoz.detener(datos))
    return false;
  digital.detener();
  efectos_digitales = !efectos_digitales;
  if (datos) {
    datos[0x9924] = 0;
    datos[0x0ad4] = efectos_digitales ? 3 : 1;
  }
  std::printf("[partida] Efectos: %s.\n",
              efectos_digitales ? "Sound Blaster digital" : "PC Speaker");
  std::fflush(stdout);
  return SDL_ClearAudioStream(flujo);
}
// 038E/4FDF: sustituye únicamente los servicios DOS de lectura; no transforma los recursos.
bool Audio::cargar_banco_efectos(const char *nombre, std::vector<unsigned char> &destino) {
  if (!nombre[0]) {
    destino.clear();
    return true;
  }
  std::string nombre_dos = nombre;
  for (auto &letra : nombre_dos)
    if (letra >= 'a' && letra <= 'z')
      letra = static_cast<char>(letra - 'a' + 'A');
  size_t longitud = 0;
  auto *archivo = SDL_LoadFile((carpeta_datos + "/" + nombre_dos).c_str(), &longitud);
  if (!archivo || longitud > 65536 || longitud < 2) {
    SDL_free(archivo);
    return false;
  }
  const auto *bytes = static_cast<const unsigned char *>(archivo);
  destino.assign(bytes, bytes + longitud);
  SDL_free(archivo);
  return true;
}
// 4FD1: recarga el banco antes de atender las entidades de la nueva escena y cancela el anterior.
bool Audio::cargar_efectos(unsigned short escena) {
  char nombre[13]{};
  digital.detener();
  if (banco_general.empty() && (!motor::nombre_banco_sonido(referencias, 0xffff, nombre) ||
                                !cargar_banco_efectos(nombre, banco_general)))
    return false;
  if (escena == 0xffff)
    banco_escena.clear();
  else if (!motor::nombre_banco_sonido(referencias, escena, nombre) ||
           !cargar_banco_efectos(nombre, banco_escena))
    return false;
  digital.conectar(banco_general.data(), static_cast<unsigned int>(banco_general.size()),
                   banco_escena.empty() ? nullptr : banco_escena.data(),
                   static_cast<unsigned int>(banco_escena.size()));
  if (escena != 0xffff) {
    std::printf("[partida] Banco de efectos: %s; generales: STD.SFX.\n",
                nombre[0] ? nombre : "ninguno");
    std::fflush(stdout);
  }
  return true;
}

namespace {
// Escribe los campos RIFF en orden little endian, independientemente del ordenador anfitrión.
void campo_wav(unsigned char *cabecera, unsigned int posicion, unsigned int valor,
               unsigned int bytes) {
  for (unsigned int i = 0; i < bytes; ++i)
    cabecera[posicion + i] = static_cast<unsigned char>(valor >> (i * 8));
}
} // namespace

// Contrasta ocho segundos de música real en un bloque y en fragmentos de tamaños distintos.
bool comprobar_musica_adlib(const std::string &carpeta, const char *ruta, unsigned short tema) {
  size_t longitud = 0;
  void *archivo = SDL_LoadFile((carpeta + "/../CURSE.EXE").c_str(), &longitud);
  if (!archivo || longitud > 0xffffffffu) {
    SDL_free(archivo);
    return false;
  }
  static MusicaAdlib continua, fragmentada;
  const bool preparadas = continua.iniciar(static_cast<const unsigned char *>(archivo),
                                           static_cast<unsigned int>(longitud), tema) &&
                          fragmentada.iniciar(static_cast<const unsigned char *>(archivo),
                                              static_cast<unsigned int>(longitud), tema);
  SDL_free(archivo);
  if (!preparadas)
    return false;
  constexpr unsigned int cantidad = 48000 * 8;
  std::vector<short> primera(cantidad), segunda(cantidad), primera_opl2(cantidad),
      segunda_opl2(cantidad);
  if (!continua.generar(primera.data(), cantidad, primera_opl2.data()))
    return false;
  constexpr unsigned int tamanos[] = {1, 682, 683, 1024, 4096};
  unsigned int posicion = 0, bloque = 0;
  while (posicion < cantidad) {
    auto tamano = tamanos[bloque++ % 5];
    if (tamano > cantidad - posicion)
      tamano = cantidad - posicion;
    if (!fragmentada.generar(segunda.data() + posicion, tamano, segunda_opl2.data() + posicion))
      return false;
    posicion += tamano;
  }
  unsigned int no_nulas = 0, no_nulas_opl2 = 0, distintas = 0;
  for (unsigned int i = 0; i < cantidad; ++i) {
    if (primera[i] != segunda[i] || primera_opl2[i] != segunda_opl2[i]) {
      std::fprintf(stderr, "Partición FM distinta en muestra %u: OPL3 %d/%d, OPL2 %d/%d.\n", i,
                   primera[i], segunda[i], primera_opl2[i], segunda_opl2[i]);
      return false;
    }
    no_nulas += primera[i] != 0 ? 1u : 0u;
    no_nulas_opl2 += primera_opl2[i] != 0 ? 1u : 0u;
    distintas += primera[i] != primera_opl2[i] ? 1u : 0u;
  }
  constexpr auto pulsos = static_cast<unsigned long long>(cantidad) * 14318180 / 576000 / 0x4250;
  if (no_nulas < cantidad / 4 || no_nulas_opl2 < cantidad / 4 || distintas < cantidad / 10 ||
      continua.pulsos() != pulsos || fragmentada.pulsos() != pulsos) {
    std::fprintf(
        stderr, "Señal FM: OPL3 %u, OPL2 %u, distintas %u, IRQ %llu/%llu, previstas %llu.\n",
        no_nulas, no_nulas_opl2, distintas, continua.pulsos(), fragmentada.pulsos(), pulsos);
    return false;
  }
  unsigned char cabecera[44] = {
      'R', 'I', 'F', 'F', 0, 0, 0, 0, 'W', 'A', 'V', 'E', 'f', 'm', 't', ' ', 16,  0,   0, 0, 1, 0,
      1,   0,   0,   0,   0, 0, 0, 0, 0,   0,   2,   0,   16,  0,   'd', 'a', 't', 'a', 0, 0, 0, 0};
  campo_wav(cabecera, 4, cantidad * 2 + 36, 4);
  campo_wav(cabecera, 24, 48000, 4);
  campo_wav(cabecera, 28, 96000, 4);
  campo_wav(cabecera, 40, cantidad * 2, 4);
  std::vector<unsigned char> bytes(cantidad * 2);
  for (unsigned int i = 0; i < cantidad; ++i)
    campo_wav(bytes.data(), i * 2, static_cast<unsigned short>(primera[i]), 2);
  auto *salida = SDL_IOFromFile(ruta, "wb");
  if (!salida)
    return false;
  const bool guardado = SDL_WriteIO(salida, cabecera, 44) == 44 &&
                        SDL_WriteIO(salida, bytes.data(), bytes.size()) == bytes.size();
  const bool cerrado = SDL_CloseIO(salida);
  for (unsigned int i = 0; i < cantidad; ++i)
    campo_wav(bytes.data(), i * 2, static_cast<unsigned short>(primera_opl2[i]), 2);
  auto *salida_opl2 = SDL_IOFromFile((std::string(ruta) + ".opl2.wav").c_str(), "wb");
  if (!salida_opl2)
    return false;
  const bool guardado_opl2 = SDL_WriteIO(salida_opl2, cabecera, 44) == 44 &&
                             SDL_WriteIO(salida_opl2, bytes.data(), bytes.size()) == bytes.size();
  const bool cerrado_opl2 = SDL_CloseIO(salida_opl2);
  if (!guardado_opl2 || !cerrado_opl2)
    return false;
  std::printf(
      "OPL2 y OPL3: %u muestras distintas entre chips; ambas salidas coinciden al fragmentar.\n",
      distintas);
  // Ejercita la salida común, el cambio de opción y su continuidad al desconectar una escena.
  Audio audio;
  static unsigned char segmento[65536]{};
  if (!guardado || !cerrado || !audio.abrir(carpeta))
    return false;
  SDL_Delay(25);
  bool integrada = audio.actualizar() && audio.conectar(segmento);
  const bool con_mt32 = segmento[0x0ad3] == 4;
  if (!con_mt32)
    integrada = integrada && audio.alternar_dispositivo();
  integrada = integrada && (segmento[0x0ad3] == 2 || con_mt32) && audio.alternar_dispositivo() &&
              segmento[0x0ad3] == 0 && audio.alternar_silencio();
  SDL_Delay(25);
  integrada =
      integrada && audio.actualizar() && audio.alternar_dispositivo() && segmento[0x0ad3] == 2;
  integrada = integrada && audio.alternar_silencio();
  const auto senal_anterior = audio.muestras_audibles();
  SDL_Delay(300);
  integrada = integrada && audio.muestras_audibles() > senal_anterior;
  integrada = integrada && audio.alternar_dispositivo() && segmento[0x0ad3] == 2;
  if (con_mt32)
    integrada = integrada && audio.alternar_dispositivo() && segmento[0x0ad3] == 4;
  audio.desconectar();
  const auto antes_de_carga = audio.muestras_musicales();
  SDL_Delay(150);
  integrada = integrada && audio.muestras_musicales() > antes_de_carga;
  SDL_Delay(25);
  integrada = integrada && audio.actualizar();
  audio.cerrar();
  if (!integrada)
    return false;
  if (guardado && cerrado)
    std::printf("Música AdLib: %u muestras idénticas entre bloques, %llu IRQ; WAV: %s\n", cantidad,
                pulsos, ruta);
  return guardado && cerrado;
}

// Valida formato, señal no vacía, alimentación y arranque usando el mismo API que la partida.
bool comprobar_salida_audio(const char *ruta) {
  SDL_AudioSpec formato{};
  Uint8 *muestras = nullptr;
  Uint32 longitud = 0;
  if (!SDL_InitSubSystem(SDL_INIT_AUDIO) || !SDL_LoadWAV(ruta, &formato, &muestras, &longitud))
    return false;
  bool audible = false;
  for (Uint32 i = 0; i < longitud; ++i)
    audible = audible || muestras[i] != 0;
  auto *flujo =
      SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &formato, nullptr, nullptr);
  const bool correcto = flujo && audible && formato.format == SDL_AUDIO_S16 &&
                        (formato.channels == 1 || formato.channels == 2) && formato.freq == 48000 &&
                        SDL_PutAudioStreamData(flujo, muestras, static_cast<int>(longitud)) &&
                        SDL_GetAudioStreamQueued(flujo) == static_cast<int>(longitud) &&
                        SDL_ResumeAudioStreamDevice(flujo);
  SDL_free(muestras);
  if (flujo)
    SDL_DestroyAudioStream(flujo);
  if (correcto)
    std::printf("Salida SDL: PCM de 48.000 Hz aceptado por el dispositivo.\n");
  return correcto;
}

// Recorre 32 segundos de cada banco, incluida la percusión posterior al arranque de RS1.
bool comprobar_musica_mt32(const std::string &carpeta, const char *ruta) {
  size_t longitud = 0;
  void *archivo = SDL_LoadFile((carpeta + "/../CURSE.EXE").c_str(), &longitud);
  static MusicaMt32 sintetizador;
  const bool preparado = archivo && longitud <= 0xffffffffu &&
                         sintetizador.iniciar(carpeta, static_cast<const unsigned char *>(archivo),
                                              static_cast<unsigned int>(longitud));
  SDL_free(archivo);
  if (!preparado)
    return false;
  constexpr unsigned short escenas[] = {0, 64, 3, 15, 0xffff};
  constexpr unsigned int cantidad = 48000 * 32;
  std::vector<short> muestras(cantidad * 2);
  // Mantiene voces de la presentación antes de cambiar de banco, como en una partida real.
  if (!sintetizador.generar(muestras.data(), 48000 * 8)) {
    sintetizador.cerrar();
    return false;
  }
  bool guardado = false;
  for (unsigned int banco = 0; banco < 5; ++banco) {
    if (!sintetizador.cambiar_escena(escenas[banco]) || sintetizador.banco() != banco ||
        !sintetizador.generar(muestras.data(), cantidad)) {
      sintetizador.cerrar();
      return false;
    }
    unsigned int audibles = 0, limitadas = 0, pico = 0;
    for (const auto valor : muestras) {
      audibles += valor > 64 || valor < -64 ? 1u : 0u;
      const auto amplitud = static_cast<unsigned int>(valor < 0 ? -static_cast<int>(valor) : valor);
      if (amplitud > pico)
        pico = amplitud;
      limitadas += amplitud == 28000 ? 1u : 0u;
    }
    if (audibles < 100) {
      sintetizador.cerrar();
      return false;
    }
    std::printf("MT-32: CURSE.RS%u genera %u muestras con señal; pico %u; muestras limitadas %u.\n",
                banco + 1, audibles, pico, limitadas);
    if (banco == 0) {
      if (!sintetizador.cambiar_escena(1) || sintetizador.banco() != 0 ||
          !sintetizador.cambiar_escena(2) || sintetizador.banco() != 0) {
        sintetizador.cerrar();
        return false;
      }
      unsigned char cabecera[44] = {'R', 'I', 'F', 'F', 0,   0,   0,   0, 'W', 'A', 'V',
                                    'E', 'f', 'm', 't', ' ', 16,  0,   0, 0,   1,   0,
                                    1,   0,   0,   0,   0,   0,   0,   0, 0,   0,   2,
                                    0,   16,  0,   'd', 'a', 't', 'a', 0, 0,   0,   0};
      campo_wav(cabecera, 4, cantidad * 4 + 36, 4);
      campo_wav(cabecera, 22, 2, 2);
      campo_wav(cabecera, 32, 4, 2);
      campo_wav(cabecera, 24, 48000, 4);
      campo_wav(cabecera, 28, 192000, 4);
      campo_wav(cabecera, 40, cantidad * 4, 4);
      std::vector<unsigned char> bytes(cantidad * 4);
      for (unsigned int i = 0; i < cantidad * 2; ++i)
        campo_wav(bytes.data(), i * 2, static_cast<unsigned short>(muestras[i]), 2);
      auto *salida = SDL_IOFromFile(ruta, "wb");
      if (!salida) {
        sintetizador.cerrar();
        return false;
      }
      guardado = SDL_WriteIO(salida, cabecera, 44) == 44 &&
                 SDL_WriteIO(salida, bytes.data(), bytes.size()) == bytes.size();
      guardado = SDL_CloseIO(salida) && guardado;
    }
  }
  sintetizador.cerrar();
  return guardado;
}

// Comprueba el servicio usado por la partida, incluidos F3, F4 y M, sin exigir hardware de audio.
bool comprobar_efectos_digitales(const std::string &carpeta, const char *ruta) {
  Audio audio;
  static unsigned char datos[65536]{};
  size_t longitud = 0;
  void *archivo = SDL_LoadFile((carpeta + "/../CURSE.EXE").c_str(), &longitud);
  const bool preparado = archivo && longitud <= 0xffffffffu &&
                         motor::leer_datos_iniciales(static_cast<const unsigned char *>(archivo),
                                                     static_cast<unsigned int>(longitud), datos);
  SDL_free(archivo);
  if (!preparado || !audio.abrir(carpeta))
    return false;
  datos[0x2c4e] = 100;
  datos[0x2c50] = 100;
  datos[0x2c52] = 32;
  datos[0x2c54] = 48;
  bool correcto = true;
  for (unsigned short escena = 0; escena < 4 && correcto; ++escena) {
    datos[0x8082] = static_cast<unsigned char>(escena);
    datos[0x8083] = 0;
    correcto = audio.conectar(datos) && datos[0x0ad4] == 3 && audio.reproducir(datos, 0x2c4c, 15);
    SDL_Delay(30);
    correcto = correcto && audio.actualizar() && audio.alternar_efectos() && datos[0x0ad4] == 1 &&
               audio.reproducir(datos, 0x2c4c, 15) && audio.alternar_efectos() &&
               datos[0x0ad4] == 3 && audio.alternar_dispositivo() && datos[0x0ad4] == 3 &&
               audio.ajustar_volumen(-40) && audio.alternar_silencio() &&
               audio.reproducir(datos, 0x2c4c, 0);
    SDL_Delay(30);
    correcto =
        correcto && audio.actualizar() && audio.alternar_silencio() && audio.ajustar_volumen(40);
    audio.desconectar();
    correcto = correcto && datos[0x0ad4] == 0;
  }
  correcto = correcto && audio.ajustar_volumen(999) && audio.nivel_volumen() == 400 &&
             audio.alternar_silencio() && audio.nivel_volumen() == 400 &&
             audio.alternar_silencio() && audio.ajustar_volumen(-999) &&
             audio.nivel_volumen() == 0 && audio.ajustar_volumen(100) &&
             audio.nivel_volumen() == 100;
  const auto antes = audio.muestras_musicales();
  SDL_Delay(150);
  correcto = correcto && audio.muestras_musicales() > antes;
  audio.cerrar();
  if (correcto)
    std::printf("Efectos SDL: cuatro escenas, Sound Blaster/PC Speaker, música, silencio y volumen "
                "comprobados.\n");
  return correcto && comprobar_salida_audio(ruta);
}

namespace {
// La exploración del guion musical no necesita producir señal ni modificar registros del chip.
void omitir_registro_exportacion(void *, unsigned short, unsigned char) {}
// FM tiene nueve bucles independientes: termina cuando todos han repetido su estado de control.
unsigned int longitud_tema_fm(const std::vector<unsigned char> &ejecutable, unsigned short tema) {
  unsigned char codigo[65536];
  for (unsigned int i = 0; i < 65536; ++i)
    codigo[i] = ejecutable[512 + i];
  if (!motor::iniciar_musica(codigo, tema))
    return 0;
  std::unordered_set<std::string> estados[9];
  bool completos[9]{};
  constexpr motor::SalidaAdlib salida{nullptr, omitir_registro_exportacion};
  for (unsigned int irq = 0; irq < 43000; ++irq) {
    if (motor::palabra(codigo, 0x8e5b) == 1) {
      bool terminado = true;
      for (unsigned short canal = 0; canal < 9; ++canal) {
        const auto posicion = static_cast<unsigned short>(0x8ce2 + canal * 17);
        if (!completos[canal]) {
          const std::string clave(reinterpret_cast<const char *>(codigo + posicion), 17);
          completos[canal] = motor::palabra(codigo, posicion + 11) == 0xffff ||
                             !estados[canal].insert(clave).second;
        }
        terminado = terminado && completos[canal];
      }
      if (terminado)
        return static_cast<unsigned int>(
            (static_cast<unsigned long long>(irq) * 0x4250 * 576000 + 14318179) / 14318180);
    }
    if (!motor::avanzar_musica(codigo, salida))
      return 0;
  }
  return 0;
}
// RS contiene un salto FF al principio del tramo repetido; incluye la entrada antes de ese salto.
unsigned int longitud_banco_rs(const std::string &carpeta, unsigned int indice,
                               unsigned char tempo) {
  size_t longitud = 0;
  auto *archivo =
      SDL_LoadFile((carpeta + "/CURSE.RS" + std::to_string(indice + 1)).c_str(), &longitud);
  if (!archivo || longitud > 65536 || !tempo) {
    SDL_free(archivo);
    return 0;
  }
  const auto *bytes = static_cast<const unsigned char *>(archivo);
  unsigned short posicion = 0;
  unsigned char estado = 0;
  unsigned long long pulsos = 0;
  unsigned int cantidad = 0;
  for (unsigned int evento = 0; evento < 100000 && posicion < longitud; ++evento) {
    if (bytes[posicion] == 0xff && posicion + 2u < longitud &&
        motor::palabra(bytes, posicion + 1) < posicion) {
      cantidad = static_cast<unsigned int>((pulsos * 24000 + tempo - 1) / tempo);
      break;
    }
    motor::EventoMusicaRS orden{};
    if (!motor::leer_evento_musica_rs(bytes, static_cast<unsigned int>(longitud), posicion, estado,
                                      orden))
      break;
    pulsos += orden.tiempo;
    if (pulsos * 24000 > static_cast<unsigned long long>(tempo) * 48000 * 600)
      break;
  }
  SDL_free(archivo);
  return cantidad;
}
// Guarda PCM de 48 kHz con cabecera RIFF explícita para evitar depender del orden de bytes
// anfitrión.
bool guardar_pista_wav(const std::string &ruta, const std::vector<short> &muestras,
                       unsigned int canales) {
  unsigned char cabecera[44] = {
      'R', 'I', 'F', 'F', 0, 0, 0, 0, 'W', 'A', 'V', 'E', 'f', 'm', 't', ' ', 16,  0,   0, 0, 1, 0,
      0,   0,   0,   0,   0, 0, 0, 0, 0,   0,   0,   0,   16,  0,   'd', 'a', 't', 'a', 0, 0, 0, 0};
  const auto longitud = static_cast<unsigned int>(muestras.size()) * 2;
  campo_wav(cabecera, 4, longitud + 36, 4);
  campo_wav(cabecera, 22, canales, 2);
  campo_wav(cabecera, 24, 48000, 4);
  campo_wav(cabecera, 28, 48000 * canales * 2, 4);
  campo_wav(cabecera, 32, canales * 2, 2);
  campo_wav(cabecera, 40, longitud, 4);
  auto *archivo = SDL_IOFromFile(ruta.c_str(), "wb");
  if (!archivo)
    return false;
  bool correcto = SDL_WriteIO(archivo, cabecera, 44) == 44;
  unsigned char bloque[4096];
  for (unsigned int inicio = 0; correcto && inicio < muestras.size();) {
    auto cantidad = static_cast<unsigned int>(muestras.size()) - inicio;
    if (cantidad > 2048)
      cantidad = 2048;
    for (unsigned int i = 0; i < cantidad; ++i)
      campo_wav(bloque, i * 2, static_cast<unsigned short>(muestras[inicio + i]), 2);
    correcto = SDL_WriteIO(archivo, bloque, cantidad * 2) == cantidad * 2;
    inicio += cantidad;
  }
  return SDL_CloseIO(archivo) && correcto;
}
} // namespace

// Lee únicamente recursos originales locales y renderiza sin abrir un dispositivo de reproducción.
bool exportar_musica_wav(const std::string &carpeta, const std::string &destino,
                         unsigned int segundos) {
  if (segundos > 600)
    return false;
  size_t longitud = 0;
  auto *archivo = SDL_LoadFile((carpeta + "/../CURSE.EXE").c_str(), &longitud);
  if (!archivo || longitud > 0xffffffffu) {
    SDL_free(archivo);
    return false;
  }
  const auto *bytes = static_cast<const unsigned char *>(archivo);
  std::vector<unsigned char> ejecutable(bytes, bytes + longitud);
  SDL_free(archivo);
  unsigned char datos_originales[65536];
  if (!motor::leer_datos_iniciales(ejecutable.data(), static_cast<unsigned int>(longitud),
                                   datos_originales))
    return false;
  MusicaAdlib fm;
  std::vector<short> opl3, opl2;
  for (unsigned short tema = 0; tema < 2; ++tema) {
    const auto cantidad = segundos ? segundos * 48000 : longitud_tema_fm(ejecutable, tema);
    if (!cantidad)
      return false;
    opl3.resize(cantidad);
    opl2.resize(cantidad);
    if (!fm.iniciar(ejecutable.data(), static_cast<unsigned int>(longitud), tema) ||
        !fm.generar(opl3.data(), cantidad, opl2.data()) ||
        !guardar_pista_wav(destino + "/adlib_opl2_tema_" + std::to_string(tema) + ".wav", opl2,
                           1) ||
        !guardar_pista_wav(destino + "/sb16_opl3_tema_" + std::to_string(tema) + ".wav", opl3, 1))
      return false;
    std::printf("Exportados OPL2 y OPL3: tema %u, %.3f segundos.\n", tema, cantidad / 48000.0);
    std::fflush(stdout);
  }
  MusicaMt32 roland;
  constexpr unsigned short escenas[] = {0, 64, 3, 15, 0xffff};
  std::vector<short> muestras;
  for (unsigned int banco = 0; banco < 5; ++banco) {
    const auto referencia = motor::seleccionar_musica_rs(datos_originales, escenas[banco]);
    const auto cantidad =
        segundos ? segundos * 48000 : longitud_banco_rs(carpeta, banco, referencia.tempo);
    if (!cantidad)
      return false;
    muestras.resize(cantidad * 2);
    const bool correcto =
        roland.iniciar(carpeta, ejecutable.data(), static_cast<unsigned int>(longitud),
                       escenas[banco]) &&
        roland.generar(muestras.data(), cantidad) &&
        guardar_pista_wav(destino + "/mt32_curse_rs" + std::to_string(banco + 1) + ".wav", muestras,
                          2);
    roland.cerrar();
    if (!correcto)
      return false;
    std::printf("Exportado MT-32: CURSE.RS%u, %.3f segundos.\n", banco + 1, cantidad / 48000.0);
    std::fflush(stdout);
  }
  return true;
}

// El postmix recibe flotantes despues de ganancias y mezcla; no sustituye ningun sintetizador.
void SDLCALL Audio::registrar_mezcla(void *contexto, const SDL_AudioSpec *formato, float *muestras,
                                     int bytes) {
  auto &a = *static_cast<Audio *>(contexto);
  if (a.grabacion && (formato->freq != a.formato_grabacion.freq ||
                      formato->channels != a.formato_grabacion.channels || bytes < 0 ||
                      SDL_WriteIO(a.grabacion, muestras, static_cast<size_t>(bytes)) !=
                          static_cast<size_t>(bytes)))
    a.fallo_grabacion.store(true);
  // La copia conserva musica y efectos; solo la salida fisica de este juego se pone a cero.
  if (a.grabacion_silenciosa && bytes > 0)
    SDL_memset(muestras, 0, static_cast<size_t>(bytes));
}

// Reserva una cabecera WAV IEEE float; SDL sincroniza la instalacion del callback con el
// dispositivo.
bool Audio::grabar_salida(const char *ruta) {
  if (!dispositivo || grabacion ||
      !SDL_GetAudioDeviceFormat(dispositivo, &formato_grabacion, nullptr))
    return false;
  const bool pausado = SDL_AudioDevicePaused(dispositivo);
  if (!pausado && !SDL_PauseAudioDevice(dispositivo))
    return false;
  // Detiene el callback antes de cambiar su archivo; la pausa evita cualquier salida intermedia.
  if (!SDL_SetAudioPostmixCallback(dispositivo, nullptr, nullptr))
    return false;
  grabacion = SDL_IOFromFile(ruta, "wb");
  unsigned char cabecera[44]{};
  if (!grabacion || SDL_WriteIO(grabacion, cabecera, sizeof(cabecera)) != sizeof(cabecera)) {
    terminar_grabacion();
    return false;
  }
  fallo_grabacion.store(false);
  if (!SDL_SetAudioPostmixCallback(dispositivo, registrar_mezcla, this)) {
    terminar_grabacion();
    return false;
  }
  return pausado || SDL_ResumeAudioDevice(dispositivo);
}

// Espera al callback antes de cerrar y completa longitudes, canales y frecuencia reales del WAV.
void Audio::terminar_grabacion() {
  if (!grabacion)
    return;
  const bool pausado = SDL_AudioDevicePaused(dispositivo);
  if (!pausado)
    SDL_PauseAudioDevice(dispositivo);
  SDL_SetAudioPostmixCallback(dispositivo, nullptr, nullptr);
  const auto longitud = SDL_TellIO(grabacion);
  unsigned char cabecera[44]{};
  const auto escribir16 = [&](unsigned int p, unsigned int valor) {
    cabecera[p] = static_cast<unsigned char>(valor);
    cabecera[p + 1] = static_cast<unsigned char>(valor >> 8);
  };
  const auto escribir32 = [&](unsigned int p, unsigned int valor) {
    escribir16(p, valor & 65535u);
    escribir16(p + 2, valor >> 16);
  };
  const char firma[] = "RIFF    WAVEfmt ";
  for (unsigned int i = 0; i < 16; ++i)
    cabecera[i] = firma[i];
  cabecera[36] = 'd';
  cabecera[37] = 'a';
  cabecera[38] = 't';
  cabecera[39] = 'a';
  escribir32(4, static_cast<unsigned int>(longitud - 8));
  escribir32(16, 16);
  escribir16(20, 3);
  escribir16(22, static_cast<unsigned int>(formato_grabacion.channels));
  escribir32(24, static_cast<unsigned int>(formato_grabacion.freq));
  escribir32(28,
             static_cast<unsigned int>(formato_grabacion.freq * formato_grabacion.channels * 4));
  escribir16(32, static_cast<unsigned int>(formato_grabacion.channels * 4));
  escribir16(34, 32);
  escribir32(40, static_cast<unsigned int>(longitud - 44));
  if (longitud < 44 || longitud > 0xffffffffll || SDL_SeekIO(grabacion, 0, SDL_IO_SEEK_SET) < 0 ||
      SDL_WriteIO(grabacion, cabecera, sizeof(cabecera)) != sizeof(cabecera))
    std::fprintf(stderr, "No se ha completado la grabacion WAV de la partida\n");
  SDL_CloseIO(grabacion);
  grabacion = nullptr;
  if (grabacion_silenciosa)
    SDL_SetAudioPostmixCallback(dispositivo, registrar_mezcla, this);
  if (!pausado)
    SDL_ResumeAudioDevice(dispositivo);
}
