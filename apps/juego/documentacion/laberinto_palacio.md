# Los desvíos después de la mano

Las vistas60–63 corresponden a escenas59–62: El paso del guardia, La sala de las dos puertas, El desvío central y La sala de las cerillas. Comparten ICEROOM.MAP/SPR e ICEPAL.SFX. CS:17A3 tiene manejador nulo en las cuatro; no se les inventa una rutina de puzle autónoma.

## Puertas y azar

27BA–27F1 conserva las decisiones originales. La puerta105 de la mano escoge105/106 con azar y llega a59/60. La puerta107 de59 solo conduce al guardia63 si se lleva la caja161: sin ella entrega108 y llega a61. Las dos puertas de60 utilizan108, que escoge108/109 con azar para llegar a61/62. Desde61 o62 se vuelve por110 a58. La mano ya eliminada conserva su estado al regresar.

El recorrido comprobado atraviesa veinte puertas sin cambiar semillas ni forzar destinos. Visita las cuatro salas, retorna varias veces a58, recoge las cerillas y termina solicitando107. El guardia63 permanece como siguiente habitación pendiente. Dos huecos con aspecto distinto pueden compartir el mismo comportamiento original.

## Caja y cerilla

La caja161 está en62. 60B2–60C5 comparte615C y llama62C9 para insertar también162. Usa la entidad2C4C, destino x+5/y+alto−2 y los canales de recogida baja. Conserva capacidad, icono, puntuación envolvente, modelo20/normal, retirada del descriptor persistente y el contador posterior a la planificación. Cuando no hay espacio para la cerilla adicional, se conserva el resultado del original; no se fuerza una plaza.

No hay personajes conversables en estas cuatro salas. Se auditan las opciones que ofrecen los objetos y el inventario antes y después de cambios, además de las respuestas negativas compartidas de los otros útiles.

## Comprobaciones

`pnpm comparar:laberinto-palacio --muestra` contrasta3073 casos/pasos, incluidas36 variantes de recogida y128 combinaciones de inventario; DS completo, azar, ordenación y trece imágenes, con ocho estados auditados. Parte de la puerta105 alcanzada tras liberarse de la mano. El recorrido conserva todos los objetos y banderas originales.

`pnpm comprobar:laberinto-palacio` verifica las trece imágenes SDL, guardar/restaurar y126 cambiosF5/F6 entre63 vistas. Incluye las dos puertas visibles de60 en el catálogo; el recorrido gráfico usa la izquierda y el detector original común a ambas conserva sus rectángulos separados. La equivalencia de los dos caminos y las ramas105/106,108/109 se basa en el detector contrastado y las tablas originales. El destino107 y los capítulos posteriores siguen pendientes.
