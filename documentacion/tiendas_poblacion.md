# Interiores laterales de la población

Se conectan las puertas de las calles que anteriormente registraban «Ruta pendiente»: ROGUE19 → BENN24 por35, GYPSY20 → SALLY26 por38 y GYPSY20 → DERELICT27 por39. Los regresos46/48/49 conservan inventario, puntuación y descriptores persistentes. DERELICT27 y DERELICT23 comparten ilustración, pero son habitaciones distintas.

## Comportamiento reconstruido

- **Benn:** su aparición y oferta dependen del progreso original. Se conserva la visita temprana, el saludo del actor activo (35B3), las respuestas negativas y la oferta al darle dinero (44AB). No se adelantan las banderas de capítulos posteriores para hacer aparecer al personaje o entregar el vestido. La rutina19DF delimita su proximidad y conserva la secuencia y las partículas del progreso posterior.
- **Adivina:** la entrada38 usa el modelo18 y sus tres secuencias originales; después devuelve el control a Brad normal. Saludar activa35DC. Dar dinero76 al personaje81 reproduce420F, con las ramas de35C9 ya reconstruidas; no se sustituye por texto ni un diálogo inventado.
- **Casa abandonada:** entrada39 y regreso49, sus entidades y sus listas de menú originales. Su salida está al borde inferior del pequeño interior; no se utiliza la salida45 de la otra tienda abandonada.

El motor, las entidades y el sonido continúan siendo comunes. La plataforma solo conecta las habitaciones e incorpora sus nombres al catálogo. Guardar/cargar admite los nuevos índices dentro del alcance reconstruido. F5/F6 añade tres vistas, hasta29, sin trasladar el inventario entre esas sesiones de desarrollo.

## Comprobación y límites

La comprobación ejecutada contrasta **7.261 casos y pasos**, incluidas **681 selecciones** en **siete estados de menú disponibles** y **ocho composiciones**.

`pnpm comparar:tiendas --muestra` continúa las instantáneas contrastadas de las calles, camina por BOXDET, entra, interactúa y regresa. Compara DS completo, estado aleatorio, decisiones y composición frente a instrucciones del EXE. Audita las opciones ofrecidas en las posiciones e hitos del recorrido. Las variantes aisladas de Benn exploran las fases0–4, puntuación0/65535, actor0/1/3, progreso0/4 y canales0/1383/1A2F; no se incorporan a la partida persistente.

`pnpm comprobar:tiendas` regenera las referencias con una permutación inicial de dibujo compartida, como en las comparaciones de población y calles; ejecuta la ruta en SDL y compara ocho capturas de64.000 píxeles, incluida la conversación y pago a la adivina. Añade58 cambios entre las29 vistas F5/F6 y la restauración de sus tres nuevos índices en memoria, sin escribir las partidas del usuario. No prueba el historial gráfico desde el arranque ni la cadencia del monitor.

El probador25 (36), la continuación35 (52), otro transporte del mago31 (72) y la jugabilidad de EDGE30 siguen pendientes. La oferta de vestido tiene contraste aislado; no se presenta como resuelto todo el capítulo del disfraz. La auditoría de menús de estos hitos tampoco acredita todas las posiciones, inventarios o regresos de capítulos posteriores.

Resultados locales: `trabajo/comparacion_tiendas/` y `trabajo/comprobacion_tiendas_sdl/`. La guía de prueba propia está en [11_tiendas_laterales.txt](recorridos/11_tiendas_laterales.txt).
