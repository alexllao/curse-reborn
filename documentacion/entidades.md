# Creación y actualización de entidades

El traslado a C++ cubre `0000:10EC`–`0000:1790`, la creación dinámica de `8B6F`–`8C25` y la transición de `C4DC`–`C5DF`. Se ha compilado con avisos tratados como errores y ha pasado una muestra diferencial de **374 casos**. El propietario ha confirmado también la coincidencia de los **3.028 casos** de la comparación amplia.

## Relación con el original

| Tramo original | Archivo en `apps/juego/motor/entidades/` | Alcance                                                                                               |
| -------------- | ---------------------------------------- | ----------------------------------------------------------------------------------------------------- |
| `10EC`–`1118`  | `actualizar.cpp`                         | Limpia dos bytes y cuatro palabras de límites; prepara el recorrido.                                  |
| `1119`–`1785`  | `actualizar.cpp`                         | Actualiza un registro activo según su tipo de comportamiento.                                         |
| `1786`–`1790`  | `actualizar.cpp`                         | Avanza 33 bytes y repite hasta recorrer 68 registros.                                                 |
| `8B6F`–`8C25`  | `crear.cpp`                              | Busca un hueco, copia un descriptor y su definición, consulta dimensiones y prepara fases aleatorias. |
| `C4DC`–`C5DF`  | `actualizar.cpp`                         | Cambia secuencias y posiciones del personaje y de dos registros auxiliares.                           |

La siguiente instrucción, `1791`, despacha una rutina particular por escena y queda fuera de este traslado. La creación puede llamar al generador original y la actualización puede llamar a creación, reinicio de acción, planificación y transición; la comparación ejecuta todas esas llamadas originales sin sustituirlas por simulaciones.

## Registros y descriptores

Los registros ocupan 33 bytes. El recorrido empieza en `DS:2C4C`; la reserva general de creación empieza en `2C0A` y contempla 70 posiciones, descontando la cantidad reservada de `3588`.

| Desplazamiento en el registro | Tamaño               | Uso observado                                                                                              |
| ----------------------------- | -------------------- | ---------------------------------------------------------------------------------------------------------- |
| `00`                          | Byte                 | Estado; la actualización estudiada procesa únicamente el valor uno.                                        |
| `01`                          | Byte                 | Índice de imagen con posibles bits adicionales.                                                            |
| `02`, `04`                    | Palabras             | Coordenadas X e Y.                                                                                         |
| `06`, `08`                    | Palabras             | Ancho y alto obtenidos del recurso.                                                                        |
| `0A`, `0C`                    | Palabras             | Campos copiados de la definición; significado completo pendiente.                                          |
| `0E`                          | Palabra              | Tipo de comportamiento.                                                                                    |
| `10`, `15`, `1A`              | Cinco bytes cada uno | Referencia de secuencia, índice de fase, contador e intervalo; uso precisado por el traslado de animación. |
| `1F`                          | Palabra              | Referencia de segmento de recurso conservada como dato del original.                                       |

El descriptor de creación ocupa nueve bytes: banderas, estado, imagen, X, Y y referencia a una definición de 18 bytes. A la coordenada X se le resta `DS:99B8`. Las banderas `02h` y `04h` activan la elección aleatoria de fase y contador; `80h` puede impedir la creación según `DS:0AD6`.

El conteo de fases incluye el terminador `7Fh`. Antes del segundo valor aleatorio, `MOV AL,[DI+14h]` conserva AH: el límite de esa división puede ocupar dos bytes. El C++ conserva ambos detalles.

## Comportamientos trasladados

