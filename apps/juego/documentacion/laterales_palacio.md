# Las cuatro habitaciones laterales restantes

Las escenas52,53,54 y55 comparten ICEROOM, con entradas95/97 desde el segundo corredor y99/101 desde el tercero; regresos96/98/100/102. Mantienen las condiciones de puertas elegidas porB740, sin fijar tiradas, inventario ni banderas.

`pnpm comparar:laterales-palacio` contrasta29136 casos/pasos,31 composiciones,5002 selecciones en18 estados auditados. Obtiene151,152,153 y154 en una única partida real que ya lleva el gato149 y la lupa150. Reutiliza el azar compartido y regresa a los dados con todos los útiles.

En52 y55 el objeto empieza alto, enY89. Atacar con la escoba141 solo funciona con actividad1 y esa altura. La animación baja el objeto y modifica también su posición persistente. Se comparan24 variantes por sala con actividades, alturas fronterizas, canales centinela y puntuación envolvente. Tras la primera caída no se puede repetir para sumar puntos.

Las recogidas conservan su altura específica:151/152 usan y+alto−4,153 −5 y154 −2.151 no llama6285;152/153/154 comprueban los siete útiles que habilitan el silbato. Cada objeto incluye36 variantes de capacidad/modelo/icono/puntuación y128 máscaras de inventario. Las variantes se separan del recorrido real.

La vista53 añade la habitación del arma. `pnpm comprobar:arma-palacio` compara ocho imágenes64000 píxeles: tirada, puerta, entrada, caída con escoba, proximidad, recogida, regreso y vuelta a los dados. Guardar/restaurar y106 cambiosF5/F6 contrastados. Las otras tres habitaciones se habilitan por separado, con los resultados que se detallan a continuación.

Los útiles enlazan con las poleas y la ventana de los corredores. La resolución se comprueba desde la partida con todos los objetos; los resultados de variantes aisladas anteriores no sustituyen ese recorrido.

`pnpm comparar:resolucion-palacio` ya contrasta9608 casos/pasos y diez composiciones: insertar151 en142, recoger144/145, oferta147 por6285, recogida, combinación147/152, apertura con148 y salto146 hasta solicitar90. Las capturasSDL de esta resolución se incorporan al habilitar las tres vistas laterales siguientes.

Vista54: La habitación del megáfono. `pnpm comprobar:megafono-palacio` verifica7 imágenes, restauración y108 cambiosF5/F6, reutilizando el recorrido nativo de29136 casos/pasos.

Vista55: La habitación del cubito de hielo. `pnpm comprobar:hielo-palacio` verifica7 imágenes, restauración y110 cambiosF5/F6, reutilizando el recorrido nativo de29136 casos/pasos.

Vista56: La habitación del carámbano. `pnpm comprobar:carambano-palacio` verifica9 imágenes, restauración y112 cambiosF5/F6, reutilizando el recorrido nativo de29136 casos/pasos.

La resolución también se comprueba con `pnpm comprobar:resolucion-palacio`: diez imágenesSDL, guardar/restaurar y112 cambiosF5/F6. Las poleas y la ventana se resuelven con los objetos obtenidos durante el recorrido completo. El [recorrido en texto](recorridos/22_laterales_y_ventana_del_palacio.txt) explica los pasos y los límites.

El salto146 se contrasta en el núcleo hasta solicitar90. Su destino es la sala del láser56, todavía sin integrar;SDL se comprueba hasta la ventana abierta. Quedan dos salidas pendientes desde las56 vistas,112 del loro y90 de la ventana. Esto no equivale al número total de escenas o puzles que faltan del juego.

## Revisión ampliada de consecuencias del21 de octubre

Regenerado `pnpm comparar:laterales-palacio --muestra`:29.136 casos/pasos,5.002 selecciones y31 composiciones coincidentes. Las31 DS anteriores mantienen su SHA256 y guardan contexto5800/fase0 real. Las nuevas tiradas para52/53/54/55 son0/2/0/2; cero indica una puerta ya abierta por el azar previo. No se añaden útiles ni se fijan marcas.

Las alternativas de arma_entrada52, megafono_ante_objeto53, hielo_ante_objeto54 y carambano_entrada55 suman246 observaciones,31.488 pasos,108 ramas CLC y891 selecciones coincidentes en DS/azar/fase; todas terminan en reposo con menú en128 pasos.52/55 parten antes de bajar el objeto alto; el recorrido positivo conserva512 pasos de escoba y192 de recogida. Esta tanda no sustituye los estados posteriores al descenso ni todos los usos de los objetos ya recogidos.

Comandos, informes separados y cobertura actual en [auditoría vigente](auditoria_pantallas.md). Quedan otros inventarios, regresos y consecuencias alternativas de poleas y ventana. Las carencias de integración de las tandas iniciales son históricas; esta ampliación no modifica el motor ni añade capturas SDL.

## Consecuencias de la resolución con útiles reales

Regenerado `pnpm comparar:resolucion-palacio --muestra`:9.608 casos/pasos,3.200 selecciones y diez composiciones coincidentes. Las11 DS previas mantienen su SHA256 y guardan contexto5800/fase0 real; se conserva ahora expresamente ventana_solicitud90 tras verificar el retorno, sin otra imagen ni cargar56.

Las alternativas de poleas_con_arma47, poleas_cristal47, ventana_silbato49 y ventana_abierta49 suman361 observaciones,46.103 pasos,148 ramas CLC y1.384 selecciones coincidentes en DS/azar/fase.360 terminan en reposo con menú; saltar desde ventana_abierta solicita90 al paso23 (casillas0-based6/0). Los útiles proceden del recorrido real de los seis laterales, sin inventario o banderas preparados. La prueba termina en la petición, sin presentar su destino como comprobado en esta tanda.

La [auditoría vigente](auditoria_pantallas.md) contiene comandos, informes separados y límites. El indicador sigue55/73 escenas con muestras: se profundiza en47/49 con inventarios posteriores. No hay cambios del motor ni nuevas capturas SDL. Quedan otros puntos intermedios, órdenes, regresos y capítulos56+; no se declara fidelidad1:1 cerrada.
