# Pruebas del motor

`pnpm comprobar:continuidad` compara el recorrido SDL hasta el final y cinco desvíos con y sin recargas intermedias en la misma partida. Perfiles, SAV, imágenes y resultados se conservan exclusivamente en `../trabajo/continuidad_*`. Alcance y límites en [partidas](../documentacion/partidas.md) y [auditoría](../documentacion/auditoria_pantallas.md).

`pnpm comparar:consecuencias` prolonga las selecciones que cierran el menú de las referencias iniciales con semilla. Compara128–512 pasos contra el EXE y señala las ramas sin reposo; no equivale a revisar todos los estados ni todos los capítulos. El informe está en `../trabajo/auditoria_consecuencias/resultado.json`.

| Carpeta         | Contenido                                                                         |
| --------------- | --------------------------------------------------------------------------------- |
| `motor/`        | Estado, acciones comunes, azar, secuencias, partidas y puente C++ de comparación. |
| `recorridos/`   | Puzles, personajes, salidas y recorridos persistentes de las salas.               |
| `audio/`        | Sonido digital, altavoz, música y sintetizadores.                                 |
| `video/`        | Composición, paneles, gráficos, compresión y escaladores.                         |
| `distribucion/` | Pruebas Node de imágenes, integridad y exclusiones del builder beta.              |

Las órdenes `pnpm comparar:*` no cambian. `ejecutar.py` es su entrada común: dispone las importaciones compartidas y ejecuta el comparador solicitado. Para una ejecución manual se usa `python apps/juego/pruebas/ejecutar.py comparar_aleatorio.py --biblioteca <biblioteca>` desde la raíz del repositorio.

`python apps/juego/pruebas/ejecutar.py --comprobar-importaciones` revisa las importaciones de las cuatro áreas sin ejecutar los recorridos. Los resultados y las cachés no se guardan aquí: siguen en `../trabajo/`, fuera de Git. Las comprobaciones específicas de SDL están en `../plataformas/sdl/pruebas/`.

`pnpm comprobar:builder` ejecuta las seis pruebas Node de `distribucion/`: selección de habitación y parámetros, conservación de píxeles y alfa, dimensiones de ambos perfiles, integridad/privacidad del manifiesto y rechazo de enlaces. No necesita datos comerciales. Con `pnpm empaquetar:beta --con-datos`, el empaquetador comprueba además las habitaciones incluidas y el recorrido final en SDL usando la copia de distribución y perfiles aislados. El comprobador recorre el número real de diseños compilados, sin asumir cinco. La verificación del ZIP extraído y los límites de estas pruebas se describen en [Distribución](../documentacion/distribucion.md).

`comparar:consecuencias --incluir-clc` observa también los verbos que mantienen abierto el menú tras cerrarlo con clic derecho; `--solo-clc` evita repetir las observaciones STC. Las copias solo avanzan después del cierre lógico; no se prueba aquí el dibujo del menú abierto. Alcance, referencias e informes de población y visitas posteriores en la auditoría.

Las referencias nuevas conservan `.contexto.json` con banco de modelo y fase externa disponible. `comparar:consecuencias` admite25 solo con su contexto real y contrasta CS:1BA2 además de DS y azar. `--reanudar --estado <referencia.ds.bin>` observa una secuencia en curso sin abrir el menú hasta reposo o petición de salida; no debe contarse como una interacción elegida por el jugador. Referencias y resultados del vestido en la auditoría y en [probador](../documentacion/probador_benn.md).

La prueba selectiva `pnpm comparar:ajo-primera-zona` vive en `recorridos/comparar_ajo_primera_zona.py`: reproduce desde la referencia real del cementerio la suspensión observada en el EXE al comer170 antes de114, con4.096 pasos comparados. No corrige el juego ni interpreta un plazo agotado como reposo. Requiere regenerar primero `pnpm comparar:cementerio --muestra` y sus antecedentes; véase `documentacion/cementerio.md`.

`pnpm comparar:orden-poleas` contrasta el orden real cristal145/aceite144, la oferta147 y el regreso a la ventana hasta solicitar90. El comparador está en `recorridos/comparar_orden_poleas.py`; conserva las referencias con DS/semilla/contexto en `trabajo/comparacion_orden_poleas`, sin recursos versionados ni capturas nuevas. Véase `documentacion/laterales_palacio.md`.

`pnpm comparar:regreso-remote` comprueba60/56/61 con106 y correo enviado, esperando el control antes de cada clic. `pnpm comparar:plazos-guardia` comprueba cuatro demoras reales al colocar162 en163. Ambos están en `recorridos`, requieren sus referencias locales con semilla/contexto y escriben resultados ignorados en `trabajo/comparacion_regreso_remote` y `trabajo/comparacion_plazos_guardia`. Véase `documentacion/auditoria_pantallas.md`.

`pnpm comparar:tropiezos-cementerio` combina768 contratos aislados de orientación9/10/11 y una caída real con recuperación113 y continuación114. Usa `comparacion_cementerio/cementerio_util_167` y `cementerio_regreso` con sus semillas/contextos. El comparador reside en `recorridos` y escribe resultados ignorados en `trabajo/comparacion_tropiezos_cementerio`.

`pnpm comparar:aproximaciones-cementerio` ejecuta ocho entradas dirigidas a los tropiezos9/10 y compara sus recuperaciones113; `--casos 9_arriba 10_abajo` selecciona una tanda parcial. Requiere `comparacion_tropiezos_cementerio/cementerio_caida_recuperada` con semilla/contexto y guarda ocho referencias nuevas e informe en `trabajo/comparacion_aproximaciones_cementerio`. El comparador está en `recorridos/comparar_aproximaciones_cementerio.py`.

`pnpm comparar:recaidas-vestibulo` contrasta seis secuencias de dos caidas11 con actor activo y demoras0/32/96. Requiere DS/semilla/contexto de `comparacion_vestibulo_castillo/vestibulo_control` y `comparacion_estudio_castillo/estudio_vestibulo`. Comprueba inventario, reinicio del canal, recuperacion y continuacion120. Resultado ignorado en `trabajo/comparacion_recaidas_vestibulo/resultado.json`; fuente en `recorridos/comparar_recaidas_vestibulo.py`.
