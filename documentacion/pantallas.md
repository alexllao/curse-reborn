# Nombres de las pantallas

Nombres descriptivos propios del port, en castellano; no se presentan como títulos oficiales del juego original. Se comparten en la consola, los cambios F5/F6 y el título de la ventana, también al atravesar una salida normal o cargar una partida.

El número de vista sigue el orden de F5/F6. El índice de escena es el identificador estable del ejecutable DOS y se conserva para las partidas y las futuras pistas. Core, título e introducción quedan fuera de las veintiséis vistas de desarrollo.

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
| 17    | 16         | El camino del asaltante         | COUNTRY2 |
| 18    | 21         | La entrada de Paso del Dragón   | DRAGON   |
| 19    | 29         | La tienda de Frank              | FRANKS   |
| 20    | 17         | La plaza de la fuente           | XROADS   |
| 21    | 18         | La calle de las maravillas      | SORCERY  |
| 22    | 22         | La tienda del mago              | MAGE     |
| 23    | 19         | La calle de los disfraces       | ROGUE    |
| 24    | 20         | La calle de la adivina          | GYPSY    |
| 25    | 23         | La tienda abandonada            | DERELICT |
| 26    | 28         | La posada Red Dragon            | INNSIDE  |

Los cuatro corredores de las cuevas son escenas distintas aunque comparten CAVECOR. Las cuevas del monitor y de los cuatro agujeros comparten CAVE, pero conservan nombres e índices independientes. Un recurso reutilizado no determina el nombre de una escena.

Las nueve vistas de la población cubren el trayecto de comida y transporte del mago, las dos calles de la fuente, la tienda abandonada y la posada con sus regresos. Las tiendas situadas en esas nuevas calles y las visitas posteriores siguen pendientes. El número de vistas no representa pantallas completadas al 100 %. Véanse el [alcance de la población](puzles_poblacion.md) y la [auditoría de rutas](auditoria_pantallas.md).

EDGE (escena 30) se identifica como **El borde del mundo (pendiente)**. Se carga al terminar el transporte y permanece detenido; no se ofrece como vista jugable F5/F6.

El catálogo está en `plataformas/sdl/vistas.hpp`. Al añadir una escena se incorpora allí su nombre y se actualiza esta tabla; los nombres no se utilizan como claves de partidas. El futuro sistema de pistas podrá referirse al índice de escena sin depender del texto visible.

Cuando exista el HUD propio, mostrará brevemente el nombre de la pantalla cada vez que se acceda a ella, también al regresar a una ya visitada. Reutilizará este catálogo. La duración y el aspecto quedan pendientes de decidir; este aviso todavía no está implementado.

El título de la ventana incluye el número de vista, por ejemplo «Curse Reborn — 1: La mazmorra» y «Curse Reborn — 4: El fondo del lago». Se actualiza al entrar, volver y cargar una partida; los números DOS del archivo de guardado no cambian.
