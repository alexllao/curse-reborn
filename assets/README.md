# Recursos propios del port

Esta carpeta contiene material adicional aportado o creado para Curse Reborn que puede versionarse. Los archivos comerciales del juego y las ROM siguen siendo locales en `datos_originales/`.

- `original/marcos/`: marcos de estilo pixel art para acompañar la reconstrucción fiel, opcionales con F1.
- `remaster/marcos/`: ubicación prevista para futuros marcos del remaster; todavía no contiene material.

`original` identifica el estilo de la versión del port. Estos marcos no proceden del juego DOS. Àlex ha aportado `default.png` y sus metadatos y ha autorizado incluirlos en el repositorio. El PNG se conserva sin editar; su SHA-256 es `083ee51a182d570125cd97cb4d3783c6311333afec90190018f3a3933aadbd93`. Esta atribución no asigna una licencia al resto del proyecto.

## Asignar marcos por pantalla

Cada imagen tiene sus metadatos en `<nombre>.png.json`. `original/marcos/catalogo.json` elige el identificador `predeterminado` y permite asignaciones en `pantallas`, con claves `core`, `titulo`, `introduccion` o `escena_N`, donde N es el índice original de la escena. Por ejemplo, `escena_0` corresponde a la mazmorra inicial. Los valores son identificadores de imágenes sin `.png`.

De momento solo se utiliza `default`, en todas las pantallas, incluida la mazmorra. Àlex ha descartado los marcos específicos de corredor, lago y mazmorra porque no encajan con el acabado deseado. Se han retirado sus PNG y metadatos de la versión actual; las referencias externas permanecen sin modificar. El catálogo conserva la posibilidad de asignar marcos por escena en el futuro.

Al compilar, Node valida recortes, dimensiones, huella y asignaciones, genera un catálogo C++ local y copia los assets junto al ejecutable. Añadir o cambiar una asignación requiere recompilar. El programa no depende de la ruta del proyecto anterior.

## Encuadre y profundidad

Se respetan los recortes `left` y `right`, `innerPadding`, `fit` y `mirrorRight` del JSON. El ornamento queda junto al juego y el panel derecho se refleja según los metadatos. `innerOverlap`, opcional, permite de cero a cuatro píxeles lógicos de solape; por defecto son dos, según lo solicitado por Àlex. El solape adicional se reduce cuando la banda es muy estrecha y se redondea al píxel físico. El margen `innerPadding` posiciona el cuerpo del panel; su remate artístico se dibuja completo, con transparencia, aunque sobresalga más sobre el juego o su barra. Àlex solicita conservar ese relieve sin recortarlo; cualquier ajuste si tapa el HUD se hará después. No se agranda ni se estira el área del juego ni se cambian los clics. También se rellenan las bandas superior e inferior en ventanas verticales.

F1 activa y desactiva esta capa, inicialmente oculta, mediante un zoom del 110 al 100 % y un fundido de 250 ms. La salida invierte el efecto; se puede cambiar de dirección durante la animación. Solo se transforma el marco. Los filtros de F11 se aplican al juego y el marco conserva su textura pixel art. `pnpm comprobar:marcos` verifica veintinueve capturas: cinco fases de entrada y salida animadas, además de apaisado, vertical, encuadre sin bandas, dimensiones impares, xBRZ y el predeterminado en mazmorra, corredor y lago sobre una imagen de prueba común. Comprueba la simetría de las fases intermedias, la reversibilidad de F1 y que el interior del juego quede intacto fuera del remate artístico y su solape adicional. También comprueba que haya píxeles del relieve donde antes se recortaba. Las capturas BMP se leen respetando la alineación de filas, también con anchuras impares.
