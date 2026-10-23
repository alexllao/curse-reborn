#include "marcos.hpp"
#include "cargar_fondo.hpp"
#include "catalogo_marcos.hpp"
#include "../registro.hpp"
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
  visible = !visible;
  inicio_animacion = ahora;
  registrar_accion("Marcos: %s", nombre_actual());
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
  textura = nullptr;
  if (moldura)
    SDL_DestroyTexture(moldura);
  moldura = nullptr;
  actual = nullptr;
}

// Los marcos conservan el encuadre máximo; la habitación configura aparte su propio viewport.
bool MarcosPantalla::preparar_encuadre(SDL_Renderer *dibujante, Uint64 ahora) const {
  if (!SDL_SetRenderScale(dibujante, 1, 1) || !SDL_SetRenderViewport(dibujante, nullptr))
    return false;
  (void)ahora;
  return SDL_SetRenderLogicalPresentation(dibujante, 320, 200, SDL_LOGICAL_PRESENTATION_LETTERBOX);
}

// Los cursores usan la misma zona física que el dibujo y la conversión de eventos de SDL.
bool MarcosPantalla::area_juego(SDL_Renderer *dibujante, SDL_FRect &juego) {
  if (SDL_RenderViewportSet(dibujante)) {
    SDL_Rect zona;
    float sx = 1, sy = 1;
    if (!SDL_GetRenderViewport(dibujante, &zona) || !SDL_GetRenderScale(dibujante, &sx, &sy))
      return false;
    juego = {zona.x * sx, zona.y * sy, zona.w * sx, zona.h * sy};
    return true;
  }
  return SDL_GetRenderLogicalPresentationRect(dibujante, &juego);
}

