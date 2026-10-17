# Objetos, composición y guiones de la mazmorra

La mazmorra ya carga la lista original de entidades y superposiciones, las ordena y avanza sus secuencias. La puerta, las cadenas, la antorcha y el resto de piezas se componen desde `DUNGEON.SPR`; Brad usa su banco original. La antorcha y los demás elementos que tienen secuencias autónomas se animan. **El menú de acciones y las decisiones del jugador todavía no están conectados; no es una partida jugable.**

## Rutinas reconstruidas

| Original                     | C++                                   | Alcance                                                                                                                                                |
| ---------------------------- | ------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------ |
| `4C43`–`4D28`, `5145`–`517D` | `motor/estado/inicializar.cpp`        | Entrada particular de la mazmorra, dimensiones del modelo y controles iniciales.                                                                       |
| `8AA6`–`8B6E`                | `motor/entidades/cargar.cpp`          | Lista por escena, entidades, superposiciones, desplazamiento de cámara y reservas. Reutiliza el bloque común de creación ya contrastado.               |
| `F29A`–`F30E`                | `motor/video/orden.cpp`               | Claves de profundidad e intercambios de los 69 registros, con comparaciones de palabra con signo.                                                      |
| `EBBD`–`EC89`                | `motor/video/entidades.cpp`           | Entidades visibles en su orden original y recortes que vuelven a quedar por delante.                                                                   |
| `183F`–`1904`                | `motor/entidades/mazmorra.cpp`        | Rutina de la escena cinco; véase la corrección de despacho documentada abajo.                                                                          |
| `891E`–`89D8`                | `motor/acciones/objetos_cercanos.cpp` | Objetos compatibles con una acción y próximos a los pies del personaje. Es la base de las listas de interacción, no una selección por píxel del ratón. |

La lista de la mazmorra empieza en DS:35D0 y contiene quince descriptores de entidades y dos de superposiciones. Cinco entidades están condicionadas por el bit 80 de su descriptor y DS:0AD6. Se conservan estas condiciones; no se decide su presencia a partir de una interpretación visual.

La profundidad combina Y, alto y el campo de profundidad, con envoltura de 16 bits. La ordenación conserva exactamente los saltos e intercambios del original; sustituirla por una ordenación estable podría cambiar empates. Las superposiciones se dibujan únicamente cuando solapan una entidad con estado exactamente uno, como en `EC41`.

## Datos originales y adaptación portable

Para reproducir las definiciones y guiones se lee el segmento inicial de datos de `CURSE.EXE`. Node verifica su SHA-256 contra el mapa antes de abrir la escena. El lector C++ comprueba el tamaño y la cabecera de la versión estudiada y copia desde `512 + 0F31h × 16`, completando a cero hasta 65.536 bytes. No desensambla ni ejecuta instrucciones durante la partida. Solo se leen además las cuatro palabras iniciales del generador, como datos.

La aplicación necesita ahora `datos_originales/CURSE/CURSE.EXE` para la mazmorra, además de `DUNGEON.MAP`, `DUNGEON.SPR`, `SPRITES.PAL`, `STD.BRD`, `MANA.BRD` y los bancos que soliciten sus guiones. Core y título siguen usando sus propios DAT/PAL. Las tablas, guiones y recursos originales no se copian al historial de Git. Una ejecución directa del binario, al margen del lanzador Node, exige que el llamador aporte la misma versión verificada del EXE.

`plataformas/sdl/mazmorra.cpp` aporta los bancos y los servicios de dimensiones y carga. Las referencias `4000`, `4100` y `4200` son etiquetas de escena, personaje estándar y modelo: nunca son direcciones accesibles de DOS o del proceso. F8 carga el nombre acotado del recurso solicitado. La modalidad de sonido se fija a cero; las órdenes se consumen sin bloquear los guiones ni abrir audio.

El avance reúne secuencias, comportamientos y rutina particular, en ese orden. La presentación restaura el fondo y recompone la imagen completa, sustituyendo las restauraciones entre páginas VGA. El paso de plataforma usa cinco ticks a la referencia nominal de 70 Hz existente. El original programa un divisor PIT `4250h`; la precisión temporal completa y los movimientos de ventana siguen pendientes de contrastar. No se anuncian tiempos físicos exactos.

F5/F6 siguen recorriendo Core, título y mazmorra. Cada entrada de desarrollo a la mazmorra prepara un estado nuevo; no carga ni escribe partidas guardadas.

## Comparación agrupada

```sh
pnpm comparar:objetos
```

El comando compila una vez y ejecuta las dos comparaciones. La muestra comprobada con `--muestra` contiene **145 casos/pasos**:

| Grupo                                                | Muestra comprobada | Comparación amplia preparada |
| ---------------------------------------------------- | -----------------: | ---------------------------: |
| Lectura MZ y rechazo de versiones incompatibles      |                  4 |                            4 |
| Entrada del personaje                                |                  1 |                            1 |
| Listas reales de mazmorra, pasillo y caída           |                  6 |                           18 |
| Banderas de carga, fases y contadores aleatorios     |                  8 |                            8 |
| Rutina particular de mazmorra                        |                 16 |                           16 |
| Ordenación con empates y desbordamientos de palabra  |                  8 |                          128 |
| Objetos próximos, máscaras y estados                 |                 36 |                           84 |
| Fondo original                                       |                  1 |                            1 |
| Composición de todas las entidades y superposiciones |                 33 |                          257 |
| Avance persistente con bancos y guiones reales       |                 32 |                          256 |
| **Total**                                            |            **145** |                      **773** |

Se compara DS completo y el estado aleatorio, además de 64.000 píxeles en cada composición y guardas fuera del búfer. La preparación recorre los fragmentos originales de entrada sin simular sus resultados: solo la lectura DOS del modelo se sustituye por el banco real ya cargado. El avance ejecuta las secuencias y el recorrido original completo de entidades, incluido su despacho por `CS:17A3`; en la escena cero no llama a `183F`.

El propietario ha confirmado los **773 casos/pasos** de la comparación amplia y que las animaciones se ven en la mazmorra. Las imágenes y los informes se guardan en `trabajo/comparacion_objetos/`. La lista de fuentes C++ se comparte entre las herramientas y CMake mediante `herramientas/fuentes_motor.json`.

```sh
pnpm comprobar:escena
```

Esta comprobación usa SDL con ventana oculta y dibujante de software: introduce F6, compara el estado inicial y vuelve a capturar tras 32 pasos de escena. Ambas capturas coinciden con la composición contrastada contra el ejecutable. No verifica aún clics, menú, acciones, inventario ni puzles, ni sustituye la prueba manual del dibujante visible.

## Siguiente conexión

El [menú original y las primeras interacciones](menu_e_interacciones.md) ya conectan `DAE2` y varios destinos de la tabla de acciones. La [integración de movimiento y colisiones](movimiento_y_colisiones.md) permite caminar tras la liberación y corrige el despacho que desplazaba indebidamente la puerta. Debe conservarse la distinción entre objeto visible, objeto disponible para una acción y efecto real de esa acción sobre el estado del puzle.

La atribución inicial de `183F` a la primera escena era incorrecta: corresponde al índice cinco de la tabla. La [corrección y su regresión completa](movimiento_y_colisiones.md) distinguen la traducción aislada de la asignación de escena.
