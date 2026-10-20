#include "habitacion.hpp"
#include "catalogo_habitaciones.hpp"
#include "../registro.hpp"
#include <algorithm>
#include <cmath>
#include <iterator>
#include <string>

// Consulta el catálogo compilado para no asumir cinco diseños en una entrega reducida.
unsigned int Habitacion::cantidad_temas() {
  return static_cast<unsigned int>(std::size(temas_habitaciones));
}

// Restaura el diseño inicial y el estado apagado conservando el catálogo de esta compilación.
void Habitacion::iniciar() {
  habilitada = true;
  tema = habitacion_inicial;
  estado = Estado::apagado;
  sonido_pendiente = 0;
  encendido_programado = 0;
  inicio_estado = inicio_zoom = 0;
  zoom_origen = zoom_destino = 0;
  registrar_accion("Habitación: %s; pulsa el botón del PC para encender",
                   temas_habitaciones[tema].nombre);
}

// Escoge una imagen según la relación de aspecto, nunca por el número bruto de píxeles.
const VarianteHabitacion &Habitacion::variante(int ancho, int alto) const {
  const auto &t = temas_habitaciones[tema];
  const VarianteHabitacion *mejor = &variantes_habitaciones[t.inicio];
  float distancia = 1000;
  for (unsigned int i = t.inicio; i < t.inicio + t.cantidad; ++i) {
    const auto &v = variantes_habitaciones[i];
    const float actual = std::abs(std::log((float(ancho) / alto) / (float(v.ancho) / v.alto)));
    if (actual < distancia) {
      distancia = actual;
      mejor = &v;
    }
  }
  return *mejor;
}

// Mantiene la continuidad cuando se invierte una transición antes de terminarla.
float Habitacion::nivel_zoom(Uint64 ahora) const {
  if (!inicio_zoom)
    return zoom_destino;
  const float t =
      std::clamp(float(ahora >= inicio_zoom ? ahora - inicio_zoom : 0) / 650000000.0f, 0.0f, 1.0f);
  return zoom_origen + (zoom_destino - zoom_origen) * t * t * (3 - 2 * t);
}

// Anima el acercamiento sin cambiar escena, música ni posición del personaje.
void Habitacion::ajustar_zoom(float destino, Uint64 ahora) {
  zoom_origen = nivel_zoom(ahora);
  zoom_destino = std::clamp(destino, 0.0f, 1.0f);
  inicio_zoom = ahora;
  registrar_accion("Habitación: %s",
                   zoom_destino == 0 ? "vista de la habitación" : "acercamiento al juego");
}

// Solo el encuadre final permite dibujar los marcos decorativos por separado.
bool Habitacion::ampliada(Uint64 ahora) const {
  return !habilitada || nivel_zoom(ahora) >= 1;
}

// Usa una sola transformación uniforme para imagen, pantalla y controles.
bool Habitacion::geometria(SDL_Renderer *r, SDL_FRect &imagen, SDL_FRect &pantalla,
                           SDL_FRect &juego, Uint64 ahora) const {
  int w = 0, h = 0;
  if (!SDL_GetRenderOutputSize(r, &w, &h) || w <= 0 || h <= 0)
    return false;
  const auto &v = variante(w, h);
  const float base = std::min(float(w) / v.ancho, float(h) / v.alto);
  const float ajuste = std::min(v.pantalla.w / 320, v.pantalla.h / 200);
  const float final = std::min(float(w) / 320, float(h) / 200) / ajuste;
  const float z = nivel_zoom(ahora), escala = base + (final - base) * z;
  const float inicial_x = (w - v.ancho * base) / 2, inicial_y = (h - v.alto * base) / 2;
  const float final_x = w / 2.0f - (v.pantalla.x + v.pantalla.w / 2) * final;
  const float final_y = h / 2.0f - (v.pantalla.y + v.pantalla.h / 2) * final;
  imagen = {inicial_x + (final_x - inicial_x) * z, inicial_y + (final_y - inicial_y) * z,
            v.ancho * escala, v.alto * escala};
  pantalla = {imagen.x + v.pantalla.x * escala, imagen.y + v.pantalla.y * escala,
              v.pantalla.w * escala, v.pantalla.h * escala};
  const float e = ajuste * escala;
  juego = {pantalla.x + (pantalla.w - 320 * e) / 2, pantalla.y + (pantalla.h - 200 * e) / 2,
           320 * e, 200 * e};
  return true;
}

