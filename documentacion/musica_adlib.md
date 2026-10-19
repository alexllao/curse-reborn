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

La llamada encontrada al iniciador musical elige el tema uno antes de Core. El arranque reconstruido conserva ese tema en Core, título, introducción y escenas jugables. La entrada 4DD5–4DDC omite el cambio de banco salvo en modalidad cuatro (MT-32). Con AdLib, modalidad dos, el tema sigue sonando al entrar en la mazmorra y al cambiar de habitación; no se reinicia ni se silencia. Las 78 referencias de escena atraviesan esa ruta original sin modificar CS musical, DS ni puertos, comprobado con Unicorn. Solo se ha localizado una llamada directa a 8DB0 en la imagen de código: la del arranque con tema uno. No asignar una pista distinta a cada habitación por intuición. La tabla contiene también un tema cero, que se conserva y compara, aunque no se selecciona en el arranque reconstruido. La ruta [MT-32](musica_mt32.md) es independiente y ya utiliza los bancos RS originales por zona.

## Síntesis y reloj

`apps/juego/motor/sonido/musica.cpp` reconstruye el reproductor y emite los mismos registros para las dos opciones FM:

- **AdLib OPL2**: YM3812 de [ymfm](https://github.com/aaronsgiles/ymfm), de Aaron Giles, con su DAC de mantisa/exponente. Revisión y huellas en `herramientas/dependencias_opl2.json`; licencia BSD de tres cláusulas conservada y copiada como `LICENCIA_YMFM.txt`.
- **Sound Blaster 16 OPL3**: [Nuked OPL3](https://github.com/nukeykt/Nuked-OPL3), fijado en `herramientas/dependencias_audio.json`, reproduce las escrituras originales compatibles con OPL2 en un YMF262. El juego no habilita voces adicionales ni una mezcla estéreo OPL3.

No son dos etiquetas para el mismo PCM. El adaptador `apps/juego/plataformas/sdl/opl2.cpp` conserva el reloj 3.579.545/72 Hz y convierte a 48 kHz; las escrituras del YM3812 se espacian dos muestras nativas para que un apagado y un encendido consecutivos no se fusionen; ambos chips reciben la misma partitura y continúan avanzando al cambiar F4. Los temporizadores de detección del chip y las esperas del bus ISA no se ejecutan: el reloj musical procede del PIT reconstruido. Ninguna de estas comprobaciones acredita equivalencia acústica exacta con todas las revisiones de tarjetas físicas.

La fuente externa se conserva sin cambios en `apps/juego/terceros/nuked_opl3`, con su licencia LGPL 2.1 o posterior. El resto del proyecto mantiene su situación de licencia. La compilación copia el aviso junto al ejecutable. La procedencia, huellas y reconstrucción están documentadas en esa carpeta.

Se genera PCM mono a 48.000 Hz, con el reloj nominal del PIT de 14.318.180/12 Hz y divisor 4250h. Las órdenes se aplican al terminar la muestra que cruza la IRQ, con una precisión de una muestra. La calibración DOS de esperas de puerto no se emula: el chip recibe las órdenes por su cola temporal. El volumen musical deja margen para mezclar efectos digitales o PC Speaker y la suma se limita al rango de 16 bits.

El flujo musical se alimenta desde el callback de SDL, separado del hilo que carga imágenes. Dos flujos comparten un dispositivo: música autónoma y efectos de la partida. El cambio de vista conserva el búfer musical; los cambios de banco y perfil se serializan mediante el bloqueo del flujo. Una prueba bloquea el hilo principal 150 ms y comprueba que se siguen produciendo muestras; no mide cortes acústicos en un dispositivo físico.

## Controles y consola

- **F4** recorre sin música, AdLib OPL2, Sound Blaster 16 OPL3 (ambos con el tema continuo) y MT-32 (música por zona). **F3** selecciona efectos Sound Blaster digitales o PC Speaker de forma independiente. Omite MT-32 cuando no se puede preparar. Los relojes musicales siguen avanzando al cambiar de perfil.
- **M** silencia o reactiva toda la mezcla sin detener los secuenciadores.
- Los [efectos digitales Sound Blaster](efectos_digitales.md) ya están conectados al servicio común de las escenas. AdLib conserva el tema continuo original; MT-32 ofrece los cinco bancos por zona.
- `pnpm ventana --sin-sonido` desactiva el dispositivo desde el arranque.
- En Windows se configura la consola como UTF-8 antes de imprimir los mensajes propios con tildes.

La consola registra apertura y cierre del menú, selecciones y resultado del despacho, destinos de ratón, pulsación y liberación de flechas, bloqueos de movimiento, inventario, cambios de escena y opciones de depuración. Incluye identificador y manejador originales en las selecciones para facilitar su investigación. Una acción despachada puede ser rechazada por el guion original; el registro no implica que el puzle esté resuelto. No se imprimen los fotogramas de animación ni los movimientos del puntero.

## Verificación

`pnpm comprobar:musica --muestra` compara **2.530 casos**, incluyendo 1.024 IRQ de cada tema, inicialización, límites del reloj, órdenes de los nueve canales y las 78 transiciones originales en modalidad AdLib. Coinciden todo CS, la pila y **5.442 escrituras OPL** observadas en los temas. Unicorn ejecuta el reproductor original; el oráculo no es otro intérprete del formato. Las esperas de puerto se fijan a uno para verificar la traza sin depender de la velocidad del PC DOS.

La comprobación SDL genera ocho segundos del tema uno: **384.000 muestras** coinciden al producirlas de una vez o en bloques de distintos tamaños, con **562 IRQ**. Se exige señal no nula y se guarda `trabajo/comparacion_musica/musica_adlib.wav`. También se ejercitan salida común, cambio de perfil, silencio y desconexión de escena con un dispositivo `dummy`. Esto no escucha los altavoces físicos.

`pnpm comparar:musica` amplía a 8.192 IRQ por tema. La comparación amplia queda disponible para el propietario; sus resultados se registran aparte de la muestra.

**Volumen:** `+` y `−` ajustan el nivel general en pasos del 10 %, entre 0 y 200 %. También funcionan `=` y las teclas del teclado numérico. La consola muestra el porcentaje al arrancar y al cambiarlo. `M` silencia y recupera el ajuste elegido; cambiar de perfil no lo reinicia.

La comprobación nativa de música genera ocho segundos de **cada chip**, exige señal audible, salida distinta entre OPL2/OPL3 y coincidencia exacta de cada salida al fragmentar los bloques. Guarda el OPL3 en `trabajo/comparacion_musica/musica_adlib.wav` y el OPL2 en `musica_adlib.wav.opl2.wav`. También recorre F4 por las dos opciones FM antes de volver a MT-32.

El volumen común admite de cero a **200 %**: cien conserva la ganancia normal y los valores superiores la amplifican, con posible saturación. M conserva el porcentaje elegido. La integración digital comprueba los límites cero/doscientos y su conservación al silenciar.

`pnpm exportar:musica` utiliza estos mismos núcleos para guardar referencias WAV de ambos temas internos y de los cinco bancos MT-32. La duración se calcula desde los guiones: los RS incluyen su entrada y terminan en el primer salto de repetición; FM espera a que los nueve canales hayan repetido su estado de control o terminado. Esto cubre una pasada de cada voz, sin alargar el archivo hasta el mínimo común múltiplo de bucles independientes. `--segundos 1…600` permite forzar un corte de prueba; los bucles no se convierten en finales inventados. Se guardan en `trabajo/exportacion_musica/` junto con su inventario y huellas, sin efectos ni amplificación del volumen de la partida. Sirven como material para preparar un remaster separado del port fiel.
