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

// Carga únicamente una ilustración propia y conserva su transparencia al reducirla.
SDL_Texture *cargar_textura_propia(SDL_Renderer *dibujante, const char *archivo) {
  const char *base = SDL_GetBasePath();
  const std::string ruta = std::string(base ? base : "") + archivo;
  SDL_Surface *imagen = SDL_LoadPNG(ruta.c_str());
  if (!imagen)
    return nullptr;
  SDL_Texture *textura = SDL_CreateTextureFromSurface(dibujante, imagen);
  SDL_DestroySurface(imagen);
  if (textura && (!SDL_SetTextureBlendMode(textura, SDL_BLENDMODE_BLEND) ||
                  !SDL_SetTextureScaleMode(textura, SDL_SCALEMODE_LINEAR))) {
    SDL_DestroyTexture(textura);
    return nullptr;
  }
  return textura;
}

// La habitación y la carcasa solo ocupan bandas: ni un píxel tapa la imagen jugable.
bool dibujar_exterior(SDL_Renderer *dibujante, SDL_Texture *textura, const SDL_FRect &destino,
                      const SDL_FRect &juego, int ancho, int alto) {
  const int izquierda = int(std::floor(juego.x)), derecha = int(std::ceil(juego.x + juego.w));
  const int arriba = int(std::floor(juego.y)), abajo = int(std::ceil(juego.y + juego.h));
  SDL_Rect recorte;
  const bool recortado = SDL_RenderClipEnabled(dibujante);
  if (!SDL_GetRenderClipRect(dibujante, &recorte))
    return false;
  bool resultado = true;
  for (const SDL_Rect &banda : {SDL_Rect{0, 0, ancho, arriba},
                                {0, abajo, ancho, alto - abajo},
                                {0, arriba, izquierda, abajo - arriba},
                                {derecha, arriba, ancho - derecha, abajo - arriba}}) {
    if (banda.w > 0 && banda.h > 0 &&
        (!SDL_SetRenderClipRect(dibujante, &banda) ||
         !(textura ? SDL_RenderTexture(dibujante, textura, nullptr, &destino)
                   : SDL_RenderFillRect(dibujante, &destino)))) {
      resultado = false;
      break;
    }
  }
  return SDL_SetRenderClipRect(dibujante, recortado ? &recorte : nullptr) && resultado;
}

// Conserva el remate interior completo, incluido su relieve transparente sobre el juego.
bool dibujar_panel(SDL_Renderer *dibujante, SDL_Texture *textura, const PanelMarco &panel,
                   const SDL_Rect &banda, const SDL_FRect &juego, bool derecha, bool cubrir,
                   bool reflejar, float solape, float zoom, int ancho_rotado = 0) {
  if (banda.w <= 0 || banda.h <= 0)
    return true;
  const float cuerpo = panel.ancho - panel.margen;
  float escala = std::max(juego.h / panel.alto, cubrir ? banda.w / cuerpo : 0.0f);
  SDL_FRect destino{derecha ? juego.x + juego.w - solape - panel.margen * escala
                            : juego.x + solape - cuerpo * escala,
                    juego.y + (juego.h - panel.alto * escala) / 2.0f, panel.ancho * escala,
                    panel.alto * escala};
  // La mirilla se aproxima al centro de la ventana; solo se transforma el marco.
  const float centro_x = juego.x + juego.w / 2.0f;
  const float centro_y = juego.y + juego.h / 2.0f;
  destino.x = centro_x + (destino.x - centro_x) * zoom;
  destino.y = centro_y + (destino.y - centro_y) * zoom;
  destino.w *= zoom;
  destino.h *= zoom;
  escala *= zoom;
  // Solo limita el exterior a la ventana: el margen artístico interior nunca se usa como tijera.
  const float limite_x = derecha ? destino.x : float(banda.x);
  const float limite_derecho = !derecha ? destino.x + destino.w : float(banda.x + banda.w);
  const float x = std::max(destino.x, limite_x);
  const float y = std::max(destino.y, float(banda.y));
  const float ancho = std::min(destino.x + destino.w, limite_derecho) - x;
  const float alto = std::min(destino.y + destino.h, float(banda.y + banda.h)) - y;
  if (ancho <= 0 || alto <= 0)
    return true;
  const SDL_FRect origen{panel.x + (reflejar ? destino.x + destino.w - x - ancho : x - destino.x) /
                                       escala,
                         panel.y + (y - destino.y) / escala, ancho / escala, alto / escala};
  SDL_FRect visible{x, y, ancho, alto};
  if (ancho_rotado) {
    // Gira el panel lateral completo: el remate que miraba al juego queda abajo/arriba.
    const float centro_real_x = static_cast<float>(ancho_rotado) - (y + alto / 2.0f);
    const float centro_real_y = x + ancho / 2.0f;
    visible = {centro_real_x - ancho / 2.0f, centro_real_y - alto / 2.0f, ancho, alto};
  }
  return SDL_RenderTextureRotated(dibujante, textura, &origen, &visible, ancho_rotado ? 90.0 : 0.0,
                                  nullptr, reflejar ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
}
} // namespace