// Coloca los botones en sus píxeles físicos, incluso con escalado de escritorio.
bool Habitacion::controles(SDL_Renderer *r, SDL_FRect &pc, SDL_FRect &monitor, Uint64 ahora) const {
  SDL_FRect imagen, pantalla, juego;
  int w = 0, h = 0;
  if (!geometria(r, imagen, pantalla, juego, ahora) || !SDL_GetRenderOutputSize(r, &w, &h))
    return false;
  const auto &v = variante(w, h);
  const float e = imagen.w / v.ancho;
  pc = {imagen.x + v.boton_pc.x * e, imagen.y + v.boton_pc.y * e, v.boton_pc.w * e,
        v.boton_pc.h * e};
  monitor = {imagen.x + v.boton_monitor.x * e, imagen.y + v.boton_monitor.y * e,
             v.boton_monitor.w * e, v.boton_monitor.h * e};
  return true;
}

// Prepara un viewport que también convierte correctamente los eventos del ratón.
bool Habitacion::preparar_encuadre(SDL_Renderer *r, Uint64 ahora) const {
  if (!habilitada)
    return true;
  SDL_FRect imagen, pantalla, juego;
  if (!geometria(r, imagen, pantalla, juego, ahora) ||
      !SDL_SetRenderLogicalPresentation(r, 0, 0, SDL_LOGICAL_PRESENTATION_DISABLED))
    return false;
  const float e = juego.w / 320;
  const SDL_Rect viewport{int(std::round(juego.x / e)), int(std::round(juego.y / e)), 320, 200};
  return SDL_SetRenderScale(r, e, e) && SDL_SetRenderViewport(r, &viewport);
}

// Distingue alimentación del PC, acercamiento y controles propios de la habitación.
bool Habitacion::evento(SDL_Window *ventana, SDL_Renderer *r, const SDL_Event &evento,
                        Uint64 ahora) {
  if (!habilitada)
    return false;
  if (evento.type == SDL_EVENT_KEY_DOWN && !evento.key.repeat) {
    if ((evento.key.mod & SDL_KMOD_CTRL) && evento.key.key == SDLK_H) {
      tema = (tema + 1) % std::size(temas_habitaciones);
      cerrar();
      registrar_accion("Habitación: %s", temas_habitaciones[tema].nombre);
      return true;
    }
    if ((evento.key.mod & SDL_KMOD_CTRL) && evento.key.key == SDLK_R) {
      ajustar_zoom(0, ahora);
      return true;
    }
    if ((evento.key.mod & SDL_KMOD_CTRL) &&
        (evento.key.key == SDLK_MINUS || evento.key.key == SDLK_KP_MINUS ||
         evento.key.key == SDLK_0 || evento.key.key == SDLK_KP_0 || evento.key.key == SDLK_PLUS ||
         evento.key.key == SDLK_EQUALS || evento.key.key == SDLK_KP_PLUS)) {
      if (estado == Estado::encendido)
        ajustar_zoom(
            evento.key.key == SDLK_MINUS || evento.key.key == SDLK_KP_MINUS ? zoom_destino - 0.2f
            : evento.key.key == SDLK_0 || evento.key.key == SDLK_KP_0       ? 1
                                                                            : zoom_destino + 0.2f,
            ahora);
      return true;
    }
  }
  if (evento.type == SDL_EVENT_MOUSE_WHEEL && (SDL_GetModState() & SDL_KMOD_CTRL)) {
    const float sentido =
        evento.wheel.direction == SDL_MOUSEWHEEL_FLIPPED ? -evento.wheel.y : evento.wheel.y;
    if (estado == Estado::encendido)
      ajustar_zoom(zoom_destino + (sentido > 0 ? 0.2f : -0.2f), ahora);
    return true;
  }
  if (evento.type != SDL_EVENT_MOUSE_BUTTON_DOWN)
    return false;
  int w = 0, h = 0, vw = 0, vh = 0;
  SDL_FRect imagen, pantalla, juego, pc, monitor;
  if (!SDL_GetRenderOutputSize(r, &w, &h) || !SDL_GetWindowSize(ventana, &vw, &vh) || vw <= 0 ||
      vh <= 0 || !geometria(r, imagen, pantalla, juego, ahora) || !controles(r, pc, monitor, ahora))
    return false;
  const float x = evento.button.x * w / vw, y = evento.button.y * h / vh;
  // Todos los hitboxes comparten coordenadas físicas, sin invadir las acciones del juego ampliado.
  const auto dentro = [&](const SDL_FRect &zona) {
    return x >= zona.x && y >= zona.y && x < zona.x + zona.w && y < zona.y + zona.h;
  };
  if (!ampliada(ahora) && evento.button.button == SDL_BUTTON_LEFT) {
    if (dentro(pc)) {
      encendido_programado = 0;
      if (estado == Estado::apagado) {
        encender(ahora);
      } else if (estado != Estado::apagando) {
        imagen_al_apagar = estado == Estado::encendido;
        estado = Estado::apagando;
        sonido_pendiente = 2;
        inicio_estado = ahora;
        registrar_accion("PC: apagando");
      }
      return true;
    }
    if (estado == Estado::encendido && dentro(monitor)) {
      sonido_pendiente = 3;
      ajustar_zoom(1, ahora);
      return true;
    }
  }
  return reiniciando() || !dentro(juego);
}

