# Puente del castillo

Escena68, CASTLENT.MAP/SPR y FINAL.SFX, entrada118 desdeGATE67. CS17A3[68]=0. Los canales de entrada84E7/8503/851E reproducen la travesía y el derrumbe mediante los servicios de secuencias comunes.

La secuencia pide119 automáticamente tras85 pasos en el recorrido contrastado. No se inventa una espera de control ni un clic para entrar al vestíbulo. No hay un estado libre de menú entre118 y119 en esta ruta.

`pnpm comparar:puente-castillo --muestra` retoma verja_solicitud118 y compara180 casos/pasos, incluyendo85 pasos jugables, composición y orden de dibujo. Guarda cuatro imágenes: llegada, paso32, paso64 y paso80. Conserva DS completo y azar; no escribe banderas para forzar el derrumbe. Cero estados de menú auditados, porque el jugador está bajo control del guion.

`pnpm comprobar:puente-castillo` verifica las cuatro imágenes SDL, restauración en memoria y 140 cambios F5/F6 en las70 vistas. La instantánea puente_castillo_solicitud119 permite continuar conCASTHALL69; ese destino sigue pendiente de integrar. Esta escena automática se cuenta como vista, no como un puzle adicional.
