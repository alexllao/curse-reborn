# El guardia del palacio

La vista64 incorpora la escena63, ICEGUARD.MAP/SPR e ICEPAL.SFX, por la salida107 desde59. Se continúa la partida persistente del laberinto con sus cerillas y su semilla, sin preparar un inventario de prueba para resolver la sala.

## Sueño y cerillas

D41C–D578 limita la proximidad, duerme al guardia cuando Brad queda detrás de la columna y lo despierta al agotarse el contador. La condición original usa x<=80 y la distancia de la coordenada superior a122 menor que5. El contador35CE conserva el enfriamiento−20, el límite100 y los saltos con signo y desbordamiento de INC/DEC. Las partículas de Benn consumen tres sorteos por intento incluso sin entidades libres.

BD73–BDD7 inserta162 en el pie163 en ambos órdenes. BDD8–BEA6 enciende con Atacar161 o Usar161. Ambas acciones exigen contador positivo y que el tiempo de aproximación más6 sea menor que el tiempo restante. La suma afecta únicamente a AL. El intento tardío conserva los objetos y cambia los canales pendientes aAD50; el intento válido consume el útil y suma5. No se añade una condición artificial de cerilla colocada: el encendido conserva las comprobaciones efectivas del EXE.

La columna tiene obstáculos BOXDET: para regresar hay que rodear su base. La región de tipo3/parámetro8 se ignora en el original, como los parámetros5–8 del despacho2868; no se inventa un comportamiento especial.

## Extintor y regreso

60C6/62CF–6322 recoge164 mediante el servicio común de capacidad, iconos y puntuación. Conserva los otros bits de35C9 al añadir4, activa41EC y prepara la aparición de Benn y el transporte111 a24. Recoger el pie163 deshace la inserción común y responde mediante88A0; no deja un objeto nuevo en el inventario.

El recorrido incluye esconderse, esperar el sueño, aproximarse, colocar la cerilla, rodear la columna para volver a esconderse, encender, recoger el extintor y regresar a Benn. Los capítulos del cementerio y castillo posterior siguen pendientes.

## Comprobaciones

`pnpm comparar:guardia-palacio --muestra` contrasta5293 casos/pasos con memoria DS completa, azar y composición original durante el recorrido. Incluye384 variantes de rutina,480 de las cerillas y36 de capacidad/iconos/progreso del extintor, once imágenes y cinco estados auditados. Las pruebas aisladas no sustituyen el recorrido persistente.

`pnpm comprobar:guardia-palacio` reproduce el recorrido en SDL, compara diez imágenes y verifica restauración y128 cambiosF5/F6 entre64 vistas. Los resultados concretos de la tanda se registran también en AGENTS.md. No se afirma que el resto del juego esté terminado.