// Usa exactamente el mismo arranque al pulsar el botón o vencer el plazo inicial.
void Habitacion::encender(Uint64 ahora) {
  encendido_programado = 0;
  estado = Estado::arrancando;
  sonido_pendiente = 1;
  inicio_estado = ahora;
  registrar_accion("PC: encendido; BIOS y DOS completos");
}

// La solicitud se entrega una vez, después de siete segundos íntegros de BIOS y DOS.
bool Habitacion::tomar_reinicio(Uint64 ahora) {
  if (habilitada && encendido_programado && ahora >= encendido_programado) {
    encendido_programado = 0;
    if (estado == Estado::apagado)
      encender(ahora);
  }
  if (!habilitada || ahora < inicio_estado)
    return false;
  if (estado == Estado::apagando && ahora - inicio_estado >= 700000000ull)
    estado = Estado::apagado;
  if (estado != Estado::arrancando || ahora - inicio_estado < 7000000000ull)
    return false;
  estado = Estado::encendido;
  registrar_accion("DOS: juego cargado en el monitor");
  return true;
}

// Conserva la animación CRT sin deformar la geometría permanente del juego.
SDL_FRect Habitacion::destino_juego(Uint64 ahora) const {
  if (!reiniciando())
    return {0, 0, 320, 200};
  if (estado != Estado::apagando || !imagen_al_apagar)
    return {0, 100, 320, 0};
  const float t = std::clamp(
      float(ahora >= inicio_estado ? ahora - inicio_estado : 0) / 140000000.0f, 0.0f, 1.0f);
  const float h = 200 * (1 - t) * (1 - t);
  return {0, (200 - h) / 2, 320, h};
}

// Devuelve la propiedad de las texturas antes de cambiar de variante o cerrar.
void Habitacion::cerrar() {
  if (textura)
    SDL_DestroyTexture(textura);
  if (textura_bios)
    SDL_DestroyTexture(textura_bios);
  textura_bios = nullptr;
  for (auto &luz : pilotos_rojos) {
    if (luz)
      SDL_DestroyTexture(luz);
    luz = nullptr;
  }
  textura = nullptr;
  cargada = nullptr;
}

// Cambia únicamente los píxeles verdes del piloto, preservando brillo y carcasa del maestro.
static SDL_Texture *crear_piloto_apagado(SDL_Renderer *r, SDL_Surface *s, const SDL_FRect &zona) {
  SDL_Surface *luz = SDL_CreateSurface(int(zona.w), int(zona.h), SDL_PIXELFORMAT_RGBA32);
  if (!luz)
    return nullptr;
  bool bien = true;
  for (int y = 0; bien && y < luz->h; ++y)
    for (int x = 0; bien && x < luz->w; ++x) {
      Uint8 rojo = 0, verde = 0, azul = 0, alpha = 0;
      bien =
          SDL_ReadSurfacePixel(s, int(zona.x) + x, int(zona.y) + y, &rojo, &verde, &azul, &alpha);
      if (verde > rojo * 0.85f && verde > azul * 1.1f) {
        const auto intensidad = verde;
        verde = rojo / 4;
        rojo = intensidad;
      }
      bien = bien && SDL_WriteSurfacePixel(luz, x, y, rojo, verde, azul, alpha);
    }
  SDL_Texture *resultado = bien ? SDL_CreateTextureFromSurface(r, luz) : nullptr;
  SDL_DestroySurface(luz);
  if (resultado)
    SDL_SetTextureScaleMode(resultado, SDL_SCALEMODE_LINEAR);
  return resultado;
}

