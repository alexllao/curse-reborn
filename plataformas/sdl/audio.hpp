#pragma once

#include <SDL3/SDL.h>
#include "../../motor/sonido/pcm.hpp"
#include "../../motor/sonido/digital.hpp"
#include "musica_adlib.hpp"
#include "musica_mt32.hpp"
#include <atomic>
#include <string>
#include <vector>

// Salida compartida entre escenas; toda la lógica del juego permanece en el hilo principal.
class Audio {
public:
  // Abre una salida estéreo con efectos mono; SDL convierte al formato y frecuencia del dispositivo
  // real.
  bool abrir(const std::string &carpeta);
  // Cierra el flujo antes de SDL_Quit, incluso cuando falla la carga de una escena.
  void cerrar();
  // Asocia una sesión nueva, conserva el dispositivo y limpia efectos de la sesión anterior.
  bool conectar(unsigned char *segmento);
  // Deja de atender los efectos de la sesión; el tema continúa en la presentación.
  void desconectar();
  // Produce el tiempo transcurrido, también con menú abierto o durante fundidos.
  bool actualizar();
  // Atiende una orden de 5288 después de llevar el reloj del altavoz hasta el momento actual.
  bool reproducir(unsigned char *segmento, unsigned short entidad, unsigned char identificador);
  // Silencia la salida audible; las secuencias y prioridades originales siguen avanzando.
  bool alternar_silencio();
  // Ajusta el volumen general entre cero y doscientos sin alterar los relojes musicales.
  bool ajustar_volumen(int cambio);
  // Permite consultar el ajuste guardado, incluso durante el silencio temporal de M.
  unsigned int nivel_volumen() const {
    return volumen;
  }
  // F4 recorre las opciones musicales; los efectos se eligen por separado con F3.
  bool alternar_dispositivo();
  // F3 cambia los efectos entre Sound Blaster digital y PC Speaker sin modificar la música.
  bool alternar_efectos();

  // Selecciona el banco de la escena bajo el bloqueo del flujo musical, sin vaciar sus muestras.
  bool cambiar_escena(unsigned short escena);
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
  SDL_AudioDeviceID dispositivo = 0;
  SDL_AudioStream *flujo_musical = nullptr;
  MusicaMt32 mt32;
  std::atomic<bool> fallo_musical{false};
  std::atomic<unsigned long long> muestras_producidas{0}, muestras_con_senal{0};
  unsigned int perfil = 1;
  // Produce solo música; este callback nunca accede al DS ni a las entidades del juego.
  static void alimentar_musica(void *contexto, SDL_AudioStream *flujo, int adicional, int total);
  SDL_AudioStream *flujo = nullptr;
  unsigned char *datos = nullptr;
  motor::AltavozPcm altavoz{};
  motor::DigitalPcm digital{};
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
  bool silencio = false;
  unsigned int volumen = 100;
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
