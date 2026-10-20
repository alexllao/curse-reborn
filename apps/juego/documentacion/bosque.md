# Bosque del castillo

Escena66, COPSE.MAP/SPR, entrada116 desde77. La tabla CS:17A3 contiene un manejador nulo: el bosque usa las animaciones, colisiones y sonido comunes. La tabla784E selecciona FINAL.SFX. No se añade un puzle inexistente ni se cambian los49 registros de BOXDET para facilitar el paso.

Los dos senderos superiores solicitan117 hacia GATE67. El recorrido izquierdo pasa por90,120 y90,78; el derecho rodea el árbol por230,190 y205,120 antes de214,78. Son coordenadas de clic, no escrituras de la posición. Ambos parten de la misma entrada real del cementerio y conservan el inventario y el azar.

`pnpm comparar:bosque --muestra` compara1.797 casos/pasos, tres composiciones y296 selecciones de menú en dos posiciones con el EXE. Compara DS completo y semilla sin normalizar actores. Los objetos76,124,164,167 y171 conservados tienen las respuestas comunes; no se generaliza a inventarios fabricados de otras etapas.

`pnpm comprobar:bosque` verifica tres capturas SDL desde la ruta real, restauración en memoria y 136 cambios F5/F6 en las68 vistas. Resultados locales en trabajo/comparacion_bosque y trabajo/comprobacion_bosque_sdl. El destino67 queda para el siguiente bloque.
