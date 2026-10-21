# Grabación de una partida guiada

Desde la raíz del repositorio, con FFmpeg y ffprobe instalados:

```text
pnpm grabar:recorrido --muestra 35
pnpm grabar:recorrido
```

El comando compila SDL y ejecuta `herramientas/grabacion/grabar_recorrido.py`. Para una compilación ya preparada se puede llamar directamente al Python local:

```text
apps/juego/trabajo/entorno/Scripts/python.exe apps/juego/herramientas/grabacion/grabar_recorrido.py --muestra 35
```

Admite `--programa <ejecutable>` y `--destino <carpeta-nueva>`. Por defecto crea una carpeta nueva bajo `trabajo/grabaciones/<fecha-hora>`. Rechaza un destino existente para conservar grabaciones anteriores.

## Presentación y ritmo

La salida es MP4 H.264 con audio AAC estéreo,1920×1080 y30 fps. El juego usa scanlines; el panorama Cielo encantado y su moldura conservan su imagen sin filtros. No se captura el escritorio.

La ruta guiada utiliza los mismos movimientos, flechas y menús del recorrido SDL hasta THE END. Empieza en la mazmorra con inventario vacío. La lógica espera cinco ticks del reloj de70Hz entre pasos (14 pasos por segundo), igual que el bucle interactivo de `principal.cpp` y su referencia EAD5; los fundidos de32 retrazos y los desplazamientos de cámara de ocho se muestran completos. El menú queda visible medio segundo antes de cada selección, mientras la música continúa. La mano se desplaza hacia las casillas que realmente se dibujan. Se conserva el desenlace durante cinco segundos adicionales.

El recorrido no asigna inventario, puntuación ni banderas y no sustituye escenas futuras por vistas F5/F6. Se desactiva el modo de recargas de comprobación para mantener una sola partida viva. Esta es una reproducción automática de entradas normales con un ritmo legible; la ruta es la de comprobación y no se acredita como la solución humana más corta.

## Sonido sin molestar al usuario

SDL entrega la mezcla final de los flujos del juego al callback postmix. Se guarda esa mezcla en WAV antes de poner a cero el búfer que este dispositivo lógico envía a la salida. Así el vídeo conserva música y efectos mientras la aplicación grabada permanece inaudible. No se cambia el volumen del sistema, de otras aplicaciones ni las preferencias de M del usuario.

La mezcla intermedia usa RF64 de64bits (extensión `.wav`), para que una toma larga no quede limitada por los4GiB del RIFF clásico. Conserva los flotantes originales del postmix. La captura se silencia desde que se abre el dispositivo, antes de la presentación. Al terminar el WAV se mantiene ese silencio hasta cerrar la sesión. El controlador de audio normal sigue fijando la cadencia; el controlador dummy no se utiliza para la toma final porque en la primera investigación produjo menos muestras que el tiempo real transcurrido.

## Archivos y finalización

`partida_completa.mp4` es el archivo de entrega. Una muestra se llama `muestra.mp4` y su informe mantiene `completo: false`. La carpeta conserva el vídeo intermedio, el WAV, los registros, `partida.escenas.tsv` con los cambios de escena y `resultado.json` con duración, formato y SHA-256.

La grabación usa una tubería anónima heredada entre el juego y FFmpeg para evitar fotogramas sin comprimir en disco. Los mensajes de consola se desvían a `partida.consola.log`. No se permite que una interrupción o error de ruta se presente como partida terminada: se exige llegar realmente a73 y producir la captura final. Los resultados permanecen excluidos de Git.

## Comprobaciones del 21 de octubre

Muestra de35 segundos verificada:1050 fotogramas,1920×1080,30 fps, vídeo y audio de35,000 segundos, estéreo48000Hz con señal efectiva. Se visitan0/1 mediante la liberación y la salida normales. Tras proteger también el alta y cierre del archivo de audio, otra muestra de10,033 segundos conserva301 fotogramas y mezcla no silenciada en el archivo. La salida física del juego recibe ceros desde la apertura hasta el cierre del dispositivo.

