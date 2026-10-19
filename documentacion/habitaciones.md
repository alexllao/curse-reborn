# Habitaciones interactivas

Son una presentación externa al juego. Los marcos decorativos rellenan bandas; las habitaciones representan un ordenador que se enciende y permite entrar al juego. Los decorados ampliados continúan aparcados.

El arranque normal siempre muestra Buhardilla con el ordenador apagado, también con `--sin-sonido`. Botón del PC → BIOS 386 con distintivo Energy Star → DOS → Core/título/introducción dentro del monitor. Tras los siete segundos, clic en pantalla o botón del monitor → acercamiento suave de 650 ms hasta el máximo disponible. Ctrl+R vuelve sin reiniciar escena ni música. Ctrl+H recorre los cinco diseños. Alt+Intro conserva su función de pantalla completa de la ventana.

## Añadir otro diseño

1. Crear una carpeta en `assets/original/habitaciones/` con las imágenes PNG, manteniendo sus dimensiones originales.
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
