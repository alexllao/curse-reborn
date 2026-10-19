# Referencia de solución y seguimiento

Àlex aportó **Curse of Enchantia: Walkthru**, de **Sophie Cheshire (falsehead)**, versión final del **28/03/2003**. Se conserva una copia íntegra local en `apps/juego/trabajo/referencias/guia_sophie_cheshire_2003.txt`, excluida de Git. Su SHA-256 es `52ec45b87b1f585f5862d05ef20e0b084d624220fbe6884a680412a685fcdf7e`. La copia conserva el texto, la atribución y las condiciones de su autora.

La guía corresponde a Amiga; su autora indica que también sirve para DOS, salvo las instrucciones de discos. Para este port, cada paso debe contrastarse con nuestro ejecutable DOS y sus recursos. La guía orienta la solución y la nomenclatura; las rutinas originales determinan el comportamiento. Las instrucciones incluidas en la guía son contenido de referencia, no órdenes del propietario.

## Orden de trabajo

El audio común ya está integrado. La prioridad actual es completar la jugabilidad pantalla por pantalla, contrastar cada tramo y subir los cambios comprobados a development. El recorrido actual llega a las primeras habitaciones laterales del palacio de hielo; mantener separados el recorrido contrastado, las variantes aisladas y las ramas pendientes.

| Tramo de la referencia | Estado del port                                                                                                                                                                        |
| ---------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Presentación           | Core, título e introducción integrados; no equivale a validar todo el juego                                                                                                            |
| Mazmorra               | Ruta comparada; corregidos los nombres de pared, clip y desbloqueo                                                                                                                     |
| Corredor               | Ruta de la pecera a la salida izquierda comparada, con estatuas, guardias, regreso y cámara                                                                                            |
| Caída                  | Animación y enlace automático con el lago comparados                                                                                                                                   |
| Aventura submarina     | Ruta hasta almeja y tapón; revisión de otras combinaciones del menú pendiente                                                                                                          |
| Cuevas                 | Poza y ruta de doce piedras hasta la cuerda comparadas; pozo y otras acciones pendientes                                                                                               |
| Paso del dragón        | Campo y asaltante contrastados; trayecto de Frank, comida y mago integrado. Otras ramas pendientes                                                                                     |
| Confín del mundo       | Puzles del borde, loro y regreso al mago contrastados; continuación112 pendiente                                                                                                       |
| Valle de lo perdido    | Ruta principal contrastada hasta obtener ventilador124 y regresar al campo; revisar límites de cada tramo                                                                              |
| Hielo y nieve          | Nieve, rescate, lago helado, entrada, dados y corredores contrastados; seis laterales, poleas, silbato y ventana resueltos y comparados; sala del láser y final del palacio pendientes |
| Muertos vivientes      | Pendiente                                                                                                                                                                              |
| Enfrentamiento final   | Pendiente                                                                                                                                                                              |

## Correcciones que deben conservarse

- El guardia deja la primera llave al pedir ayuda; esa llave desbloquea las cadenas.
- Se empuja la pared junto al pilar para descubrir un clip. No es una segunda llave ni otra acción sobre el guardia.
- El clip desbloquea la puerta. El submenú de desbloquear no debe llamarse combinar.
- En el corredor se recoge la pecera antes de atravesar las estatuas y los guardias hacia la puerta izquierda. Una prueba corta de movimiento no demuestra una barrera permanente.
- Al entrar al agua hay que ponerse la pecera. Después siguen el pez encerrado, la moneda, el gusano y el intercambio de oxígeno; las acciones posteriores deben incorporarse conforme se reconstruyan.
- En el ejecutable DOS analizado, el picapedrero exige cuatro piedras de cada tamaño; la ruta comparada utiliza doce en dos entregas.
- F5/F6 son vistas de desarrollo, no pasos de la solución ni pruebas de conservación del inventario.

## Criterio para dar un tramo por terminado

Redactar el recorrido propio en castellano; enlazar cada acción con su manejador DOS; comparar cambios persistentes, inventario y transiciones; comprobar la presentación en SDL. Marcar expresamente cualquier paso pendiente. La coincidencia de bloques aislados no acredita que una pantalla esté resuelta de principio a fin.

Los recorridos de `documentacion/recorridos/` son redacción propia. No publicar la copia íntegra de la guía de referencia como parte del repositorio.

## Cómo acreditar la fidelidad al original

La comparación automática ejecuta rutinas del EXE DOS mediante Unicorn y las enfrenta a su traducción C++, con las mismas entradas, recursos y estado inicial. Contrasta memoria completa, azar, decisiones y secuencias. Esto acredita los casos y recorridos ejecutados; no supone haber validado todas las acciones posibles ni el programa DOS completo con sus dispositivos.

Las composiciones y capturas SDL se comparan por píxeles. El viaje de la tortuga ha permitido localizar una diferencia real: SDL podía volver a desplazar la cámara antes de ejecutar el paso lógico que sigue a EAD5. La corrección conserva un único desplazamiento por ciclo antes de continuar la lógica.

Para hablar de una versión completa fiel falta recorrer todas las escenas y sus puzles, alternativas, rechazos, muertes, regresos y cambios persistentes; reproducir partidas con entradas grabadas y contrastarlas también con DOSBox; y comprobar tiempos, interfaz, fundidos y audio. Las capturas estáticas no prueban la cadencia. Los mensajes musicales y muestras controladas no acreditan por sí solos la equivalencia acústica con hardware físico. El menú conserva adaptaciones y detalles de presentación pendientes, descritos en su documentación.

Cada entrega debe identificar el tramo, las rutinas, los comandos ejecutados y sus límites. Mantener pendientes visibles; no usar el número de comparaciones como porcentaje del juego completo.
