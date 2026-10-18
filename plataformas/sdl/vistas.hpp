#pragma once
#include <string>

// Cada atajo inicia la entrada DOS indicada; no copia el inventario de otra vista.
struct VistaDesarrollo {
  unsigned short escena;
  unsigned short origen;
  unsigned short salida;
  const char *nombre;
  const char *banco;
};

inline constexpr VistaDesarrollo vistas_desarrollo[] = {
    {0, 0, 0xffff, "La mazmorra", "DUNGEON"},
    {1, 0, 0, "El corredor de las estatuas", "CORRIDOR"},
    {2, 1, 1, "La caída al lago", "DROP"},
    {3, 2, 3, "El fondo del lago", "LAKEBED"},
    {8, 3, 4, "La poza de las cuevas", "POOL"},
    {4, 8, 17, "Primer corredor de las cuevas", "CAVECOR"},
    {5, 4, 5, "Segundo corredor de las cuevas", "CAVECOR"},
    {6, 5, 7, "Tercer corredor de las cuevas", "CAVECOR"},
    {9, 4, 6, "El pozo de los deseos", "WELL"},
    {10, 5, 9, "La cueva del picapedrero", "CAVERBMI"},
    {7, 6, 11, "Cuarto corredor de las cuevas", "CAVECOR"},
    {11, 6, 12, "La cueva de la catapulta", "SEESAW"},
    {13, 7, 15, "La cueva del monitor", "CAVE"},
    {14, 7, 16, "La cueva de los cuatro agujeros", "CAVE"},
    {12, 6, 13, "El pozo del cubo", "SHAFT"},
    {15, 12, 24, "La salida al campo", "COUNTRY1"},
    {16, 15, 25, "El camino del asaltante", "COUNTRY2"},
    {21, 16, 26, "La entrada de Paso del Dragón", "DRAGON"},
    {29, 21, 42, "La tienda de Frank", "FRANKS"},
    {17, 21, 43, "La plaza de la fuente", "XROADS"},
    {18, 17, 28, "La calle de las maravillas", "SORCERY"},
    {22, 18, 32, "La tienda del mago", "MAGE"},
};
inline constexpr unsigned int cantidad_vistas =
    sizeof(vistas_desarrollo) / sizeof(vistas_desarrollo[0]);

// Comparte los nombres entre navegación, consola y título sin confundir el índice DOS con F5/F6.
inline const char *nombre_escena(unsigned short escena) {
  for (const auto &vista : vistas_desarrollo)
    if (vista.escena == escena)
      return vista.nombre;
  return escena == 30 ? "El borde del mundo (pendiente)" : "Escena pendiente";
}

// Numera el título con el catálogo F5/F6, conservando el identificador DOS para las partidas.
inline std::string nombre_numerado_escena(unsigned short escena) {
  for (unsigned int i = 0; i < cantidad_vistas; ++i)
    if (vistas_desarrollo[i].escena == escena)
      return std::to_string(i + 1) + ": " + vistas_desarrollo[i].nombre;
  return nombre_escena(escena);
}