// Recorta el cristal una sola vez al cargar la variante; el maestro PNG permanece intacto.
static SDL_Surface *recortar_cristal(SDL_Surface *original, const VarianteHabitacion &v) {
  SDL_Surface *rgba = SDL_ConvertSurface(original, SDL_PIXELFORMAT_RGBA32);
  if (!rgba)
    return nullptr;
  float minimo_y = float(v.alto), maximo_y = 0;
  for (const auto &p : v.cristal) {
    minimo_y = std::min(minimo_y, p.y);
    maximo_y = std::max(maximo_y, p.y);
  }
  // Cuatro muestras verticales suavizan el contorno; el interior transparente se calcula por fila.
  for (int y = std::max(0, int(std::floor(minimo_y)));
       y < std::min(v.alto, int(std::ceil(maximo_y))); ++y) {
    float izquierdas[4], derechas[4];
    float minimo_x = float(v.ancho), maximo_x = 0;
    for (int muestra = 0; muestra < 4; ++muestra) {
      const float fila = y + (muestra + 0.5f) / 4;
      float izquierda = float(v.ancho), derecha = 0;
      for (unsigned int i = 0; i < 16; ++i) {
        const auto &a = v.cristal[i];
        const auto &b = v.cristal[(i + 1) % 16];
        if ((a.y <= fila && b.y > fila) || (b.y <= fila && a.y > fila)) {
          const float x = a.x + (fila - a.y) * (b.x - a.x) / (b.y - a.y);
          izquierda = std::min(izquierda, x);
          derecha = std::max(derecha, x);
        }
      }
      izquierdas[muestra] = izquierda;
      derechas[muestra] = derecha;
      minimo_x = std::min(minimo_x, izquierda);
      maximo_x = std::max(maximo_x, derecha);
    }
    auto *fila = static_cast<Uint8 *>(rgba->pixels) + y * rgba->pitch;
    for (int x = std::max(0, int(std::floor(minimo_x)));
         x < std::min(v.ancho, int(std::ceil(maximo_x))); ++x) {
      float cobertura = 0;
      for (int muestra = 0; muestra < 4; ++muestra)
        cobertura += std::clamp(std::min(float(x + 1), derechas[muestra]) -
                                    std::max(float(x), izquierdas[muestra]),
                                0.0f, 1.0f) /
                     4;
      const auto alpha = Uint8(std::round(fila[x * 4 + 3] * (1 - cobertura)));
      fila[x * 4 + 3] = alpha;
      if (alpha == 0)
        fila[x * 4] = fila[x * 4 + 1] = fila[x * 4 + 2] = 0;
    }
  }
  return rgba;
}

