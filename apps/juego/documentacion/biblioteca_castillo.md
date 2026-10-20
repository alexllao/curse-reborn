# Biblioteca del castillo

Escena71, LIBRARY.MAP/SPR y FINAL.SFX, entrada122 desdeCASTHALL69 y regreso123. CS17A3[71]=0. Conserva las34 cajas de BOXDET y las estanterías: para llegar al libro hay que rodear la primera por la parte baja, sin atravesar su colisión.

El objeto176 corresponde al mecanismo del libro. Examinar conserva la respuesta común3B4F y deja el menú abierto: el recorrido lo cierra con clic derecho antes de continuar; no se añade una condición de haber examinado antes ni un cambio de progreso ficticio. Empujar176 sigue48B0–4924: exige que el segundo actor esté en187 global, prepara destino142−cámara/159 y añade un nodo por179 si la Y de Brad es>=132 con signo. Mantiene los campos auxiliares no escritos, los canales2B24/2B34 y dos auxiliares de actores. No añade puntos ni consume objetos.

La puerta abierta desplaza el actor hasta155 global; es la condición original de la salida124. Después de la apertura se rodea la estantería por142,190 y205,190 antes de subir a205,154. La solicitud124 se guarda para continuar con la batalla72, aún pendiente.

`pnpm comparar:biblioteca-castillo --muestra` retoma el vestíbulo con el anillo obtenido realmente en el estudio. Contrasta6.524 casos/pasos, ocho imágenes y853 selecciones en cuatro estados auditados. Incluye48 variantes de la acción (186/187/188 global, cámara, frontera con signo deY y campos auxiliares previos); están incluidas en las selecciones. DS completo y azar coinciden con el EXE.

Se comprueban entrada122, regreso123, segunda visita y pasadizo. `pnpm comprobar:biblioteca-castillo` reproduce el mismo camino en SDL, compara ocho capturas y verifica restauración en memoria y 146 cambios F5/F6 en las73 vistas. Los recursos comerciales y los resultados quedan fuera de Git.
