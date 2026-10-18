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

**Mayús + F1** activa o desactiva los **decorados ampliados**, independientemente de F1. Rellenan el vacío negro dentro de la imagen original con arte adicional propio del port. Están disponibles en Frank's Food; el decorado del pozo se ha retirado a petición de Àlex y esa escena conserva su imagen original. Arrancan desactivados, mantienen la preferencia durante la sesión e informan en consola. Reciben todos los filtros de F11, incluidos xBRZ y 2xSaI. No cambian los clics, las paredes ni los puzles. F2 continúa controlando el límite de FPS.

**F1** recorre **predeterminado → monitor de PC 286 → sin marco**. Arranca sin marco y cada cambio aparece en consola. El predeterminado conserva el PNG de Àlex, su relieve y la entrada/salida con zoom y fundido de 250 ms; en ventanas altas gira sus paneles laterales para ocupar las bandas superior e inferior.

El monitor añade una carcasa beige con una habitación de finales de los 80 y principios de los 90 detrás. **No reduce automáticamente el juego**: conserva el tamaño máximo salvo que se elija un zoom menor. La carcasa rodea esa imagen y puede quedar parcialmente fuera de la ventana cuando no cabe; no gira, estira ni tapa la escena. La habitación rellena el espacio exterior que quede libre, especialmente en pantallas 32:9. Ratón, paneles y depuración conservan las coordenadas de siempre. Los filtros F11 afectan al juego y no al monitor o la habitación. **Mayús + F1** sigue controlando por separado los decorados ampliados. Véase [Recursos propios](../assets/README.md).

Con el monitor seleccionado, **Ctrl + rueda hacia abajo** aleja y **Ctrl + rueda hacia arriba** acerca. También sirven **Ctrl + −** y **Ctrl + +**; **Ctrl + 0** vuelve al tamaño máximo. El zoom va del **50 % al 100 %**, en pasos del 10 %, y se muestra en consola. El ajuste es manual: arranca al 100 % y conserva la preferencia durante la sesión, incluso al recorrer F1. En los otros dos modos el juego mantiene su tamaño máximo. **+ y − sin Ctrl** siguen controlando el volumen. El ratón, el cursor y los paneles se ajustan a la nueva escala.

**F11** recorre **original → suavizado → líneas de escaneo → CRT suave → xBRZ 2x → 2xSaI → original**. La consola muestra el nombre elegido. Funciona en Core, título, introducción, escenas y menús, sin avanzar la presentación ni cambiar la partida. Al mantener pulsada la tecla no se repite el cambio.

El original arranca siempre sin filtro, con vecino más próximo. Suavizado usa interpolación lineal. Líneas de escaneo atenúa filas alternas a doble resolución. CRT suave añade mezcla horizontal, máscara RGB y una viñeta tenue; es un efecto visual sencillo. Se conservan proporciones, bandas negras y coordenadas del ratón. El ajuste permanece al cambiar de escena durante la misma ejecución.

**xBRZ 2x** reconstruye diagonales y redondea contornos mediante diferencias perceptuales de color. **2xSaI** interpola diagonales con un aspecto más suave, similar al de emuladores clásicos. Ambos amplían la imagen compuesta de 320 × 200 a 640 × 400 antes de ajustarla a la ventana; también afectan al menú original. Son opciones visuales para probar, independientes de la reconstrucción del motor. Detalles y comprobaciones en [Filtros de píxeles](filtros_pixeles.md).

**Alt + Enter** alterna entre ventana y pantalla completa sin bordes en el monitor actual. También funciona con Intro del teclado numérico. Conserva las proporciones, las bandas negras, el filtro y la partida, sin avanzar las introducciones. Al volver se recupera la ventana anterior. El arranque sigue siendo en ventana.

**F12** muestra u oculta el contador de FPS y registra el cambio en consola. Arranca visible, en una placa compacta con esquinas redondeadas y texto centrado en la esquina superior derecha. Su ancho se ajusta al texto, con márgenes iguales y tamaño en píxeles de ventana. Se coloca sobre la banda negra si la hay. Actualiza el valor cada medio segundo y mide el dibujo de fotogramas, independiente del reloj del juego. La ventana tiene un máximo nominal de 70 FPS; puede rendir menos según el equipo o durante una carga. En las capturas de referencia queda oculto salvo al comprobar expresamente F12.

Está previsto un HUD con mensajes breves de cambios y acciones. Todavía no está implementado: actualmente esa información se registra en consola. El HUD y las futuras opciones dentro del juego compartirán el estilo de la interfaz original: bordes metálicos con relieve, tonos grises y detalles rojizos. La muestra del aviso de pantalla es una propuesta visual; los paneles de opciones quedan pendientes de diseñar e implementar.

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

El orden completo y los nombres están en el [catálogo de pantallas](pantallas.md): Core → título → introducción → las diecisiete vistas jugables, desde **La mazmorra** hasta **El camino del asaltante**. En los extremos no hay otra vista a la que saltar. Los nombres también se muestran en consola y en el título de la ventana. La ruta normal conserva el inventario y continúa por los puzles de las cuevas hasta salir del cubo y recorrer el campo.

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

