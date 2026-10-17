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
  // Consulta el bloqueo de animación para contrastar el final de la inmersión.
  bool secuencia_personaje_activa() const;
  // Distingue un retrazo de cámara de un paso lógico durante las comprobaciones deterministas.
  bool camara_en_presentacion() const {
    return animacion == 2;
  }
  // Recupera la posición de una entidad para automatizar clics sin modificar la partida.
  bool posicion_objeto(unsigned short id, unsigned short &x, unsigned short &y,
                       unsigned short &alto) const;
  // Localiza una casilla por su identificador original en el menú que se está mostrando.
  int casilla_objeto(unsigned short id) const;
  // partida.
  bool conserva_objeto(unsigned short identificador) const;
  // El guion 0F94 es el golpe de las estatuas; permite detener una captura en esa fase real.
  bool golpe_estatua() const;
  // Consulta la amenaza y la secuencia de descarga para comprobar el uso real de la picana.
  bool tiburon_activo() const;
  // Reconoce la respuesta original del tiburón alcanzado sin cambiar su comportamiento.
  bool tiburon_aturdido() const;
  // 3151: consulta el modelo cerrado de la almeja para esperar su condición de salto original.
  bool almeja_cerrada() const {
    return datos[0x2e3c] == 12;
  }
  // Conserva DS junto a una captura de prueba para localizar divergencias; no es una partida
  // guardada.
  bool guardar_estado_diagnostico(const std::string &ruta) const {
    return SDL_SaveFile(ruta.c_str(), datos, sizeof(datos));
  }
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
  // Devuelve la petición de cargar, guardar o borrar del menú original y la consume una sola vez.
  unsigned int tomar_solicitud_partidas() {
    const auto solicitud = solicitud_partidas;
    solicitud_partidas = 0;
    return solicitud;
  }
  // E160: ofrece la puntuación original para la ficha informativa de la plataforma.
  unsigned short puntuacion() const {
    return static_cast<unsigned short>(datos[0x35c7] |
                                       (static_cast<unsigned int>(datos[0x35c8]) << 8));
  }
  // Consulta el control original para comprobar que cargar una partida no bloquea a Brad.
  bool movimiento_habilitado() const {
    return datos[0x3594] != 0;
  }
  // E725: copia el bloque compatible con DOS a una ranura de CURSE.SAV.
  bool guardar_partida(unsigned char *archivo, unsigned int capacidad, unsigned int ranura,
                       const char *nombre) const;
  // E253–E2B3 y 4AF0 con AL=1: restaura una ranura sin sustituir la partida si faltan recursos.
  bool cargar_partida(const unsigned char *archivo, unsigned int longitud, unsigned int ranura);
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
  // 8B6F: facilita al menú el creador común con los recursos y la semilla de la partida.
  static bool crear_desde_menu(void *contexto, unsigned char *datos, unsigned short descriptor,
                               unsigned short &posicion);
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
  bool paso_tras_camara = false;
  unsigned short salida_pendiente = 0;
  unsigned long long inicio_animacion = 0;
  motor::EstadoAleatorio aleatorio{};
  std::string carpeta;
  std::vector<unsigned char> escena, personaje, modelo, iconos, raton, geometria, mapa;
  unsigned char fondo_menu[10240]{}, panel_menu[64000]{};
  bool menu_visible = false;
  unsigned int solicitud_partidas = 0;
  unsigned short direccion = 0xffff;
  unsigned short casilla_menu = 0;

  // Carga bancos, BOXDET y entidades de las cuatro escenas integradas conservando el resto del
  // segmento DS.
  bool cambiar_habitacion(unsigned short salida);
  // Comparte los recursos y las entidades del cargador DOS entre transición y restauración.
  bool cargar_habitacion_actual(unsigned short salida, bool actualizar_sonido = true,
                                bool reconstruir_entidades = true);
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
