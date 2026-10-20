#pragma once
#include "../mazmorra.hpp"
#include "../../../motor/estado/partidas.hpp"
#include <cstdio>
#include <memory>
#include <set>
#include <string>

// Reanuda la misma partida entre las entradas reales del recorrido, sin fabricar inventario ni
// banderas.
class ComprobarContinuidad {
public:
  // La carpeta solo se admite desde el recorrido de pruebas y mantiene los SAV fuera del perfil
  // real.
  explicit ComprobarContinuidad(const char *destino) : carpeta(destino ? destino : "") {}

  // Guarda una vez por escena/progreso/inventario y deja que las acciones siguientes continúen tras
  // cargar.
  bool comprobar(Mazmorra &partida) {
    if (carpeta.empty() || !partida.reposo_para_comprobar_partida())
      return true;
    std::vector<unsigned char> archivo(motor::bytes_archivo_partidas, 0);
    if (!partida.guardar_partida(archivo.data(), static_cast<unsigned int>(archivo.size()), 0,
                                 "CONTINUIDAD"))
      return false;
    // E718 incluye estos campos persistentes; el azar y CS siguen vivos como al cargar en DOS.
    const auto byte = [&](unsigned int direccion) {
      return archivo[1 + motor::caracteres_nombre_partida + direccion -
                     motor::inicio_estado_partida];
    };
    std::string clave = std::to_string(partida.escena_actual()) + ':';
    for (unsigned int p = 0x35c7; p <= 0x35ca; ++p)
      clave.push_back(static_cast<char>(byte(p)));
    for (unsigned int p = 0x5225; p < 0x5239; ++p)
      clave.push_back(static_cast<char>(byte(p)));
    if (!vistos.insert(clave).second)
      return true;
    auto imagenes = std::make_unique<motor::ImagenIndexada[]>(2);
    const auto escena = partida.escena_actual();
    const auto ventana = partida.ventana_actual();
    if (!partida.dibujar(imagenes[0]))
      return false;
    const std::string nombre = std::to_string(vistos.size()) + "_escena_" + std::to_string(escena);
    if (!SDL_SaveFile((carpeta + '/' + nombre + ".sav").c_str(), archivo.data(), archivo.size()) ||
        !partida.cargar_partida(archivo.data(), static_cast<unsigned int>(archivo.size()), 0) ||
        escena != partida.escena_actual() || ventana != partida.ventana_actual() ||
        !partida.dibujar(imagenes[1]))
      return fallar(nombre, "guardar, cargar o reconstruir escena");
    for (unsigned int i = 0; i < 64000; ++i)
      if (imagenes[0].indices[i] != imagenes[1].indices[i])
        return fallar(nombre, "píxeles tras cargar");
    for (unsigned int i = 0; i < 768; ++i)
      if (imagenes[0].paleta[i] != imagenes[1].paleta[i])
        return fallar(nombre, "paleta tras cargar");
    informe += nombre + '\t' + std::to_string(ventana) + '\n';
    if (!SDL_SaveFile((carpeta + "/continuidad.tsv").c_str(), informe.data(), informe.size()))
      return false;
    return true;
  }

private:
  std::string carpeta, informe;
  std::set<std::string> vistos;
  // Identifica el punto exacto que no puede reanudarse sin ocultar el error del adaptador.
  static bool fallar(const std::string &nombre, const char *motivo) {
    std::fprintf(stderr, "Continuidad %s: %s.\n", nombre.c_str(), motivo);
    return false;
  }
};
