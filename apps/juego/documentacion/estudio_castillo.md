# Estudio del castillo

Escena70, STUDY.MAP/SPR y FINAL.SFX, entrada120 desdeCASTHALL69 y regreso121. CS17A3[70]=0. Se conserva el anillo175 bajo la ventana y el comportamiento de sus27 cajas de BOXDET.

Recoger175 sigue563A/6141–6153/633F/61A7–61F1: capacidad e iconos de puntuación comunes, OR1 en53AD, aproximación a la entidad2C4C más5 en horizontal y alto+1 en vertical. Anula el descriptor persistente, prepara el canal1A2E y elige96CF/96D8 o9705/970E con modelo20. El contador conserva el byte bajo de los pasos más seis. No fuerza huecos de inventario ni borra los otros bits de53AD.

`pnpm comparar:estudio-castillo --muestra` compara2.193 casos/pasos, seis imágenes y766 selecciones en cuatro estados auditados. Incluye72 variantes de recogida (capacidad, iconos, modelo, puntos y banderas previas); están incluidas en el total de selecciones. DS completo y semilla coinciden con el EXE.

La ruta parte del acceso real120, recoge el anillo, vuelve por121 y entra otra vez por120. Comprueba que el anillo permanece en el inventario y no reaparece en el estudio. La instantánea estudio_vestibulo conserva el regreso con anillo para continuar hacia la biblioteca, sin crear objetos ni progreso de prueba.

`pnpm comprobar:estudio-castillo` contrasta las seis imágenes SDL y la restauración en memoria durante esos mismos pasos, además de144 cambios F5/F6 en las72 vistas. La biblioteca71 y la batalla final siguen pendientes.

Revisión ampliada21/10: referencias del recorrido real regeneradas con contexto; consecuencias, respuestas negativas, cierresCLC y regresos documentados en la sección «Consecuencias del cementerio al desenlace» de `auditoria_pantallas.md`. El desenlace sin acciones se comprueba mediante reanudación explícita, no con una lista vacía. No cambia el motor ni los recursos.
