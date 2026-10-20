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
