# Calcetín, monedas y guardia del barco

La comparación continúa la misma partida que recoge el detergente enGHETTO y regresa aDINO. Rodea la pila por450/115 antes de acercarse500/115 al calcetín100; intentar llegar directamente desde abajo choca con los rectángulos originales deBOXDET. Recoge100 y entra enCELESTE por53. El recorrido usa clics y cámara reales, sin alterar posiciones ni inventario.

## Combinación con el montón de monedas

3FDF–3FF6 admite100/107 en ambos órdenes. A952 retira100 e intenta insertar104, pero no retira107: es el montón de monedas del escenario, no una moneda que haya que recoger previamente. Tampoco añade un rechazo si falta100 o no cabe104 en una variante artificial. Programa552-cámara/163, A2C0/A2CF y suma cinco puntos, conservando el tercer guion. Las48 variantes cubren ambos órdenes, inventarios ausentes/duplicados/llenos, cámara y puntuación envolvente.

## Golpe y paso al interior

30DE–30E3/A982–AA69 exigeCELESTE y distancia firmada de hasta40 entreBrad y el guardia. La resta y su negación conservan la envoltura de16 bits del EXE. Retira104, actualiza los descriptores persistentes, conserva los campos diferidos no escritos y programa cinco actores con intervalos distintos. No se sustituyen por una animación aproximada. Suma otros cinco puntos. Veintiocho variantes comprueban distancia, ausencia/presencia104, puntuación y canales ajenos.

La partida combina el calcetín realmente recogido con107 junto a552/163. Se acerca360/170 y elige Golpear con104, dejando768 pasos para las secuencias originales. El guardia acaba en fotograma19 y104 desaparece. La entrada62 se recorre hacia340/140 y cargaHOLD36 con sus bancos originales.

C8B1–C8F8 ajusta la proximidad según los dos tablones en fotograma2. Se contrastan las16 combinaciones de cuadros0/1/2/255, incluida la ausencia de escritura si ninguno es2. El recorrido compara también los primeros pasos y composición del interior.

Resultado: **8.202 casos/pasos y cinco composiciones coincidentes**, con48 variantes de combinación,28 del golpe y16 de proximidad deHOLD.

Prueba: `pnpm comparar:guardia-barco --muestra`. Los menús se auditan junto al oro, antes del golpe y con el guardia apartado. Se comparan DS completo, azar, dibujo y composiciones; las variantes aisladas no se presentan como estados obtenidos jugando.

**Pendiente:** puzle de tablones, recogida del trapo y demás rutas del valle. La llegada aHOLD no equivale a tener su interior terminado. Sigue sin activarseSDL del valle ni aumentar el catálogo de32 vistas. Sonido común, contador, marcos y monitor se conservan; decorados ampliados aparcados.
