# Regreso a Benn después del valle

La continuación parte de la misma partida que vuelve de ELECTRO por69 aCOUNTRY2/16. Conserva70,76,101 y124 y el progreso real del valle. No se asigna el vestido79 ni se preparan sus puertas para probarlo.

## Recorrido contrastado

El camino vuelve por26 aDRAGON,43 a la fuente,29 a la calle de los disfraces y35 aBENN. Hay que acercarse al vendedor por128/145 antes de dar76 a78; desde la entrada todavía no aparece como destinatario cercano. El pago44AB conserva el dinero y ofrece79; después de256 pasos se recoge el vestido con5CAF y se espera192. La salida36 lleva al probadorCHANGE25.

Vestir79 ejecuta430F, el movimiento original y512 pasos de animación: aparece el modeloCHANGE19 y vuelveSTD255, consume79 y conserva124. El byte externoCS:1BA2 se compara después de cada paso, además deDS completo y las cuatro palabras del generador. La puerta solicita74 al caminar por160/160 y120/142. La prueba nativa termina antes de cargarSNOWAST4/43.

`pnpm comparar:regreso-benn --muestra`: **10.109 casos/pasos,1.064 selecciones, ocho estados de menú y siete composiciones coincidentes**. La entrada74 queda como petición comprobada, no como habitación reconstruida.

## Menús del inventario que vuelve del valle

El bolígrafo101 y ventilador124 permanecen en el campo. Sus respuestas neutras se rechazaban como pendientes en el port. Se conserva ahora88A0 para esas acciones enCOUNTRY2. El ataque124 llega a49DA, que exige escena72 antes de probar35CA: el efecto exitoso de72 sigue pendiente; este bloque solo conserva su rechazo fuera de esa escena. Examinar mantiene el contrato sin cambios de3B4F. La auditoría del recorrido revisa las opciones ofrecidas con ese inventario real, sin incorporar sus variantes a la partida.

El adaptador de pruebas avanzar_probador_bancos reúne la referencia5800 y la faseCS:1BA2 explícita. Mantiene intactas las ABI anteriores; no usa una variable global entre sesiones ni cambia el formatoCURSE.SAV.

## SDL y límites

`pnpm comprobar:regreso-benn` prolonga la partidaSDL completa y compara **seis imágenes de64000 píxeles**: entrada aBenn, aproximación, pago, vestido recogido, entrada al probador y final de vestir. Guardar/restaurar en esos hitos y80 cambiosF5/F6 entre40 vistas coinciden. La séptima imagen nativa, petición74, se conserva como referencia anterior al cargador; SDL todavía detiene ese enlace porque la sala43 no está integrada.

No se aumenta el número de vistas por completar otra fase deBenn yCHANGE. El siguiente trabajo es investigar y reconstruirSNOWAST4/43 y sus puzles, no habilitarla por cargar su fondo. Se conservan sonido común, tiempo efectivo, marcos y monitor; los decorados ampliados siguen aparcados.
