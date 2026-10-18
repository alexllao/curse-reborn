# Recursos propios del port

Esta carpeta contiene material adicional aportado o creado para Curse Reborn que puede versionarse. Los archivos comerciales del juego y las ROM siguen siendo locales en `datos_originales/`.

- `original/marcos/`: marcos de estilo pixel art para acompañar la reconstrucción fiel, opcionales con F1.
- `original/decorados/`: decorados ampliados propios del port, opcionales con Mayús+F1; rellenan huecos dentro de la imagen del juego y reciben sus filtros.
- `remaster/marcos/`: ubicación prevista para futuros marcos del remaster; todavía no contiene material.

`original` identifica el estilo de la versión del port. Estos marcos no proceden del juego DOS. Àlex ha aportado `default.png` y sus metadatos y ha autorizado incluirlos en el repositorio. El PNG se conserva sin editar; su SHA-256 es `083ee51a182d570125cd97cb4d3783c6311333afec90190018f3a3933aadbd93`. Esta atribución no asigna una licencia al resto del proyecto.

## Asignar marcos por pantalla

Cada imagen tiene sus metadatos en `<nombre>.png.json`. `original/marcos/catalogo.json` elige el identificador `predeterminado` y permite asignaciones en `pantallas`, con claves `core`, `titulo`, `introduccion` o `escena_N`, donde N es el índice original de la escena. Por ejemplo, `escena_0` corresponde a la mazmorra inicial. Los valores son identificadores de imágenes sin `.png`.

De momento solo se utiliza `default`, en todas las pantallas, incluida la mazmorra. Àlex ha descartado los marcos específicos de corredor, lago y mazmorra porque no encajan con el acabado deseado. Se han retirado sus PNG y metadatos de la versión actual; las referencias externas permanecen sin modificar. El catálogo conserva la posibilidad de asignar marcos por escena en el futuro.

Al compilar, Node valida recortes, dimensiones, huella y asignaciones, genera un catálogo C++ local y copia los assets junto al ejecutable. Añadir o cambiar una asignación requiere recompilar. El programa no depende de la ruta del proyecto anterior.

## Encuadre y profundidad

Se respetan los recortes `left` y `right`, `innerPadding`, `fit` y `mirrorRight` del JSON. El ornamento queda junto al juego y el panel derecho se refleja según los metadatos. `innerOverlap`, opcional, permite de cero a cuatro píxeles lógicos de solape; por defecto son dos, según lo solicitado por Àlex. El solape adicional se reduce cuando la banda es muy estrecha y se redondea al píxel físico. El margen `innerPadding` posiciona el cuerpo del panel; su remate artístico se dibuja completo, con transparencia, aunque sobresalga más sobre el juego o su barra. Àlex solicita conservar ese relieve sin recortarlo; cualquier ajuste si tapa el HUD se hará después. No se agranda ni se estira el área del juego ni se cambian los clics. También se rellenan las bandas superior e inferior en ventanas verticales.

F1 recorre predeterminado, monitor de PC 286 y sin marco; arranca sin marco. El predeterminado aparece mediante un zoom del 110 al 100 % y un fundido de 250 ms. La salida invierte el efecto; se puede cambiar de dirección durante la animación. Solo se transforma el marco. Los filtros de F11 se aplican al juego y el marco conserva su textura pixel art. `pnpm comprobar:marcos` verifica las capturas del ciclo completo: cinco fases de entrada y salida animadas, además de apaisado, vertical, encuadre sin bandas, dimensiones impares, xBRZ y el predeterminado en mazmorra, corredor y lago sobre una imagen de prueba común. Comprueba la simetría de las fases intermedias, la reversibilidad de F1 y que el interior del juego quede intacto fuera del remate artístico y su solape adicional. También comprueba que haya píxeles del relieve donde antes se recortaba. Las capturas BMP se leen respetando la alineación de filas, también con anchuras impares.

En ventanas altas, los paneles laterales se giran90° para cubrir las bandas superior e inferior. Su remate sigue orientado hacia el juego y conserva transparencia, relieve y animación F1; no se estira una pared vertical sobre una banda horizontal.

## Decorados ampliados

La fachada propia de Frank's Food está en `original/decorados/franks_food.png`, a1586×992, con el arco dibujado en el PNG. Se activa con Mayús+F1 y recibe los filtros del juego. La composición uniforme conserva personajes, objetos, interior y menús; no deforma el arte ni fabrica arcos por código.

El decorado del pozo del cubo se ha retirado del catálogo por petición de Àlex: no se adapta bien al original. El pozo conserva su vista original incluso cuando los decorados están activados. El PNG descartado se mantiene como referencia sin asignación, no como una decoración disponible.

`pnpm comprobar:decorados` verifica Frank con los seis filtros, bandas de menú y restauración exacta al desactivar; también comprueba que pasar al pozo con la opción activa no herede ningún decorado.

## Monitor de PC 286 y habitación

`original/marcos/monitor.png` contiene la carcasa propia. `habitacion_90.png` es una habitación de época generada para usar detrás: póster ilustrado de Monkey Island, cajas de aventuras gráficas, una caja de Curse of Enchantia, disquetes, joystick, revista, casetes, lámpara y radio. Son guiños artísticos a títulos de terceros, no portadas extraídas de sus juegos. Ambos PNG son arte adicional, sin imágenes del juego DOS. `monitor.json` describe sus dimensiones y la abertura de la carcasa.

F1 recorre predeterminado → monitor → sin marco. El monitor arranca con el mismo encuadre máximo320×200 que los otros modos. El usuario puede alejarlo expresamente entre el50 % y el100 % con Ctrl+rueda o Ctrl+−/+, y recuperar el máximo con Ctrl+0. La preferencia dura la sesión y no afecta a los otros modos. Se coloca la abertura del monitor alrededor de esa imagen: si la carcasa no cabe, queda fuera de la ventana. No se reduce automáticamente el juego para mostrar los botones del monitor; el zoom manual permite ver más habitación cuando se desea. La habitación usa un ajuste proporcional para cubrir el espacio exterior, anclado arriba para conservar el título del póster en32:9; las dos ilustraciones se recortan exclusivamente a las bandas y no pisan un solo píxel del juego. No se giran en ventanas verticales. Los filtros de F11 permanecen en la imagen del juego.

`pnpm comprobar:monitor` verifica seis formatos, incluido3840×1080 (32:9), vertical y anchura impar. Comprueba dieciocho combinaciones de formato y zoom (100,70 y50 %), comparando todos los píxeles jugables antes y después del marco. Revisa las cuatro uniones del bisel negro para detectar rendijas o residuos de color, el ratón, el relevo de FPS, los atajos de rueda/teclado, los límites, el retorno al100 % y la conservación de la preferencia con F1. Las capturas se guardan en `trabajo/comprobacion_monitor/`.

El PNG del monitor conservaba restos de color junto a la abertura transparente. Se dibuja un bisel negro uniforme que cubre esa franja exterior, sin tapar ni recortar píxeles de la imagen jugable. La misma área física determina la posición del monitor, los clips y las coordenadas del ratón, también con zoom y tamaños impares.
