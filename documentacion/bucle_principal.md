# Bucle central y selección de rutinas

## Qué se ha comprobado

El análisis estático permite separar el arranque de un bucle central con cabecera en `0000:06E5`. Su retorno habitual está en `0000:0BC4`, que salta de nuevo a `0000:06E5`. Hay más retornos anticipados a esa misma cabecera durante el tratamiento de la entrada. El recorrido de salida comienza en `0000:0BC7` y termina con `RET` en `0000:0BF5`.

Las direcciones se refieren a la copia identificada en [el análisis inicial](analisis_ejecutable.md). Sigue pendiente comprobar este recorrido durante una ejecución real en DOSBox.

## Desde el arranque hasta el bucle

| Tramo | Evidencias observadas |
| --- | --- |
| `0000:0276` en adelante | Inicialización, llamadas auxiliares y reservas de memoria; búsqueda y lectura de `std.brd` y `mouse.spr`, contrastando los punteros a sus nombres. |
| `0000:0498`–`0000:057C` | Lectura de `core.pal` y `core.dat`, llamada a `0000:64E0` sobre el bloque cargado y presentación inicial. La semántica completa de `64E0` sigue pendiente. |
| `0000:0584`–`0000:0612` | Lectura de `title.pal` y `title.dat`, tratamiento del bloque y preparación de una espera de entrada. |
| `0000:061D`–`0000:0671` | Espera de teclado o dispositivo de entrada; una ruta llama a `0000:E19F`. No se asigna aún un nombre funcional definitivo a esa llamada. |
| `0000:06B2`–`0000:06B7` | Prepara `BX=FFFFh` y `AL=0` y llama a `0000:4AF0`, la rutina que participa en la selección y carga del estado de escena. |
| `0000:06BA`–`0000:06E2` | Prepara campos de control y, bajo una condición de configuración, inicializa coordenadas del dispositivo de entrada. |

## Recorrido de una iteración

| Entrada | Trabajo observado | Precisión de la interpretación |
| --- | --- | --- |
| `0000:06E5` | Llama a `0000:EAD5` y guarda `DS:2C2D` y `DS:2C2F` en `DS:0AE6` y `DS:0AE8`. | `EAD5` recorre presentación, desplazamiento de vista y espera del contador del temporizador; falta desglosar todos sus efectos. |
| `0000:06F4` | Consulta dos posiciones de la tabla de teclado y puede saltar a la salida `0BC7`. | La combinación consulta los índices `1Dh` y `2Eh`, correspondientes a códigos de exploración del teclado PC. |
| `0000:0706`–`0000:096A` | Trata estados de entrada y opciones; construye un índice de movimiento y puede llamar a `0000:0BF8`. | Hay rutas de teclado y dispositivos seleccionados por configuración. Algunas llamadas regresan directamente a la cabecera del bucle. |
| `0000:096D`–`0000:0997` | Si `DS:359A` es distinto de cero, suma pares de palabras con propagación de acarreo a los campos relacionados con las coordenadas y reduce el contador. | Se observa una actualización fraccionaria y entera de 16 bits; falta fijar unidades y nombres definitivos. |
| `0000:099A`–`0000:0A56` | Recorre enlaces desde `DS:0DFA`, puede llamar a `0000:0F5F` y actualiza otros bloques de estado. | Su significado completo todavía requiere seguir las estructuras enlazadas. |
| `0000:0A59` | Llama a `0000:83DE`; si `DS:9975` contiene un valor distinto de `FFh`, lo consume y entra en la ruta de cambio de escena. | Se identifica una solicitud pendiente; todavía falta documentar quién la produce en todos los casos. |
| `0000:0A78`–`0000:0BA0` | Calcula límites a partir de posición, tamaño y otros campos; llama a `0000:254A` y decide entre varias rutas. | Candidato a comprobación espacial y transiciones. La interpretación completa de los bits devueltos sigue pendiente. |
| `0000:0BA6`–`0000:0BC1` | Desplaza nueve palabras de una tabla, incorpora el valor de `DS:2C2D` y llama a `0000:10EC`. | `10EC` recorre 68 registros de 33 bytes y termina consultando una rutina particular por escena. |
| `0000:0BC4` | Vuelve a `0000:06E5`. | Retorno explícito del bucle central. |