| Tipo   | Comparación original | Efecto observado                                                                                             |
| ------ | -------------------- | ------------------------------------------------------------------------------------------------------------ |
| `0400` | `112C`               | Ajusta X respecto al registro anterior y al personaje.                                                       |
| `0401` | `115E`               | Calcula posición desde tablas de desplazamientos de la imagen del personaje, con extensión de signo de byte. |
| `0406` | `11C0`               | Desactiva al salir por abajo y puede cambiar secuencias en la escena ocho.                                   |
| `0407` | `121F`               | Reacciona a proximidad horizontal, inicia secuencias y puede crear una entidad.                              |
| `0408` | `12C8`               | Actualiza límites y desactiva al salir por la izquierda.                                                     |
| `0409` | `12F6`               | Actualiza límites, aproxima X o cambia el estado del personaje.                                              |
| `040A` | `135D`               | Comprueba solapamiento horizontal y elige secuencias según el lado.                                          |
| `040B` | `13E6`               | Cambia secuencias según diferencia vertical y fase.                                                          |
| `040C` | `14C2`               | Avanza, ajusta altura y reacciona a distancia, modelo e imagen del personaje.                                |
| `040D` | `15BD`               | Prepara cinco creaciones con posición e imagen aleatorias.                                                   |
| `040F` | `15FE`               | Actualiza límites o prepara un nodo y planifica movimiento.                                                  |
| `0411` | `1673`               | Comprueba proximidad y activa la transición `C4DC`.                                                          |
| `0410` | `16A3`               | Sigue al personaje con imagen y desplazamientos condicionados por su imagen.                                 |
| `0412` | `16EE`               | Comprueba proximidad, crea tres entidades y cambia secuencias del personaje.                                 |

Estos nombres describen efectos de las instrucciones; la identidad visual y narrativa de cada tipo todavía requiere comprobarse en el juego. Los tipos sin rama y los estados distintos de uno conservan el registro.

## Separación de los recursos

`RecursosEntidades` recibe una función que aporta ancho y alto de una imagen. El núcleo no accede a direcciones del proceso ni a segmentos DOS para leerlos. La palabra `segmento_original` se mantiene exclusivamente para conservar el campo de referencia y poder contrastar el estado; habrá que sustituir su interpretación al integrar la gestión portable de recursos.

En la comparación, las instrucciones originales leen una tabla de párrafos y cabeceras sintéticas en el segmento `4000h`. El C++ recibe las mismas 128 parejas de dimensiones mediante la función. Esto comprueba la selección de imagen y sus efectos en los registros, pero no demuestra que se haya trasladado la carga o descodificación de gráficos reales.

La API necesita un segmento de 65.536 bytes y un estado aleatorio explícitos. `valido=false` o el retorno `false` identifica entradas fuera del dominio admitido: cantidad reservada igual o superior a 70, lector ausente, secuencia sin terminador o división inválida. Puede haber escrituras anteriores al fallo; no se promete reversión. Las definiciones y descriptores deben ser válidos y no solaparse con el registro de destino. La comparación utiliza geometría acotada sin desbordamientos de comparaciones con signo.

## Comparación agrupada

```sh
pnpm comparar:entidades
```

| Grupo                                                          | Casos amplios preparados | Muestra comprobada |
| -------------------------------------------------------------- | -----------------------: | -----------------: |
| Creación con banderas, huecos y rechazo                        |                      280 |                 70 |
| Catorce comportamientos y cinco tipos sin rama                 |                    2.432 |                228 |
| Fronteras de imagen, estado, altura y variantes independientes |                       60 |                 60 |
| Transición `C4DC`                                              |                      128 |                  8 |
| Recorrido conjunto de 68 registros                             |                      128 |                  8 |
| **Total**                                                      |                **3.028** |            **374** |

Se comparan los 65.536 bytes de DS, las cuatro palabras del generador y, cuando se crea un registro, su destino. Se comprueban también el equilibrio de pila y la restauración de DS del original. Los recorridos conjuntos permiten observar que una creación puede afectar a un registro visitado posteriormente en la misma iteración.

Los informes se guardan por separado en `trabajo/comparacion_entidades/muestra.json` y `resultado.json`, incluyendo huellas del ejecutable y de la biblioteca C++. La muestra se ejecuta con `pnpm comparar:entidades --muestra`. Ninguna de las dos modalidades comprueba dibujo, avance temporal general de fotogramas, rutinas particulares por escena ni una partida completa.

El [avance de secuencias de animación](secuencias_animacion.md) continúa este traslado con los tres canales y la retirada de entidades. La integración de [objetos de la mazmorra](objetos_mazmorra.md) añade la carga original, la composición, servicios de bancos reales y la rutina particular de la escena cero. Las acciones de usuario siguen pendientes.
