# Avance de secuencias de animación

El C++ de [secuencias.cpp](../motor/entidades/secuencias.cpp) traslada el recorrido de `0000:83DE`–`0000:8821` dentro del dominio descrito aquí. Avanza tres canales por registro, consulta dimensiones y retira entidades fuera de los límites. La muestra de **462 casos** coincide con las instrucciones originales. La comparación amplia de **8.924 casos** queda preparada para ejecutarla en el equipo del propietario.

## Recorrido original

| Original | Operación trasladada |
| --- | --- |
| `83DE`–`83E9` | Selecciona los datos del juego y prepara 70 registros desde `2C0A`. |
| `83EA`–`85FF` | Interpreta el canal de imagen de cada registro no nulo. |
| `8600`–`86DB` | Interpreta el desplazamiento horizontal. |
| `86DC`–`87C6` | Interpreta el desplazamiento vertical. |
| `87C7`–`87D0` | Avanza al siguiente registro. |
| `87D1`–`881F` | Recorre las posiciones no reservadas y retira las que quedan fuera de límites. |
| `8820`–`8821` | Restaura el segmento de entrada y retorna. |

La API separa el avance de un registro, su retirada y el recorrido completo. El C++ recibe datos y estado aleatorio explícitos, y mantiene independiente la lectura de dimensiones. Las referencias de recurso se conservan como valores de datos y se resuelven mediante una función proporcionada por el llamador; no son direcciones de memoria del proceso.

## Tres canales de cinco bytes

En cada registro de 33 bytes, los canales empiezan en `10h`, `15h` y `1Ah`. Cada uno contiene una referencia de dos bytes, un índice de un byte, un contador de un byte y un intervalo de un byte. La investigación de esta rutina precisa el uso temporal de los campos descritos en [entidades.md](entidades.md).

Una referencia nula omite el canal. Un contador se reduce con aritmética de byte y solo interpreta la secuencia al llegar a cero; entonces se recarga desde el intervalo. **Cero pasa a 255**, por lo que no provoca una interpretación inmediata. En el original, `OR BX,BX` despeja el acarreo antes de `DEC` y del salto `JBE`.

El índice también envuelve a ocho bits. La lectura se realiza con la referencia del canal y el índice anterior al incremento. Se mantienen las lecturas y escrituras en su orden original.

| Byte de orden | Efecto común |
| --- | --- |
| `7F` | Vuelve al índice cero y continúa interpretando. |
| `7E` | Lee una nueva referencia de secuencia, reinicia el índice y continúa. |
| `7D` | Desactiva la entidad, limpia las tres referencias y termina el registro. |
| `7C` | Limpia la referencia del canal y pasa al siguiente. |
| `7B` | Consume un multiplicador, guarda el byte bajo del producto por el intervalo y termina el canal. |
| `7A` | Consume un intervalo nuevo, lo copia al contador y continúa interpretando. |
| `79` | Retrocede dos posiciones desde el índice ya incrementado y continúa. |
| `78` | Consume límite y base de espera aleatoria; aplica la espera a este canal y a los posteriores, y termina el registro. |

En los canales X/Y, `80h` acumula cien en la palabra temporal `DS:9972` y sigue leyendo. Un desplazamiento ordinario extiende el signo del byte, incorpora esa magnitud con el mismo signo y actualiza la coordenada con aritmética de 16 bits.

En el canal de imagen, un byte ordinario escribe la imagen y consulta las dimensiones usando sus siete bits bajos. El resto de las órdenes específicas es:

| Orden | Efecto observado |
| --- | --- |
| `FF`, `FE` | Limpian o activan la máscara `02h` del estado y terminan el canal. |
| `FD`, `FC` | Activan campos de control y siguen interpretando. |
| `FB` | Llama a sonido en el original; el traslado señala una operación pendiente. |
| `FA`, `F9` | Escriben uno o cero en `DS:0DEF` y continúan. |
| `F8` | Cambia modelo e intervalos; selecciona una referencia de recurso residente o señala una carga pendiente. |
| `F7` | Elige una imagen aleatoria distinta de la actual, repitiendo el generador cuando sea necesario. |
| `F6` | Consume un valor, lo escribe en `DS:9975` y lo utiliza también como imagen. |

