# Cuevas, piedras y picapedrero

La ruta persistente continúa desde POOL, después de recoger las algas y abrir la puerta secreta. Se recorren los corredores de CAVECOR de las escenas cuatro, cinco y seis y la habitación CAVERBMI del picapedrero, escena diez. Recoger piedras las retira de sus listas persistentes: regresar conserva tanto el inventario como la ausencia de las piedras ya recogidas. WELL, escena nueve, está disponible como vista de desarrollo; sus puzles se reconstruyen en el bloque final de las cuevas.

## Rutinas trasladadas

| Rutina DOS                     | Comportamiento                                                                      | Traslado                                       |
| ------------------------------ | ----------------------------------------------------------------------------------- | ---------------------------------------------- |
| `5849–59A3`, ayudante `615C`   | Recogida por identificador y lista persistente, capacidad, animación y demora       | `motor/acciones/cuevas.cpp`                    |
| `3E89–3F25`                    | Clasificación de piedras pequeñas, medianas y grandes                               | `motor/acciones/cuevas.cpp`                    |
| `9F39`, `9F85`, `9FCD`         | Entrega por tamaño, contador, desaparición de puntos del cartel y consumo de piedra | `motor/acciones/cuevas.cpp`                    |
| `A00A–A07B`                    | Recompensa de cuerda, acercamiento, guiones del picapedrero y puntuación            | `motor/acciones/cuevas.cpp`                    |
| `1905–1962`                    | Rutina ambiental de la escena seis y sus llamadas al generador aleatorio            | `motor/entidades/mazmorra.cpp`                 |
| `4B05`, `4C43`, `4D2B`, `5145` | Entradas según las tablas originales, incluida la conservación de listas            | `motor/estado/inicializar.cpp` y adaptador SDL |

En este ejecutable el cartel empieza con **cuatro piedras de cada tamaño**, doce en total. Los contadores son bytes con signo en `DS:35CB–35CD`. La traducción conserva el comportamiento de `DEC/JGE`, incluido el desbordamiento de `80h` a `7Fh`; no convierte ese caso en un descuento válido. Cada entrega suma cinco puntos con aritmética de 16 bits. La última entrega válida proporciona el identificador 55, cuerda, mediante la lista original de inventario.

La ruta automatizada recoge primero 27, 28 y 29 en la escena cuatro y 31, 32 y 34 en la cinco. Después de entregarlas quedan dos piedras de cada tamaño. Regresa a la cinco para recoger 33 y 35, pasa a la seis para obtener 38, 39, 40 y 41 y vuelve al picapedrero. La segunda entrega deja los tres contadores a cero y añade la cuerda. Todo se ejecuta mediante caminar y el menú; no se resuelven puzles escribiendo banderas.

Las salidas normales integradas en este tramo son 5/8 entre las escenas cuatro y cinco, 7/10 entre cinco y seis, 9/19 entre cinco y diez y 6/18 entre cuatro y nueve. Las salidas 11–16 y 20–24 se integran en los bloques de catapulta y final de las cuevas. F5/F6 incluyen las nuevas vistas, pero reinician la sesión y no sirven para conservar piedras de una vista anterior.

## Comprobaciones realizadas

- `comparar_cuevas.py --muestra`, con la biblioteca C++ actual: **16.204 casos y pasos coincidentes** desde la partida inicial hasta la cuerda y los regresos de control. Se comparan DS completo, azar, decisiones y composiciones con las rutinas del EXE mediante Unicorn.
- Variantes de recogida con inventario de cero, nueve y diez entradas, y puntuaciones cero y 65.535. Variantes de entrega de los tres tamaños con contadores cero, uno, cuatro, 127, 128 y 255 y otros tamaños completos o incompletos.
- Cuatro capturas SDL: primeras piedras, primera entrega, piedras del tercer corredor y cuerda. Coinciden sus 64.000 píxeles con la composición original del recorrido. La comprobación común actual recorre las quince vistas con treinta cambios F5/F6.

Comandos reproducibles: `pnpm comparar:cuevas --muestra` y `pnpm comprobar:cuevas`. El segundo prepara también las referencias y las capturas SDL. Para ahorrar recompilaciones durante el desarrollo se han ejecutado por separado el comparador con la biblioteca actual y las comprobaciones de captura con el ejecutable recién compilado. La comparación amplia sin `--muestra` queda disponible para ampliar las variantes heredadas.

El comprobador descarta la caché de traducción de Unicorn al recuperar bancos de una instantánea. Esto evita reutilizar sus bloques anteriores al regresar desde el recorrido ampliado a otra rama de prueba; no modifica el motor C++ ni el estado original de la partida.

## Pendientes

Este comando acredita la recogida, las entregas, animaciones y transiciones de las piedras. La ampliación de catapulta y final de las cuevas cubre las acciones posteriores. Sigue sin acreditar por sí solo todos los estados posibles o la cadencia completa de DOSBox.

El tramo posterior ya continúa por [catapulta e imán](catapulta_y_alambre.md) y [barro, pozo y cubo](final_cuevas.md). Este documento y su comando específico conservan el alcance de las piedras.
