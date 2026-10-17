#pragma once
#include <SDL3/SDL.h>
#include <string>
#include <vector>
#include "mazmorra.hpp"

// Menús de plataforma para las siete ranuras del formato DOS, separados de la lógica reconstruida.
class PartidasSDL {
public:
  // Abre cargar (1), guardar (2) o borrar (3), sin consumir pasos del juego.
  void abrir(unsigned int modo, SDL_Window *ventana, unsigned short puntos = 0);
  // Consume las entradas del panel; devuelve si la escena necesita dibujarse tras una carga.
  bool evento(const SDL_Event &evento, SDL_Window *ventana, Mazmorra &partida, bool &redibujar);
  // Presenta las ranuras y los botones en el encuadre lógico del juego.
  bool dibujar(SDL_Renderer *dibujante) const;
  // Permite pausar exclusivamente la lógica mientras la música continúa.
  bool abierta() const {
    return modo != 0;
  }

private:
  unsigned int modo = 0, seleccion = 0;
  unsigned short puntos = 0;
  Uint32 evento_importacion = 0;
  bool disponible = false, confirmacion = false;
  std::string ruta, nombre, mensaje;
  std::vector<unsigned char> archivo;
  // Lee el perfil sin modificar el CURSE.SAV comercial; los errores impiden sobrescribirlo.
  void preparar();
  // Conserva copia anterior y reemplaza el archivo mediante un temporal en la misma carpeta.
  bool persistir();
  // Cambia de ranura y dispone su nombre editable, conservando las demás descripciones.
  void seleccionar(unsigned int ranura);
  // Aplica la acción elegida y exige confirmar sobrescritura y eliminación.
  bool aceptar(SDL_Window *ventana, Mazmorra &partida, bool &redibujar);
  // Importa únicamente en ranuras libres, sin modificar el archivo original ni ocupar otras
  // partidas.
  void importar(const char *ruta);
  // Cierra el panel y termina la entrada de texto antes de devolver el control al juego.
  void cerrar(SDL_Window *ventana);
  // Recibe el selector nativo en cualquier hilo y entrega la ruta al bucle SDL mediante un evento.
  static void SDLCALL recibir_archivo(void *contexto, const char *const *archivos, int filtro);
};
