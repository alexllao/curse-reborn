# Corredores del palacio de hielo

Las escenas DOS47,48 y49 comparten ICECORR, pero tienen listas, objetos y salidas distintas. Sus entradas84/86/88 y regresos85/87/89 se reconstruyen desde la partida real posterior a los dados, sin añadir inventario ni elegir a mano las marcas de las puertas.

`pnpm comparar:corredores-palacio` contrasta5791 casos/pasos, trece composiciones, diez estados auditados y1377 selecciones. Compara el segmentoDS, el azar compartido, las órdenes y la imagen original. Las tres escenas tienen despacho de rutina propia nulo; se mantienen los servicios comunes de entidades, movimiento, sonido y guiones.

El recorrido real inspecciona el soporte143 y la ventana146, vuelve a los dados y atraviesa los tres corredores con sus regresos. El salto por la ventana se rechaza mientras esté cerrada;147 continúa oculto. No se considera resuelto el puzle posterior.

Los manejadoresB968/B9F4/BA1F/BA76 y las recogidas144/145/147 se contrastan también en variantes aisladas: inserción151+142 en ambos órdenes, combinación147+152, apertura con148, salto146, capacidad/iconos/puntuación y256 combinaciones para6285. Esta última solo ofrece147 cuando están153,154,152,149,150,144,145 y retira la escoba141. Estas variantes no son una partida completada: faltan las habitaciones que proporcionan esos útiles y su recorrido real.

Las salidas laterales91/93/95/97/99/101 conservan las condiciones originales de las puertas elegidas por los dados. Sus habitaciones, la continuación90 y la solución final de la ventana siguen pendientes de integrar. Un corredor accesible no equivale a todo el palacio jugable.

La vista48 añade el primer corredor y su regreso a los dados. `pnpm comprobar:poleas-palacio` compara cinco capturas de64000 píxeles, restauración y96 cambiosF5/F6 entre48 vistas. Las referencias nativas se generan antes con el comparador y se reutilizan al comprobarSDL.

La vista49 habilita el segundo corredor. `pnpm comprobar:pasillo-hielo` compara dos imágenes, guardar/restaurar y98 cambiosF5/F6. La comprobación conserva las marcas aleatorias del mismo recorrido y vuelve por87 al primer corredor.

La vista50 añade el corredor de la ventana. `pnpm comprobar:ventana-palacio` compara las seis imágenes restantes del recorrido, guardar/restaurar y100 cambiosF5/F6 entre50 vistas. Entre las tres comprobacionesSDL se cubren las trece referencias nativas. No se abre la ventana ni se ofrece147 antes de cumplir el contrato original.

`pnpm auditar:rutas` revisa78 tablas y detecta ocho salidas pendientes desde las vistas actuales:112 del loro y las siete del palacio (91,93,95,97,90,99,101). Las seis laterales conducen a50..55;90 continúa a56. Se conservan como pendientes explícitos, sin desviar al jugador a otra habitación.