## Guardar, cargar y borrar

Abrir el menú con Escape o botón derecho y elegir el octavo icono, **partidas**. Su submenú ofrece cargar, guardar y borrar. El décimo icono abre la ficha de información y puntuación.

Los paneles mantienen la imagen, los textos y FONT.SPR del original, con ocho ranuras. Ratón o flechas arriba/abajo seleccionan; clic izquierdo o Intro confirman. Borrar actúa directamente, sin confirmación adicional; el perfil conserva copia anterior. Clic derecho cancela y Escape vuelve desde el selector.

Guardar abre el teclado original de trece caracteres. Flechas desplazan el cursor; Insertar pulsa su tecla. También se puede escribir con el teclado físico, usar Espacio y Retroceso, y guardar con Intro. Clic derecho cancela el editor; Escape no lo cierra. No hay Ctrl+A ni botones nuevos. Los huecos entre las letras del teclado en pantalla no escriben.

La importación DOS queda como integración de escritorio mediante arrastrar CURSE.SAV a la ventana; no aparece un botón Importar dentro del panel original. Se añaden partidas en ranuras libres sin sobrescribir las existentes ni modificar el origen. Véanse [formato y límites comprobados](partidas.md), incluida la peculiaridad del borrado de la octava ranura en el EXE.

### Fichas de información

El último icono del menú abre las cuatro páginas originales con puntuación y créditos. Clic izquierdo, Intro o Espacio avanzan; después de la cuarta página se vuelve al juego. Escape o clic derecho cierran directamente. La música continúa y el personaje permanece detenido durante la consulta.

La última vista F5/F6 es **El camino del asaltante (COUNTRY2)**, después de **La salida al campo**: diecisiete escenas jugables. Reproduce la entrada desde el pozo como una sesión nueva de desarrollo. Para conservar los objetos obtenidos hay que llegar por las salidas normales.

F5/F6 incorpora entrada de Paso del Dragón, Frank, plaza de la fuente, calle de las maravillas y mago: veintidós vistas de desarrollo. Estos atajos no transfieren inventario. Para probar la compra y el mago en una partida continua, sigue el [recorrido de la población](recorridos/10_paso_del_dragon.txt). El borde del mundo es todavía un punto de llegada detenido; F5/F6 permite volver a las vistas disponibles.

Los marcos se orientan según las bandas: laterales en ventanas anchas y girados90° arriba/abajo en ventanas altas. F1 mantiene su fundido y zoom. El título de ventana numera las vistas como «1: La mazmorra».

El decorado ampliado de Frank se superpone entero sobre la escena. La transparencia del PNG define su abertura; el arco y las piedras quedan delante del interior y pueden ocultar partes de personajes u objetos en sus bordes. Los menús y el cursor siguen por encima. Al desactivarlo se recupera la imagen original.

La mano original del ratón aparece únicamente dentro de la imagen jugable. Sobre el marco, la habitación o las bandas, y al salir de la ventana, se recupera la flecha normal. Volver al juego restaura la mano a su escala actual, también con zoom. Los paneles DOS ocultan el cursor nativo solo mientras el puntero permanece dentro del juego, para evitar duplicar su mano incorporada.

## Botón del monitor

Con el marco de PC seleccionado, el primer clic izquierdo en su botón apaga el ordenador: piloto rojo, cierre CRT y pantalla negra. Permanece apagado hasta volver a pulsar. El segundo clic enciende el piloto verde y reproduce siete segundos de arranque: BIOS inspirada en los 386 de AMI, contador de memoria, comprobación de unidades y presentación de DOS cargando CURSE. **El arranque se muestra completo y no se puede saltar.** Después comienza Core, seguido del título y la introducción. Es una recreación visual propia, sin firmware ni ejecución real de DOS.

Si el botón queda fuera de la ventana, aleja el monitor con Ctrl+rueda o Ctrl+−, también mientras está apagado. Durante el apagado y el arranque se pausan la partida y el sonido, se ocultan los FPS y se mantiene la flecha normal. Pulsar el botón durante el arranque vuelve a apagarlo; los rebotes durante el cierre CRT se ignoran.

Al terminar el arranque se cierran los paneles y comienza de nuevo la música, conservando volumen, silencio, perfil de música y efectos, filtros, marco, zoom y pantalla completa. No escribe ni borra partidas guardadas. El clic derecho y los clics fuera del botón no activan la alimentación. La consola registra apagado, encendido y carga del juego. Se puede cerrar la aplicación mediante su ventana en cualquier momento.

F5/F6 incorpora también Benn, la tienda de la adivina y la segunda casa abandonada: veintinueve vistas. Las puertas35/38/39 y sus regresos46/48/49 conservan la partida; el atajo sigue abriendo una sesión independiente.