// Conserva el detalle del fondo y la moldura; los filtros pertenecen solo al juego y su cursor.
bool MarcosPantalla::dibujar(SDL_Renderer *dibujante, const char *pantalla, Uint64 ahora) {
  const float nivel = nivel_animacion(ahora);
  if (nivel <= 0.0f && !visible)
    return true;
  const auto *seleccion = elegir_marco(pantalla);
  int salida_ancho = 0, salida_alto = 0;
  if (!SDL_GetRenderOutputSize(dibujante, &salida_ancho, &salida_alto) || salida_alto <= 0)
    return false;
  if (seleccion && seleccion->panoramico) {
    const float proporcion = float(salida_ancho) / salida_alto;
    float distancia = 1000.0f;
    for (const auto &marco : catalogo_marcos) {
      if (!marco.panoramico)
        continue;
      const float diferencia = std::abs(std::log(proporcion / marco.proporcion));
      if (diferencia < distancia) {
        distancia = diferencia;
        seleccion = &marco;
      }
    }
  }
  if (!seleccion)
    return true;
  if (actual != seleccion || !textura) {
    cerrar();
    const char *base = SDL_GetBasePath();
    const std::string ruta = std::string(base ? base : "") + seleccion->archivo;
    SDL_Surface *imagen = cargar_fondo(ruta);
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
    if (!SDL_SetTextureScaleMode(textura, seleccion->panoramico ? SDL_SCALEMODE_LINEAR
                                                                : SDL_SCALEMODE_NEAREST) ||
        !SDL_SetTextureBlendMode(textura, SDL_BLENDMODE_BLEND) ||
        !SDL_SetTextureColorMod(textura, seleccion->luminosidad, seleccion->luminosidad,
                                seleccion->luminosidad))
      return false;
    if (seleccion->panoramico) {
      const std::string ruta_moldura = std::string(base ? base : "") + archivo_moldura;
      SDL_Surface *original = SDL_LoadPNG(ruta_moldura.c_str());
      if (!original)
        return false;
      moldura = SDL_CreateTextureFromSurface(dibujante, original);
      SDL_DestroySurface(original);
      if (!moldura || !SDL_SetTextureScaleMode(moldura, SDL_SCALEMODE_LINEAR) ||
          !SDL_SetTextureBlendMode(moldura, SDL_BLENDMODE_BLEND))
        return false;
    }
    actual = seleccion;
    registrar_accion("Marco de pantalla: %s", actual->nombre);
  }
  int ancho = 0, alto = 0, logico_ancho = 0, logico_alto = 0;
  SDL_RendererLogicalPresentation presentacion;
  SDL_FRect juego;
  SDL_Rect viewport;
  float escala_x = 1, escala_y = 1;
  const bool viewport_fijado = SDL_RenderViewportSet(dibujante);
  if (!SDL_GetRenderViewport(dibujante, &viewport) ||
      !SDL_GetRenderScale(dibujante, &escala_x, &escala_y) ||
      !SDL_GetRenderOutputSize(dibujante, &ancho, &alto) ||
      !SDL_GetRenderLogicalPresentation(dibujante, &logico_ancho, &logico_alto, &presentacion) ||
      !area_juego(dibujante, juego) ||
      !SDL_SetRenderLogicalPresentation(dibujante, 0, 0, SDL_LOGICAL_PRESENTATION_DISABLED) ||
      !SDL_SetRenderViewport(dibujante, nullptr) || !SDL_SetRenderScale(dibujante, 1, 1))
    return false;
  const auto restaurar_encuadre = [&]() {
    return SDL_SetRenderLogicalPresentation(dibujante, logico_ancho, logico_alto, presentacion) &&
           SDL_SetRenderScale(dibujante, escala_x, escala_y) &&
           SDL_SetRenderViewport(dibujante, viewport_fijado ? &viewport : nullptr);
  };
  if (actual->panoramico) {
    float tw = 0, th = 0;
    bool ok = SDL_GetTextureSize(textura, &tw, &th);
    const float escala = std::max(ancho / tw, alto / th);
    const float ox = (tw * escala - ancho) / 2.0f;
    const float oy = (th * escala - alto) / 2.0f;
    const SDL_FRect bandas[] = {{0, 0, juego.x, float(alto)},
                                {juego.x + juego.w, 0, ancho - juego.x - juego.w, float(alto)},
                                {juego.x, 0, juego.w, juego.y},
                                {juego.x, juego.y + juego.h, juego.w, alto - juego.y - juego.h}};
    ok = ok && SDL_SetTextureAlphaMod(textura, static_cast<Uint8>(nivel * 255.0f));
    for (const auto &banda : bandas) {
      if (banda.w <= 0.01f || banda.h <= 0.01f)
        continue;
      const SDL_FRect origen{(banda.x + ox) / escala, (banda.y + oy) / escala, banda.w / escala,
                             banda.h / escala};
      ok = SDL_RenderTexture(dibujante, textura, &origen, &banda) && ok;
    }
    // Los adornos conservan su escala; solo se estiran los tramos lisos.
    // El relieve transparente se dibuja completo por encima del juego.
    if (moldura && (juego.x > 0.5f || juego.y > 0.5f)) {
      float mw = 0, mh = 0;
      ok = SDL_GetTextureSize(moldura, &mw, &mh) && ok;
      const float s = juego.h / 200.0f * escala_moldura;
      const SDL_FRect destino{juego.x - vano_moldura.x * s, juego.y - vano_moldura.y * s,
                              juego.w + (mw - vano_moldura.w) * s,
                              juego.h + (mh - vano_moldura.h) * s};
      // Cinco filas mantienen el rombo central a escala uniforme, igual que las esquinas.
      const float centro = destino.y + destino.h / 2.0f;
      const float medio = (adorno_central_moldura[1] - adorno_central_moldura[0]) * s / 2.0f;
      const float sx[] = {0, esquinas_moldura[0], mw - esquinas_moldura[1], mw};
      const float sy[] = {0,
                          esquinas_moldura[2],
                          adorno_central_moldura[0],
                          adorno_central_moldura[1],
                          mh - esquinas_moldura[3],
                          mh};
      const float dx[] = {destino.x, destino.x + esquinas_moldura[0] * s,
                          destino.x + destino.w - esquinas_moldura[1] * s, destino.x + destino.w};
      const float dy[] = {destino.y,
                          destino.y + esquinas_moldura[2] * s,
                          centro - medio,
                          centro + medio,
                          destino.y + destino.h - esquinas_moldura[3] * s,
                          destino.y + destino.h};
      // El PNG se compone completo sobre el juego: no cortar el relieve interior.
      ok = SDL_SetTextureAlphaMod(moldura, static_cast<Uint8>(nivel * 255.0f)) && ok;
      for (int fila = 0; fila < 5; ++fila)
        for (int columna = 0; columna < 3; ++columna) {
          if (columna == 1 && fila > 0 && fila < 4)
            continue;
          const SDL_FRect origen{sx[columna], sy[fila], sx[columna + 1] - sx[columna],
                                 sy[fila + 1] - sy[fila]};
          const SDL_FRect pieza{dx[columna], dy[fila], dx[columna + 1] - dx[columna],
                                dy[fila + 1] - dy[fila]};
          ok = SDL_RenderTexture(dibujante, moldura, &origen, &pieza) && ok;
        }
      ok = SDL_SetTextureAlphaMod(moldura, 255) && ok;
    }
    const bool alfa = SDL_SetTextureAlphaMod(textura, 255);
    const bool restaurar = restaurar_encuadre();
    return ok && alfa && restaurar;
  }
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
  const bool restaurado = restaurar_encuadre();
  return dibujado && opacidad_restaurada && restaurado;
}
