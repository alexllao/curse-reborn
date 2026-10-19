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

**M** muestra «Sonido silenciado» o «Sonido activado» con el mismo texto blanco y fundido de los otros avisos. Silencia música y efectos y recupera el porcentaje anterior al reactivarlo; también funciona durante BIOS o con el PC apagado. Las secuencias conservan su reloj mientras están silenciadas.

El volumen va del 0 al 200 % y se muestra en consola al arrancar y al modificarlo. **+ y − del teclado numérico también funcionan.** MT-32 aparece cuando se pueden cargar sus ROM locales. AdLib mantiene su tema continuo; MT-32 usa los bancos musicales de cada zona. Los efectos digitales Sound Blaster están activos al arrancar; F3 permite comparar con PC Speaker sin reiniciar la escena. Ambas opciones comparten el volumen y el silencio.

El **100 %** es el nivel normal. Entre **110 % y 200 %** se aplica amplificación adicional, que puede saturar en los pasajes fuertes. M conserva también los ajustes superiores al 100 %.

## Aspecto de pantalla

**Mayús + F1** queda reservado a los **decorados ampliados**, actualmente aparcados. La consola informa de ello y la imagen permanece original. No hay decorados asignados a ninguna escena; se conservan los PNG como referencia. F1 sigue controlando los marcos exteriores, y F2 el límite de FPS.

**F1** alterna el marco predeterminado y las bandas sin marco. Conserva su zoom y fundido de 250 ms. En ventanas altas gira los paneles laterales hacia arriba y abajo. Esta opción se ve cuando el juego está ampliado; no cambia la habitación.

El port arranca siempre en una **habitación interactiva**, inicialmente **Buhardilla**, con el PC apagado. Pulsa el botón de la torre para encenderlo. BIOS y DOS duran siete segundos completos y no se saltan con Intro. Después carga Core, título e introducción dentro del monitor. El sonido y la lógica permanecen detenidos mientras está apagado o arrancando.

Se puede jugar dentro del monitor pequeño: sus clics llegan al juego y no cambian el zoom. Una vez cargado, pulsa **el botón físico del monitor** para acercar el juego al máximo disponible, conservando su proporción y la sesión. **Ctrl + R** vuelve a la habitación sin reiniciar. **Escape** conserva exclusivamente su función en el juego y sus menús. **Ctrl + H** recorre las cinco habitaciones y muestra su nombre en consola. Cada una elige la imagen más próxima a la proporción de la ventana. La ventana vertical muestra la habitación completa sin deformarla.

**Ctrl + rueda**, **Ctrl + −/+** ajustan el acercamiento en pasos del 20 % entre la habitación completa y el juego máximo; **Ctrl + 0** alcanza el máximo. **+ y − sin Ctrl** conservan su función de volumen. Los filtros se aplican al juego y a su mano original, conservando intacta la ilustración de la habitación.

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

El orden completo y los nombres están en el [catálogo de pantallas](pantallas.md): Core → título → introducción → las40 vistas de desarrollo, desde **La mazmorra** hasta **La plataforma eléctrica**. En los extremos no hay otra vista a la que saltar. Los nombres también se muestran en consola y en el título de la ventana. La ruta normal conserva el inventario y continúa por los puzles de las cuevas hasta salir del cubo y recorrer el campo.

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

Las últimas vistas F5/F6 son **El borde del mundo (EDGE)** y **La habitación del loro sabio (WISE)**:32 vistas de desarrollo. Reproduce la entrada desde el pozo como una sesión nueva de desarrollo. Para conservar los objetos obtenidos hay que llegar por las salidas normales.

F5/F6 incluye también la entrada de Paso del Dragón, Frank, plaza de la fuente, calle de las maravillas y mago. Estos atajos no transfieren inventario. Para probar la compra y el mago en una partida continua, sigue el [recorrido de la población](recorridos/10_paso_del_dragon.txt). El borde del mundo ya permite resolver su recorrido hasta el loro y regresar a la población.

Los marcos se orientan según las bandas: laterales en ventanas anchas y girados90° arriba/abajo en ventanas altas. F1 mantiene su fundido y zoom. El título de ventana numera las vistas como «1: La mazmorra».

Frank y el pozo se muestran sin decorados ampliados. Su posible revisión visual queda para más adelante; la prioridad vuelve a los puzles y caminos del port fiel.

La mano original del ratón aparece únicamente dentro de la imagen jugable. Sobre el marco, la habitación o las bandas, y al salir de la ventana, se recupera la flecha normal. Volver al juego restaura la mano a su escala actual, también con zoom. Los paneles DOS ocultan el cursor nativo solo mientras el puntero permanece dentro del juego, para evitar duplicar su mano incorporada.

## Alimentación de la habitación

