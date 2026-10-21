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

La ruta guiada utiliza los mismos movimientos, flechas y menús del recorrido SDL hasta THE END. Empieza en la mazmorra con inventario vacío. La lógica conserva sus70 ciclos por segundo; los fundidos de32 retrazos y los desplazamientos de cámara de ocho se muestran completos. El menú queda visible medio segundo antes de cada selección, mientras la música continúa. La mano se desplaza hacia las casillas que realmente se dibujan. Se conserva el desenlace durante cinco segundos adicionales.

El recorrido no asigna inventario, puntuación ni banderas y no sustituye escenas futuras por vistas F5/F6. Se desactiva el modo de recargas de comprobación para mantener una sola partida viva. Esta es una reproducción automática de entradas normales con un ritmo legible; la ruta es la de comprobación y no se acredita como la solución humana más corta.

## Sonido sin molestar al usuario

SDL entrega la mezcla final de los flujos del juego al callback postmix. Se guarda esa mezcla en WAV antes de poner a cero el búfer que este dispositivo lógico envía a la salida. Así el vídeo conserva música y efectos mientras la aplicación grabada permanece inaudible. No se cambia el volumen del sistema, de otras aplicaciones ni las preferencias de M del usuario.

La captura se silencia desde que se abre el dispositivo, antes de la presentación. Al terminar el WAV se mantiene ese silencio hasta cerrar la sesión. El controlador de audio normal sigue fijando la cadencia; el controlador dummy no se utiliza para la toma final porque en la primera investigación produjo menos muestras que el tiempo real transcurrido.

## Archivos y finalización

`partida_completa.mp4` es el archivo de entrega. Una muestra se llama `muestra.mp4` y su informe mantiene `completo: false`. La carpeta conserva el vídeo intermedio, el WAV, los registros, `partida.escenas.tsv` con los cambios de escena y `resultado.json` con duración, formato y SHA-256.

La grabación usa una tubería anónima heredada entre el juego y FFmpeg para evitar fotogramas sin comprimir en disco. Los mensajes de consola se desvían a `partida.consola.log`. No se permite que una interrupción o error de ruta se presente como partida terminada: se exige llegar realmente a73 y producir la captura final. Los resultados permanecen excluidos de Git.

## Comprobaciones del 21 de octubre

Muestra de35 segundos verificada:1050 fotogramas,1920×1080,30 fps, vídeo y audio de35,000 segundos, estéreo48000Hz con señal efectiva. Se visitan0/1 mediante la liberación y la salida normales. Tras proteger también el alta y cierre del archivo de audio, otra muestra de10,033 segundos conserva301 fotogramas y mezcla no silenciada en el archivo. La salida física del juego recibe ceros desde la apertura hasta el cierre del dispositivo.

La regresión sin modo de grabación recorre de nuevo la aventura y produce THE END: sus64.000 píxeles coinciden con `comparacion_desenlace/desenlace_final_cpp.png`. Los perfiles de esas pruebas son locales y aislados. La grabación completa tiene su propio resultado; las muestras no se presentan como una partida terminada.
