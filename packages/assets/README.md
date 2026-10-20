# Recursos propios del port

Esta carpeta contiene material adicional aportado o creado para Curse Reborn que puede versionarse. Los archivos comerciales del juego y las ROM siguen siendo locales en `apps/juego/datos_originales/`.

Al empaquetar se comprimen copias de los PNG, manteniendo sus dimensiones y los maestros intactos. Por defecto solo se incluye la habitación inicial del JSON; `--habitacion todas` incluye todos los diseños y `--habitacion <id>` elige otro. El catálogo compilado y el JSON entregado se ajustan a esa selección. El perfil equilibrado puede reducir colores en ilustraciones opacas; `--sin-perdida` conserva exactamente los píxeles. Véase [Distribución](../../apps/juego/documentacion/distribucion.md).

- `lanzador/presentacion/`: banners de arranque y tiempos en `presentacion.json`, sin música.
- `lanzador/icono/`: monograma propio CR en SVG, PNG y un ICO con siete tamaños para ejecutable y ventana.
- `lanzador/sonidos_pc/`: sonidos de botón y hardware durante BIOS/DOS.
- `lanzador/habitaciones/`: habitaciones interactivas, cinco diseños con cinco proporciones y catálogo independiente.
- `marcos/`: todos los marcos exteriores, tanto el predeterminado como las alternativas ilustradas, opcionales con F1.
- `original/decorados/`: PNG de decorados ampliados conservados como referencia; están aparcados y su catálogo activo está vacío.
- `marcos/cielo_encantado/`: cuatro variantes ilustradas y su catálogo, dentro del mismo grupo de marcos.

Estos marcos no proceden del juego DOS y se agrupan sin separar original/remaster. Àlex ha aportado `default.png` y sus metadatos y ha autorizado incluirlos en el repositorio. El PNG se conserva sin editar; su SHA-256 es `083ee51a182d570125cd97cb4d3783c6311333afec90190018f3a3933aadbd93`. Esta atribución no asigna una licencia al resto del proyecto.

## Asignar marcos por pantalla

Cada imagen tiene sus metadatos en `<nombre>.png.json`. `marcos/catalogo.json` elige el identificador `predeterminado` y permite asignaciones en `pantallas`, con claves `core`, `titulo`, `introduccion` o `escena_N`, donde N es el índice original de la escena. Por ejemplo, `escena_0` corresponde a la mazmorra inicial. Los valores son identificadores de imágenes sin `.png`.

El catálogo predeterminado utiliza `default` en todas las pantallas, incluida la mazmorra; F1 permite elegir también Cielo encantado. Àlex ha descartado los marcos específicos de corredor, lago y mazmorra porque no encajan con el acabado deseado. Se han retirado sus PNG y metadatos de la versión actual; las referencias externas permanecen sin modificar. El catálogo conserva la posibilidad de asignar marcos por escena en el futuro.

Al compilar, Node valida recortes, dimensiones, huella y asignaciones, genera un catálogo C++ local y copia los assets junto al ejecutable. Añadir o cambiar una asignación requiere recompilar. El programa no depende de la ruta del proyecto anterior.

## Encuadre y profundidad

Se respetan los recortes `left` y `right`, `innerPadding`, `fit` y `mirrorRight` del JSON. El ornamento queda junto al juego y el panel derecho se refleja según los metadatos. `innerOverlap`, opcional, permite de cero a cuatro píxeles lógicos de solape; por defecto son dos, según lo solicitado por Àlex. El solape adicional se reduce cuando la banda es muy estrecha y se redondea al píxel físico. El margen `innerPadding` posiciona el cuerpo del panel; su remate artístico se dibuja completo, con transparencia, aunque sobresalga más sobre el juego o su barra. Àlex solicita conservar ese relieve sin recortarlo; cualquier ajuste si tapa el HUD se hará después. No se agranda ni se estira el área del juego ni se cambian los clics. También se rellenan las bandas superior e inferior en ventanas verticales.

F1 recorre sin marco → predeterminado → Cielo encantado → sin marco. La habitación interactiva tiene sus propios controles y arranca siempre antes del juego. El predeterminado aparece mediante un zoom del 110 al 100 % y un fundido de 250 ms. La salida invierte el efecto; se puede cambiar de dirección durante la animación. Solo se transforma el marco. Los seis filtros de F11 se aplican tanto al juego y su cursor como al marco. El arranque normal selecciona líneas de escaneo; dentro de la habitación siguen suspendidos. `pnpm comprobar:marcos` verifica las capturas del ciclo completo: cinco fases de entrada y salida animadas, además de apaisado, vertical, encuadre sin bandas, dimensiones impares, xBRZ y el predeterminado en mazmorra, corredor y lago sobre una imagen de prueba común. Comprueba la simetría de las fases intermedias, la reversibilidad de F1 y que el interior del juego quede intacto fuera del remate artístico y su solape adicional. También comprueba que haya píxeles del relieve donde antes se recortaba. Las capturas BMP se leen respetando la alineación de filas, también con anchuras impares.

