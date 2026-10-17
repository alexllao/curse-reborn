# Teclas y controles

Referencia de los controles disponibles en Curse Reborn. Cada atajo nuevo se añade aquí. Las asignaciones actuales podrán reorganizarse durante el desarrollo.

## Sonido

| Tecla  | Función                                                                                                         |
| ------ | --------------------------------------------------------------------------------------------------------------- |
| **+**  | Subir el volumen general un 10 %, hasta el 200 %. También sirve **=**.                                          |
| **−**  | Bajar el volumen general un 10 %.                                                                               |
| **M**  | Silenciar o reactivar música y efectos, conservando el volumen elegido.                                         |
| **F3** | Cambiar los efectos entre Sound Blaster digital y PC Speaker, conservando la música.                            |
| **F4** | Cambiar la música entre ninguna, AdLib OPL2, Sound Blaster 16 OPL3 y MT-32. La consola indica la opción activa. |

El volumen va del 0 al 200 % y se muestra en consola al arrancar y al modificarlo. **+ y − del teclado numérico también funcionan.** MT-32 aparece cuando se pueden cargar sus ROM locales. AdLib mantiene su tema continuo; MT-32 usa los bancos musicales de cada zona. Los efectos digitales Sound Blaster están activos al arrancar; F3 permite comparar con PC Speaker sin reiniciar la escena. Ambas opciones comparten el volumen y el silencio.

El **100 %** es el nivel normal. Entre **110 % y 200 %** se aplica amplificación adicional, que puede saturar en los pasajes fuertes. M conserva también los ajustes superiores al 100 %.

## Aspecto de pantalla

**F1** muestra u oculta el marco decorativo y registra el cambio en consola. Arranca desactivado. El marco por defecto es el PNG de Àlex, conservado en `assets/original/marcos/`; de momento todas las pantallas utilizan únicamente `default`, incluida la mazmorra. Rellena las bandas y conserva completo el remate artístico del borde interior, incluida su transparencia, más un solape adicional de dos píxeles lógicos con redondeo al píxel físico. El relieve puede superponerse al juego y a su barra; no se recorta para proteger el HUD. No cambia las coordenadas de clic. Funciona también con los filtros y en pantalla completa; si no sobra espacio alrededor del juego, no aparece. Los futuros marcos del remaster irán separados. Véase [Recursos propios](../assets/README.md).

**F11** recorre **original → suavizado → líneas de escaneo → CRT suave → xBRZ 2x → 2xSaI → original**. La consola muestra el nombre elegido. Funciona en Core, título, introducción, escenas y menús, sin avanzar la presentación ni cambiar la partida. Al mantener pulsada la tecla no se repite el cambio.

El original arranca siempre sin filtro, con vecino más próximo. Suavizado usa interpolación lineal. Líneas de escaneo atenúa filas alternas a doble resolución. CRT suave añade mezcla horizontal, máscara RGB y una viñeta tenue; es un efecto visual sencillo. Se conservan proporciones, bandas negras y coordenadas del ratón. El ajuste permanece al cambiar de escena durante la misma ejecución.

**xBRZ 2x** reconstruye diagonales y redondea contornos mediante diferencias perceptuales de color. **2xSaI** interpola diagonales con un aspecto más suave, similar al de emuladores clásicos. Ambos amplían la imagen compuesta de 320 × 200 a 640 × 400 antes de ajustarla a la ventana; también afectan al menú original. Son opciones visuales para probar, independientes de la reconstrucción del motor. Detalles y comprobaciones en [Filtros de píxeles](filtros_pixeles.md).

**Alt + Enter** alterna entre ventana y pantalla completa sin bordes en el monitor actual. También funciona con Intro del teclado numérico. Conserva las proporciones, las bandas negras, el filtro y la partida, sin avanzar las introducciones. Al volver se recupera la ventana anterior. El arranque sigue siendo en ventana.

**F12** muestra u oculta el contador de FPS y registra el cambio en consola. Arranca visible, en una placa pequeña en la esquina superior derecha, con tamaño fijo en píxeles de ventana. Se coloca sobre la banda negra si la hay. Actualiza el valor cada medio segundo y mide el dibujo de fotogramas, independiente del reloj del juego. La ventana tiene un máximo nominal de 70 FPS; puede rendir menos según el equipo o durante una carga. En las capturas de referencia queda oculto salvo al comprobar expresamente F12.

Está previsto un HUD con mensajes breves de cambios y acciones. Todavía no está implementado: actualmente esa información se registra en consola.

**F2** alterna entre el límite de 70 FPS y la presentación sin límite, con aviso en consola. Arranca limitado y conserva la opción al cambiar de escena durante la misma ejecución. No acelera la lógica ni el audio, no avanza las introducciones y no se repite al mantener la tecla pulsada.

## Jugar

| Control                                              | Función                                                            |
| ---------------------------------------------------- | ------------------------------------------------------------------ |
| **Clic izquierdo**, con el menú cerrado              | Indicar adónde debe caminar Brad, cuando el guion permite moverlo. |
| **Flechas**, con el menú cerrado                     | Caminar, cuando Brad tiene el control.                             |
| **Escape** o **clic derecho**, en una escena jugable | Abrir o cerrar el menú de acciones, cuando el guion lo permite.    |
| **Ratón + clic izquierdo**, con el menú abierto      | Señalar y elegir una casilla del menú.                             |
| **← / →**, con el menú abierto                       | Cambiar de casilla.                                                |
| **Intro**, con el menú abierto                       | Confirmar la casilla seleccionada.                                 |

En Core y título, una tecla o un clic izquierdo permite avanzar. En la introducción solicita omitirla en el siguiente punto previsto por el original. Los controles de sonido, F1, F2, F11, F12 y Alt + Enter funcionan sin avanzar la presentación.

**Escape cierra el programa en Core, título e introducción.** Dentro de una escena jugable abre o cierra el menú. El botón de cierre de la ventana termina el programa desde cualquier vista.

## Recorrer pantallas durante el desarrollo

| Tecla  | Función                               |
| ------ | ------------------------------------- |
| **F5** | Ir directamente a la vista anterior.  |
| **F6** | Ir directamente a la vista siguiente. |

Orden actual: **Core → título → introducción → mazmorra → corredor → caída → lago → poza → primer corredor de las cuevas → segundo corredor → tercer corredor → pozo → picapedrero**. En los extremos no hay otra vista a la que saltar. La ruta normal permite recoger doce piedras y entregarlas al picapedrero para obtener la cuerda. Los puzles del pozo y otras interacciones de las cuevas siguen pendientes.

**Al entrar en una vista jugable mediante F5/F6 se prepara una sesión nueva.** Estos saltos sirven para probar pantallas: no conservan el progreso ni resuelven los puzles anteriores. Para continuar la misma partida, utiliza las salidas normales del escenario.

## Ver objetos y límites

Estos atajos funcionan en las escenas jugables y no modifican el estado del juego.

| Tecla   | Función                                                                                  |
| ------- | ---------------------------------------------------------------------------------------- |
| **F7**  | Mostrar u ocultar toda la depuración gráfica.                                            |
| **F8**  | Mostrar u ocultar las cajas de entidades; activa la depuración.                          |
| **F9**  | Mostrar u ocultar las zonas de interacción y proximidad; activa la depuración.           |
| **F10** | Mostrar u ocultar encuadre, umbrales, paredes y salidas de BOXDET; activa la depuración. |

Colores: **verde**, objetos; **cian**, Brad; **magenta**, zonas de interacción; **naranja**, proximidad de los pies; **amarillo**, encuadre y paredes; **azul**, salidas. La consola registra los cambios de estas opciones.
