# Reproductor del altavoz del PC

El código de [altavoz.cpp](../motor/sonido/altavoz.cpp) traslada tres rutinas del reproductor seleccionado por la modalidad uno. Su escritura de `B6h` en `43h`, los dos bytes del divisor en `42h` y el control de `61h` identifican la ruta del temporizador y el altavoz del PC.

Se han compilado los bloques con avisos tratados como errores. Una muestra de **310 casos** coincide con el original en memoria y en cada operación de puerto. El propietario ha confirmado también la coincidencia de los **3.312 casos** de la comparación amplia.

## Tramos trasladados

| Original                         | Función                                   | Comportamiento                                                                                      |
| -------------------------------- | ----------------------------------------- | --------------------------------------------------------------------------------------------------- |
| `7A46`–`7A60`                    | `iniciar_altavoz`                         | Guarda referencia en `DS:9920`, contador uno en `9922` y actividad uno en `9924`.                   |
| `7A61`–`7A76`                    | `detener_altavoz`                         | Limpia la referencia, lee `61h` y escribe el valor con máscara `FCh`.                               |
| `7AC6`–`7B0C`                    | `avanzar_altavoz`                         | Reduce el contador; al agotarlo lee duración, divisor y programa la salida, o termina la secuencia. |
| Selector `5288` y llamada `7A46` | `atender_orden_altavoz` junto al selector | Atiende una orden nula o una secuencia del altavoz; otras modalidades continúan pendientes.         |

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

`PuertosAltavoz` recibe funciones explícitas de lectura y escritura. El C++ no ejecuta instrucciones privilegiadas ni toca puertos del ordenador anfitrión. El generador PCM traduce estas operaciones a un estado de temporizador y la plataforma lo envía a SDL.

## Integración y límites

El selector y el reproductor pueden atender la modalidad uno dentro del servicio `sonido` de las secuencias. Los casos integrados ejecutan las instrucciones originales de animación, selector y arranque, sin sustituir esos bloques por auxiliares falsas. Las modalidades cero y dos siguen retornando sin emitir órdenes.

El avance por temporizador conserva su llamada explícita y ahora se integra con el reloj PCM y la salida SDL descritos más abajo. La reproducción de muestras y los demás dispositivos siguen pendientes.

El llamador debe aportar un segmento de 65.536 bytes y servicios de puerto válidos y serializar las llamadas al estado compartido. El original protege el arranque con `CLI/STI`; el C++ no cambia las interrupciones del sistema anfitrión ni ofrece sincronización entre hilos. Las lecturas de palabras de prueba se mantienen alineadas, incluyendo una nota que comienza en `FFFEh` y cuyo divisor se lee desde cero tras avanzar la referencia.

## Comparación agrupada

```sh
pnpm comparar:altavoz
```

| Grupo                                           | Comparación amplia confirmada | Muestra comprobada |
| ----------------------------------------------- | ----------------------------: | -----------------: |
| Arranque                                        |                            80 |                 10 |
| Parada y 256 valores del puerto de control      |                           512 |                 16 |
| Temporizador, duración, divisor y envolvimiento |                         1.152 |                144 |
| Arranque de secuencias persistentes             |                            64 |                  4 |
| Pasos consecutivos con paradas intercaladas     |                         1.024 |                 64 |
| Selector conectado al reproductor               |                           240 |                 36 |
| Animación conectada al selector y reproductor   |                           240 |                 36 |
| **Total**                                       |                     **3.312** |            **310** |

Unicorn ejecuta las instrucciones originales. Sus entradas y salidas de puerto se registran y reciben el mismo valor inicial de control que las funciones nativas. Se compara la traza completa en orden, el control final, DS completo, la prioridad guardada en CS, la pila y SI donde se declara conservado. No se compara una grabación de audio ni la temporización del hardware.

Los informes separados se guardan en `apps/juego/trabajo/comparacion_altavoz/muestra.json` y `resultado.json`, con huellas de ejecutable y biblioteca y direcciones visitadas. La muestra se ejecuta añadiendo `--muestra` al comando. La comparación amplia fue ejecutada y confirmada por el propietario.

## Salida audible común

`apps/juego/motor/sonido/pcm.cpp` convierte las escrituras de las rutinas originales a PCM mono de 16 bits a 48.000 Hz. El modelo digital usa un reloj nominal de 14.318.180/12 Hz, el contador de modo tres, byte bajo/alto, compuerta 61h y divisor cero equivalente a 65.536. Integra la onda lógica por muestra y conserva su fase entre bloques. No pretende reproducir la respuesta acústica de una caja o un altavoz físico; los contadores extremos se contrastan contra este modelo digital, no contra una medición de hardware.

La inicialización original 01A8–01B9 programa 4250h en el canal cero y la IRQ 0240–0247 llama a 7AC6 cuando 0AD4 es uno. El generador reproduce esa división sin redondear a 70 llamadas por segundo. La lógica y los fundidos de plataforma todavía utilizan su referencia nominal de 70 Hz; no se afirma sincronización global exacta de toda la partida.

La plataforma `apps/juego/plataformas/sdl/audio.cpp` mantiene una salida compartida y procesa las secuencias en el hilo principal. SDL recibe únicamente muestras; su hilo de dispositivo nunca escribe DS. Cambiar de habitación conserva dispositivo y prioridad; abrir otra vista de desarrollo limpia los efectos de la sesión anterior. El menú pausa la lógica, pero no la IRQ de sonido. La tecla M silencia las muestras sin cambiar el modo del juego. Tras una suspensión superior a cinco segundos se descarta el efecto interrumpido; los cuadros lentos descartan audio atrasado para limitar la latencia.

Los servicios FB, las respuestas de menú de 3545/37FF y el burbujeo BF3C–BF41 del lago utilizan el mismo selector 5288. No se añaden pitidos de prueba a las acciones del juego. La música tiene su propia modalidad 0AD3: [AdLib](musica_adlib.md) conserva la presentación y [MT-32](musica_mt32.md) reproduce los bancos por zona. Los [efectos digitales Sound Blaster](efectos_digitales.md) están activos por defecto; F3 permite volver a PC Speaker conservando la música.

## Comprobación de PCM y SDL

`pnpm comprobar:audio --muestra` ejecuta los 310 casos del reproductor original y contrasta 16 selecciones de efecto en las cuatro escenas integradas, 24 límites de contador/compuerta y 8 respuestas de menú. Los casos del menú comprueban las modalidades cero/uno y la visibilidad. Se comparan DS completo y PCM con las escrituras DOS ejecutadas por Unicorn en los instantes de IRQ; la referencia integra tramos analíticamente, de forma distinta al generador C++. También se exige que dividir las muestras en bloques produzca exactamente la misma señal y estado.

Después, SDL carga el WAV generado, acepta las muestras y arranca un dispositivo de prueba `dummy`. Esto comprueba formato y alimentación de la salida, no que un altavoz real se oiga. El resultado y el WAV quedan en `apps/juego/trabajo/comparacion_audio/`. La comparación amplia se solicita con `pnpm comparar:audio`; se registra por separado de la muestra.
