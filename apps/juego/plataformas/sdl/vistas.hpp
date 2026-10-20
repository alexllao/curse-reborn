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
    {36, 33, 62, "El interior del barco", "HOLD"},
    {39, 32, 68, "La plataforma eléctrica", "ELECTRO"},
    {43, 25, 74, "La llanura del ser azul", "SNOWAST4"},
    {42, 43, 80, "La morsa y el amigo del hielo", "SNOWAST3"},
    {41, 42, 78, "El agujero del pez y el esquimal", "SNOWAST2"},
    {40, 41, 76, "La cornisa de los amigos", "SNOWAST1"},
    {44, 40, 81, "El lago helado", "ICELAKE"},
    {45, 44, 82, "La entrada del palacio de hielo", "ICEPAL"},
    {46, 45, 83, "La sala de los dados", "ICEROOM"},
    {47, 46, 84, "El corredor de las poleas", "ICECORR"},
    {48, 47, 86, "El segundo corredor de hielo", "ICECORR"},
    {49, 48, 88, "El corredor de la ventana", "ICECORR"},
    {50, 47, 91, "La habitación del gato mecánico", "ICEROOM"},
    {51, 47, 93, "La habitación de la lupa", "ICEROOM"},
    {52, 48, 95, "La habitación del arma", "ICEROOM"},
    {53, 48, 97, "La habitación del megáfono", "ICEROOM"},
    {54, 49, 99, "La habitación del cubito de hielo", "ICEROOM"},
    {55, 49, 101, "La habitación del carámbano", "ICEROOM"},
    {56, 49, 90, "La sala del láser", "ICEROOM"},
    {57, 56, 103, "El trono de la reina del hielo", "ICEQUEEN"},
    {58, 57, 104, "La sala de la mano", "ICEROOM"},
    {59, 58, 105, "El paso del guardia", "ICEROOM"},
    {60, 58, 106, "La sala de las dos puertas", "ICEROOM"},
    {61, 59, 108, "El desvío central", "ICEROOM"},
    {62, 60, 109, "La sala de las cerillas", "ICEROOM"},
    {63, 59, 107, "La sala del guardia dormilón", "ICEGUARD"},
    {64, 22, 112, "El interior del ataúd", "GRAVE"},
    {65, 64, 113, "Las tumbas del cementerio", "GRAVEYD1"},
    {77, 65, 114, "La verja del cementerio", "GRAVEYD2"},
    {66, 77, 116, "El bosque del castillo", "COPSE"},
    {67, 66, 117, "La verja del castillo", "GATE"},
    {68, 67, 118, "El puente del castillo", "CASTLENT"},
    {69, 68, 119, "El vestíbulo del castillo", "CASTHALL"},
    {70, 69, 120, "El estudio del castillo", "STUDY"},
    {71, 69, 122, "La biblioteca del castillo", "LIBRARY"},
};
inline constexpr unsigned int cantidad_vistas =
    sizeof(vistas_desarrollo) / sizeof(vistas_desarrollo[0]);

// Única lista de enlaces habilitados; la auditoría la cruza con BOXDET y la tabla DOS.
struct SalidaIntegrada {
  unsigned short escena, salida;
};
inline constexpr SalidaIntegrada salidas_integradas[] = {
    {0, 0},    {1, 1},    {1, 2},    {2, 3},    {3, 4},    {8, 17},   {4, 5},    {4, 6},
    {5, 7},    {5, 8},    {5, 9},    {6, 10},   {6, 11},   {6, 12},   {6, 13},   {7, 14},
    {7, 15},   {7, 16},   {9, 18},   {10, 19},  {11, 20},  {12, 21},  {12, 24},  {13, 22},
    {14, 23},  {15, 25},  {16, 26},  {21, 42},  {21, 43},  {29, 51},  {17, 28},  {17, 31},
    {18, 32},  {18, 34},  {22, 44},  {22, 47},  {17, 29},  {17, 30},  {18, 33},  {21, 41},
    {19, 37},  {20, 40},  {23, 45},  {28, 50},  {19, 35},  {20, 38},  {20, 39},  {24, 46},
    {26, 48},  {27, 49},  {24, 36},  {25, 27},  {30, 72},  {31, 73},  {22, 72},  {20, 52},
    {35, 60},  {34, 56},  {33, 61},  {33, 55},  {35, 71},  {33, 52},  {35, 57},  {32, 58},
    {33, 54},  {32, 53},  {33, 70},  {32, 71},  {32, 59},  {35, 66},  {38, 67},  {32, 64},
    {37, 65},  {33, 62},  {36, 63},  {32, 68},  {39, 69},  {25, 74},  {43, 80},  {42, 79},
    {42, 78},  {41, 77},  {41, 76},  {40, 75},  {40, 81},  {44, 82},  {45, 83},  {46, 84},
    {47, 85},  {47, 86},  {48, 87},  {48, 88},  {49, 89},  {47, 91},  {50, 92},  {47, 93},
    {51, 94},  {48, 95},  {52, 96},  {48, 97},  {53, 98},  {49, 99},  {54, 100}, {49, 101},
    {55, 102}, {49, 90},  {56, 103}, {57, 104}, {58, 105}, {58, 106}, {59, 108}, {60, 108},
    {60, 109}, {61, 110}, {62, 110}, {59, 107}, {63, 111}, {22, 112}, {31, 112}, {64, 113},
    {65, 113}, {65, 114}, {77, 115}, {77, 116}, {66, 117}, {67, 118}, {68, 119}, {69, 120},
    {70, 121}, {69, 122}, {71, 123}};

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
