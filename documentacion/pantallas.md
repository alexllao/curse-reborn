# Nombres de las pantallas

Nombres descriptivos propios del port, en castellano; no se presentan como títulos oficiales del juego original. Se comparten en la consola, los cambios F5/F6 y el título de la ventana, también al atravesar una salida normal o cargar una partida.

El número de vista sigue el orden de F5/F6. El índice de escena es el identificador estable del ejecutable DOS y se conserva para las partidas y las futuras pistas. Core, título e introducción quedan fuera de las dieciséis escenas jugables.

| Vista | Escena DOS | Nombre                          | Recurso  |
| ----- | ---------- | ------------------------------- | -------- |
| 1     | 0          | La mazmorra                     | DUNGEON  |
| 2     | 1          | El corredor de las estatuas     | CORRIDOR |
| 3     | 2          | La caída al lago                | DROP     |
| 4     | 3          | El fondo del lago               | LAKEBED  |
| 5     | 8          | La poza de las cuevas           | POOL     |
| 6     | 4          | Primer corredor de las cuevas   | CAVECOR  |
| 7     | 5          | Segundo corredor de las cuevas  | CAVECOR  |
| 8     | 6          | Tercer corredor de las cuevas   | CAVECOR  |
| 9     | 9          | El pozo de los deseos           | WELL     |
| 10    | 10         | La cueva del picapedrero        | CAVERBMI |
| 11    | 7          | Cuarto corredor de las cuevas   | CAVECOR  |
| 12    | 11         | La cueva de la catapulta        | SEESAW   |
| 13    | 13         | La cueva del monitor            | CAVE     |
| 14    | 14         | La cueva de los cuatro agujeros | CAVE     |
| 15    | 12         | El pozo del cubo                | SHAFT    |
| 16    | 15         | La salida al campo              | COUNTRY1 |

Los cuatro corredores de las cuevas son escenas distintas aunque comparten CAVECOR. Las cuevas del monitor y de los cuatro agujeros comparten CAVE, pero conservan nombres e índices independientes. Un recurso reutilizado no determina el nombre de una escena.

La entrada siguiente, COUNTRY2 (escena 16), se identifica como **El camino del asaltante (pendiente)**. Sus puzles no están terminados y no se cuenta como otra pantalla completada.

El catálogo está en `plataformas/sdl/vistas.hpp`. Al añadir una escena se incorpora allí su nombre y se actualiza esta tabla; los nombres no se utilizan como claves de partidas. El futuro sistema de pistas podrá referirse al índice de escena sin depender del texto visible.