El botón de la torre apaga el PC con cierre CRT y piloto rojo; otra pulsación reproduce de nuevo BIOS/DOS y carga Core. Apagar durante BIOS no muestra un fotograma del juego. Los paneles se cierran al comenzar otra sesión; se conservan volumen, silencio, perfil musical, efectos, filtro y pantalla completa. No escribe ni borra partidas. El botón del monitor sirve para acercar la vista; no apaga el PC.

El cursor es la flecha del sistema fuera del juego. Los clics sobre muebles y bandas no llegan a los puzles. Alt+Intro está disponible también con el PC apagado. El contador de tiempo efectivo excluye apagado y arranque. Véase [Habitaciones y catálogo](habitaciones.md).

El catálogo incluye Benn, la tienda de la adivina, la segunda casa abandonada y el probador. Las puertas35/38/39 y sus regresos46/48/49 conservan la partida; el atajo sigue abriendo una sesión independiente.

F5/F6 añade el borde del mundo y la habitación del loro:32 vistas,64 cambios comprobados. La partida normal conserva el inventario al cruzar el acantilado y regresar a la casa abandonada. Los atajos empiezan otra sesión y no entregan guantes ni cuerda. Los marcos, filtros y teclas existentes conservan su funcionamiento.

## Mejoras opcionales y controles fieles

Las flechas ya permiten mover a Brad cuando el guion lo autoriza, usando el movimiento y las colisiones originales. Al perder el foco o minimizar se liberan las direcciones retenidas; al volver hay que pulsarlas de nuevo.

Àlex pide que las mejoras adicionales estén desactivadas por defecto y se agrupen en un futuro menú para activarlas individualmente. Esta unificación sigue pendiente: los valores iniciales actuales de todos los efectos y ayudas aún no se han revisado. Conservar los atajos de esta página hasta integrar el menú. Las correcciones de errores de entrada no requieren una opción para activarlas.

F5/F6 dispone41 vistas hasta SNOWAST4, la llanura del ser azul.82 cambios contrastados;74 mantiene la partida desde el probador.

La vista42 añade la morsa y el amigo del hielo (SNOWAST3).84 cambiosF5/F6 contrastados y salidas80/79 persistentes.

La vista43 añade el agujero del pez y el esquimal (SNOWAST2), con78/77 y86 cambiosF5/F6 contrastados.

F5/F6 incluye44 vistas hasta la cornisa de los amigos (SNOWAST1);88 cambios comprobados. La navegación normal76/75 conserva tablón, perfume y liberaciones.

La vista45 añade el lago helado (ICELAKE), con salto y cenizas mediante el menú original.90 cambiosF5/F6 contrastados.

La vista46 añade la entrada del palacio (ICEPAL) y el puzle de los cuatro carámbanos.92 cambiosF5/F6 contrastados.

La vista47 añade la sala de dados (ICEROOM). Recoger140 cierra su puerta y Lanzar140 la abre con el azar original; hay94 cambiosF5/F6 contrastados.

La vista48 es el corredor de las poleas, con entrada84 y regreso85.96 cambiosF5/F6 contrastados; los atajos no resuelven las puertas de los dados.

La vista49 añade el segundo corredor de hielo, entrada86 y regreso87, con98 cambiosF5/F6 comprobados.

El catálogo llega a50 vistas con el corredor de la ventana, entrada88 y regreso89. F5/F6 permite recorrerlas y se contrasta en100 cambios. Las puertas y los objetos del recorrido normal conservan sus condiciones originales.

Vista51: habitación del gato mecánico.102 cambiosF5/F6 comprobados; en partida normal la puerta91 depende de los dados, no del atajo.

Vista52: habitación de la lupa, con entrada93 y regreso94.104 cambiosF5/F6 contrastados; el recorrido normal conserva el gato149 y la lupa150 al volver a los dados.

Vista53: habitación del arma, entrada95 y regreso96. La escoba baja la repisa en partida normal;106 cambiosF5/F6 contrastados.

F11 y +/− muestran un aviso de letras blancas en la esquina superior izquierda: nombre del filtro o porcentaje de volumen. Entra en120ms,permanece1,2s yse desvanece en500ms. Las pulsaciones seguidas actualizan yprolongan eltexto. No pausa la partida ysu tamaño sigue la resolución física.

Vista54: La habitación del megáfono, entrada97/regreso98; 108 cambiosF5/F6 contrastados.

Vista55: La habitación del cubito de hielo, entrada99/regreso100; 110 cambiosF5/F6 contrastados.

Vista56: La habitación del carámbano, entrada101/regreso102; 112 cambiosF5/F6 contrastados.

Con los filtros F11, la mano original se compone antes de filtrar el fotograma, también en los paneles de partidas e información. Fuera de la imagen jugable se recupera la flecha del escritorio; al volver al modo original se conserva el cursor nativo. La mano filtrada se recorta al área del juego y no deja rastros.
