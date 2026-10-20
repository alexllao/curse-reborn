#pragma once
#include "../tiempo_juego.hpp"
#include "../registro.hpp"

// Lee únicamente archivos del perfil aislado de pruebas para comprobar su conservación.
inline std::string leer_perfil_tiempo(const std::string &ruta) {
  size_t longitud = 0;
  auto *contenido = static_cast<char *>(SDL_LoadFile(ruta.c_str(), &longitud));
  const std::string texto = contenido ? std::string(contenido, longitud) : "";
  SDL_free(contenido);
  return texto;
}

// Contrasta pausas, persistencia, interrupciones y errores mediante relojes simulados, sin esperas.
inline bool comprobar_tiempo_juego(const std::string &carpeta) {
  const auto perfil = carpeta + "/perfil.json";
  const auto partida_dos = carpeta + "/CURSE.SAV";
  const std::string inicial = "{\"version\":1,\"milisegundos_jugados\":1000}";
  const std::string referencia_dos = "Referencia de prueba: no modificar";
  if (!SDL_CreateDirectory(carpeta.c_str()) ||
      !SDL_SaveFile(perfil.c_str(), inicial.data(), inicial.size()) ||
      !SDL_SaveFile(partida_dos.c_str(), referencia_dos.data(), referencia_dos.size()))
    return false;
  const auto foco = SDL_WINDOW_INPUT_FOCUS;
  if (!TiempoJuego::debe_contar(true, foco, false, false) ||
      TiempoJuego::debe_contar(false, foco, false, false) ||
      TiempoJuego::debe_contar(true, 0, false, false) ||
      TiempoJuego::debe_contar(true, foco | SDL_WINDOW_MINIMIZED, false, false) ||
      TiempoJuego::debe_contar(true, foco | SDL_WINDOW_HIDDEN, false, false) ||
      TiempoJuego::debe_contar(true, foco, true, false) ||
      TiempoJuego::debe_contar(true, foco, false, true))
    return SDL_SetError("No coincide la definición de tiempo efectivo");
  {
    TiempoJuego tiempo;
    tiempo.iniciar(perfil);
    if (!tiempo.admite_guardado() || tiempo.milisegundos_totales() != 1000 ||
        tiempo.milisegundos_sesion())
      return SDL_SetError("No se recupera el acumulado del perfil");
    tiempo.actualizar(false, 1000000000ull);
    tiempo.actualizar(false, 31000000000ull);
    tiempo.actualizar(true, 32000000000ull);
    tiempo.actualizar(true, 33000000000ull);
    tiempo.actualizar(false, 34000000000ull);
    if (tiempo.milisegundos_totales() != 2000 ||
        leer_perfil_tiempo(perfil).find("2000") == std::string::npos)
      return SDL_SetError("Una pausa suma tiempo o no conserva el acumulado");
    tiempo.actualizar(false, 100000000000ull);
    tiempo.actualizar(true, 101000000000ull);
    tiempo.actualizar(true, 101250000000ull);
    tiempo.actualizar(true, 112000000000ull);
    tiempo.actualizar(true, 112500000000ull);
    if (tiempo.milisegundos_sesion() != 1750 || tiempo.milisegundos_totales() != 2750)
      return SDL_SetError("La suspensión del reloj o el segundo plano suman tiempo");
    tiempo.cerrar();
    tiempo.cerrar();
  }
  {
    TiempoJuego tiempo;
    tiempo.iniciar(perfil);
    tiempo.actualizar(true, 0);
    tiempo.actualizar(true, 1000000000ull);
    if (tiempo.milisegundos_totales() != 3750 || tiempo.milisegundos_sesion() != 1000)
      return SDL_SetError("El acumulado no continúa entre ejecuciones");
    // Cada segundo es inferior al corte de suspensión y alcanza el guardado periódico.
    for (unsigned int segundo = 2; segundo <= 30; ++segundo)
      tiempo.actualizar(true, Uint64(segundo) * 1000000000ull);
    if (leer_perfil_tiempo(perfil).find("32750") == std::string::npos)
      return SDL_SetError("No se guarda automáticamente a los treinta segundos");
  }
  for (unsigned int fps : {30u, 60u, 144u}) {
    const auto archivo = carpeta + "/frecuencia_" + std::to_string(fps) + ".json";
    const std::string cero = "{\"version\":1,\"milisegundos_jugados\":0}";
    if (!SDL_SaveFile(archivo.c_str(), cero.data(), cero.size()))
      return false;
    TiempoJuego tiempo;
    tiempo.iniciar(archivo);
    tiempo.actualizar(true, 0);
    for (unsigned int paso = 1; paso <= fps * 10; ++paso)
      tiempo.actualizar(true, 10000000000ull * paso / (fps * 10));
    if (tiempo.milisegundos_totales() != 10000)
      return SDL_SetError("La duración depende de los FPS");
  }
  for (const std::string &invalido :
       {std::string("{\"version\":2,\"milisegundos_jugados\":9}"),
        std::string("{\"version\":1,\"milisegundos_jugados\":-1}"),
        std::string("{\"version\":1,\"milisegundos_jugados\":18446744073709551616}"),
        std::string("archivo incompleto")}) {
    const auto archivo = carpeta + "/desconocido.json";
    if (!SDL_SaveFile(archivo.c_str(), invalido.data(), invalido.size()))
      return false;
    {
      TiempoJuego tiempo;
      tiempo.iniciar(archivo);
      tiempo.actualizar(true, 0);
      tiempo.actualizar(true, 1000000000ull);
      if (tiempo.admite_guardado() || tiempo.guardar() || tiempo.milisegundos_sesion() != 1000)
        return SDL_SetError("Un perfil inválido bloquea la sesión o permite sobrescribirlo");
    }
    if (leer_perfil_tiempo(archivo) != invalido)
      return SDL_SetError("Se ha sobrescrito un perfil no reconocido");
  }
  if (leer_perfil_tiempo(partida_dos) != referencia_dos ||
      TiempoJuego::formatear(129662000) != "36:01:02" || TiempoJuego::formatear(0) != "00:00:00")
    return SDL_SetError("Se modifica CURSE.SAV o las horas se reinician al superar un día");
  registrar_accion("Tiempo efectivo: actividad, pausas, suspensión, tres frecuencias, guardado "
                   "periódico, recuperación y perfiles inválidos comprobados; CURSE.SAV intacto");
  return true;
}
