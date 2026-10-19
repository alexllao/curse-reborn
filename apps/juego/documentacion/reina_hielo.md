# El trono de la reina del hielo

La vista58 integra ICEQUEEN (escena57) mediante la salida103 de la reja del láser. Conserva ICEPAL.SFX según la tabla original784E. El trono solicita104, cuyo destino58 todavía no está integrado.

D114–D36B permite seleccionar la reina160 desde lejos mediante3593, inicia sus tres canales de ataque cuando Brad avanza más allá de40 y no hay una descarga en marcha, y comprueba el solapamiento horizontal del proyectil activo. Los cuadros82/83 del modelo9 evitan el golpe; por encima del umbral vertical125 tampoco se procesa el impacto. Un salto temprano recibe el proyectil al aterrizar. El modelo24 retrocede hacia40 y recupera después el control. Se conservan comparaciones con signo, contadores y canales originales.

Saltar sobre160 ejecuta BCDF–BCFD: cancela el camino y programa ACC2/ACD8/ACE7 mediante el iniciador común886B. No se añade un botón nuevo, invulnerabilidad, temporizador o salto automático. Al alcanzar197 y la franja del suelo con el control habilitado, D135 programa el acercamiento al trono, los actores auxiliares y el planificador0F5F. Las secuencias originales sientan y expulsan a Brad hacia104.

`pnpm comparar:reina-hielo --muestra` contrasta1237 casos/pasos, incluidos540 variantes directas de D114 y36 selecciones de salto. Compara DS completo, azar, ordenación gráfica, once composiciones y las opciones ofrecidas en tres estados. La partida parte de la reja resuelta anteriormente; no se añaden objetos ni se cambian posiciones para superar el encuentro. Incluye salto temprano, golpe, recuperación y dos saltos correctos hasta la petición104.

`pnpm comprobar:reina-hielo` comprueba el mismo recorrido SDL, once imágenes, guardado/restauración y116 cambiosF5/F6 entre58 vistas. Los servicios de movimiento, secuencias, modelos, entidades y sonido son los compartidos. La comparación no acredita la jugabilidad del destino104 ni todos los tiempos posibles del enfrentamiento.
