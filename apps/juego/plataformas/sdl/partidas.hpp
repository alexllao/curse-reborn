#pragma once
#include "creditos_port.hpp"
#include <SDL3/SDL.h>
#include <string>
#include <vector>
#include "mazmorra.hpp"

// Entrada y archivos SDL para los paneles originales de ocho ranuras del formato DOS.
class PartidasSDL {
public:
  // Abre cargar (1), guardar (2), borrar (3) o información (4), sin consumir pasos del juego.
  void abrir(unsigned int modo, SDL_Window *ventana);
  // Libera la textura antes de destruir el dibujante SDL.
  ~PartidasSDL();
  // Consume las entradas del panel; devuelve si la escena necesita dibujarse tras una carga.
  bool evento(const SDL_Event &evento, SDL_Window *ventana, Mazmorra &partida, bool &redibujar);
  // Presenta las fichas originales o las ranuras en el encuadre lógico del juego.
  bool dibujar(SDL_Renderer *dibujante, const Mazmorra &partida);
  // Compone el panel y su mano incorporada antes del filtro, conservando sus píxeles transparentes.
  bool componer_rgba(const Mazmorra &partida, unsigned char *rgba) const;
  // Cierra el panel y termina la entrada de texto antes de devolver el control al juego.
  void cerrar(SDL_Window *ventana);
  // Permite pausar exclusivamente la lógica mientras la música continúa.
  bool abierta() const {
    return modo != 0;
  }
  // Las ocho ranuras y el teclado ya dibujan el cursor DOS dentro de su imagen.
  bool incorpora_cursor() const {
    return modo >= 1 && modo <= 3;
  }

private:
  unsigned int modo = 0, seleccion = 0;
  unsigned int pagina_informacion = 0, pagina_preparada = paginas_informacion;
  SDL_Texture *textura_informacion = nullptr;
  SDL_FRect placa_informacion{};
  bool disponible = false, editando = false;
  int cursor_x = 108, cursor_y = 70;
  std::string ruta, nombre, mensaje;
  std::vector<unsigned char> archivo;
  // Lee el perfil sin modificar el CURSE.SAV comercial; los errores impiden sobrescribirlo.
  void preparar();
  // Conserva copia anterior y reemplaza el archivo mediante un temporal en la misma carpeta.
  bool persistir();
  // Cambia de ranura y dispone su nombre editable, conservando las demás descripciones.
  void seleccionar(unsigned int ranura);
  // E23B/E3BC/E81F: escoge ranura, abre el editor o ejecuta la operación original.
  bool aceptar(SDL_Window *ventana, Mazmorra &partida, bool &redibujar);
  // Importa únicamente en ranuras libres, sin modificar el archivo original ni ocupar otras
  // partidas.
  void importar(const char *ruta);

  // E595–E6F6: interpreta la casilla elegida del teclado original.
  bool pulsar_tecla(SDL_Window *ventana, Mazmorra &partida, bool &redibujar);
};
