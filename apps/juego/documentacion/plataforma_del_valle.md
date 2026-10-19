# Plataforma del valle y regreso con el ventilador

La comparación continúa desde el regreso del barco con119. Vuelve aPOST por61 y recoge la bandeja115 junto a94/165. Su recogida es5DCB–5DD9/615C: entidad2C8E, desplazamientos+5/-2, capacidad e iconos originales. No se añade al inventario para abrir la siguiente zona.

Con115/119/120/121 reales, C5E0 abre la puerta deDINO mediante sus canales originales. El recorrido llega por57 y rodea el esqueleto por250/170,340/170 y320/140 antes de295/130: salida68 aELECTRO39. Acercarse directamente desde la izquierda no basta; también se atasca en el original.

## Rutina de la plataforma

CAE1–CBCF mantiene el límite de la barrera según su altura firmada y los bordes inclinados que provocan la caída. Se conserva la diferencia de16 bits antes deSAR, el redondeo negativo deSAR1 y la división firmada entre49 para cada lado. La caída bloquea el control y programaB2E4/0E26 o0E36/0E46. El ventilador redefine la zona próxima de la derecha. Los estados artificiales cuyoIDIV original desbordaría16 bits se rechazan por la API; no se introducen condiciones de puzle.

## Limpieza y cruce

- Lanzar120: AFBE retira el detergente y prepara la pared, destino39/155 yA438/A441.
- Empujar119 desde el inventario: B000 exige2CD0=1, retira el trapo y descubre122 conA447/A450.
- Empujar122: B065 programaA456/A467 y la barrera. ConservaA0/A2 del primer actor diferido: solo escribeA4; el segundo conservaA2/A4.
- Lanzar121: B0A9 exige que2C71 sea distinto de cero y prepara las canicas.
- Lanzar115: B101 exige2CF1=1 y enlaza la animación automática de cruce, sin teletransportar aBrad.
- Recoger124: 5E9A–5EA8 usa2D33/+5/+1 y61A7. El ventilador se conserva al regresar.
- Desbloquear123 con3: B16D admite ambos órdenes, retira clip3 y pase99, activa el bit2 de35C9 y los cambios persistentes del capítulo. La puerta solicita69 automáticamente; no se añade otro clic después de la petición.

Cada una de las seis acciones de puzle suma cinco puntos. No se exige artificialmente que el objeto esté presente en los contratos aislados cuando el EXE ignora el resultado de retirarlo. Los campos diferidos no escritos se conservan. El recorrido compara cada paso y selección con el original, además de estados de menú antes de limpiar, con el botón descubierto y junto al ventilador.

## Marcador F6 y lectura acotada

La salida69 usaF6 en el canal de un actor. 85D0–85F6 escribe también ese valor como imagen y lee sus dimensiones antes de0A5C, aunque esa imagen nunca se presenta. EnELECTRO el párrafo de esa supuesta imagen cae fuera del archivoSPR. El EXE lee RAM por una dirección real de16 bits; en la referenciaDOS empleada devuelve ancho/alto cero.

La API portable no lee RAM ajena al recurso: si falta esa cabecera exclusivamente para el marcadorF6, normaliza ancho/alto a cero y continúa los demás canales hasta cargar la nueva escena. Los fotogramas normales fuera del banco siguen siendo un error. Esto coincide con DS en la referencia comprobada, pero no pretende emular bytes arbitrarios de RAM de otra asignaciónDOS. Las comparaciones de secuencias y servicios comunes se repiten tras este ajuste.

Resultado: **17.039 casos/pasos,1.026 selecciones, tres estados auditados y doce composiciones coincidentes**. Se añaden720 contratos deCAE1,180 de las cinco acciones iniciales,36 del desbloqueo y18 por cada recogida115/124.

Prueba: `pnpm comparar:plataforma-valle --muestra`. Los recursos y estados reales están en las carpetas ignoradas de trabajo. El final vuelve aCOUNTRY2/16 con124 y el bit2 activo; los menús de esa fase posterior del campo todavía requieren su propia revisión.

**Pendiente:** menús posteriores del campo, otros estados argumentales y siguientes capítulos. ELECTRO ya está integrado enSDL. Los decorados ampliados siguen aparcados; se conservan sonido común, contador, marcos y monitor.

## Caídas persistentes e integración SDL

Se añaden dos ramas sobre copias de estados reales, sin modificar sus posiciones ni inventarios. Con la barrera abierta, clic100/160,192 pasos y128 de espera provoca la caída izquierda. Después de cruzar, clic180/160 y los mismos tiempos provoca la derecha. Ambas pasan por modeloFALL6 y recuperanSTD255/control dentro deELECTRO, sin consumir objetos ni puntos. B2E4/0E26/0E36/0E46 no solicitan otra escena; no se inventa una recarga68. Las variantes de rutinaCAE1 se mantienen además del recorrido persistente de las caídas.

`pnpm comprobar:plataforma-valle` prolonga enSDL toda la partida desde el inicio y contrasta doce imágenes de64000 píxeles: bandeja, puertaDINO, entradaELECTRO, detergente, botón, barrera, caída izquierda, canicas, cruce, caída derecha, ventilador y regreso al campo. Comprueba guardar/restaurar los doce hitos y80 cambiosF5/F6 entre40 vistas. El regreso conserva124, consume3/99 y activa la siguiente fase, sin declarar completos sus menús posteriores.
