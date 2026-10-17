#pragma once

// Cada atajo inicia la entrada DOS indicada; no copia el inventario de otra vista.
struct VistaDesarrollo {
  unsigned short escena;
  unsigned short origen;
  unsigned short salida;
  const char *nombre;
  const char *banco;
};

inline constexpr VistaDesarrollo vistas_desarrollo[] = {
    {0, 0, 0xffff, "mazmorra", "DUNGEON"},
    {1, 0, 0, "corredor", "CORRIDOR"},
    {2, 1, 1, "caída", "DROP"},
    {3, 2, 3, "lago", "LAKEBED"},
    {8, 3, 4, "poza de las cuevas", "POOL"},
    {4, 8, 17, "primer corredor de las cuevas", "CAVECOR"},
    {5, 4, 5, "segundo corredor de las cuevas", "CAVECOR"},
    {6, 5, 7, "tercer corredor de las cuevas", "CAVECOR"},
    {9, 4, 6, "pozo", "WELL"},
    {10, 5, 9, "picapedrero", "CAVERBMI"},
    {7, 6, 11, "cuarto corredor de las cuevas", "CAVECOR"},
    {11, 6, 12, "tablón y catapulta", "SEESAW"},
    {13, 7, 15, "cueva del monitor", "CAVE"},
    {14, 7, 16, "cueva de los cuatro agujeros", "CAVE"},
    {12, 6, 13, "pozo de extracción y cubo", "SHAFT"},
    {15, 12, 24, "salida al campo", "COUNTRY1"},
};
inline constexpr unsigned int cantidad_vistas =
    sizeof(vistas_desarrollo) / sizeof(vistas_desarrollo[0]);
