# Habitaciones interactivas

Son una presentación externa al juego. Los marcos decorativos rellenan bandas; las habitaciones representan un ordenador que se enciende y permite entrar al juego. Los decorados ampliados continúan aparcados.

El arranque normal muestra primero el banner silencioso del lanzador durante siete segundos (entrada0,8s, visible5s, salida1,2s), escogiendo su variante por proporción y cubriendo la ventana sin deformar. Después aparece Buhardilla con el ordenador apagado. A los dos segundos se enciende automáticamente una sola vez, también con `--sin-sonido`. Un clic previo en el botón anula ese plazo y conserva el control manual. Encendido → BIOS 386 con distintivo Energy Star → DOS → Core/título/introducción dentro del monitor. Tras los siete segundos, clic en botón físico del monitor → acercamiento suave de 650 ms hasta el máximo disponible. Ctrl+R vuelve sin reiniciar escena ni música. Ctrl+H recorre los cinco diseños. Alt+Intro conserva su función de pantalla completa de la ventana.

## Añadir otro diseño

1. Crear una carpeta en `packages/assets/lanzador/habitaciones/` con las imágenes PNG, manteniendo sus dimensiones originales.
2. Añadir una entrada a `habitaciones.json`: `id`, `nombre`, `carpeta` y `variantes`. `inicial` elige el identificador al abrir.
3. Cada variante declara `archivo`, `resolucion: [ancho, alto]` y cinco rectángulos `[x, y, ancho, alto]`: `pantalla`, `botonPC`, `botonMonitor`, `pilotoPC`, `pilotoMonitor`. Todas las coordenadas están en píxeles del PNG, nunca de la ventana. `pantalla` es el interior útil del cristal, sin bisel. El juego cabe dentro sin estirarse ni recortarse.
4. Volver a compilar con `pnpm sdl:compilar` o iniciar con `pnpm ventana`. Node valida tamaños, rutas relativas seguras, proporciones únicas y zonas dentro de la imagen, genera el catálogo y copia únicamente las imágenes registradas. No hay que modificar C++ para añadir una habitación. **En esta versión el catálogo se incorpora durante la compilación; editar el JSON junto a un ejecutable ya construido no lo recarga.**
5. Ejecutar `pnpm comprobar:habitaciones` para comprobar geometría, clics, arranque y capturas locales. La prueba automática actual recorre los cinco diseños entregados; revisar visualmente también cualquier diseño nuevo.

Se elige la relación de aspecto con menor diferencia logarítmica. La habitación se ajusta completa con escala uniforme; en proporciones no disponibles puede conservar bandas negras. Al acercar, el juego alcanza exactamente el máximo 320:200 que cabe en la ventana y la habitación desaparece; los marcos F1 pueden rellenar las bandas. La pantalla, botones y ratón comparten la misma transformación.

En LucasArts 4:3, el PNG aportado recorta el botón de la torre. Esa variante utiliza `botonPCAuxiliar: true`, un pequeño interruptor dibujado sobre la parte visible de la torre en el rectángulo configurado. No se falsea la presencia de un botón fuera de la imagen y no se modifica el maestro. Este parámetro es opcional en los demás diseños.

## Fidelidad y distribución

BIOS/DOS son una presentación propia con tiempos de plataforma, no firmware ni emulación de un sistema DOS. Las marcas evocadas pertenecen a sus titulares. Durante apagado y arranque, el juego y el audio permanecen en pausa y no se suma tiempo efectivo. El encendido vuelve a Core sin escribir ni borrar partidas. Volver de la vista ampliada a la habitación conserva la sesión.

Las comparaciones con el EXE omiten esta presentación y siguen entrando directamente al bloque que verifican. Los filtros permanecen limitados a la imagen del juego, incluyendo el cursor original. No se activa ningún decorado ampliado.

Los PNG originales se preservan. La compresión o conversión del paquete final deberá producir copias de distribución y mantener los metadatos coherentes con sus dimensiones; todavía no hay conversión con pérdida ni empaquetado final implementado.

## Cristal transparente y juego en pequeño

La habitación se compone como una textura RGBA sobre el juego: el cristal tiene una abertura transparente y sus bordes originales quedan por delante. `contornoCristal` define un polígono convexo de 16 puntos `[x, y]` en píxeles del PNG. Sigue el cristal completo, incluidas sus esquinas; `pantalla` sigue siendo el rectángulo seguro donde cabe todo el juego. Las zonas libres del cristal dejan ver el negro de fondo, sin conservar la fotografía gris original.

La abertura se prepara una sola vez al cargar o cambiar la variante, con suavizado en el borde y memoria liberada al cambiar. Cada fotograma dibuja la habitación en una sola pasada; no recalcula la máscara. Los PNG maestros permanecen sin cambios y una textura que ya tenga transparencia la conserva.

Clic dentro de la imagen jugable se entrega al juego también en pequeño; no amplía la vista. Solo el botón físico del monitor solicita el acercamiento. Ctrl+R vuelve a la habitación y Escape sigue reservado al juego.

El monitor pequeño utiliza siempre el muestreo original, sin filtros F11. La selección para la vista ampliada permanece guardada; no se calculan xBRZ, 2xSaI ni CRT mientras se ve la habitación. Al ampliar se recupera el filtro, incluida la mano original. F11 en pequeño muestra un aviso y no cambia la selección.

## Sonidos de presentación

El botón de la torre reproduce siete segundos de ventilador/disco, clic y pulsos del recuento de RAM. Apagar sustituye el arranque por un clic; el botón del monitor también tiene clic. Los clics del juego no producen ese sonido. Al entrar en Core se vacía la cola antes de reiniciar la música original. Un flujo SDL independiente permite oír la BIOS mientras el audio DOS permanece pausado; comparte M y volumen, y respeta --sin-sonido. Los fallos del recurso se informan y no bloquean el juego.

Los WAV redistribuibles están en packages/assets/lanzador/sonidos_pc junto a ORIGEN.txt. Ventilador/disco derivan de la vista previa CC0 de squashy555; clic/RAM son síntesis propia CC0. No se afirma que la grabación proceda de un 386. generar_sonidos_pc.py permite regenerar los recursos con Python/ffmpeg; no se necesita ffmpeg para jugar.

## Recursos del lanzador

`packages/assets/lanzador/` reúne `habitaciones/`, `presentacion/` y `sonidos_pc/`, fuera de `original`. `presentacion/presentacion.json` declara las variantes y los tiempos de entrada, permanencia, salida y espera del encendido en milisegundos. El logo independiente se conserva como fuente y no se copia al arranque. Los PNG maestros no se recomprimen ni editan. La presentación permite cerrar la ventana y alternar Alt+Intro; los clics durante el banner no llegan al juego. No tiene música, sonido ni filtros. El motor, su música y el contador efectivo permanecen suspendidos hasta que termine BIOS/DOS.

La prueba oculta `pnpm comprobar:habitaciones` añade seis capturas de banner (cinco proporciones y ventana vertical), negro inicial y plazos simulados: dos segundos de espera, siete segundos completos de BIOS/DOS, una sola solicitud y cancelación manual del encendido pendiente. También conserva las comprobaciones de habitación, ratón y audio.
