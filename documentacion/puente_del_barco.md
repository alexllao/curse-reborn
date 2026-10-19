# Puente y trapo del barco

`comparar:puente-barco` continúa la misma partida del calcetín y el guardia desde la entrada real62 aHOLD36. No añade tablones, objetos ni posiciones preparadas al recorrido.

## Recogida de los tablones

5DDC–5E43 comparte el preámbulo563A de capacidad e iconos de puntos. Después de intentar insertar117 o118, comprueba que no se lleve el otro. Si ya está en el inventario, retira el identificador de la recogida y conserva el rechazo original; no permite llevar ambos en la partida normal.

Cada tablón conserva su descriptor y recibe1A34, fase0 e intervalo2, con contadorAX+8 tras886B. Los destinos son entidadx+35/y+2 y los canales97EE/97FA/9801. Brad cambia al modeloFE con los sprites de la escena: no mantiene las dimensiones de caminar sin carga. De ahí que los clics para colocar el puente sean distintos de los que se usarían con su tamaño normal.

## Colocación

AC8F–AFBB admite Empujar y Lanzar para117/118. Si no se lleva el tablón señalado pero sí el otro, usa el que se lleva. Si no hay ninguno, rechaza. Retira el objeto antes de decidir su emplazamiento.

Con el otro tablón aún sin colocar, acepta x119–139/y90–100 del registro deBrad y dispone el tramo horizontal. Con el primero ya colocado, acepta x146–166/y90–100 y dispone el tramo vertical. Se conservan las comparaciones firmadas, los cambios de flags5337/5339, los descriptores46E8/46F1 y las diferencias entre campos escritos y preservados. No añade puntuación.

Fuera de esas ventanas, el tablón cae al suelo conA422/A42A/A42F yA434; no se fuerza una colocación correcta ni se borra el objeto definitivamente. La comparación aislada cubre ambos identificadores/verbos, falta de tablón, sustitución por el otro, inventarios artificiales con ambos y posiciones de aceptación/rechazo.

## Recorrido persistente

Se recoge117 cerca de70/173. Al llevarlo, el recorrido alcanza136/166 pasando por170/180 y lo empuja desde el inventario: queda en fotograma9. Se recoge118 cerca de91/171 y se aproxima por175/166 antes de163/166 para colocarlo en fotograma10. Los dos cambios coinciden con las animaciones originales.

El puente permite cruzar por170/165,170/150,170/120,230/105 y281/102. Recoger119 usa5E46–5E54/615C, retira su oferta persistente, programa la aproximación normal y conserva el resto del inventario. Se esperan192 pasos y se vuelve por63, pasando170/120,170/175 y280/199. El trapo se conserva enCELESTE; los tablones ya colocados quedan fuera del inventario.

Las listas se auditan junto a los tablones, después del cruce y de regreso al exterior. Se comparan DS completo, azar, orden de dibujo y composiciones con el EXE, además de variantes aisladas. Las auditorías cubren esos estados concretos, no todos los posibles.

Resultado: **11.453 casos/pasos y cinco composiciones coincidentes**.

**Pendiente:** bandeja115, siguientes rutas y puzles del valle, ELECTRO39 e integraciónSDL. No cambia el catálogo de32 vistas. Se conservan sonido común, contador, marcos y monitor; decorados ampliados aparcados.
