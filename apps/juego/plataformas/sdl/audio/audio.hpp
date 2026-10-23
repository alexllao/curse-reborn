#pragma once

#include <SDL3/SDL.h>
#include "../../../motor/sonido/pcm.hpp"
#include "../../../motor/sonido/digital.hpp"
#include "musica_adlib.hpp"
#include "musica_mt32.hpp"
#include "musica_remaster.hpp"
#include "estereo_posicional.hpp"
#include <atomic>
#include <string>
#include <vector>

// Salida compartida entre escenas; toda la lógica del juego permanece en el hilo principal.
class Audio {
public:
  // Abre una salida estéreo con efectos posicionados; SDL convierte al formato del dispositivo
  // real.
  bool abrir(const std::string &carpeta, bool conservar_seleccion = false,
             bool iniciar_pausado = false);
  // Detiene la salida durante el apagado visual sin alterar volumen, silencio ni perfiles.
  bool pausar_reinicio();
  bool pausar_menu(bool pausa);
  bool efectos_sound_blaster() const {
    return efectos_digitales;
  }
  // Arranca la música desde el principio conservando las opciones elegidas por el usuario.
  bool reiniciar();
  // Cierra el flujo antes de SDL_Quit, incluso cuando falla la carga de una escena.
  void cerrar();
  // Asocia una sesión nueva, conserva el dispositivo y limpia efectos de la sesión anterior.
  bool conectar(unsigned char *segmento);
  // Deja de atender los efectos de la sesión; el tema continúa en la presentación.
  void desconectar();
  // Produce el tiempo transcurrido, también con menú abierto o durante fundidos.
  bool actualizar();
  // Registra la mezcla final del dispositivo para una partida filmada, sin cambiar lo audible.
  bool grabar_salida(const char *ruta);
  // Retira el callback antes de completar el WAV, conservando el silencio de la toma.
  void terminar_grabacion();
  // Atiende una orden de 5288 después de llevar el reloj del altavoz hasta el momento actual.
  bool reproducir(unsigned char *segmento, unsigned short entidad, unsigned char identificador);
  // Silencia la salida audible; las secuencias y prioridades originales siguen avanzando.
  bool alternar_silencio();
  // Consulta el silencio general para mostrar el estado real de música y efectos en el HUD.
  bool silenciado() const {
    return silencio;
  }
  // Ajusta el volumen general entre cero y cuatrocientos sin alterar los relojes musicales.
  bool ajustar_volumen(int cambio);
  bool ajustar_volumen_musica(int cambio);
  bool ajustar_volumen_efectos(int cambio);
  unsigned int nivel_volumen_musica() const {
    return volumen_musica;
  }
  unsigned int nivel_volumen_efectos() const {
    return volumen_efectos;
  }
  // Permite consultar el ajuste guardado, incluso durante el silencio temporal de M.
  unsigned int nivel_volumen() const {
    return volumen;
  }
  // F4 recorre las opciones musicales; los efectos se eligen por separado con F3.
  bool alternar_dispositivo(int sentido = 1);
  void configurar_musica() {
    usar_configuracion = true;
  }
  void preferir_musica(unsigned int seleccion) {
    perfil_preferido = seleccion;
  }
  std::string descripcion_musica() const;
  // F3 cambia los efectos entre Sound Blaster digital y PC Speaker sin modificar la música.
  bool alternar_efectos();
  void configurar_estereo(bool activar);
  bool estereo_posicional() const {
    return espacial.activo();
  }

  // Selecciona el banco de la escena bajo el bloqueo del flujo musical, sin vaciar sus muestras.
  bool cambiar_escena(unsigned short escena);
  // Identifica la selección musical conservada al reiniciar el monitor.
  unsigned int perfil_musical() const {
    return perfil;
  }
  // Cuenta muestras producidas desde el hilo SDL para comprobar continuidad durante cargas.
  unsigned long long muestras_musicales() const {
    return muestras_producidas.load();
  }

  // Cuenta señal efectiva de la mezcla musical para detectar silencios de integración
  // involuntarios.
  unsigned long long muestras_audibles() const {
    return muestras_con_senal.load();
  }

private:
  SDL_IOStream *grabacion = nullptr;
  SDL_AudioSpec formato_grabacion{};
  std::atomic<bool> fallo_grabacion{false};
  bool grabacion_silenciosa = false;
  static void SDLCALL registrar_mezcla(void *, const SDL_AudioSpec *, float *, int);
  SDL_AudioDeviceID dispositivo = 0;
  SDL_AudioStream *flujo_musical = nullptr;
  MusicaMt32 mt32;
  MusicaRemaster remaster;
  bool usar_configuracion = false;
  unsigned int perfil_preferido = 5;
  std::atomic<bool> fallo_musical{false};
  std::atomic<unsigned long long> muestras_producidas{0}, muestras_con_senal{0};
  unsigned int perfil = 1;
  // Produce solo música; este callback nunca accede al DS ni a las entidades del juego.
  static void alimentar_musica(void *contexto, SDL_AudioStream *flujo, int adicional, int total);
  SDL_AudioStream *flujo = nullptr;
  unsigned char *datos = nullptr;
  motor::AltavozPcm altavoz{};
  motor::DigitalPcm digital{};
  EstereoPosicional espacial;
  bool efectos_digitales = true;
  unsigned char referencias[65536]{};
  std::string carpeta_datos;
  std::vector<unsigned char> banco_general, banco_escena;
  // Carga los SFX mediante los nombres de la tabla original; no deduce nombres del fondo gráfico.
  bool cargar_efectos(unsigned short escena);
  // Lee un archivo de audio acotado y mantiene su propiedad durante la sesión.
  bool cargar_banco_efectos(const char *nombre, std::vector<unsigned char> &destino);
  MusicaAdlib musica{};
  Uint64 ultimo = 0, resto = 0;
  bool menu_pausado = false;
  bool silencio = false;
  unsigned int volumen = 100, volumen_musica = 100, volumen_efectos = 100;
  bool ajustar_canal(SDL_AudioStream *canal, unsigned int &nivel, int cambio, const char *nombre);
  friend bool comprobar_volumenes_audio(const std::string &carpeta);
  // Informa del volumen efectivo y conserva el ajuste elegido mientras M silencia.
  void registrar_volumen() const;
};

// Exporta referencias WAV de duración elegida, sin aplicar el volumen ni el silencio de la partida.
bool exportar_musica_wav(const std::string &carpeta, const std::string &destino,
                         unsigned int segundos);
// Contrasta efectos reales en SDL, el cambio F3, el volumen y la continuidad musical.
bool comprobar_efectos_digitales(const std::string &carpeta, const char *ruta);
// Comprueba la apertura y alimentación SDL con un WAV generado por las pruebas del motor.
bool comprobar_salida_audio(const char *ruta);
// Genera un tema real, contrasta particiones y reloj y guarda un WAV local para su escucha.
bool comprobar_musica_adlib(const std::string &carpeta, const char *ruta, unsigned short tema = 1);
// Valida los cinco bancos con las ROM locales y guarda una muestra de la música de mazmorra.
bool comprobar_musica_mt32(const std::string &carpeta, const char *ruta);

// Comprueba las ganancias separadas y su conservación durante M, pausas y reinicios.
bool comprobar_volumenes_audio(const std::string &carpeta);
