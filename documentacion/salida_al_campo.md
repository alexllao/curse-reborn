# Decimosexta pantalla: salida al campo

COUNTRY1 (escena 15) continúa la partida después de las cuevas. El monstruo iza a Brad, se cae la máscara y el monstruo huye por el pozo. El personaje recupera el control y puede recorrer la explanada. Entrar en el sendero superior activa el paseo en perspectiva hacia la población, hasta la salida 25 y la entrada en COUNTRY2 (16).

F5/F6 añaden **salida al campo** después del cubo: son diecisiete vistas jugables, con Core, título e introducción aparte. El atajo inicia su entrada original 24 como una sesión nueva, sin conceder objetos de las cuevas. Atravesar las salidas normales conserva el inventario y el estado de la partida.

## Reconstrucción

- La entrada 24 utiliza el descriptor DS:86FC y los canales de Brad DS:82D9, 82F0 y 8308. COUNTRY1.SPR contiene los actores y la máscara; las secuencias originales del segmento DS gobiernan el izado y la reacción. El despacho CS:17A3 tiene entrada cero para la escena 15: no se inventa una rutina particular por fotograma.
- BOXDET define el acceso al sendero como tipo 3, identificador 0, en el rectángulo `(80,119)–(138,122)`. **0AC8 llama a 53B4–53E6**: prepara los tres canales 93DC, 944D y 94B9, el destino `(100,122)` e inicia la acción mediante 886B. Limpia 2C0B y 2C1A; el bucle continúa por 0BA0. Su traducción está en `motor/estado/paso_escena.cpp`.
- Las secuencias del sendero utilizan el cambio de modelo y los canales del motor común. No se simula un acercamiento con escalado gráfico ni se teletransporta al personaje.
- La salida 25 entra en COUNTRY2 con el cargador original. Se admite esa entrada y su rutina 1993–19A6 para evitar interrumpir el motor al llegar; El encuentro y recorrido de COUNTRY2 se han incorporado después en [El camino del asaltante](camino_del_asaltante.md); los puzles de DRAGON siguen pendientes.
- Las respuestas comunes de hablar, examinar, usar, saltar y manipular se contrastan desde las listas ofrecidas en tres posiciones. La máscara caída no se ofrece como recogible: su máscara original de opciones no habilita Recoger. No se añade esa acción por aparecer un objeto dibujado.

## Comprobaciones

`pnpm comparar:campo --muestra` retoma la instantánea real y contrastada `final_salida_cuevas.ds.bin` y su semilla. Si falta, genera antes el recorrido de las cuevas; esa primera ejecución es más larga. Compara **722 casos y pasos**, con **473 pasos jugables**, **226 selecciones de menú** en tres posiciones, nueve fondos y nueve composiciones, tres clics y la transición con carga persistente. Se comprueban DS completo, azar, retornos, entidades y píxeles frente a las instrucciones originales.

`pnpm comprobar:campo` añade nueve capturas SDL de 64.000 píxeles y treinta y cuatro cambios F5/F6 entre las diecisiete vistas. Las capturas recorren el izado, la pérdida de la máscara, la devolución del control, dos posiciones de la explanada, el sendero y la entrada siguiente. Los resultados quedan en `trabajo/comparacion_campo/` y `trabajo/comprobacion_campo_sdl/`.

Las pruebas cubren ese recorrido y los menús ofrecidos en sus posiciones; no acreditan todos los tiempos y posiciones posibles. El audio usa el sistema común existente y esta ampliación no incluye una nueva comparación acústica. El recorrido posterior continúa en COUNTRY2; la población queda para el siguiente tramo.
