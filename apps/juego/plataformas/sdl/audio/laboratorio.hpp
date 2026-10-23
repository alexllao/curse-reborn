#pragma once
#include "audio.hpp"
#include "../interfaz/configuracion.hpp"
#include "../../../motor/estado/inicializar.hpp"
#include <array>
#include <memory>

// Preescucha independiente: jamás recibe el segmento ni los sintetizadores de la partida.
class LaboratorioAudio {
public:
  ~LaboratorioAudio() {
    cerrar();
  }
  unsigned int version() const {
    return modo;
  }
  unsigned int pista() const {
    return tema;
  }
  unsigned int voz() const {
    return identificador;
  }
  unsigned int posicion() const {
    return lugar;
  }
  bool estereo() const {
    return posicional;
  }
  bool reproduciendo() const {
    return tocando;
  }
  unsigned int cantidad_pistas() const {
    return modo < 3 ? 2 : 5;
  }
  std::string nombre_version() const {
    const char *nombres[] = {"", "AdLib OPL2", "Sound Blaster OPL3", "Roland MT-32", "Remaster"};
    return nombres[modo];
  }
  std::string nombre_pista() const {
    if (modo < 3)
      return "Tema FM " + std::to_string(tema);
    std::string nombre = "CURSE.RS" + std::to_string(tema + 1);
    if (modo == 4) {
      if (!titulos_leidos) {
        const auto config = configuracion_inicial();
        const auto ruta = config["musica"]["catalogo"].string_value();
        const auto catalogo =
            leer_json_recurso(ruta.empty() ? "assets/remaster/audios/titulos.json" : ruta);
        for (unsigned int i = 0; i < 5; ++i)
          titulos[i] = catalogo["rs" + std::to_string(i + 1)]["titulo"].string_value();
        titulos_leidos = true;
      }
      const auto &titulo = titulos[tema];
      if (!titulo.empty())
        nombre += " · " + titulo;
    }
    return nombre;
  }
  void asegurar_version(unsigned int disponibles) {
    if (disponibles && !(disponibles & (1u << modo)))
      seleccionar_version(1, disponibles);
  }
  void seleccionar_version(int sentido, unsigned int disponibles) {
    detener();
    for (unsigned int i = 0; i < 4; ++i) {
      modo = 1 + (modo - 1 + (sentido < 0 ? 3u : 1u)) % 4;
      if (disponibles & (1u << modo))
        break;
    }
    tema %= cantidad_pistas();
  }
  void seleccionar_pista(int sentido) {
    detener();
    const auto n = cantidad_pistas();
    tema = (tema + n + (sentido < 0 ? -1 : 1)) % n;
  }
  void seleccionar_voz() {
    detener();
    identificador = 1 - identificador;
  }
  void alternar_estereo() {
    posicional = !posicional;
    detener();
  }
  void detener() {
    tocando = false;
    if (!audio)
      return;
    audio->pausar_menu(true);
    audio->digital.detener();
    if (!datos.empty())
      audio->altavoz.detener(datos.data());
    if (audio->flujo)
      SDL_ClearAudioStream(audio->flujo);
    if (audio->flujo_musical)
      SDL_ClearAudioStream(audio->flujo_musical);
  }
  void cerrar() {
    detener();
    if (audio)
      audio->cerrar();
    audio.reset();
    datos.clear();
  }

  // Copia solo ajustes de escucha; la pausa y el estado de la partida siguen siendo suyos.
  bool ajustar(unsigned int general, unsigned int musica, unsigned int efectos, bool silencio) {
    volumen = general;
    volumen_musica = musica;
    volumen_efectos = efectos;
    silenciado = silencio;
    if (!audio)
      return true;
    return (audio->nivel_volumen() == general ||
            audio->ajustar_volumen(int(general) - int(audio->nivel_volumen()))) &&
           (audio->nivel_volumen_musica() == musica ||
            audio->ajustar_volumen_musica(int(musica) - int(audio->nivel_volumen_musica()))) &&
           (audio->nivel_volumen_efectos() == efectos ||
            audio->ajustar_volumen_efectos(int(efectos) - int(audio->nivel_volumen_efectos()))) &&
           (audio->silenciado() == silencio || audio->alternar_silencio());
  }
  bool actualizar() {
    if (!audio)
      return true;
    if (tocando && !es_musica && !audio->digital.activa()) {
      // Dejar que termine la cola real, sin seguir añadiendo silencio cada fotograma.
      if (SDL_GetAudioStreamQueued(audio->flujo) == 0)
        detener();
      return true;
    }
    if (!audio->actualizar())
      return false;
    return true;
  }