La ruta de cambio de escena observada en `0000:0AE5` limpia la parte alta de BX, prepara `AL=0`, llama a `0000:4AF0` y vuelve a la cabecera. Es una ruta concreta confirmada, no una afirmación de que todas las transiciones del juego pasen exactamente por ella.

## Índice de escena y carga

En `0000:4D2B` se conserva el índice actual de `DS:8082`. El nuevo valor se obtiene consultando `DS:7482 + BX×2`. Si difiere del actual, `0000:4D3C` lo guarda en `DS:8082`. Por tanto, el parámetro BX de entrada a `4AF0` no debe confundirse directamente con el índice final de recurso.

Después, `0000:4EC8` utiliza el índice final para consultar registros de ocho bytes en `DS:784E`. El primer campo es un puntero al nombre que abre la llamada DOS de `0000:4ED9`. Se han contrastado estos ejemplos:

| Índice final | Primer nombre del registro |
| --- | --- |
| 0 | `dungeon.map` |
| 1 | `corridor.map` |
| 3 | `lakebed.map` |
| 5 | `cavecor.map` |
| 36 | `hold.map` |
| 77 | `graveyd2.map` |

Esto relaciona el estado interno con nombres concretos de escenarios mediante la rutina original, sin deducir el formato únicamente de la extensión del archivo.

## Tablas indirectas incorporadas al recorrido

El archivo [mapa_ejecutable.json](mapa_ejecutable.json) describe las ubicaciones, los tamaños, los orígenes y las evidencias. Los punteros se leen de la copia local y se guardan en el informe local. El mapa exige que el SHA-256 coincida con la versión estudiada.

| Transferencia | Tabla | Posiciones | Interpretación |
| --- | --- | --- | --- |
| `0000:0C0E` | `CS:0C13` | 16 | Movimiento bajo la condición especial comprobada en `0BF8`: `DS:8082=24h` y `DS:358C=FEh`. |
| `0000:0D0F` | `CS:0D14` | 16 | Movimiento en el resto de los casos. |
| `0000:17A0` | `CS:17A3` | 78 | Rutinas particulares seleccionadas por `DS:8082`. Las entradas nulas hacen que la rutina retorne. Hay 31 destinos distintos no nulos en el tramo anotado. |
| `0000:52AD` | `CS:52B2` | 5 | Selección condicionada por `DS:0AD4`, campo que también gobierna rutas de sonido. La función de cada modalidad sigue pendiente. |
| `0000:EB48` | `CS:EB4D` | 3 | Selección de una rama de presentación según `DS:0DEF`. |

El índice de las dos tablas de movimiento se construye con cuatro bits en `083D`–`086D` y `0942`–`096A`, y se duplica antes de consultar palabras. Esto justifica las 16 combinaciones. El límite de 78 posiciones de la tabla de escenas y el de tres de presentación se infieren de la disposición del código contiguo; requieren validación adicional en ejecución. Anotar una tabla no demuestra que todos sus índices sean utilizados.

## Resultado y límites

Con las cinco tablas anotadas, el recorrido alcanza 12.553 instrucciones candidatas y 104 destinos de llamada directa. Ninguna instrucción de esta pasada se inicia en el interior de los cinco tramos de tabla anotados. No se debe interpretar esta cifra como porcentaje de motor recuperado.

Quedan tres transferencias indirectas sin destinos incorporados:

- `0000:0264` y `0000:8C81`: encadenan vectores de interrupción anteriores, cuyos segmentos dependen del entorno DOS.
- `0000:DE2B`: llama a una entrada de la tabla de trabajo `DS:B60C`. Su contenido inicial es nulo y hay código que lo modifica. Usar los ceros del archivo como si fueran una tabla definitiva ocultaría comportamiento del juego. El siguiente paso es seguir cómo se construye esa tabla en memoria.

Todavía no se ha trasladado ninguna de estas rutinas a C++. Antes de hacerlo hay que completar las estructuras que leen y escriben, y preparar una comparación con el comportamiento original.
