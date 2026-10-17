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
};
inline constexpr unsigned int cantidad_vistas =
    sizeof(vistas_desarrollo) / sizeof(vistas_desarrollo[0]);