  bool reproducir_musica(const std::string &carpeta, unsigned int disponibles,
                         bool salida_activa = true) {
    detener();
    if (!(disponibles & (1u << modo)) || !preparar(carpeta))
      return false;
    if (!SDL_LockAudioStream(audio->flujo_musical))
      return false;
    bool ok = true;
    if (modo < 3)
      ok = audio->musica.iniciar(ejecutable.data(), static_cast<unsigned int>(ejecutable.size()),
                                 static_cast<unsigned short>(tema));
    else if (modo == 3) {
      constexpr unsigned short escenas[] = {0, 64, 3, 15, 0xffff};
      ok = audio->mt32.iniciar(carpeta, ejecutable.data(),
                               static_cast<unsigned int>(ejecutable.size()), escenas[tema]);
    } else {
      ok = audio->remaster.disponible();
      if (ok)
        audio->remaster.cambiar_banco(tema, true);
    }
    audio->perfil = modo;
    SDL_UnlockAudioStream(audio->flujo_musical);
    es_musica = true;
    tocando = ok && (!salida_activa || audio->pausar_menu(false));
    return tocando;
  }

  bool reproducir_voz(const std::string &carpeta, unsigned int punto, unsigned int disponibles,
                      bool salida_activa = true) {
    detener();
    if (!disponibles || punto > 4 || !preparar(carpeta))
      return false;
    es_musica = false;
    lugar = punto;
    constexpr short centros[] = {0, 80, 160, 240, 320};
    motor::guardar(datos.data(), 0x2c2d, static_cast<unsigned short>(centros[punto] - 16));
    audio->perfil = 0;
    audio->configurar_estereo(posicional);
    if (!audio->reproducir(datos.data(), 0x2c2b, static_cast<unsigned char>(identificador)))
      return false;
    tocando = !salida_activa || audio->pausar_menu(false);
    return tocando;
  }

private:
  std::unique_ptr<Audio> audio;
  std::vector<unsigned char> datos, ejecutable;
  unsigned int modo = 4, tema = 0, identificador = 0, lugar = 2;
  unsigned int volumen = 100, volumen_musica = 100, volumen_efectos = 100;
  bool posicional = true, silenciado = false, tocando = false, es_musica = false;
  mutable bool titulos_leidos = false;
  mutable std::array<std::string, 5> titulos;
  bool preparar(const std::string &carpeta) {
    if (audio)
      return true;
    size_t n = 0;
    auto *p = static_cast<unsigned char *>(SDL_LoadFile((carpeta + "/../CURSE.EXE").c_str(), &n));
    if (!p || n > 0xffffffffu) {
      SDL_free(p);
      return false;
    }
    ejecutable.assign(p, p + n);
    SDL_free(p);
    datos.resize(65536);
    if (!motor::leer_datos_iniciales(ejecutable.data(), static_cast<unsigned int>(n), datos.data()))
      return false;
    motor::guardar(datos.data(), 0x8082, 0);
    audio = std::make_unique<Audio>();
    audio->configurar_musica();
    if (!audio->abrir(carpeta, false, true) || !audio->conectar(datos.data()) ||
        !audio->pausar_menu(true) ||
        !ajustar(volumen, volumen_musica, volumen_efectos, silenciado)) {
      cerrar();
      return false;
    }
    motor::guardar(datos.data(), 0x2c2f, 50);
    motor::guardar(datos.data(), 0x2c31, 32);
    motor::guardar(datos.data(), 0x2c33, 48);
    // Las dos voces se escogen del banco general real y no necesitan datos de ninguna escena.
    return true;
  }
  friend bool comprobar_laboratorio_audio(const std::string &, const std::string &);
};
