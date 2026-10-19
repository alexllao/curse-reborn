# Las cuatro habitaciones laterales restantes

Las escenas52,53,54 y55 comparten ICEROOM, con entradas95/97 desde el segundo corredor y99/101 desde el tercero; regresos96/98/100/102. Mantienen las condiciones de puertas elegidas porB740, sin fijar tiradas, inventario ni banderas.

`pnpm comparar:laterales-palacio` contrasta29136 casos/pasos,31 composiciones,5002 selecciones en18 estados auditados. Obtiene151,152,153 y154 en una única partida real que ya lleva el gato149 y la lupa150. Reutiliza el azar compartido y regresa a los dados con todos los útiles.

En52 y55 el objeto empieza alto, enY89. Atacar con la escoba141 solo funciona con actividad1 y esa altura. La animación baja el objeto y modifica también su posición persistente. Se comparan24 variantes por sala con actividades, alturas fronterizas, canales centinela y puntuación envolvente. Tras la primera caída no se puede repetir para sumar puntos.

Las recogidas conservan su altura específica:151/152 usan y+alto−4,153 −5 y154 −2.151 no llama6285;152/153/154 comprueban los siete útiles que habilitan el silbato. Cada objeto incluye36 variantes de capacidad/modelo/icono/puntuación y128 máscaras de inventario. Las variantes se separan del recorrido real.

La vista53 añade la habitación del arma. `pnpm comprobar:arma-palacio` compara ocho imágenes64000 píxeles: tirada, puerta, entrada, caída con escoba, proximidad, recogida, regreso y vuelta a los dados. Guardar/restaurar y106 cambiosF5/F6 contrastados. Las otras tres habitaciones ya tienen recorrido nativo; se habilitan enSDL por separado tras sus comprobaciones.

Los útiles enlazan con las poleas y la ventana de los corredores. La resolución se comprueba desde la partida con todos los objetos; los resultados de variantes aisladas anteriores no sustituyen ese recorrido.

`pnpm comparar:resolucion-palacio` ya contrasta9608 casos/pasos y diez composiciones: insertar151 en142, recoger144/145, oferta147 por6285, recogida, combinación147/152, apertura con148 y salto146 hasta solicitar90. Las capturasSDL de esta resolución se incorporan al habilitar las tres vistas laterales siguientes.

Vista54: La habitación del megáfono. `pnpm comprobar:megafono-palacio` verifica7 imágenes, restauración y108 cambiosF5/F6, reutilizando el recorrido nativo de29136 casos/pasos.

Vista55: La habitación del cubito de hielo. `pnpm comprobar:hielo-palacio` verifica7 imágenes, restauración y110 cambiosF5/F6, reutilizando el recorrido nativo de29136 casos/pasos.
