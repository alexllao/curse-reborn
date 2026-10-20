# Grabación en el valle

## Cinta e inserción reales; grabación aislada

`pnpm comparar:grabacion-valle --muestra` retoma CELESTE y recoge la cinta105 por sus listas reales:330/170,90/170,74/145, Recoger105 y128 pasos. Vuelve por61 y rodea el equipo por debajo antes de insertar105 con113. Aproximarse desde arriba hasta760/172 deja a Brad bloqueado antes del borde útil también en el EXE; la ruta comprobada pasa por704/185,760/185 y760/171. Tras256 pasos, la cinta desaparece del inventario y2C6D queda en tipo1. La recogida y la inserción tienen imágenes y estados comparados con el original. Resultado:6.521 casos/pasos,1.817 selecciones, dos estados auditados y dos composiciones coincidentes.

AB60 conserva las comparaciones firmadas del equipo,4628=1/4629=47,462C+7, destino752-cámara/172, guionesA362/A370, actor diferido209/A34F/A355 y puntos+5. Se comprueban320 contratos aislados de ambos órdenes, bordes del equipo, cinta ausente/presente, cámara y puntuación envolvente;108 variantes adicionales cubren la recogida105. Se conservaA2 sin escribirlo: el tercer canal se deposita enA4.

ABD5 se contrasta con1.152 variantes aisladas de actor, posiciones, cámaras, puntuación y capacidad. Exige2C6D=1 y mantiene las dos ramas del EXE, incluida la aceptación a la derecha del equipo que no repite la condicióny>=123. Retira108 e intenta insertar106; si el inventario artificial está lleno y no contiene108, el EXE continúa aunque no pueda insertar106. No introducir un rechazo que el original no tenga. Programa23B/A359/A35F, limpia4628 y suma5 puntos, conservandoA2.

Para reproducir esta prueba desde una copia sin resultados locales, preparar antes `comparar:ramas-valle --muestra` y `comparar:correo --muestra`; este último prepara DINO si falta su instantánea. La prueba avisa explícitamente si falta alguno de esos estados.

**La grabación completa permanece pendiente.** El recorrido real de esta prueba solo recoge y coloca105. Los contratos deABD5 parten de otra instantánea dePOST con inventarios preparados explícitamente: no se concede108 a la partida real ni se afirma haber grabado106. Falta unir la recogida del mando tras el estornudo con su uso y las entregas105/106→98. Tampoco se habilita SDL del valle ni cambia su catálogo de32 vistas. Los servicios de sonido comunes siguen intactos; decorados ampliados aparcados, contador/marcos/monitor conservados.

## Grabación persistente con el mando realmente recogido

Esta ampliación resuelve la unión que quedaba pendiente en la prueba anterior de grabación. `pnpm comparar:grabacion-completa --muestra` parte de la instantánea real de DINO después del estornudo, con sello y bolígrafo. No usa los inventarios preparados de las variantes aisladas.

Sale por59 mediante400/185 y550/189 y cargaREMOTE desde el origen32 con el modelo22. Tras256 pasos conserva2C4C=0: A81A dejó45E7=0 y el protector no reaparece al cargar. El mando108 se recoge realmente junto a110/165. Sigue56 haciaCELESTE, recoge105 de la misma forma ya contrastada, vuelve por61 aPOST y coloca la cinta en113 rodeando el equipo por abajo.

Para empujar el mando del inventario hay que abrir Manipular→Empujar, seleccionar la casilla0 que abre el inventario y después108. No confundir la lista de objetos cercanos con la del inventario. El recorrido espera768 pasos originales y obtiene106 mientras desaparecen108 y105. Cada selección y paso compara DS completo y azar; las composiciones abarcan entrada59, mando recogido, cinta y mando juntos, cinta insertada, grabación terminada y las tres fases del correo.

**La grabación106 ya tiene una ruta persistente contrastada en el núcleo.** La entrega a98, la recuperación del pase y los restantes puzles del valle siguen pendientes. Todavía no se activa SDL ni se incrementa el catálogo de32 vistas. La prueba continúa con esa misma grabación106: recoge111, combina el sello109 realmente conservado y envía112 al buzón114. Se comprueba que106 permanece y que105/108/109/111/112 han desaparecido. Resultado:13.171 casos/pasos y ocho composiciones coincidentes; la entrega a98 y la obtención del pase todavía no están unidas ni comprobadas. Los decorados ampliados siguen aparcados; sonido común, contador, marcos y monitor permanecen.

El [recorrido en texto](recorridos/13_valle_hasta_grabacion_y_correo.txt) resume las acciones de esta misma partida hasta dejar la carta en el buzón con la grabación en el inventario.

## Revisión ampliada de consecuencias

La regeneración del recorrido completo vuelve a coincidir en13.171 casos/pasos y ocho composiciones, conservando contexto5800 en sus ocho hitos y las DS previas sin cambios. La revisión adicional de mando_recogido, cinta_insertada, grabacion_y_carta y grabacion_y_correo_enviado contrasta225 observaciones y28.800 pasos; incluye100 ramas CLC tras cierre lógico del menú. Todas coinciden con el EXE y llegan a reposo con menú.

Quedan representados Empujar108 con la cinta insertada y la combinación de sello/carta en ambos órdenes, además de respuestas negativas y otros verbos ofrecidos en esos estados. La espera larga de768 pasos de la grabación y la secuencia real del envío siguen respaldadas por el recorrido completo; no se deduce la duración audiovisual desde las observaciones mínimas. Comandos, informes y límites en la [auditoría vigente](auditoria_pantallas.md). Esta ampliación no agota todas las fases ni la entrega posterior a98; las limitaciones históricas de integración SDL anteriores no representan el catálogo global actual.
