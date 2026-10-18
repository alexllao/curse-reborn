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

Àlex autoriza guardar y utilizar la fachada de Frank's Food y pide otra para el pozo del cubo. Son ilustraciones adicionales generadas para Curse Reborn a partir de referencias visuales, sin personajes ni interiores comerciales incorporados al PNG del decorado. Se conserva la resolución generada, sin ampliarla artificialmente: Frank, 1586×992; pozo, 1586×992. El montaje de referencia completo permanece en `trabajo/referencias/marcos/`, fuera de Git; mezcla una captura del juego con la propuesta artística y no es el asset utilizado por SDL.

`original/decorados/catalogo.json` asigna cada ilustración a su escena DOS, reserva el interior original. Los PNG se superponen con coordenadas uniformes; no se deforman ni se ajustan por filas. El arco de Frank está dibujado en la propia ilustración. Node valida dimensiones y límites y copia los PNG junto al programa. SDL compone únicamente sobre el vacío negro antes de los filtros, preservando la reserva central y las bandas originales del menú. No toca índices VGA, estado, inventario ni colisiones. El marco exterior sigue siendo otra capa independiente.

`pnpm comprobar:decorados` comprueba los dos decorados, la conservación de los píxeles de las escenas y sus menús, seis capturas filtradas por decorado y la recuperación exacta de la imagen original al desactivar la opción. Las imágenes locales quedan en `trabajo/comprobacion_decorados/`.

Los metadatos permiten un solape de dos píxeles sobre el borde del fondo estático. El arco superior de Frank pertenece al PNG; los personajes y objetos se protegen comparando la imagen actual con el fondo sin entidades, y el menú conserva su franja completa. El pozo utiliza promedio de área con color premultiplicado al reducir la ilustración: evita el muestreo de un único punto y los halos transparentes. Las sumas de color se preparan una vez por imagen para mantener acotado el coste durante la partida. El muestreo de Frank permanece por vecino más próximo.

## Monitor de PC 286

`original/marcos/monitor.png` contiene solo la carcasa propia generada a partir del concepto aprobado por Àlex, sin el título ni otros gráficos comerciales. `monitor.json` define las dimensiones y la abertura transparente. El concepto completo con la captura DOS se guarda únicamente como referencia local. F1 permite seleccionarlo en todas las escenas. Se encaja completo, sin girar ni recortar, y el juego conserva 320×200 dentro de la abertura. En ventanas altas quedan bandas negras exteriores; las proporciones se conservan. `pnpm comprobar:monitor` verifica cuatro formatos, transparencia central, conversión del ratón y restauración sin marco.
