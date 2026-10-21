# Los desvíos después de la mano

Las vistas60–63 corresponden a escenas59–62: El paso del guardia, La sala de las dos puertas, El desvío central y La sala de las cerillas. Comparten ICEROOM.MAP/SPR e ICEPAL.SFX. CS:17A3 tiene manejador nulo en las cuatro; no se les inventa una rutina de puzle autónoma.

## Puertas y azar

27BA–27F1 conserva las decisiones originales. La puerta105 de la mano escoge105/106 con azar y llega a59/60. La puerta107 de59 solo conduce al guardia63 si se lleva la caja161: sin ella entrega108 y llega a61. Las dos puertas de60 utilizan108, que escoge108/109 con azar para llegar a61/62. Desde61 o62 se vuelve por110 a58. La mano ya eliminada conserva su estado al regresar.

El recorrido comprobado atraviesa veinte puertas sin cambiar semillas ni forzar destinos. Visita las cuatro salas, retorna varias veces a58, recoge las cerillas y termina solicitando107. El guardia63 está integrado y comprobado en su bloque posterior. Dos huecos con aspecto distinto pueden compartir el mismo comportamiento original.

La revisión adicional parte de la primera visita real a60 y atraviesa por separado los accesos en115/140 y204/140. Cada rama conserva la misma semilla de partida, entra a61/62 y vuelve por110 a58; comprueba que el inventario sigue intacto. No se limita al detector de los rectángulos.

## Caja y cerilla

La caja161 está en62. 60B2–60C5 comparte615C y llama62C9 para insertar también162. Usa la entidad2C4C, destino x+5/y+alto−2 y los canales de recogida baja. Conserva capacidad, icono, puntuación envolvente, modelo20/normal, retirada del descriptor persistente y el contador posterior a la planificación. Cuando no hay espacio para la cerilla adicional, se conserva el resultado del original; no se fuerza una plaza.

No hay personajes conversables en estas cuatro salas. Se auditan las opciones que ofrecen los objetos y el inventario antes y después de cambios, además de las respuestas negativas compartidas de los otros útiles.

## Comprobaciones

`pnpm comparar:laberinto-palacio --muestra` contrasta3.381 casos/pasos, incluidas36 variantes de recogida y128 combinaciones de inventario; DS completo, azar, ordenación y17 imágenes, con ocho estados auditados. Incluye las dos ramas adicionales de60 hasta regresar a58. Parte de la puerta105 alcanzada tras liberarse de la mano. El recorrido conserva todos los objetos y banderas originales.

`pnpm comprobar:laberinto-palacio` conserva la tanda inicial de trece imágenes SDL, guardar/restaurar y126 cambiosF5/F6 entre63 vistas históricas. La revisión adicional `pnpm comprobar:continuidad --solo-laberinto` recorre cada puerta y su regreso con y sin recargas:270 recargas por rama y resultado idéntico. Las dos imágenes de regreso SDL se han comparado también con las referencias nativas:64.000 píxeles coincidentes en ambas. Las cuatro capturas nuevas del núcleo incluyen destino y regreso; la tanda SDL adicional solo acredita las dos de regreso.

Revisión ampliada21/10: referencias reales regeneradas con contexto de banco y fase, negativas y cierresCLC, más regreso posterior aBenn24 con extintor. Cifras, reproducción y límites en la sección «Mano, laberinto, guardia y regreso con el extintor» de `auditoria_pantallas.md`. No cambia el motor ni añade vistas SDL.
