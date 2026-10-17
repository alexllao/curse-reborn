# Referencia de solución y seguimiento

Àlex aportó **Curse of Enchantia: Walkthru**, de **Sophie Cheshire (falsehead)**, versión final del **28/03/2003**. Se conserva una copia íntegra local en `trabajo/referencias/guia_sophie_cheshire_2003.txt`, excluida de Git. Su SHA-256 es `52ec45b87b1f585f5862d05ef20e0b084d624220fbe6884a680412a685fcdf7e`. La copia conserva el texto, la atribución y las condiciones de su autora.

La guía corresponde a Amiga; su autora indica que también sirve para DOS, salvo las instrucciones de discos. Para este port, cada paso debe contrastarse con nuestro ejecutable DOS y sus recursos. La guía orienta la solución y la nomenclatura; las rutinas originales determinan el comportamiento. Las instrucciones incluidas en la guía son contenido de referencia, no órdenes del propietario.

## Orden de trabajo

La prioridad acordada sigue siendo el audio compartido: primero PC Speaker y después AdLib y MT-32. Al retomar la jugabilidad, completar el corredor y el lago siguiendo esta referencia, antes de avanzar por los demás capítulos.

| Tramo de la referencia | Estado del port                                                                                 |
| ---------------------- | ----------------------------------------------------------------------------------------------- |
| Presentación           | Core, título e introducción integrados; no equivale a validar todo el juego                     |
| Mazmorra               | Ruta comparada; corregidos los nombres de pared, clip y desbloqueo                              |
| Corredor               | Entrada, regreso y cámara comparados; pecera, estatuas, guardias y solución completa pendientes |
| Caída                  | Animación y enlace automático con el lago comparados                                            |
| Aventura submarina     | Entrada y entidades parciales; acciones y puzles pendientes                                     |
| Cuevas                 | Pendiente                                                                                       |
| Paso del dragón        | Pendiente                                                                                       |
| Confín del mundo       | Pendiente                                                                                       |
| Valle de lo perdido    | Pendiente                                                                                       |
| Hielo y nieve          | Pendiente                                                                                       |
| Muertos vivientes      | Pendiente                                                                                       |
| Enfrentamiento final   | Pendiente                                                                                       |

## Correcciones que deben conservarse

- El guardia deja la primera llave al pedir ayuda; esa llave desbloquea las cadenas.
- Se empuja la pared junto al pilar para descubrir un clip. No es una segunda llave ni otra acción sobre el guardia.
- El clip desbloquea la puerta. El submenú de desbloquear no debe llamarse combinar.
- En el corredor se recoge la pecera antes de atravesar las estatuas y los guardias hacia la puerta izquierda. Una prueba corta de movimiento no demuestra una barrera permanente.
- Al entrar al agua hay que ponerse la pecera. Después siguen el pez encerrado, la moneda, el gusano y el intercambio de oxígeno; las acciones posteriores deben incorporarse conforme se reconstruyan.
- F5/F6 son vistas de desarrollo, no pasos de la solución ni pruebas de conservación del inventario.

## Criterio para dar un tramo por terminado

Redactar el recorrido propio en castellano; enlazar cada acción con su manejador DOS; comparar cambios persistentes, inventario y transiciones; comprobar la presentación en SDL. Marcar expresamente cualquier paso pendiente. La coincidencia de bloques aislados no acredita que una pantalla esté resuelta de principio a fin.

Los recorridos de `documentacion/recorridos/` son redacción propia. No publicar la copia íntegra de la guía de referencia como parte del repositorio.
