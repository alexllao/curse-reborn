# Efectos digitales Sound Blaster

El port sigue las órdenes originales de `CURSE.EXE`. No añade efectos nuevos, reverberación, normalización ni arreglos de sonido. La mejora sonora queda para la futura remasterización.

## Rutinas originales

| Dirección        | Contrato reconstruido                                                                                                                 |
| ---------------- | ------------------------------------------------------------------------------------------------------------------------------------- |
| 0386–03FE        | En modalidad tres, carga `STD.SFX`; el nombre está en DS:0E11.                                                                        |
| 4EC8 / 4FD1–504F | Registro de escena de ocho bytes en DS:784E. El tercer campo es el nombre SFX o cero.                                                 |
| 5288–53B2        | Recorte de visibilidad, banco general para identificadores menores que 15, grupo de escena DS:9048, prioridad y selección de muestra. |
| 7FB3–806D        | Detiene DMA anterior, fija constante temporal DSP, programa dirección/página/cuenta e inicia reproducción PCM.                        |
| 82C6–8350        | Continúa el bloque al cruzar una frontera de 64 KiB; la última interrupción libera CS:8351.                                           |

`apps/juego/motor/sonido/seleccionar.cpp` conserva la selección ya contrastada. Cada entrada digital tiene cinco bytes: prioridad, constante temporal, palabra que se resta de la longitud y número de muestra. El número selecciona una palabra de desplazamiento en el banco; allí hay una palabra de longitud seguida de PCM mono de ocho bits **sin signo**. El ajuste de longitud no adelanta el inicio del audio.

La orden DSP `40h` fija la frecuencia mediante `1000000/(256-constante)`; `14h` inicia DMA de ocho bits. Es el modo Sound Blaster que usa el juego, compatible con SB16; no requiere emular todas las funciones de una SB16. La [guía de programación de Creative](https://flint.cs.yale.edu/cs422/readings/hardware/SoundBlaster.pdf) documenta estos comandos y la constante temporal.

## Traslado e integración

`apps/juego/motor/sonido/digital.cpp` sustituye el DSP y DMA por un lector PCM portable de 48 kHz. Conserva el reloj racional, la duración y la fase entre bloques. Copia la muestra en curso para mantener su propiedad; no cambia la prioridad, excepto al terminar o detenerla. Una orden de prioridad igual o inferior se omite mientras hay una muestra activa; una superior la interrumpe. No se inventa una cola ni reproducción simultánea de varios efectos.

`preparar_dma` conserva dirección, página, cuenta menos uno y bytes restantes del primer bloque original, incluso los casos límite comprobados. SDL reproduce los bytes contiguos sin necesitar páginas DMA ni interrupciones ISA. La conversión de muestra es `(byte-128)*256`, retenida hasta el siguiente instante del reloj DSP. No reproduce la respuesta analógica ni los filtros de una tarjeta física.

La plataforma carga el nombre indicado por cada registro. Mazmorra, corredor y caída comparten `DUNGEON.SFX`; el lago usa `LAKEBED.SFX`. Las entidades, las secuencias FB, las respuestas del menú y el burbujeo del lago utilizan el mismo servicio; únicamente sonarán las órdenes de acciones ya integradas en el motor.

Los efectos digitales se seleccionan al arrancar. **F3** alterna con PC Speaker, **F4** cambia solo la música y **M**, **+** y **−** afectan a ambos. Cambiar de efectos detiene la muestra anterior; no reinicia la escena ni el reloj musical. Los cambios se registran en consola. Véase la [referencia común de controles](controles.md).

## Comprobaciones

`pnpm comprobar:digital` ejecuta la comparación amplia del motor y la integración SDL. `pnpm comparar:digital` solo ejecuta la primera. Ambas admiten `--muestra` para reducir las combinaciones de bancos.

- 140 programaciones DSP/DMA: Unicorn ejecuta 7FB3 y se comparan dirección, página, cuenta, bytes restantes, escrituras DMA y comandos DSP.
- 78 referencias de escena contrastadas y todos sus archivos SFX presentes comprobados.
- 134 efectos de 18 combinaciones distintas de grupo y banco: 5288 original determina posición, longitud y constante temporal; la referencia de conversión comprueba **3.442.325 muestras** contra C++, además del resultado fragmentado y DS sin cambios.
- Prioridad FF que impide interrumpir una voz con un efecto ambiental, finalización, parada y silencio posterior.
- SDL con dispositivo `dummy`: cuatro escenas, F3, F4, volumen, silencio y continuidad musical durante la desconexión. Comprueba alimentación y estado, no una escucha física.

La muestra corta compara 31 efectos, 465.270 muestras y las mismas 140 programaciones DMA. Los resultados y el WAV con tres respuestas generales quedan en `trabajo/comparacion_digital/`, excluido de Git. El contraste verifica estas rutas digitales y sus datos; no acredita equivalencia acústica con hardware físico ni completa las acciones pendientes del juego.
