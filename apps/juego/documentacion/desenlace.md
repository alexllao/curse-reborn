# Regreso a casa

Escena73, BASEBAT.MAP, OUTRO.SPR y ENDGAME.SFX, entrada125 desde el duelo. CS17A3[73]=0. Las escenas74–76 reutilizan DUNGEON como entradas de reserva; no se cuentan como tres habitaciones pendientes de la aventura.

4D40–4D8C distingue el índice de escena73 después de consultar la tabla de destinos. EscribeFFFF en las doce palabras5225–523B: vacía el inventario y termina la lista de temas. El comparador de entradas ejecuta también esa rama del EXE; no basta con la preparación común anterior a4D40.

Los tres actores de la lista5206 usan los canales7403/7415/7424 y desplazamientos7433/7459. El golpe de bate, el sonido15 y la pelota terminan sus nueve canales tras51 pasos y muestran el «THE END» gráfico original. El menú queda habilitado como en el original; no se inventa otro puzle, movimiento libre o salida posterior al final.

`pnpm comparar:desenlace --muestra` retoma bruja_solicitud125 del duelo real. Contrasta391 casos/pasos, seis imágenes y19 selecciones en un estado de menú. Los179 pasos jugables incluyen51 de animación y128 de reposo; comprueba que los canales no reaparecen ni se solicita otro destino. DS completo y azar coinciden con el EXE.

`pnpm comprobar:desenlace` sigue en SDL toda la ruta existente hasta el anillo y el regreso a casa, compara los seis fotogramas y verifica guardar/cargar en memoria y150 cambios F5/F6 en las75 vistas. Las compilaciones Node y CMake coinciden en64.000 píxeles del fotograma final; CMake también contrasta el anillo64 del duelo. El catálogo suma75 vistas, escenas0–73 y77. Esto cierra la integración de escenas, no la revisión integral de una partida de principio a fin con todas sus alternativas.

Los resultados y recursos comerciales permanecen solo en trabajo/ y datos_originales/. La revisión continua de la aventura y el vídeo optimizado de la guía quedan para el siguiente trabajo, tal como se acordó con Àlex.