// Compone únicamente el exterior del monitor sobre el juego ya renderizado.
bool Habitacion::dibujar(SDL_Renderer *r, Uint64 ahora) {
  if (!habilitada || (ampliada(ahora) && estado == Estado::encendido))
    return true;
  int w = 0, h = 0;
  SDL_FRect imagen, pantalla, juego;
  if (!SDL_GetRenderOutputSize(r, &w, &h) || !geometria(r, imagen, pantalla, juego, ahora))
    return false;
  const auto &v = variante(w, h);
  if (cargada != &v) {
    cerrar();
    const char *base = SDL_GetBasePath();
    const std::string ruta = std::string(base ? base : "") + v.archivo;
    SDL_Surface *s = SDL_LoadPNG(ruta.c_str());
    if (!s)
      return false;
    SDL_Surface *recortada = recortar_cristal(s, v);
    textura = recortada ? SDL_CreateTextureFromSurface(r, recortada) : nullptr;
    SDL_DestroySurface(recortada);
    pilotos_rojos[0] = crear_piloto_apagado(r, s, v.piloto_pc);
    pilotos_rojos[1] = crear_piloto_apagado(r, s, v.piloto_monitor);
    SDL_DestroySurface(s);
    if (!textura || !pilotos_rojos[0] || !pilotos_rojos[1] ||
        (!SDL_SetTextureScaleMode(textura, SDL_SCALEMODE_LINEAR) ||
         !SDL_SetTextureBlendMode(textura, SDL_BLENDMODE_BLEND)))
      return false;
    cargada = &v;
  }
  // Comparte el viewport físico redondeado con BIOS, apagado CRT y el ratón.
  SDL_Rect viewport;
  float sx = 1, sy = 1;
  if (!SDL_GetRenderViewport(r, &viewport) || !SDL_GetRenderScale(r, &sx, &sy))
    return false;
  juego = {viewport.x * sx, viewport.y * sy, viewport.w * sx, viewport.h * sy};
  if (!SDL_SetRenderViewport(r, nullptr) || !SDL_SetRenderScale(r, 1, 1))
    return false;
  // Una textura RGBA sobre el juego sustituye los cuatro recortes rectangulares por fotograma.
  bool bien = SDL_RenderTexture(r, textura, nullptr, &imagen);
  const float e = imagen.w / v.ancho;
  // Pilotos discretos cubren las luces dibujadas en los maestros, sin reescribir sus PNG.
  const auto piloto = [&](const SDL_FRect &zona, SDL_Texture *luz) {
    const SDL_FRect destino{imagen.x + zona.x * e, imagen.y + zona.y * e, zona.w * e, zona.h * e};
    return SDL_RenderTexture(r, luz, nullptr, &destino);
  };
  if (v.boton_pc_auxiliar && !ampliada(ahora)) {
    SDL_FRect pc, monitor;
    if (!controles(r, pc, monitor, ahora))
      return false;
    bien = bien && SDL_SetRenderDrawColor(r, 180, 166, 141, 255) && SDL_RenderFillRect(r, &pc) &&
           SDL_SetRenderDrawColor(r, 45, 40, 35, 255) && SDL_RenderRect(r, &pc) &&
           SDL_RenderDebugText(r, pc.x + pc.w / 2 - 4, pc.y + pc.h / 2 - 4, "I");
  }
  if (!ampliada(ahora) && estado != Estado::encendido && estado != Estado::arrancando)
    bien =
        bien && piloto(v.piloto_pc, pilotos_rojos[0]) && piloto(v.piloto_monitor, pilotos_rojos[1]);
  if (estado == Estado::arrancando)
    bien = bien && dibujar_arranque(r, juego, ahora);
  if (estado == Estado::apagando && ahora >= inicio_estado + 100000000ull &&
      ahora < inicio_estado + 260000000ull) {
    const float resto = 1 - float(ahora - inicio_estado - 100000000ull) / 160000000.0f;
    const SDL_FRect linea{juego.x + juego.w * (1 - resto * resto) / 2, juego.y + juego.h / 2,
                          juego.w * resto * resto, std::max(1.0f, juego.h / 200)};
    bien = bien && SDL_SetRenderDrawColor(r, 225, 245, 255, 255) && SDL_RenderFillRect(r, &linea);
  }
  return SDL_SetRenderScale(r, sx, sy) && SDL_SetRenderViewport(r, &viewport) && bien;
}

// Recrea en trazos VGA el distintivo amarillo de Energy Star de las BIOS de época.
static bool dibujar_distintivo_energia(SDL_Renderer *dibujante, float x, float y) {
  // Las coordenadas comparten la escala 8x16 del texto y quedan en la esquina superior derecha.
  const SDL_FPoint arco[] = {{x + 452, y + 27}, {x + 457, y + 17}, {x + 470, y + 9},
                             {x + 490, y + 4},  {x + 516, y + 2},  {x + 548, y + 2},
                             {x + 575, y + 5},  {x + 598, y + 10}};
  const SDL_FPoint estrella[] = {{x + 596, y + 10}, {x + 602, y + 20}, {x + 624, y + 20},
                                 {x + 607, y + 26}, {x + 613, y + 36}, {x + 596, y + 30},
                                 {x + 579, y + 36}, {x + 585, y + 26}, {x + 568, y + 20},
                                 {x + 590, y + 20}, {x + 596, y + 10}};
  const SDL_FPoint subrayado[] = {{x + 452, y + 37}, {x + 565, y + 37}, {x + 581, y + 34}};
  return SDL_SetRenderDrawColor(dibujante, 255, 255, 0, 255) &&
         SDL_RenderLines(dibujante, arco, int(SDL_arraysize(arco))) &&
         SDL_RenderDebugText(dibujante, x + 468, y + 24, "energy") &&
         SDL_RenderLines(dibujante, estrella, int(SDL_arraysize(estrella))) &&
         SDL_RenderLines(dibujante, subrayado, int(SDL_arraysize(subrayado))) &&
         SDL_RenderDebugText(dibujante, x + 452, y + 41, "EPA POLLUTION PREVENTER") &&
         SDL_SetRenderDrawColor(dibujante, 192, 192, 192, 255);
}

