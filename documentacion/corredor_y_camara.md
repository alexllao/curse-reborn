# Corredor, cámara y regreso persistente

La salida cero de la celda enlaza con el índice de escena uno mediante DS:7482. La salida dos vuelve al índice cero. `Mazmorra::cambiar_habitacion` conserva DS y el generador, carga los bancos MAP/SPR de la escena, dispone BOXDET y vuelve a crear las entidades desde las listas persistentes ya modificadas. No vuelve a leer el estado inicial del EXE.

## Bloques y recursos

| Original                       | Reconstrucción                                                                          |
| ------------------------------ | --------------------------------------------------------------------------------------- |
| 0AE5, 4AF0                     | Petición de salida, fundido de 32 pasos y carga de la escena admitida                   |
| 4C43–4D40, 5145–5181           | Personaje, referencias de modelo, controles y estado de entrada                         |
| 4E98–4EC7                      | Longitud, referencia de tiras y ajuste de la ventana                                    |
| 50D2–513A                      | Fondo visible por diez tiras MAP originales                                             |
| 8AA6                           | Reconstitución de las entidades desde descriptores persistentes                         |
| EAFE–EB1D                      | Centro de Brad y umbrales con signo; capacidad de cámara izquierda/derecha              |
| EC99/ED41–ED92, EF8A/F05C–F0AD | Paso de 32 píxeles, 69 registros de entidad, superposiciones y tres destinos de acción  |
| EC8A–EF79, EF7F–F294           | Ocho retrazos de cámara, cuatro píxeles por retrazo, conservando la franja exterior VGA |
| 3858–389E                      | Mecanismo de identificador ocho: aproximación y dos entidades programadas               |
| BEA8–BEFC                      | Rutina específica del corredor, según ventana y estado                                  |

Fuentes principales: `apps/juego/motor/video/camara.cpp`, `apps/juego/motor/estado/inicializar.cpp`, `apps/juego/motor/acciones/menu.cpp`, `apps/juego/motor/acciones/corredor.cpp` y `apps/juego/plataformas/sdl/mazmorra.cpp`.

Los recursos del corredor son `CORRIDOR.MAP`, `CORRIDOR.SPR`, STD y los modelos que soliciten los guiones. El adaptador conserva las referencias como etiquetas de bancos separados. Los fundidos y desplazamientos detienen los pasos lógicos mientras la plataforma presenta sus retrazos nominales a 70 Hz.

## Comprobaciones

`pnpm comparar:corredor --muestra` prolonga la partida comprobada de la mazmorra hasta salir, recoger la pecera y atravesar el corredor con sus golpes de estatua, caída y llegada al agua. También conserva la rama de mecanismo y regreso a la celda. Contrasta DS completo, generador y composición. Comprueba explícitamente las llaves conservadas y la puerta abierta al regresar.

Las **24 decisiones de umbral**, **87 traslados de cámara** y **18 posiciones gráficas** se contrastan en estados separados. Las posiciones gráficas se obtienen ejecutando las rutinas VGA completas, con sus copias, descompresión y escrituras de origen CRTC, y recuperando los píxeles de los cuatro planos. Incluyen posición inicial y ocho retrazos en cada dirección.

`pnpm comprobar:corredor` compila SDL y compara ocho capturas RGB de 64.000 píxeles: entrada, mecanismo, regreso, pecera, golpe, caída y llegada al agua sin audio y con audio activo en un dispositivo de prueba. La comprobación omite esperas de tiempo real; el uso normal presenta las esperas. La cadencia física de monitor y los tiempos de lectura de archivos DOS no se igualan.

La muestra verificada contiene **3.848 casos/pasos**, con regresión y capturas SDL repetidas tras ampliar las reservas para LAKEBED. `pnpm comparar:corredor` amplía las fases del guardia heredadas del puzle de la celda; no es necesario repetir todas las comparaciones anteriores.

## Límite de jugabilidad

La recogida de la pecera (56DC–574D) incorpora pecera y agua al inventario, retira sus descriptores persistentes y cambia las secuencias originales de las entidades del corredor. Las recogidas 574E–57F0 y sus auxiliares 563A, 615C y 623D conservan los límites del inventario, la puntuación y la retirada de objetos. Las selecciones y combinaciones se comparan contra las instrucciones originales.

El recorrido comprobado recoge la pecera, mantiene la dirección izquierda, recibe seis golpes de las estatuas y recupera el control antes de alcanzar la salida uno. BEA8 abre la puerta izquierda al aproximarse. La caída enlaza con LAKEBED conservando pecera y agua; la llave se retira por el guion original. Esto valida esa ruta de solución, sin acreditar todas las acciones opcionales posibles. Las acciones posteriores del lago, incluida ponerse la pecera, siguen pendientes.

[Guía de la partida y regreso](recorridos/01_corredor.txt).
