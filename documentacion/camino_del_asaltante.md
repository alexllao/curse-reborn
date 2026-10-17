# Pantalla diecisiete: el camino del asaltante

COUNTRY2 (escena 16) continúa el paseo desde el campo. El asaltante tropieza y deja caer la espada. Brad puede recogerla, usarla para resolver el encuentro, recoger el dinero y seguir el sendero hasta la entrada de Paso del Dragón, DRAGON (21), mediante la salida 26.

F5/F6 añade **El camino del asaltante** después de **La salida al campo**: diecisiete vistas jugables, con Core, título e introducción aparte. El atajo utiliza la entrada original 25 desde la escena 15 y empieza una sesión nueva. Las salidas normales conservan el inventario y el estado de la partida.

## Rutinas trasladadas

| Fuente original                  | Reconstrucción                                                                                                                                                                                    |
| -------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| 1993–19A6                        | Conserva el bloqueo hasta x=228 mientras está activo el asaltante; el motor común reinicia la franja en cada paso.                                                                                |
| 563A–5679 y 5C44–5C8C            | Recoger espada 75: capacidad del inventario, postura del asaltante, destino y demora del canal de la espada.                                                                                      |
| 300B–30D4                        | Usar espada: consume el objeto, conserva los tres canales del ataque, prepara el dinero y crea cuarenta fragmentos con sus consumos de azar. Añade cinco puntos con aritmética de dieciséis bits. |
| 5C8F–5CAC, 61A7–61F0 y 623D–625F | Recoger dinero 76: retira la oferta persistente y anima su recogida; conserva los canales alternativos del modelo pequeño.                                                                        |
| 53E9–541B desde 0AC8             | Tipo 3/id 1 de BOXDET: prepara destino `(183,96)` y las secuencias 9525, 9584 y 95DC del sendero en perspectiva. Retorna al flujo de 0BA0.                                                        |
| 4AF0 y entrada 26                | Llega a DRAGON conservando los objetos; esta entrada no equivale a completar los puzles de la población.                                                                                          |

El encuentro se gobierna con los descriptores y secuencias originales de COUNTRY2.SPR. El motor no sustituye el tropiezo ni el ataque por una animación inventada. **La espada se usa con Usar**, no con el verbo Atacar: se mantiene la selección original.

El núcleo está en `motor/acciones/asaltante.*`. El servicio de creación comparte el generador del juego: el ataque conserva los 120 consumos explícitos de azar, además de los que pudiera necesitar el creador. El agotamiento normal de registros no se interpreta como un error de recursos; sigue el resultado de 8B6F, sin crear una entidad ficticia. Los paneles, música y efectos existentes siguen siendo comunes.

## Comprobación

`pnpm comparar:asaltante --muestra` reanuda `campo_salida.ds.bin` y su semilla, procedentes del recorrido contrastado. Si falta esa referencia genera primero el tramo del campo y, si es necesario, la ruta de las cuevas. El primer arranque puede tardar más. Se comparan **3.229 casos y pasos**, incluidos **1.264 pasos jugables**, **1.264 ordenaciones de dibujo frente a F29A**, **668 selecciones**, doce fondos y doce composiciones, siete clics y la transición con carga persistente.

La auditoría recorre las listas de menú ofrecidas en **siete estados y posiciones**, desde el tropiezo hasta el camino despejado. Cubre hablar, examinar, recoger, usar, saltar y las manipulaciones que ofrecen esas listas, también sus rechazos. Las variantes adicionales prueban inventarios vacíos, casi llenos y llenos; modelos normal y pequeño; puntuación al límite; ocho semillas y cero, uno o cuarenta registros libres para el ataque. Las auditorías y variantes conservan la semilla de la partida que continúa después.

`pnpm comprobar:asaltante` añade **doce capturas SDL de 64.000 píxeles** y **treinta y cuatro cambios F5/F6** entre las diecisiete vistas. Las imágenes cubren llegada, tropiezo, espada caída y recogida, inicio y tres momentos del ataque, dinero, camino, sendero y llegada a la población. Resultados en `trabajo/comparacion_asaltante/` y `trabajo/comprobacion_asaltante_sdl/`.

Estas pruebas acreditan el recorrido, variantes y menús enumerados, no todos los tiempos y posiciones posibles. La entrada en DRAGON está admitida con su despacho original nulo; sus puzles, tiendas y demás opciones siguen pendientes. El audio usa las tablas comunes y no se añade una comparación acústica específica.

Véase el [recorrido propio](recorridos/09_camino_del_asaltante.txt).

### Estado gráfico común de la prueba SDL

La ordenación original es de tipo peine y conserva un historial para las entidades de igual profundidad. El recorrido SDL dibuja antes de cada paso; el comparador de lógica anterior componía solo en las capturas. Esta diferencia de cadencia se hizo visible al superponer los cuarenta fragmentos del ataque, aunque coincidieran sus datos y su azar.

Para aislar COUNTRY2, `comprobar:asaltante` conserva el orden de entrada de su recorrido SDL y lo comparte como **entrada común** con el EXE y el C++ del comparador, comprobando que sea una permutación válida de los 69 registros. Solo se comparte DS:BDD2–BEE5: no se copian imágenes esperadas, coordenadas, inventario, canales, azar ni resultados posteriores. A partir de esa entrada, cada paso ejecuta y contrasta F29A y las composiciones originales. El archivo procede de una captura anterior al tropiezo, no del fotograma que se verifica.

Esta comparación prueba la evolución de la pantalla y su composición desde ese estado común. No acredita que el historial de ordenación de todos los fotogramas de las quince pantallas anteriores haya sido reproducido por el comparador de lógica. El informe indica si se ha compartido la entrada; `comparar:asaltante` sin la variable de prueba utiliza el historial de la instantánea DOS disponible.

El tramo siguiente ya continúa por [Frank, comida y mago](puzles_poblacion.md). Las cifras anteriores corresponden a la entrega de COUNTRY2; el catálogo actual ofrece veintidós vistas de desarrollo. Las otras rutas de la población siguen pendientes.