La regresión sin modo de grabación recorre de nuevo la aventura y produce THE END: sus64.000 píxeles coinciden con `comparacion_desenlace/desenlace_final_cpp.png`. Los perfiles de esas pruebas son locales y aislados. La grabación completa tiene su propio resultado; las muestras no se presentan como una partida terminada.

## Toma anterior: recorrido completo, velocidad incorrecta

La toma `trabajo/grabaciones/20261021-175510/partida_completa.mp4` alcanza THE END, pero su cadencia lógica es incorrecta:70 pasos por segundo frente a14 del juego interactivo. Se conserva como antecedente; NO acredita velocidad normal ni tiempo de una partida humana. Àlex detectó el movimiento acelerado al verla. El proceso nativo y el montaje final terminan con código0; `resultado.json` registra `completo: true` y `muestra: false`.

- Duración:41min8,033s;74.041 fotogramas a30 fps.
- Vídeo:H.264,1920×1080,scanlines en juego y mano, Cielo encantado y moldura sin filtros.
- Audio:AAC,48000Hz,estéreo;2468,033s frente a2468,033333s de vídeo. La mezcla WAV previa dura2468,030s. Una muestra intermedia de30s contiene señal (media−27,3dB,máximo−3,1dB), sin reproducirla por los altavoces.
- Tamaño:561.419.983 bytes.
- SHA-256:`5b56db7a9afbb75fa67ab73143014a5ef49df6bba1a585e1833daa550fa5008f`.

Se decodificaron completos el vídeo y el audio con FFmpeg: código0 y registro sin errores. Se revisaron fotogramas intermedios y el final, con el marco completo y THE END visible. La cronología registra72 índices distintos recorridos; es el inventario de esta toma, no otro porcentaje de cobertura de salas. Los archivos comerciales, WAV, MP4 y capturas siguen fuera de Git.

La compilación nativa y las muestras se comprobaron directamente. Formato213C++,Ruff102Python,ESLint,importaciones y monorepo pasan; el agregado `pnpm calidad` conserva el fallo de este entorno al iniciar `clang-format` desde Node (`EPERM`). No se acredita como aprobado ese comando agregado.

## Corrección de cadencia

El grabador confundía los70Hz del temporizador con los pasos de lógica. El bucle interactivo espera `5000000000 / 70` nanosegundos entre pasos (EAD5); el grabador solo esperaba un tick. Ahora espera cinco ticks por paso, conservando los fundidos/cámaras a70Hz, la captura a30fps y la pausa de selección en0,5s. La permanencia final se ajusta de350 a70 pasos para conservar cinco segundos. No se cambia la velocidad del juego interactivo ni se estira el MP4 o su música.

El informe nativo declara cinco ticks por paso y14Hz lógicos; el script rechaza ejecutables antiguos que no declaren esa cadencia. Una nueva toma completa a velocidad normal sigue pendiente: la anterior no la sustituye.

Muestra corregida comprobada en `trabajo/grabaciones/20261021-185220`:150,000s,4500 fotogramas, informe5 ticks/14Hz y dos escenas. La salida inicial llega a1 a los93,431s, frente a26,713s de la versión acelerada; los menús y transiciones mantienen sus duraciones propias. Decodificación completa con FFmpeg correcta, código0. `caminata_30s.mp4` extrae los segundos105–135 sin alterar velocidad ni audio, para revisar el movimiento directamente. La muestra sigue declarada incompleta; no se ha lanzado otra toma larga.

Àlex aprobó la muestra de caminata corregida. Se prepara una nueva toma completa con esa cadencia y salida silenciada. Antes de ella se verifica el contenedor RF64 con una muestra corta; no cambia las muestras de audio, la velocidad ni la presentación aprobada.

RF64 comprobado en `trabajo/grabaciones/20261021-190326`: muestra10s/300 fotogramas, IEEEfloat32/48000Hz/estéreo, longitudes64bits del chunkds64 coincidentes con el archivo, audio efectivo y MP4 de10s. Compilación nativa conO2/avisos como errores correcta. Nueva toma larga iniciada en `trabajo/grabaciones/20261021-190417`; su finalización se documentará únicamente tras verificar el resultado.
