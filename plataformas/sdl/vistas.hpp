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
    {19, 17, 29, "La calle de los disfraces", "ROGUE"},
    {20, 17, 30, "La calle de la adivina", "GYPSY"},
    {23, 18, 33, "La tienda abandonada", "DERELICT"},
    {28, 21, 41, "La posada Red Dragon", "INNSIDE"},
    {24, 19, 35, "La tienda de disfraces de Benn", "BENN"},
    {26, 20, 38, "La tienda de la adivina", "SALLY"},
    {27, 20, 39, "La casa abandonada", "DERELICT"},
    {25, 24, 36, "El probador de Benn", "CHANGE"},
    {30, 22, 47, "El borde del mundo", "EDGE"},
    {31, 30, 72, "La habitación del loro sabio", "WISE"},
    {35, 20, 52, "La encrucijada del valle", "POST"},
    {34, 35, 60, "El mando del valle", "REMOTE"},
    {33, 34, 56, "El barco del valle", "CELESTE"},
    {32, 35, 57, "El valle del dinosaurio", "DINO"},
    {38, 35, 66, "La nube del valle", "CLOUD"},
    {37, 32, 64, "La cueva del detergente", "GHETTO"},
};
inline constexpr unsigned int cantidad_vistas =
    sizeof(vistas_desarrollo) / sizeof(vistas_desarrollo[0]);

// Única lista de enlaces habilitados; la auditoría la cruza con BOXDET y la tabla DOS.
struct SalidaIntegrada {
  unsigned short escena, salida;
};
inline constexpr SalidaIntegrada salidas_integradas[] = {
    {0, 0},   {1, 1},   {1, 2},   {2, 3},   {3, 4},   {8, 17},  {4, 5},   {4, 6},   {5, 7},
    {5, 8},   {5, 9},   {6, 10},  {6, 11},  {6, 12},  {6, 13},  {7, 14},  {7, 15},  {7, 16},
    {9, 18},  {10, 19}, {11, 20}, {12, 21}, {12, 24}, {13, 22}, {14, 23}, {15, 25}, {16, 26},
    {21, 42}, {21, 43}, {29, 51}, {17, 28}, {17, 31}, {18, 32}, {18, 34}, {22, 44}, {22, 47},
    {17, 29}, {17, 30}, {18, 33}, {21, 41}, {19, 37}, {20, 40}, {23, 45}, {28, 50}, {19, 35},
    {20, 38}, {20, 39}, {24, 46}, {26, 48}, {27, 49}, {24, 36}, {25, 27}, {30, 72}, {31, 73},
    {22, 72}, {20, 52}, {35, 60}, {34, 56}, {33, 61}, {33, 55}, {35, 71}, {33, 52}, {35, 57},
    {32, 58}, {33, 54}, {32, 53}, {33, 70}, {32, 71}, {32, 59}, {35, 66}, {38, 67}, {32, 64},
    {37, 65}};

// No confunde reconocer una salida original con disponer de su habitación y sus puzles.
inline bool salida_integrada(unsigned short escena, unsigned short salida) {
  for (const auto &enlace : salidas_integradas)
    if (enlace.escena == escena && enlace.salida == salida)
      return true;
  return false;
}

// Comparte los nombres entre navegación, consola y título sin confundir el índice DOS con F5/F6.
inline const char *nombre_escena(unsigned short escena) {
  for (const auto &vista : vistas_desarrollo)
    if (vista.escena == escena)
      return vista.nombre;
  return "Escena pendiente";
}

// Numera el título con el catálogo F5/F6, conservando el identificador DOS para las partidas.
inline std::string nombre_numerado_escena(unsigned short escena) {
  for (unsigned int i = 0; i < cantidad_vistas; ++i)
    if (vistas_desarrollo[i].escena == escena)
      return std::to_string(i + 1) + ": " + vistas_desarrollo[i].nombre;
  return nombre_escena(escena);
}
