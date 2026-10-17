# Música AdLib y selección de audio

La música se reconstruye desde el reproductor interno de `CURSE.EXE`, conservando sus nueve registros de canal, patrones y tablas de instrumentos. Los datos musicales permanecen en el EXE local; no se versiona una copia ni se ejecuta su código en la partida.

## Rutas originales

| Tramo                 | Traslado                                                                     |
| --------------------- | ---------------------------------------------------------------------------- |
| 0487–048A             | Selección inicial del tema uno cuando la modalidad musical 0AD3 es dos       |
| 8DB0–8DFD             | Inicialización de nueve canales desde las tablas 8DAC                        |
| 8DFE–8E5A             | Contador global y despacho de canales por IRQ                                |
| 8E5F–8F4E             | Duraciones, notas, silencios, instrumento FE, transposición FD y patrones FF |
| 8FFC–900B             | Lista inicial de registros, desde 9749 hasta FFFF                            |
| 900C–902B             | Escrituras a 388h/389h, sustituidas por un servicio de registros             |
| 902E–90BA y 90C4–90CE | Dos operadores, frecuencia, encendido y apagado de nota                      |

El estado musical reside en **CS**, no en el DS del juego. Los nueve registros ocupan 17 bytes desde 8CE2. El traslado conserva las escrituras parciales de bytes, los signos de la repetición y las envolturas de palabra. Los contadores globales 8E5B/8E5D conservan su decremento original; la cadencia inicial es siete IRQ. Las listas de patrones admiten FFFE para repetir, FFFD para saltar y FFFF para terminar.

La llamada encontrada al iniciador musical elige el tema uno antes de Core. Esta ruta AdLib continúa por título, introducción y escenas; no se han encontrado llamadas por habitación al iniciador 8DB0. No asignar una pista distinta a cada habitación por intuición. La tabla contiene también un tema cero, que se conserva y compara, aunque no se selecciona en el arranque reconstruido. La ruta MT-32 es independiente y sigue pendiente.

## Síntesis y reloj

`motor/sonido/musica.cpp` reconstruye el reproductor y emite registros. `plataformas/sdl/musica_adlib.cpp` alimenta [Nuked OPL3](https://github.com/nukeykt/Nuked-OPL3), fijado a una revisión en `herramientas/dependencias_audio.json`. El sintetizador funciona en su modo compatible para las órdenes OPL empleadas. No se afirma equivalencia acústica exacta entre este núcleo OPL3 y cada tarjeta AdLib OPL2 física.

La fuente externa se conserva sin cambios en `terceros/nuked_opl3`, con su licencia LGPL 2.1 o posterior. El resto del proyecto mantiene su situación de licencia. La compilación copia el aviso junto al ejecutable. La procedencia, huellas y reconstrucción están documentadas en esa carpeta.

Se genera PCM mono a 48.000 Hz, con el reloj nominal del PIT de 14.318.180/12 Hz y divisor 4250h. Las órdenes se aplican al terminar la muestra que cruza la IRQ, con una precisión de una muestra. La calibración DOS de esperas de puerto no se emula: el chip recibe las órdenes por su cola temporal. El volumen musical deja margen para mezclar efectos PC Speaker y la suma se limita al rango de 16 bits.

La salida es común durante Core, título, introducción, menús, fundidos y escenas. Cambiar de vista o habitación conserva el tema. Tras una suspensión de más de cinco segundos se descarta audio atrasado y se continúa desde el estado musical conservado; no se afirma sincronía exacta con el tiempo de suspensión.

## Controles y consola

- **F4** alterna PC Speaker (efectos sin música) y AdLib (música con efectos PC Speaker). La consola indica la opción efectiva. Cambiarla conserva el avance musical, aunque su mezcla no sea audible.
- **M** silencia o reactiva toda la mezcla; el reloj sigue avanzando. La consola también avisa si se cambia de opción estando silenciado.
- SB16 digital y MT-32 se anuncian como pendientes; no aparecen como dispositivos funcionales. Àlex ha indicado que las ROM MT-32 estarán en `datos_originales/ROMS/`, siempre locales y excluidas de Git.
- `pnpm ventana --sin-sonido` desactiva el dispositivo desde el arranque.

La consola registra apertura y cierre del menú, selecciones y resultado del despacho, destinos de ratón, pulsación y liberación de flechas, bloqueos de movimiento, inventario, cambios de escena y opciones de depuración. Incluye identificador y manejador originales en las selecciones para facilitar su investigación. Una acción despachada puede ser rechazada por el guion original; el registro no implica que el puzle esté resuelto. No se imprimen los fotogramas de animación ni los movimientos del puntero.

## Verificación

`pnpm comprobar:musica --muestra` compara **2.452 casos**, incluyendo 1.024 IRQ de cada tema, inicialización, límites del reloj y órdenes de los nueve canales. Coinciden todo CS, la pila y **5.442 escrituras OPL** observadas en los temas. Unicorn ejecuta el reproductor original; el oráculo no es otro intérprete del formato. Las esperas de puerto se fijan a uno para verificar la traza sin depender de la velocidad del PC DOS.

La comprobación SDL genera ocho segundos del tema uno: **384.000 muestras** coinciden al producirlas de una vez o en bloques de distintos tamaños, con **562 IRQ**. Se exige señal no nula y se guarda `trabajo/comparacion_musica/musica_adlib.wav`. También se ejercitan salida común, cambio de perfil, silencio y desconexión de escena con un dispositivo `dummy`. Esto no escucha los altavoces físicos.

`pnpm comparar:musica` amplía a 8.192 IRQ por tema. La comparación amplia queda disponible para el propietario; sus resultados se registran aparte de la muestra.
