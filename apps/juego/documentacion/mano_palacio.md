# La sala de la mano

La vista59 incorpora la escena58, ICEROOM.MAP/SPR e ICEPAL.SFX según784E. Se entra por104 desde el trono de la reina. La salida105 hacia escena59 está solicitada y contrastada; su habitación de destino sigue pendiente.

D36C–D41B comprueba que la mano2C4C esté en actividad3, la posición vertical de Brad sea menor o igual que130 con signo y el control esté habilitado. Desactiva el descriptor persistente4D94 y el control/camino; coloca la mano en x+128,y−106 y programa B4EE/B4FA/B503 para Brad y B4CD/B4D4/B4E1 para la mano, preservando los contadores que no modifica el EXE. Si Brad tiene el bit2, la profundidad de la mano pasa a200.

Ponerse el aceite144 ejecuta BD00–BD70. Solo se admite el modelo23 en el cuadro29. Retira el aceite, programa ACF6/AD0E/AD21 y los tres canales auxiliares con contador29, suma cinco puntos con desbordamiento de16bits y llama al iniciador común886B. El original no comprueba el resultado de retirar el objeto: las variantes conservan también esa peculiaridad con inventario ausente. Los intentos en otros modelos o cuadros se rechazan. No se añade una condición inventada para abrir la puerta.

La partida real parte del estado expulsado por la reina, conserva el aceite recogido anteriormente y camina hacia la mano. Después de quedar atrapado, se pone el aceite, resbala, vuelve a la forma normal y cruza la puerta. No se alteran posiciones, inventario o banderas para superar el puzle.

`pnpm comparar:mano-palacio --muestra`:585 casos/pasos, incluidos48 variantes de D36C y36 del uso del aceite; DS completo y azar sin normalización, siete composiciones y auditoría de las opciones en tres estados. `pnpm comprobar:mano-palacio`: siete capturas SDL, restauración y118 cambiosF5/F6 entre59 vistas. Conserva los servicios compartidos de movimiento, modelos, secuencias, sonido y guardado DOS. Estos resultados no acreditan la siguiente habitación ni todos los estados posibles del juego.

Revisión ampliada21/10: referencias reales regeneradas con contexto de banco y fase, negativas y cierresCLC, más regreso posterior aBenn24 con extintor. Cifras, reproducción y límites en la sección «Mano, laberinto, guardia y regreso con el extintor» de `auditoria_pantallas.md`. No cambia el motor ni añade vistas SDL.