En ventanas altas, los paneles del marco predeterminado se giran90° para cubrir las bandas superior e inferior. Su remate sigue orientado hacia el juego y conserva transparencia, relieve y animación F1; no se estira una pared vertical sobre una banda horizontal.

## Decorados ampliados

Los decorados de Frank y del pozo están **aparcados y sin asignación** por petición de Àlex. El catálogo activo está vacío; el juego muestra las escenas originales. No volver a habilitarlos sin una nueva decisión del propietario.

Se conservan `original/decorados/franks_food.png` (1586×992) y el PNG del pozo como referencias para una revisión posterior de perspectiva, suelo y bordes. No se modifican ni se eliminan estos archivos. Mayús+F1 indica por consola que los decorados están aparcados. Los marcos exteriores permanecen disponibles con F1.

`pnpm comprobar:decorados` verifica el catálogo vacío y que Frank y el pozo mantienen sus píxeles originales con los seis filtros, incluso al intentar activar la opción.

## Habitaciones interactivas

Las ilustraciones aportadas de `01-misterio` a `05-ciencia-ficcion` viven ahora en `lanzador/habitaciones/`, con el catálogo `habitaciones.json`. Los PNG maestros se conservan byte por byte. Se han retirado `marcos/monitor.png`, `marcos/habitacion_90.png` y sus metadatos antiguos por petición de Àlex; no se utilizan en el arranque. Las otras cinco variantes `habitacion-90-*` aportadas en la misma subida se conservan en `habitaciones/alternativas/`, sin asignación ni copia a la aplicación.

El catálogo especifica cada carpeta, imagen y sus controles; instrucciones y límites en [Habitaciones](../../apps/juego/documentacion/habitaciones.md). Los maestros no se recomprimen ni se reducen al compilar. La optimización de imágenes del paquete final queda para la distribución, separada de estos originales.

## Cielo encantado

`marcos/cielo_encantado/catalogo.json` identifica las variantes 32:9,24:9,16:9 y4:3. Cada PNG tiene metadatos de dimensiones reales y huella; no son capturas del juego. Son ilustraciones nuevas generadas con la herramienta integrada a partir de la portada Reborn aportada por Àlex: cielo azul/violeta, cabello oscuro, destellos dorados y paisaje discreto, sin título ni personajes. Los originales generados se conservan sin estirar ni convertir el boceto comparativo en un recurso.

La aplicación elige la proporción más cercana mediante distancia logarítmica y cubre la ventana sin deformar el dibujo. Solo dibuja los fragmentos de las bandas sobrantes: laterales en panorámico, superior/inferior en4:3; en16:10 no se ve marco. La variante32:9 tiene maestro2172×724 (3:1), cuyo encuadre se recorta al cubrir32:9; no se presenta como un PNG nativo32:9. Las demás dimensiones son2048×768,1672×941 y1448×1086. Cada tamaño de ventana utiliza estos maestros escalables, sin duplicar archivos por resolución física.

F1 conserva el encuadre máximo del juego. El panorama no invade la imagen, no gira y no reutiliza el relieve del marco de piedra. El builder copia los cuatro PNG, sus metadatos y el catálogo; las optimizaciones modifican exclusivamente las copias entregadas. El filtro del marco se prepara al cambiar F11 o variante, no en cada fotograma; la imagen del juego conserva sus reglas originales de filtrado. Los decorados ampliados internos continúan aparcados.

Comprobación de esta integración: `pnpm comprobar:marcos` pasa65 capturas (quince ciclos de cuatro imágenes más cinco fases del predeterminado). Incluye32:9,24:9,16:9,16:10 y4:3, ventana alta e impar, seis modos y asignaciones de contexto; el interior queda intacto y el ciclo termina como empezó. También se contrastaron las bandas de ambos marcos: seis huellas diferentes para los seis filtros. `pnpm comprobar:filtros` conserva los64.000 píxeles originales al volver a modo original; `pnpm comprobar:cursor` pasa los seis modos y el relevo dentro/fuera del monitor. Calidad, monorepo y seis pruebas del builder correctos. No se ha generado un ZIP beta nuevo para esta tanda.

Corrección del encuadre desde el lanzador: los marcos consultan el área física del viewport de la habitación, desactivan temporalmente su escala/viewport al dibujar bandas y restauran ambos después. Las pruebas aisladas de marcos no ejercitaban ese estado; `pnpm comprobar:habitaciones` incluye ahora el ciclo de ambos marcos al ampliar desde el botón del monitor en seis formatos. La carpeta unificada es `packages/assets/marcos/`, sin separación original/remaster para estos recursos de presentación.

Ejecutado tras la corrección: `pnpm comprobar:habitaciones` pasa64 capturas y la comprobación de arranque normal. Incluye18 capturas nuevas del ciclo de marcos desde el monitor ampliado en seis formatos, con detección de bandas vacías, conservación del interior y regreso a la imagen sin marco. Las imágenes de referencia de ese recorrido están en `apps/juego/trabajo/comprobacion_monitor/monitor.0.<ancho>x<alto>.marco_2.png`, incluido3200×900. El fallo anterior era de coordenadas y recorte del viewport, no de carga de PNG.
