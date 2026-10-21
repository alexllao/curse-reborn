# Vestíbulo del castillo

Escena69, CASTHALL.MAP/SPR y FINAL.SFX, entrada119 desde el puente68. CS17A3[69]=0: usa animación, movimiento y sonido comunes. Conserva las44 cajas de BOXDET, incluida la zona central tipo3/id11; no se modifica el escenario para simplificar los accesos.

La escalera izquierda solicita120 hacia STUDY70. La derecha solicita122 hacia LIBRARY71. Se contrastan ambas desde el mismo estado real después del derrumbe, manteniendo el inventario y la semilla. La aproximación derecha pasa por270,190 y280,170 antes de subir hacia300,150; un clic vertical fuera de ese recorrido puede tropezar con la pendiente original.

`pnpm comparar:vestibulo-castillo --muestra` compara3.909 casos/pasos, cuatro composiciones y444 selecciones en tres estados de menú. DS completo y azar coinciden con el EXE. Guarda las solicitudes120 y122 para continuar con los interiores, sin considerarlos integrados por haber reconocido su salida. Incluye tres caídas centrales desde clics170/175/180,131, recupera el control y continúa hacia120 en cada rama. Estos recorridos no agotan todas las caídas posibles de la zona central; la ampliación de caídas se contrasta en el núcleo, sin nuevas capturas SDL.

`pnpm comprobar:vestibulo-castillo` verifica llegada, control y aproximaciones a las dos escaleras en SDL, con restauración en memoria y 142 cambios F5/F6 en las71 vistas. Las salidas120/122 se habilitarán cuando sus destinos estén contrastados.

Revisión ampliada21/10: referencias del recorrido real regeneradas con contexto; consecuencias, respuestas negativas, cierresCLC y regresos documentados en la sección «Consecuencias del cementerio al desenlace» de `auditoria_pantallas.md`. El desenlace sin acciones se comprueba mediante reanudación explícita, no con una lista vacía. No cambia el motor ni los recursos.
