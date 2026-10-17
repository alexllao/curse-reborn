#include "marcos.hpp"
#include "catalogo_marcos.hpp"
#include "registro.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <string>

namespace {
// Busca primero una asignación por pantalla y recurre al marco común cuando no hay una propia.
const DefinicionMarco *elegir_marco(const char *pantalla) {
  const char *id = marco_predeterminado;
  for (const auto &asignacion : asignaciones_marcos)
    if (std::strcmp(pantalla, asignacion.pantalla) == 0)
      id = asignacion.marco;
  for (const auto &marco : catalogo_marcos)
    if (std::strcmp(id, marco.id) == 0)
      return &marco;
  return nullptr;
}

// Conserva el remate interior completo, incluido su relieve transparente sobre el juego.
bool dibujar_panel(SDL_Renderer *dibujante, SDL_Texture *textura, const PanelMarco &panel,
                   const SDL_Rect &banda, const SDL_FRect &juego, bool derecha, bool cubrir,
                   bool reflejar, float solape, bool horizontal = false) {
  if (banda.w <= 0 || banda.h <= 0)
    return true;
  const float cuerpo = panel.ancho - panel.margen;
  const float escala = horizontal
                           ? std::max(banda.w / panel.ancho, banda.h / panel.alto)
                           : std::max(juego.h / panel.alto, cubrir ? banda.w / cuerpo : 0.0f);
  const SDL_FRect destino{horizontal ? banda.x + (banda.w - panel.ancho * escala) / 2.0f
                          : derecha  ? juego.x + juego.w - solape - panel.margen * escala
                                     : juego.x + solape - cuerpo * escala,
                          horizontal ? banda.y + (banda.h - panel.alto * escala) / 2.0f
                                     : juego.y + (juego.h - panel.alto * escala) / 2.0f,
                          panel.ancho * escala, panel.alto * escala};
  // Solo limita el exterior a la ventana: el margen artístico interior nunca se usa como tijera.
  const float limite_x = !horizontal && derecha ? destino.x : float(banda.x);
  const float limite_derecho =
      !horizontal && !derecha ? destino.x + destino.w : float(banda.x + banda.w);
  const float x = std::max(destino.x, limite_x);
  const float y = std::max(destino.y, float(banda.y));
  const float ancho = std::min(destino.x + destino.w, limite_derecho) - x;
  const float alto = std::min(destino.y + destino.h, float(banda.y + banda.h)) - y;
  if (ancho <= 0 || alto <= 0)
    return true;
  const SDL_FRect origen{panel.x + (reflejar ? destino.x + destino.w - x - ancho : x - destino.x) /
                                       escala,
                         panel.y + (y - destino.y) / escala, ancho / escala, alto / escala};
  const SDL_FRect visible{x, y, ancho, alto};
  return SDL_RenderTextureRotated(dibujante, textura, &origen, &visible, 0, nullptr,
                                  reflejar ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
}
} // namespace

// Conserva la preferencia durante los cambios de escena y los de modo de ventana.
void MarcosPantalla::alternar() {
  visible = !visible;
  registrar_accion("Marcos: %s", visible ? "activados" : "desactivados");
}

// La propiedad de la imagen queda en la plataforma; no alcanza los recursos originales.
void MarcosPantalla::cerrar() {
  if (textura)
    SDL_DestroyTexture(textura);
  textura = nullptr;
  actual = nullptr;
}

// Dibuja las bandas y el solape autorizado; los filtros siguen actuando solo sobre el juego.
bool MarcosPantalla::dibujar(SDL_Renderer *dibujante, const char *pantalla) {
  if (!visible)
    return true;
  const auto *seleccion = elegir_marco(pantalla);
  if (!seleccion)
    return true;
  if (actual != seleccion || !textura) {
    cerrar();
    const char *base = SDL_GetBasePath();
    const std::string ruta = std::string(base ? base : "") + seleccion->archivo;
    SDL_Surface *imagen = SDL_LoadPNG(ruta.c_str());
    if (imagen) {
      textura = SDL_CreateTextureFromSurface(dibujante, imagen);
      SDL_DestroySurface(imagen);
    }
    if (!textura) {
      visible = false;
      registrar_accion("Marcos desactivados: no se puede cargar %s (%s)", seleccion->archivo,
                       SDL_GetError());
      return true;
    }
    if (!SDL_SetTextureScaleMode(textura, SDL_SCALEMODE_NEAREST) ||
        !SDL_SetTextureBlendMode(textura, SDL_BLENDMODE_BLEND))
      return false;
    actual = seleccion;
    registrar_accion("Marco de pantalla: %s", actual->nombre);
  }
  int ancho = 0, alto = 0, logico_ancho = 0, logico_alto = 0;
  SDL_RendererLogicalPresentation presentacion;
  SDL_FRect juego;
  if (!SDL_GetRenderOutputSize(dibujante, &ancho, &alto) ||
      !SDL_GetRenderLogicalPresentation(dibujante, &logico_ancho, &logico_alto, &presentacion) ||
      !SDL_GetRenderLogicalPresentationRect(dibujante, &juego) ||
      !SDL_SetRenderLogicalPresentation(dibujante, 0, 0, SDL_LOGICAL_PRESENTATION_DISABLED))
    return false;
  const float deseado = actual->solape * juego.w / 320.0f;
  const float solape_x = std::min(deseado, juego.x * 0.15f);
  const float solape_y = std::min(deseado, juego.y * 0.15f);
  const int izquierda = static_cast<int>(std::floor(juego.x + solape_x));
  const int derecha = static_cast<int>(std::ceil(juego.x + juego.w - solape_x));
  const int arriba = static_cast<int>(std::floor(juego.y + solape_y));
  const int abajo = static_cast<int>(std::ceil(juego.y + juego.h - solape_y));
  const bool dibujado =
      dibujar_panel(dibujante, textura, actual->izquierda, {0, 0, izquierda, alto}, juego, false,
                    actual->cubrir, false, solape_x) &&
      dibujar_panel(dibujante, textura, actual->derecha, {derecha, 0, ancho - derecha, alto}, juego,
                    true, actual->cubrir, actual->reflejar_derecha, solape_x) &&
      dibujar_panel(dibujante, textura, actual->izquierda, {0, 0, ancho, arriba}, juego, false,
                    true, false, solape_y, true) &&
      dibujar_panel(dibujante, textura, actual->derecha, {0, abajo, ancho, alto - abajo}, juego,
                    false, true, false, solape_y, true);
  const bool restaurado =
      SDL_SetRenderLogicalPresentation(dibujante, logico_ancho, logico_alto, presentacion);
  return dibujado && restaurado;
}