// Conserva la preferencia durante los cambios de escena y los de modo de ventana.
void MarcosPantalla::alternar(Uint64 ahora) {
  origen_animacion = nivel_animacion(ahora);
  monitor_en_salida = modo == 2 && monitor_presentado;
  modo = (modo + 1) % 3;
  visible = modo != 0;
  inicio_animacion = ahora;
  registrar_accion("Marcos: %s", modo == 1   ? "predeterminado"
                                 : modo == 2 ? "monitor de PC 286"
                                             : "sin marco");
}

// Completa en 250 ms una curva suave independiente de FPS, lógica, menús y sonido.
float MarcosPantalla::nivel_animacion(Uint64 ahora) const {
  if (!inicio_animacion)
    return visible ? 1.0f : 0.0f;
  const float tiempo = std::min(
      float(ahora >= inicio_animacion ? ahora - inicio_animacion : 0) / 250000000.0f, 1.0f);
  const float suavizado = tiempo * tiempo * (3.0f - 2.0f * tiempo);
  return origen_animacion + ((visible ? 1.0f : 0.0f) - origen_animacion) * suavizado;
}

// La propiedad de la imagen queda en la plataforma; no alcanza los recursos originales.
void MarcosPantalla::cerrar() {
  if (textura)
    SDL_DestroyTexture(textura);
  if (textura_monitor)
    SDL_DestroyTexture(textura_monitor);
  if (textura_habitacion)
    SDL_DestroyTexture(textura_habitacion);
  textura_habitacion = nullptr;
  textura_monitor = nullptr;
  textura = nullptr;
  actual = nullptr;
}

// El marco nunca resta espacio al juego: todos los modos comparten el mismo encuadre máximo.
bool MarcosPantalla::preparar_encuadre(SDL_Renderer *dibujante, Uint64) const {
  return SDL_SetRenderScale(dibujante, 1, 1) && SDL_SetRenderViewport(dibujante, nullptr) &&
         SDL_SetRenderLogicalPresentation(dibujante, 320, 200, SDL_LOGICAL_PRESENTATION_LETTERBOX);
}

// Los cursores usan la misma zona física que el dibujo y la conversión de eventos de SDL.
bool MarcosPantalla::area_juego(SDL_Renderer *dibujante, SDL_FRect &juego) {
  return SDL_GetRenderLogicalPresentationRect(dibujante, &juego);
}