Para `F8`, `FFh` selecciona el recurso del personaje y un intervalo uno; `FEh` selecciona el recurso de escena y un intervalo dos. Los demás modelos usan el recurso de modelos si el valor coincide con la caché `DS:9974`. Una diferencia exige estudiar e integrar la apertura, lectura y cierre DOS de `8574`–`859D`.

## Retirada y comparaciones conservadas

La pasada final omite las posiciones reservadas indicadas por `DS:3588`. Retira y limpia las tres referencias si X alcanza 384, X más ancho queda por debajo de −64, Y alcanza 250 o Y más alto queda por debajo de −50, con las comparaciones y el orden originales.

Dentro del avance vertical existe además una condición peculiar: solo comprueba la suma con el alto si Y ya es mayor o igual a 300, y desactiva únicamente si esa suma, envuelta a 16 bits, es menor o igual a −100. Se conserva literalmente y se compara con alturas que hacen visible el desbordamiento. No se ha sustituido por una supuesta intención del programador.

## Operaciones pendientes y dominio

`EstadoSecuencias` distingue ejecución correcta, entrada inválida, sonido pendiente y recurso pendiente. Las dos operaciones pendientes detienen la función; **no se consideran una animación completada ni se omiten silenciosamente**. Se pueden haber realizado escrituras anteriores. Esta API todavía no ofrece reanudación de esas operaciones: hace falta integrar sus rutinas y definir ese contrato antes de usarla como motor de una partida.

El lector de dimensiones debe aportar valores para las referencias solicitadas. Se requiere un segmento de 65.536 bytes, referencias y operandos válidos y menos de 70 posiciones reservadas. Para evitar bloqueos con secuencias cíclicas o una imagen aleatoria imposible, la API admite hasta 4.096 órdenes por canal y 4.096 intentos de elección. El original no tiene esos límites; las comparaciones preparadas usan secuencias que terminan y divisores distintos de cero.

## Comparación agrupada

```sh
pnpm comparar:secuencias
```

| Grupo | Comparación amplia preparada | Muestra comprobada |
| --- | ---: | ---: |
| Órdenes de los tres canales y contadores | 2.112 | 264 |
| Esperas compartidas | 128 | 12 |
| Canales combinados en el mismo registro | 96 | 12 |
| Límites de índice, estado y coordenadas | 28 | 28 |
| Detención ante sonido y carga pendientes | 64 | 8 |
| Secuencias persistentes durante varias iteraciones | 6.144 | 96 |
| Retirada fuera de límites | 288 | 36 |
| Recorrido completo de 70 registros y retirada | 64 | 6 |
| **Total** | **8.924** | **462** |

Unicorn ejecuta las instrucciones originales y el generador real; las dimensiones proceden de tres tablas y cabeceras sintéticas equivalentes al lector del C++. Se compara DS completo, las cuatro palabras aleatorias y el equilibrio de la pila. Los casos pendientes se detienen en `84E9` y `8574`, antes de las llamadas externas, y contrastan también el motivo devuelto por el C++.

El informe registra las direcciones originales visitadas y las huellas del ejecutable y de la biblioteca. La instrumentación permanece activa al alternar retornos y paradas de bloque. Los informes se guardan en `trabajo/comparacion_secuencias/muestra.json` y `resultado.json`; para la muestra se añade `--muestra` al comando.

Esta comprobación no incluye sonido, lectura de archivos reales, temporización de plataforma, dibujo ni una partida completa. El siguiente trabajo es seguir las operaciones externas de las secuencias y la selección de recursos que utiliza la presentación.
