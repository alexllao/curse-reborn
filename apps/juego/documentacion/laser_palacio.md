# La sala del láser

La vista57 integra la escena56 después del salto90 desde el corredor49. La tabla original784E selecciona ICEROOM.MAP/SPR e ICEPAL.SFX, no ICEGUARD. La siguiente habitación, escena57, permanece pendiente.

BB8D–BC6E coloca una pieza seleccionada previamente sobre los soportes155–158 mediante Usar. El orden inverso se rechaza, como en el EXE. Las piezas correctas son lupa150, cubito153, carámbano154 y cristal145. La recogida5FFD–6060/61A7 permite retirar piezas equivocadas sin llamar6285 y respeta capacidad, iconos, modelo y animación.

CFD2–D113 propaga el rayo por los cuatro reflectores y destruye el emisor y su protector. Conserva los canales, máscaras, seis intentos de partículas y consumos de azar. BC71–BCDC permite insertar el gato149 bajo la reja159 solo cuando el protector está destruido. En el recorrido contrastado solicita automáticamente103 tras42 pasos; su destino aún no se carga.

El contraste conserva todo DS, incluidos los actores inactivos. Estos también intervienen en la ordenación F29A. Los valores de AX/BX del final de la rutina se reconstruyen desde el despacho1119/1791 y los resultados de creación8B6F; no se ignoran ni normalizan bytes para ocultar diferencias.

`pnpm comparar:laser-palacio --muestra` contrasta7877 casos/pasos, incluidos268 casos de la rutina completa. Añade216 variantes de colocación y reja,96 de retirada y auditorías de las opciones en siete estados del recorrido real. Nueve composiciones coinciden con el original. No se añaden objetos ni se fuerzan banderas para resolver la partida.

`pnpm comprobar:laser-palacio` verifica nueve capturas SDL, guardado/restauración y114 cambiosF5/F6 entre57 vistas. Incluye una colocación errónea, su recuperación, las cuatro piezas correctas y la apertura de la reja. Movimiento, secuencias, entidades y sonido siguen usando servicios comunes. Estos resultados no acreditan el capítulo posterior ni todas las posiciones posibles.

Revisión ampliada21/10: generador regenerado con contextos de banco y fase, incluidas las peticiones de salida. Comparación de consecuencias, respuestas negativas y cierresCLC documentada en `auditoria_pantallas.md`, sección «Láser y reina». No cambia el motor ni sus recursos.
