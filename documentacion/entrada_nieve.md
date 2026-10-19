# Entrada de la nieve: rutina contrastada

El regreso real a Benn y el vestido termina solicitando la salida74 hacia SNOWAST4 (escena43). Este bloque traslada únicamente CE35–CF25 y la conecta al despacho común del motor. La sala todavía no está habilitada en SDL. La entrada nativa74 ya se contrasta por separado desde el vestido real.

La rutina activa una reacción una sola vez, mediante el bit0 de35CA, cuando el primer actor cabe completamente en la ventana. Conserva las comparaciones firmadas y la suma de palabra con envoltura: no sustituye las condiciones originales por límites geométricos modernos. Programa seis canales, con contadores10/12 e intervalo2, sin alterar los campos vecinos. La proximidad depende de la actividad1 y de la postura32 del actor; las demás actividades conservan el rectángulo anterior.

`pnpm comparar:rutina-nieve` contrasta3600 variantes y7200 llamadas contra el EXE de huella verificada. Alterna el DS original y un segmento aleatorio reproducible; compara los65536 bytes. Incluye fronteras de visibilidad, desbordamientos, otras banderas, actividades y posturas, coordenadas verticales y cámara. Repite cada variante después de adelantar las fases para comprobar que el bit persistente impide reiniciar la reacción.

`pnpm comparar:entrada-nieve` verifica636 casos/pasos: la transición74, cámara inicial, cinco entidades,256 pasos de animación, dos composiciones y118 selecciones en la posición de entrada. La auditoría detectó y permitió completar las respuestas neutras de ataque de70/76/101/124, las combinaciones del inventario real y los sonidos0/1 de ayuda/saludo. El ventilador conserva su rechazo fuera de72. No se añaden respuestas genéricas para los objetos nuevos aún sin investigar.

Quedan pendientes las acciones y posiciones posteriores, los puzles y las salidas de esta zona. Este resultado no suma una vista jugable ni acredita un puzle completo. El último punto real sigue en `trabajo/comparacion_regreso_benn/probador_solicitud74`.
