# Sonido y carga de modelos desde las secuencias

Este traslado añade la selección de sonido de `0000:5288`–`0000:53B2` y permite atender las operaciones `FBh` y `F8h` mediante servicios explícitos. La muestra de **245 casos** coincide con el original. El propietario ha confirmado la coincidencia de los **1.285 casos** de la comparación amplia.

## Selección de sonido

[seleccionar.cpp](../apps/juego/motor/sonido/seleccionar.cpp) devuelve una `OrdenSonido` con la rutina destinataria, referencia del recurso, posición, longitud y canal cuando correspondan. La selección se detiene en la entrada al dispositivo: no produce sonido todavía.

La entrada original recibe SI como registro de entidad y AL como identificador. Primero descarta entidades cuyo X es mayor que 319, cuya suma X más ancho es menor o igual a cero, cuyo Y es mayor que 199 o cuya suma Y más alto es menor o igual a cero. Las sumas seguidas inmediatamente de `JLE` se interpretan usando el resultado y el desbordamiento con signo, sin confundirlos con una simple comprobación del bit alto del resultado envuelto.

`DS:0AD4` selecciona cinco modalidades observadas. La modalidad uno se ha identificado como altavoz del PC siguiendo las rutinas inferiores. Los otros dispositivos todavía deben estudiarse.

| Modalidad | Código de selección | Comportamiento antes del dispositivo                                                                                                                                                                           |
| --------- | ------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `0`, `2`  | `52BD`              | Retorna sin emitir una orden.                                                                                                                                                                                  |
| `1`       | `52BF`–`5305`       | Selecciona un registro de tres bytes, filtra referencias nulas y prioridades, y entrega una secuencia a `7A46`. El identificador `FFh` no emite orden.                                                         |
| `3`       | `5307`–`5374`       | Selecciona un registro de cinco bytes, consulta un recurso de muestras y entrega parámetros a `7FB3`. Un recurso de escena nulo o una prioridad insuficiente puede omitir la orden. `FFh` tampoco emite orden. |
| `4`       | `5376`–`53B2`       | Selecciona un byte, omite el valor `FFh` o lo copia a `DS:92EC` y entrega `DS:92E9` a `7D6A`. El identificador de entrada `FFh` entrega directamente `DS:8C90`.                                                |

Los identificadores inferiores a quince usan tablas generales. Los demás consultan el grupo de la escena mediante `DS:9048 + byte(DS:8082)`, eligen una tabla de referencias y restan quince al identificador. Se conservan las diferencias entre las tres modalidades.

`EstadoSonido` hace explícitas las prioridades que el original conserva en `CS:5306` y `CS:5375`, la actividad de `CS:8351`, las referencias de recursos de `CS:5284/5286` y la referencia de datos. Así pueden conservarse entre llamadas sin escribir sobre código C++. La lectura de palabras del recurso de muestras se aporta mediante una función externa.

En la modalidad tres, el registro aporta prioridad, canal, cantidad que restar y casilla de recurso. La casilla permite leer una referencia y después la longitud de la muestra. Se devuelve la posición posterior a esa cabecera y la resta con envolvimiento de dieciséis bits, sin añadir un avance de posición que el original no realiza en este tramo.

## Servicios de las secuencias

[RecursosSecuencias](../apps/juego/motor/entidades/secuencias.hpp) incorpora dos funciones opcionales, con el mismo contexto que el lector de dimensiones:

| Servicio        | Entrada y contrato                                                                                                                                                                |
| --------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `sonido`        | Recibe datos, posición de entidad e identificador. Devuelve `true` únicamente cuando se ha atendido la operación y puede continuar la secuencia.                                  |
| `cargar_modelo` | Recibe datos, referencia del nombre y capacidad 64.000. El servicio coloca el contenido en el recurso de modelos que administra su contexto. Devuelve `true` cuando ha terminado. |

Sin función, o si devuelve `false`, se conserva la parada pendiente. Al atender sonido, se consume el argumento y continúa la interpretación. Un servicio que seleccione una orden de dispositivo debe ejecutarla o conservarla en una cola que garantice su posterior tratamiento antes de dar la operación por atendida. El puente de comparación solo acepta sonidos que el selector original omite, por modalidad desactivada o por quedar fuera de pantalla.

Para un modelo no residente, se mantiene el orden original: se escriben modelo e intervalos, se actualiza `DS:9974`, se consulta la referencia del nombre en `DS:0918 + modelo × 2`, se solicita la carga y se asigna la referencia de recurso antes de continuar leyendo la secuencia. Un acierto de caché evita la solicitud. No se relee el archivo en cada cambio de imagen.

El bloque DOS `8574`–`859D` abre en lectura, solicita **64.000 bytes** a partir del inicio del segmento de modelos y cierra el identificador obtenido. Las lecturas cortas conservan el resto del búfer. El servicio portable debe mantener ese comportamiento; no debe rellenar con ceros lo que el original no escribe.

El [reproductor del altavoz](altavoz_pc.md) amplía la modalidad uno con arranque, parada y temporizador. El acceso real al sistema de archivos y la síntesis audible siguen pendientes. Los casos de comparación aportan un servicio sintético. Los fallos de apertura o lectura DOS no se han trasladado ni comparado: el original no comprueba explícitamente sus acarreos en este bloque. Una respuesta `false` es una parada del contrato nuevo, no una recuperación atribuida al original; puede haber escrituras anteriores y todavía no existe un contrato de reanudación después de un fallo.

## Comparación agrupada

```sh
pnpm comparar:operaciones
```

| Grupo                                                   | Comparación amplia confirmada | Muestra comprobada |
| ------------------------------------------------------- | ----------------------------: | -----------------: |
| Cinco modalidades, tablas, prioridades y omisiones      |                           600 |                120 |
| Fronteras de visibilidad                                |                            45 |                 45 |
| Secuencias con sonido sin emisión y servicio de modelos |                           640 |                 80 |
| **Total**                                               |                     **1.285** |            **245** |

El selector original se ejecuta hasta su retorno o hasta la entrada a `7A46`, `7FB3` o `7D6A`. Se comparan DS completo, las prioridades conservadas en CS, la orden y la profundidad de la pila en esa frontera. Las rutinas de dispositivo no se ejecutan ni se sustituyen para declarar que existe audio.

Las secuencias originales ejecutan sus llamadas reales al selector. Un manejador de la interrupción DOS y un servicio nativo suministran el mismo archivo sintético a ambos lados de la comparación y registra apertura, lectura y cierre. Se prueban archivos de cero, 128, 64.000 y 65.000 bytes, con y sin acierto de caché, y dos llamadas consecutivas. Se compara también el búfer completo de 65.536 bytes y el estado aleatorio; los archivos mayores se limitan a la petición original de 64.000.

Los informes se guardan en `trabajo/comparacion_operaciones/muestra.json` y `resultado.json`, incluyendo las huellas del ejecutable y la biblioteca. La muestra se ejecuta añadiendo `--muestra`. Además, cuatro casos selectivos conservan las paradas y selecciones anteriores cuando no se proporcionan servicios.

El siguiente paso es estudiar las rutinas de dispositivo y la carga real de recursos, antes de conectarlas a la plataforma y a una presentación completa.