// Dibuja las bandas y el solape autorizado; los filtros siguen actuando solo sobre el juego.
bool MarcosPantalla::dibujar(SDL_Renderer *dibujante, const char *pantalla, Uint64 ahora) {
  const float nivel = nivel_animacion(ahora);
  if (nivel <= 0.0f && !visible)
    return true;
  if (modo == 2 || (monitor_en_salida && !visible)) {
    monitor_presentado = true;
    if (!textura_monitor)
      textura_monitor = cargar_textura_propia(dibujante, monitor_pc.archivo);
    if (!textura_habitacion)
      textura_habitacion = cargar_textura_propia(dibujante, monitor_pc.fondo);
    if (!textura_monitor || !textura_habitacion)
      return false;
    int ancho = 0, alto = 0, logico_ancho = 0, logico_alto = 0;
    SDL_RendererLogicalPresentation presentacion;
    SDL_FRect juego;
    if (!SDL_GetRenderOutputSize(dibujante, &ancho, &alto) ||
        !SDL_GetRenderLogicalPresentation(dibujante, &logico_ancho, &logico_alto, &presentacion) ||
        !SDL_GetRenderLogicalPresentationRect(dibujante, &juego) ||
        !SDL_SetRenderLogicalPresentation(dibujante, 0, 0, SDL_LOGICAL_PRESENTATION_DISABLED))
      return false;
    // La abertura rodea la imagen máxima: la carcasa puede salir de la ventana, el juego nunca.
    const float escala = std::max(juego.w / monitor_pc.pantalla.w, juego.h / monitor_pc.pantalla.h);
    const SDL_FRect destino{
        juego.x + juego.w / 2 - (monitor_pc.pantalla.x + monitor_pc.pantalla.w / 2) * escala,
        juego.y + juego.h / 2 - (monitor_pc.pantalla.y + monitor_pc.pantalla.h / 2) * escala,
        monitor_pc.ancho * escala, monitor_pc.alto * escala};
    const float escala_fondo =
        std::max(float(ancho) / monitor_pc.ancho_fondo, float(alto) / monitor_pc.alto_fondo);
    const SDL_FRect habitacion{(ancho - monitor_pc.ancho_fondo * escala_fondo) / 2, 0,
                               monitor_pc.ancho_fondo * escala_fondo,
                               monitor_pc.alto_fondo * escala_fondo};
    const SDL_FRect abertura{destino.x + monitor_pc.pantalla.x * escala,
                             destino.y + monitor_pc.pantalla.y * escala,
                             monitor_pc.pantalla.w * escala, monitor_pc.pantalla.h * escala};
    const auto opacidad = Uint8(nivel * 255 + 0.5f);
    const bool dibujado =
        SDL_SetTextureAlphaMod(textura_monitor, opacidad) &&
        SDL_SetTextureAlphaMod(textura_habitacion, opacidad) &&
        dibujar_exterior(dibujante, textura_habitacion, habitacion, juego, ancho, alto) &&
        SDL_SetRenderDrawColor(dibujante, 0, 0, 0, opacidad) &&
        dibujar_exterior(dibujante, nullptr, abertura, juego, ancho, alto) &&
        dibujar_exterior(dibujante, textura_monitor, destino, juego, ancho, alto);
    const bool restaurado =
        SDL_SetRenderLogicalPresentation(dibujante, logico_ancho, logico_alto, presentacion);
    return dibujado && restaurado;
  }
  monitor_presentado = false;
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
      inicio_animacion = 0;
      origen_animacion = 0;
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
  const int arriba = static_cast<int>(std::floor(juego.y + solape_y));
  const int abajo = static_cast<int>(std::ceil(juego.y + juego.h - solape_y));
  // En coordenadas giradas, las bandas superior/inferior son los mismos dos laterales.
  const SDL_FRect juego_rotado{juego.y, static_cast<float>(ancho) - juego.x - juego.w, juego.h,
                               juego.w};
  const int izquierda = static_cast<int>(std::floor(juego.x + solape_x));
  const int derecha = static_cast<int>(std::ceil(juego.x + juego.w - solape_x));
  const float zoom = 1.0f + 0.10f * (1.0f - nivel);
  const bool opacidad = SDL_SetTextureAlphaMod(textura, static_cast<Uint8>(nivel * 255.0f + 0.5f));
  const bool dibujado =
      opacidad &&
      dibujar_panel(dibujante, textura, actual->izquierda, {0, 0, izquierda, alto}, juego, false,
                    actual->cubrir, false, solape_x, zoom) &&
      dibujar_panel(dibujante, textura, actual->derecha, {derecha, 0, ancho - derecha, alto}, juego,
                    true, actual->cubrir, actual->reflejar_derecha, solape_x, zoom) &&
      dibujar_panel(dibujante, textura, actual->izquierda, {0, 0, arriba, ancho}, juego_rotado,
                    false, actual->cubrir, false, solape_y, zoom, ancho) &&
      dibujar_panel(dibujante, textura, actual->derecha, {abajo, 0, alto - abajo, ancho},
                    juego_rotado, true, actual->cubrir, actual->reflejar_derecha, solape_y, zoom,
                    ancho);
  const bool opacidad_restaurada = SDL_SetTextureAlphaMod(textura, 255);
  const bool restaurado =
      SDL_SetRenderLogicalPresentation(dibujante, logico_ancho, logico_alto, presentacion);
  return dibujado && opacidad_restaurada && restaurado;
}
