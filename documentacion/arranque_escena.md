# Arranque y primer estado de escena

Los bloques de esta entrega se traducen de `CURSE.EXE`, con la misma huella fijada en el mapa. No usan un motor alternativo ni código de descompresión ajeno. La aplicación nativa lee los archivos originales, sin ejecutar el EXE ni Unicorn.

## Recorridos reconstruidos

| Original      | Traslado y alcance                                                                                                                                                                   |
| ------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| `0498`–`0513` | Lee `CORE.PAL` y `CORE.DAT`, descomprime por método uno y presenta 320 × 200. Reutiliza `preparar_titulo`, el mismo recorrido gráfico que el título.                                 |
| `0516`–`0569` | Espera hasta 2.100 retrazos o entrada y llama al fundido de 32 pasos. SDL usa una frecuencia nominal de 70 Hz; la duración exacta frente a DOSBox y un monitor real sigue pendiente. |
| `0584`–`0612` | Presentación de `TITLE.PAL` y `TITLE.DAT`, sin añadir un fundido de entrada que estas instrucciones no realizan.                                                                     |
| `0679`–`069A` | Obtiene los 64 colores altos de `SPRITES.PAL`, que el fondo de escena no reemplaza.                                                                                                  |
| `4EE5`–`4F25` | MAP: primeros 576 componentes de paleta, dos bytes de párrafos reservados y banco con referencias en párrafos de 16 bytes.                                                           |
| `50D2`–`513A` | Selecciona diez tiras de 32 × 200, descomprime mediante `67BC` y sube por planos en orden de columna nueve a cero. `FF` produce una tira negra.                                      |
| `EBE8`–`EC02` | Localiza un fotograma por el índice de siete bits y su referencia en párrafos dentro del banco BRD/SPR.                                                                              |
| `6D8C`–`7274` | Dibujo con transparencia, espejo y recorte. Se adapta a píxeles lineales; no conserva las listas de daños necesarias para restaurar páginas VGA.                                     |
| `77E4`–`7879` | Fundido: componente × 256; decremento entero dividido por el número de pasos; resta y envío de la parte alta en cada paso. Se compara cada paleta con el DAC original.               |

El formato del fotograma tiene diez bytes de cabecera: ancho, alto y tres desplazamientos. El orden real de los cuatro flujos, por el apilado de `6F31`, es base, campo ocho, campo seis y campo cuatro. En cada flujo, los códigos positivos copian colores, los negativos dejan huecos y cero termina la fila. Los huecos conservan el fondo, incluso cuando un literal tiene índice de color cero.

El descriptor inicial está en **DS:859E**: entidad activa, imagen cero, posición `(125, 98)`, secuencia **DS:8197** y modelo cero (`MANA.BRD`). Estas son direcciones del segmento de datos, no entradas CS del listado. Este primer traslado componía el estado inicial de Brad colgado boca abajo. La [integración posterior de objetos y guiones](objetos_mazmorra.md) ya carga la lista original y avanza las animaciones autónomas; no habilita movimiento ficticio ni sustituye las acciones pendientes del jugador.

`E19F`, investigada durante el seguimiento del título, abre `curse.sav`: corresponde a carga de partidas y no debe confundirse con una presentación cinematográfica.

## Controles disponibles

```sh
pnpm ventana
```

El recorrido disponible es **Core → título → introducción original → mazmorra con objetos y animaciones autónomas**. Una tecla o botón izquierdo inicia el fundido normal. En Core también comienza al acabar la espera máxima. F6 avanza directamente; F5 retrocede directamente. Los atajos cambian de vista directamente para acelerar las pruebas; al entrar en la introducción, esta comienza desde su fundido inicial. Se detienen en los extremos y vuelven a preparar el estado al entrar en la mazmorra; no escriben partidas. Escape cierra.

Los bloques disponibles no representan una partida completa. Faltan la conexión del menú y las acciones, el control del personaje y las interacciones del puzle. Otras escenas usarán el mismo cargador, pero su conexión y lógica deben trasladarse y comprobarse antes de anunciarlas como funcionales.

## Comprobaciones

```sh
pnpm comparar:presentacion
```

Este es el comando amplio pendiente para el propietario: compila una vez y agrupa RNC de método dos, montaje de fondos, sprites y fundidos. No vuelve a ejecutar las comparaciones de acciones, entidades o audio ya confirmadas.

La muestra ya comprobada con `--muestra` tiene **166 casos/pasos**: 67 de RNC2, 3 de fondos, 32 de sprites y 64 paletas de fundido. Se comparan fondos de mazmorra y pasillo, tiras negras, memoria por planos, preservación de paleta, recortes por los bordes y espejo. Las paletas del fundido coinciden byte por byte con las escrituras del DAC original. La comparación simula el retrazo; no certifica su duración física.

`pnpm comprobar:escena` utiliza ahora la muestra de [objetos y composición](objetos_mazmorra.md) y compara los estados inicial y tras 32 pasos. Introduce F6 en la cola de eventos y termina mediante Escape. El control visible, F5, la maximización y la cadencia deben comprobarse manualmente. `pnpm comprobar:ventana` conserva la comprobación de los 64.000 píxeles del título y el paso desde Core.

Los resultados y la vista PNG quedan en `trabajo/comparacion_rnc2/`, `trabajo/comparacion_escena/`, `trabajo/comparacion_fundidos/` y `trabajo/comprobacion_escena_sdl/`. Nada de esos recursos se incorpora al historial.

El bloque `28B2` está trasladado y documentado en [introduccion_original.md](introduccion_original.md): bruja, balcón y béisbol, con sus páginas, paletas y fundidos.