// Recrea el encendido de un 386 con texto de ochenta columnas y tiempos independientes de FPS.
static bool dibujar_bios(SDL_Renderer *dibujante, const SDL_FRect &juego, Uint64 ahora,
                         Uint64 inicio_estado) {
  const auto tiempo = (ahora >= inicio_estado ? ahora - inicio_estado : 0) / 1000000ull;
  if (!SDL_SetRenderDrawColor(dibujante, 0, 0, 0, 255) || !SDL_RenderFillRect(dibujante, &juego))
    return false;
  // La fuente SDL de ocho píxeles se estira a 8x16, como el texto VGA de ochenta columnas.
  const float sx = juego.w / 640, sy = juego.h / 200;
  if (!SDL_SetRenderScale(dibujante, sx, sy))
    return false;
  const float x = juego.x / sx, y = juego.y / sy;
  // Cada línea aparece completa en su instante; ninguna tecla adelanta esta presentación.
  const auto linea = [&](float fila, const char *texto) {
    return SDL_RenderDebugText(dibujante, x + 8, y + fila * 8, texto);
  };
  bool bien = SDL_SetRenderDrawColor(dibujante, 192, 192, 192, 255);
  if (tiempo < 3200) {
    bien = bien && dibujar_distintivo_energia(dibujante, x, y) &&
           linea(1, "AMI   American Megatrends") && linea(3, "Curse Reborn - 80386 DX / 33 MHz") &&
           linea(4, "Adaptador VGA detectado - 256 KB");
    if (tiempo >= 400) {
      const auto memoria = std::min<Uint64>(4096, (tiempo - 400) * 4096 / 1800);
      bien = bien &&
             SDL_RenderDebugTextFormat(dibujante, x + 8, y + 56, "Comprobando memoria: %4u KB%s",
                                       unsigned(memoria), memoria == 4096 ? " OK" : "");
    }
    if (tiempo >= 2300)
      bien = bien && linea(10, "Teclado ...................... OK") &&
             linea(11, "Unidad A: 1.44 MB ............. OK") &&
             linea(12, "Disco duro C: ................ OK");
    if (tiempo >= 2800)
      bien = bien && linea(16, "Iniciando desde el disco duro...");
  } else {
    bien = bien && linea(1, "Iniciando MS-DOS...");
    if (tiempo >= 3700)
      bien = bien && linea(4, "HIMEM: comprobacion de memoria extendida finalizada.");
    if (tiempo >= 4100)
      bien = bien && linea(6, "C:\\>SET BLASTER=A220 I5 D1 H5 T6");
    if (tiempo >= 4500)
      bien = bien && linea(8, "C:\\>CD CURSE");
    if (tiempo >= 5000)
      bien = bien && linea(10, "C:\\CURSE>CURSE");
    if (tiempo >= 5500)
      bien = bien && linea(13, "Cargando Curse of Enchantia Reborn...");
    if (tiempo >= 6100)
      bien = bien && linea(15, "Preparando graficos y datos del juego...");
    if ((tiempo / 350) % 2 == 0)
      bien = bien && linea(18, "_");
  }
  return SDL_SetRenderScale(dibujante, 1, 1) && bien;
}

// Dibuja primero a resolución VGA para que los trazos del distintivo no desaparezcan al reducir.
bool Habitacion::dibujar_arranque(SDL_Renderer *r, const SDL_FRect &juego, Uint64 ahora) const {
  if (!textura_bios) {
    textura_bios = SDL_CreateTexture(r, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, 640, 400);
    if (!textura_bios || !SDL_SetTextureScaleMode(textura_bios, SDL_SCALEMODE_LINEAR))
      return false;
  }
  SDL_Texture *anterior = SDL_GetRenderTarget(r);
  if (!SDL_SetRenderTarget(r, textura_bios))
    return false;
  const bool bien = SDL_SetRenderScale(r, 1, 1) && SDL_SetRenderViewport(r, nullptr) &&
                    dibujar_bios(r, {0, 0, 640, 400}, ahora, inicio_estado);
  const bool restaurado = SDL_SetRenderTarget(r, anterior) && SDL_SetRenderScale(r, 1, 1) &&
                          SDL_SetRenderViewport(r, nullptr);
  return bien && restaurado && SDL_RenderTexture(r, textura_bios, nullptr, &juego);
}
