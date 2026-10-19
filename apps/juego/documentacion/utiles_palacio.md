# El gato mecánico y la lupa del palacio

Dos habitaciones laterales del primer corredor: escena50 por91/regreso92 y escena51 por93/regreso94. Comparten ICEROOM, pero conservan sus listas y geometrías independientes. La tablaCS17A3 no les asigna rutina propia; se ejecutan los servicios comunes de entidades, guiones, movimiento, sonido y cámara.

El recorrido parte de la partida real posterior a la inspección de la ventana, vuelve a los dados y repite recogida140 y Lanzar140 hasta abrir cada puerta. No escribe banderas de acceso, no sustituye la semilla y no añade objetos al inventario. En esta reproducción,149 necesita dos tiradas y150 tres adicionales; otras partidas pueden necesitar cantidades distintas.

El gato149 se recoge en la habitación izquierda; la lupa150 en la derecha. Ambas recogidas conservan la capacidad y la puntuación original, ocultan el descriptor persistente y programan la animación común615C. Sus alturas son diferentes:149 usa y+alto−16 y150 y+alto−4. Las dos llaman6285 tras la inserción en inventario para comprobar la oferta futura del silbato.

`pnpm comparar:utiles-palacio` contrasta17640 casos/pasos, trece imágenes y1594 selecciones en diez estados auditados, antes y después de las recogidas. Incluye variantes aisladas de capacidad0/9/10, modelo20/normal, iconos de puntuación y desbordamiento. Para cada recogida también contrasta las128 combinaciones de los siete útiles exigidos por6285. Estas variantes no añaden útiles a la partida real.

La vista51 incorpora la habitación del gato mecánico. `pnpm comprobar:gato-palacio` verifica seis imágenes64000 píxeles, guardar/restaurar y102 cambiosF5/F6. Reutiliza las referencias producidas por el comparador nativo, que debe ejecutarse primero.

Quedan pendientes las otras cuatro habitaciones laterales, sus objetos y las acciones del palacio posteriores a la lupa. Los nuevos objetos también tienen usos en zonas posteriores todavía no integradas; no se presenta el capítulo entero como completado.

La vista52 añade la habitación de la lupa. `pnpm comprobar:lupa-palacio` verifica siete imágenes, restauración y104 cambiosF5/F6, incluida la vuelta a los dados con149 y150. Las dos comprobacionesSDL cubren las trece referencias del comparador nativo sin repetir el núcleo sin cambios.

El [recorrido en texto](recorridos/21_dados_gato_y_lupa.txt) explica la solución comprobada hasta regresar con ambos objetos, con las dependencias posteriores señaladas.

La auditoría de rutas deja seis salidas pendientes desde el catálogo actual:112 del loro y95/97/99/101/90 del palacio. No se han activado destinos que todavía necesitan reconstrucción.
