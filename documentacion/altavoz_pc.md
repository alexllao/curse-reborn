# Reproductor del altavoz del PC

El código de [altavoz.cpp](../motor/sonido/altavoz.cpp) traslada tres rutinas del reproductor seleccionado por la modalidad uno. Su escritura de `B6h` en `43h`, los dos bytes del divisor en `42h` y el control de `61h` identifican la ruta del temporizador y el altavoz del PC.

Se han compilado los bloques con avisos tratados como errores. Una muestra de **310 casos** coincide con el original en memoria y en cada operación de puerto. La comparación amplia de **3.312 casos** queda preparada para el propietario.

## Tramos trasladados

| Original | Función | Comportamiento |
| --- | --- | --- |
| `7A46`–`7A60` | `iniciar_altavoz` | Guarda referencia en `DS:9920`, contador uno en `9922` y actividad uno en `9924`. |
| `7A61`–`7A76` | `detener_altavoz` | Limpia la referencia, lee `61h` y escribe el valor con máscara `FCh`. |
| `7AC6`–`7B0C` | `avanzar_altavoz` | Reduce el contador; al agotarlo lee duración, divisor y programa la salida, o termina la secuencia. |
| Selector `5288` y llamada `7A46` | `atender_orden_altavoz` junto al selector | Atiende una orden nula o una secuencia del altavoz; otras modalidades continúan pendientes. |

La parada explícita **conserva el contador y el indicador de actividad**. El final de secuencia sí limpia la actividad. El traslado mantiene esa diferencia, aunque pueda parecer una reinicialización incompleta.

Si la referencia es nula, el temporizador no toca el contador. En otro caso lo reduce como palabra de dieciséis bits: desde cero pasa a `FFFFh`. Solo procesa una nota al llegar a cero.

## Datos y operaciones de puerto

Cada nota contiene una palabra de duración y otra de divisor. La duración cero termina la secuencia, limpia la referencia y actividad y apaga los dos bits bajos de `61h`. Una duración distinta de cero produce, en este orden:

1. Guardar la duración como nuevo contador.
2. Escribir `B6h` en `43h`.
3. Escribir el byte bajo y después el alto del divisor en `42h`.
4. Guardar la referencia de la nota siguiente, cuatro bytes después.
5. Leer `61h` y escribir el valor con los dos bits bajos activados.

Los demás bits leídos de `61h` se conservan. No se ha añadido una interpretación especial para un divisor cero: se conservan los bytes enviados por el original. La comparación incluye ese valor y `FFFFh`.

`PuertosAltavoz` recibe funciones explícitas de lectura y escritura. El C++ no ejecuta instrucciones privilegiadas ni toca puertos del ordenador anfitrión. La futura plataforma podrá traducir estas operaciones a un estado de temporizador y generar sonido para SDL.

## Integración y límites

El selector y el reproductor pueden atender la modalidad uno dentro del servicio `sonido` de las secuencias. Los casos integrados ejecutan las instrucciones originales de animación, selector y arranque, sin sustituir esos bloques por auxiliares falsas. Las modalidades cero y dos siguen retornando sin emitir órdenes.

El avance por temporizador se expone como una llamada explícita. Aún falta determinar e integrar la cadencia de llamadas del juego y convertir el divisor en audio digital. **Todavía no se genera PCM, no hay salida audible y no existe una ventana SDL.** Tampoco se ha trasladado la reproducción de muestras ni el otro dispositivo de órdenes de bytes.

El llamador debe aportar un segmento de 65.536 bytes y servicios de puerto válidos y serializar las llamadas al estado compartido. El original protege el arranque con `CLI/STI`; el C++ no cambia las interrupciones del sistema anfitrión ni ofrece sincronización entre hilos. Las lecturas de palabras de prueba se mantienen alineadas, incluyendo una nota que comienza en `FFFEh` y cuyo divisor se lee desde cero tras avanzar la referencia.

## Comparación agrupada

```sh
pnpm comparar:altavoz
```

| Grupo | Comparación amplia preparada | Muestra comprobada |
| --- | ---: | ---: |
| Arranque | 80 | 10 |
| Parada y 256 valores del puerto de control | 512 | 16 |
| Temporizador, duración, divisor y envolvimiento | 1.152 | 144 |
| Arranque de secuencias persistentes | 64 | 4 |
| Pasos consecutivos con paradas intercaladas | 1.024 | 64 |
| Selector conectado al reproductor | 240 | 36 |
| Animación conectada al selector y reproductor | 240 | 36 |
| **Total** | **3.312** | **310** |

Unicorn ejecuta las instrucciones originales. Sus entradas y salidas de puerto se registran y reciben el mismo valor inicial de control que las funciones nativas. Se compara la traza completa en orden, el control final, DS completo, la prioridad guardada en CS, la pila y SI donde se declara conservado. No se compara una grabación de audio ni la temporización del hardware.

Los informes separados se guardan en `trabajo/comparacion_altavoz/muestra.json` y `resultado.json`, con huellas de ejecutable y biblioteca y direcciones visitadas. La muestra se ejecuta añadiendo `--muestra` al comando. La comparación amplia aún no se ha ejecutado aquí.
