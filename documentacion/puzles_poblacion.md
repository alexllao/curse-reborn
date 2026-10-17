# Paso del Dragón: comida y transporte del mago

Se ha reconstruido el trayecto DRAGON → FRANKS → DRAGON → XROADS → SORCERY → MAGE, desde la partida que ganó el dinero al asaltante. Las salidas conservan inventario y estado; las vistas F5/F6 abren sesiones independientes.

Están integrados la entrada y los guardias, el saludo a Frank, dar dinero para obtener comida, comerla, el portal con perspectiva, la fuente y el intercambio con el mago hasta la salida 47. **No equivale a completar toda la ciudad**: posada, calles laterales, otros personajes y regresos de capítulos posteriores necesitan sus recorridos completos y auditorías. No se cuentan como pantallas terminadas solo por cargar sus fondos.

## Fuentes originales

| Rutina DOS            | Contrato trasladado                                                                                                            |
| --------------------- | ------------------------------------------------------------------------------------------------------------------------------ |
| 19A7–19C0             | Franja de proximidad de la fuente según la entidad activa.                                                                     |
| 358A–35B0 y 35FB–3617 | Respuestas al saludo en MAGE y FRANKS; canales y sonido comunes.                                                               |
| 41DA–420C             | Dar dinero 76 a Frank 83: elimina una comida anterior, inserta 84 y anima la entrega, incluso cuando el inventario está lleno. |
| 419E–41D7             | Comer 84: orientación original, modelo normal y consumo del objeto.                                                            |
| 43B4–44A8             | Dar dinero 76 al mago 77: tres guiones según progreso, ofertas persistentes y puntuación.                                      |
| 5482–54A9             | Portal de perspectiva hacia XROADS: conserva el reinicio y los tres canales.                                                   |
| 54AA–54EA             | Reacción del portero al acercarse a la posada; el interior sigue pendiente.                                                    |
| 5C8F–5CAC             | Recogida de dinero en DRAGON: entidad distinta de la del asaltante.                                                            |

El intercambio no descuenta dinero del inventario: se conserva el comportamiento observado en el EXE. La respuesta al identificador 81 (420F–430C) también tiene comparación aislada de sus cinco ramas; no implica que esté integrada su ruta futura.

## Comprobación

La ejecución final contrasta **8.855 casos y pasos**, incluidas **1.084 selecciones** y nueve estados auditados. Las doce capturas SDL coinciden y F5/F6 completa 44 cambios entre las 22 vistas. La reacción del portero se recorre en una copia, sin entrar en la posada ni modificar la solución principal. La regresión selectiva conserva 24 variantes de espada/dinero de COUNTRY2; guardar, cargar, borrar e importar también pasan su comprobación SDL con perfil temporal.

`pnpm comparar:poblacion --muestra` continúa una instantánea de la partida contrastada anterior; si falta, reconstruye el tramo del asaltante. Compara DS completo, generador, decisiones, orden de dibujo y composiciones con instrucciones del EXE. Revisa las opciones ofrecidas en nueve estados del recorrido y variantes de inventario, orientación, modelo, puntuación y progreso. Las variantes no alteran la partida que resuelve los puzles.

`pnpm comprobar:poblacion` añade doce capturas SDL de 64.000 píxeles, navegación por veintidós vistas de desarrollo y carga de EDGE. Comparte únicamente la permutación inicial de dibujo BDD2:BEE6 con SDL, como en la prueba del asaltante; su evolución posterior se ejecuta y compara independientemente. No acredita el historial gráfico completo desde el arranque ni todos los estados posibles. Las imágenes estáticas no validan tiempos de fundido o cadencia en un monitor real.

La entrada 47 con SMALL.BRD se contrasta con el cargador DOS. SDL carga el borde del mundo y lo deja detenido como **punto de llegada pendiente**: su rutina C093 y sus puzles aún no están trasladados. Su imagen de llegada no forma parte de las doce comparaciones por píxeles. El límite se registra en consola y se puede salir con F5/F6.

Los resultados permanecen en `trabajo/comparacion_poblacion/` y `trabajo/comprobacion_poblacion_sdl/`, fuera de Git. El recorrido propio está en [10_paso_del_dragon.txt](recorridos/10_paso_del_dragon.txt).
