#pragma once

#include <string>
#include <SDL3/SDL.h>
#include <vector>
#include "audio.hpp"
#include "../../motor/aleatorio/generador.hpp"
#include "../../motor/video/imagen.hpp"

// Adaptador de los bancos originales y el estado de escena para las rutinas C++ reconstruidas.
class Mazmorra {
public:
  // Carga solo datos del EXE y los bancos; prepara la entrada y la lista de objetos originales.
  bool iniciar(const std::string &carpeta, const motor::ImagenIndexada &fondo);
  // Vista de desarrollo: reinicia la sesión y dispone la entrada original a DROP sin resolver
  // puzles.
  bool iniciar_caida(const std::string &carpeta, const motor::ImagenIndexada &fondo);
  // Abre una escena integrada como sesión independiente; no resuelve ni conserva puzles.
  bool iniciar_vista(const std::string &carpeta, const motor::ImagenIndexada &fondo,
                     unsigned short escena);
  // Avanza las secuencias, comportamientos y lógica particular sin ejecutar instrucciones DOS.
  bool avanzar();
  // Avanza los 32 retrazos de fundido o los ocho de cámara sin consumir pasos de juego.
  bool avanzar_presentacion(unsigned long long tick);
  // Permite al bucle y a las comprobaciones respetar las esperas de la presentación original.
  bool en_presentacion() const {
    return animacion != 0;
  }
  // Consulta la escena y ventana reconstruidas para verificar recorridos sin alterar la partida.
  unsigned short escena_actual() const;
  // Consulta el inventario persistente para comprobar recogidas y transiciones sin alterar la
  // partida.
  bool conserva_objeto(unsigned short identificador) const;
  // El guion 0F94 es el golpe de las estatuas; permite detener una captura en esa fase real.
  bool golpe_estatua() const;
  // Consulta la tira inicial de la ventana visible, en unidades originales de 32 píxeles.
  unsigned char ventana_actual() const {
    return datos[0x3597];
  }
  // Restaura el fondo y compone todas las entidades en el orden reconstruido.
  bool dibujar(motor::ImagenIndexada &imagen, unsigned int paso_menu = 0);

  // Abre/cierra el menú original y procesa sus casillas desde entrada SDL en coordenadas lógicas.
  bool entrada_menu(unsigned short x, unsigned short y, bool seleccionar, bool alternar);
  // El clic izquierdo propone un destino; el control y las paredes originales deciden el avance.
  bool mover_a(unsigned short x, unsigned short y);
  // Conserva las flechas pulsadas con los cuatro bits originales de dirección.
  void tecla_direccion(unsigned short bit, bool pulsada);
  // Consulta si las animaciones deben permanecer detenidas mientras se escoge una acción.
  bool menu_abierto() const {
    return menu_visible;
  }
  // Dibuja rectángulos de diagnóstico sin escribir en el estado ni en las imágenes originales.
  bool depurar(SDL_Renderer *dibujante, bool objetos, bool interacciones, bool limites) const;

  // Abre la salida común antes de cargar una partida; un fallo de dispositivo no altera los datos.
  bool abrir_audio(const std::string &carpeta) {
    return audio.abrir(carpeta);
  }
  // Libera el dispositivo antes de cerrar SDL.
  void cerrar_audio() {
    audio.cerrar();
  }
  // Desconecta los efectos de la sesión, conservando el tema musical de la presentación.
  void desconectar_audio() {
    audio.desconectar();
  }
  // Mantiene el temporizador audible mientras la lógica está pausada por menús o fundidos.
  bool actualizar_audio() {
    return audio.actualizar();
  }
  // Vuelve a la música de presentación, conservando el dispositivo y el búfer.
  bool audio_presentacion() {
    return audio.cambiar_escena(0xffff);
  }
  // Alterna las opciones de audio disponibles e informa por consola.
  bool alternar_dispositivo_audio() {
    return audio.alternar_dispositivo();
  }
  // Permite contrastar los efectos digitales y el altavoz sin reiniciar ni cambiar la música.
  bool alternar_efectos_audio() {
    return audio.alternar_efectos();
  }
  // Alterna el silencio de plataforma con la tecla M.
  bool alternar_silencio() {
    return audio.alternar_silencio();
  }

  // Modifica la ganancia común solicitada por los atajos + y - sin afectar la partida.
  bool ajustar_volumen(int cambio) {
    return audio.ajustar_volumen(cambio);
  }

private:
  Audio audio;
  unsigned short inventario_registrado[10]{};
  unsigned char control_registrado = 0;
  bool registro_preparado = false;
  // Informa de cambios reales de inventario y control, sin escribir en el estado original.
  void registrar_estado();
  unsigned char datos[65536]{};
  motor::ImagenIndexada fondo{}, imagen_anterior{}, imagen_destino{};
  unsigned char planos_fondo[262144]{}, tira_fondo[6400]{};
  unsigned int animacion = 0, paso_animacion = 0;
  int sentido_camara = 0;
  unsigned short salida_pendiente = 0;
  unsigned long long inicio_animacion = 0;
  motor::EstadoAleatorio aleatorio{};
  std::string carpeta;
  std::vector<unsigned char> escena, personaje, modelo, iconos, raton, geometria, mapa;
  unsigned char fondo_menu[10240]{}, panel_menu[64000]{};
  bool menu_visible = false;
  unsigned short direccion = 0xffff;
  unsigned short casilla_menu = 0;

  // Carga bancos, BOXDET y entidades de las cuatro escenas integradas conservando el resto del
  // segmento DS.
  bool cambiar_habitacion(unsigned short salida);
  // Reconstruye las diez tiras visibles desde la tabla de la escena y su ventana actual.
  bool cargar_fondo_visible();
  // Lee un banco local sin asumir que sus bytes estén alineados o terminen en cero.
  bool cargar_banco(const std::string &nombre, std::vector<unsigned char> &destino);
  // Resuelve etiquetas de recurso como bancos; nunca las desreferencia como segmentos DOS.
  static const unsigned char *banco(void *contexto, unsigned short referencia,
                                    unsigned int &longitud);
  // Consulta el tamaño del fotograma para las secuencias de cualquiera de los tres bancos.
  static bool dimensiones(void *contexto, unsigned short referencia, unsigned short imagen,
                          unsigned short &ancho, unsigned short &alto);
  // Adapta las dimensiones del banco de escena al cargador y los comportamientos de entidades.
  static bool dimensiones_escena(void *contexto, unsigned short imagen, unsigned short &ancho,
                                 unsigned short &alto);
  // Proporciona la referencia de una superposición leída de la tabla original del banco.
  static bool parrafo_escena(void *contexto, unsigned short imagen, unsigned short &referencia);
  // Dirige los identificadores originales al dispositivo compartido de la partida.
  static bool reproducir_sonido(void *, unsigned char *datos, unsigned short, unsigned char);
  // Atiende F8 leyendo el nombre acotado de la tabla original y reemplazando el banco de modelo.
  static bool cargar_modelo(void *contexto, const unsigned char *datos, unsigned short nombre,
                            unsigned short capacidad);
};
