# Música grabada e inicio configurable

`apps/juego/configuracion/inicial.json` define el diseño inicial, volumen, silencio, marco, filtro y música. Se copia a `configuracion.json` junto al ejecutable. Los perfiles personales de volumen, silencio, marco, filtro y música prevalecen sobre los valores iniciales; los perfiles anteriores siguen siendo válidos. Los identificadores de habitaciones proceden del catálogo existente. El builder incluye solo la habitación seleccionada, salvo petición explícita de todas.

Las canciones aportadas por Àlex se versionan en `packages/assets/remaster/audios/`. `titulos.json` declara el archivo y título de cada RS y su SHA-256. Se eliminó la ubicación singular `remaster/audio`; no se excluyen estas canciones propias. La configuración apunta al catálogo mediante una ruta relativa al ejecutable, independiente del directorio desde el que se lance.

El inicio sin preferencia musical previa usa Remaster. F4 recorre sin música, AdLib OPL2, Sound Blaster 16 OPL3, MT-32 si hay ROM y Remaster si sus cinco archivos son válidos. El HUD muestra el título únicamente en Remaster; las músicas originales muestran solo su modo, sin atribuirles los títulos de los nuevos arreglos. F8 conserva depuración. Las tildes de los títulos se completan sobre la fuente ASCII del HUD. F3, M y volumen siguen actuando por separado.

Los bancos se eligen mediante `motor::seleccionar_musica_rs` y las tablas del EXE. La presentación usa RS5; las salas del mismo banco conservan la posición. Cambiar de banco reinicia la canción correspondiente. No se altera DS, lógica ni temporización para adaptar las composiciones. La precarga y conversión PCM quedan fuera del callback SDL; el reproductor no requiere ROM ni utiliza MIDI para producir la música remasterizada.

En desarrollo se reproducen WAV. El empaquetado convierte copias con FFmpeg/libmp3lame, VBR `-q:a 0`, estéreo a 48 kHz, y reescribe exclusivamente el catálogo entregado a MP3. `FFMPEG` permite indicar el ejecutable. dr_mp3 decodifica MP3 y SDL convierte el PCM al formato de salida; se conserva el ajuste común de volumen y mute. Las licencias de dr_mp3 y json11 acompañan la distribución.

Los nuevos arreglos repiten la canción completa con bordes de 20 ms para evitar clics. No se les atribuyen los puntos de bucle de las versiones DOS. La continuidad musical del empalme requiere escucha de las composiciones; la prueba técnica no acredita un arreglo musical sin pausa.

## Integridad

El preparador rechaza WAV cuyo SHA-256 no coincide con el catálogo. Tras sustituir deliberadamente una canción, revisar el contenido y actualizar su huella con `Get-FileHash -Algorithm SHA256`.

`packages/assets/huellas.json` registra las imágenes maestras del lanzador, marcos y decorados. Después de revisar un cambio gráfico, actualizar mediante `node apps/juego/herramientas/assets/huellas.mjs`. El builder comprueba cada maestro que incorpora y añade al manifiesto ambas huellas: maestro y copia optimizada. Para canciones registra además título, banco, tamaños y SHA-256 del WAV y del MP3. La verificación general del paquete comprueba los archivos resultantes.

## Comprobación sin sonido físico

Ejecutar con `SDL_AUDIODRIVER=dummy`:

```powershell
$env:SDL_AUDIODRIVER = 'dummy'
apps/juego/trabajo/aplicacion/curse-reborn.exe apps/juego/datos_originales/CURSE/DATA --comprobar-remaster apps/juego/trabajo/remaster_resultado.json
```

El informe acredita cinco pistas con señal, título, duración, reproducción completa más un empalme, igualdad entre generación continua y bloques de 137 fotogramas, conservación de posición en el mismo banco, F4, volumen y mute. Ejecutar también contra una copia que tenga catálogo MP3. Los resultados y audios derivados de prueba permanecen en `trabajo`.
