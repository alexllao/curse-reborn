# Cueva del valle: ayuda y detergente

La cueva GHETTO37 se alcanza por la salida 64 de DINO32 después de liberar el paso con la entrega de la grabación. La comparación continúa la misma partida del regreso de la nube: POST→57→DINO→64→GHETTO, sin cambiar inventarios, posiciones ni banderas.

La tabla CS:17A3 contiene cero para 37: no se añade un comportamiento autónomo ajeno al original. Se mantienen los bancos de escena, personaje ySTD independientes y los servicios comunes de secuencias, azar, dibujo y sonido.

## Petición de ayuda

337D–34D6 solo acepta Ayuda cuando 2DF9 tiene tipo 3. Programa el destino 159/176, secuencias 262E/263F, la bandera persistente 4794 y la retirada de trece ofertas de rocas. Las referencias de los trece actores y sus canales conservan las diferencias del original: no se reinician fases o intervalos que el EXE no escribe.

Después de 886B, el retorno AX determina los retardos escalonados de las rocas y la botella. Esto difiere de la recogida de la bolsa de la nube, que usa 359A. La comparación aislada cubre 48 variantes de estado del actor, posición, puntuación y fases previas. En la partida real se selecciona Hablar→Ayuda y se esperan 512 pasos hasta que el detergente queda en tipo 1.

## Recogida y regreso

El clic 171/145 aproxima a Brad por el terreno original hasta que se ofrece 120 en Recoger. 5E57–5E65 usa 2DF9, desplazamientos +5/+1 y la familia 61A7, con 96CF/96D8 para el modelo normal. No confundirla con 615C: emplear esa familia cambia la animación de recogida. Dieciocho variantes aisladas cubren capacidad 0/9/10, iconos de puntos y puntuación envolvente.

La partida recoge 120, espera 192 pasos y vuelve por 65 siguiendo 150/199. DINO conserva la botella y la bolsa. Los menús se auditan antes de Ayuda, con la botella expuesta y después de regresar aDINO. Las respuestas comunes de 120 fuera de su puzle mantienen el rechazo original: AFBE pertenece a la escena 39.

Resultado: **4.489 casos/pasos, 691 selecciones, tres auditorías de menú y cuatro composiciones**; incluye 48 variantes aisladas de ayuda y 18 de recogida, comparando DS completo y azar.

Prueba: `pnpm comparar:cueva-valle --muestra`. Si falta la instantánea de la nube, se prepara con su comparación previa. Los recursos y resultados permanecen en las carpetas locales ignoradas.

**Límite:** no habilita SDL del valle ni acredita los restantes puzles de DINO/CELESTE/HOLD o ELECTRO. Se mantienen 32 vistas de desarrollo. Decorados ampliados aparcados; sonido, contador, marcos y monitor conservados.

## Integración SDL

`pnpm comprobar:cueva-valle` recorre desde el comienzo la continuidad del valle y entra64 después de volver deCLOUD. Compara cuatro imágenes originales: entrada, derrumbe, detergente y regreso65. Guardar/restaurar en los cuatro estados y76 cambios deF5/F6 entre38 vistas correctos. Se conserva121 durante la recogida120; no se preparan posiciones, inventario ni actores. Se repite el comparador nativo4489 casos/pasos, utilizando las referencias comunes del valle previamente contrastadas.

## Revisión ampliada de alternativas del21 de octubre

Regenerado `pnpm comparar:cueva-valle --muestra`:4.489 casos/pasos,691 selecciones y cuatro composiciones coincidentes. Las cuatro DS anteriores mantienen su SHA256 y sus hitos conservan ahora contexto5800 real.

Las consecuencias de cueva_entrada y cueva_detergente suman92 observaciones,11.776 pasos,52 ramas CLC y402 selecciones coincidentes en DS/azar/fase; todas llegan a reposo con menú en128 pasos. La entrada incluye Hablar→Ayuda (casillas0-based4/1); el generador mantiene además la espera512 del derrumbe y el recorrido hasta obtener120. No se agotan la fase de botella expuesta antes de recoger ni otros regresos con inventarios posteriores.

Comandos, informes separados y cobertura actual en [auditoría vigente](auditoria_pantallas.md). Los pendientes de catálogo indicados en las tandas iniciales son históricos. No hay cambios del motor ni capturas SDL nuevas en esta revisión.
