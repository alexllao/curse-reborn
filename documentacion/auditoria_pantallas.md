# Auditoría de rutas y pantallas

Una vista F5/F6 no equivale a una habitación terminada. La auditoría distingue enlaces entre habitaciones, interacciones ofrecidas por el menú, variantes de progreso y capítulos posteriores.

`pnpm auditar:rutas` verifica la huella del EXE y recorre los 78 índices de BOXDET.DAT. Cruza sus umbrales de tipo 2 con la tabla DOS de destinos en DS:7482 y la lista única `salidas_integradas` de SDL. El informe completo queda en `trabajo/auditoria_rutas/resultado.json`. No ejecuta puzles, predicados ni destinos indirectos de guiones; un enlace habilitado tampoco acredita todas las opciones de su destino.

## Rutas detectadas en esta revisión

| Origen                       | Salida | Destino                            | Estado                                                   |
| ---------------------------- | ------ | ---------------------------------- | -------------------------------------------------------- |
| Fuente, derecha (17)         | 29     | Calle de los disfraces, ROGUE (19) | Entrada y regreso 37 integrados.                         |
| Fuente, izquierda (17)       | 30     | Calle de la adivina, GYPSY (20)    | Entrada y regreso 40 integrados.                         |
| Calle de las maravillas (18) | 33     | Tienda abandonada, DERELICT (23)   | Entrada y regreso 45 integrados.                         |
| Entrada del pueblo (21)      | 41     | Posada Red Dragon, INNSIDE (28)    | Entrada, reacción del mostrador y regreso 50 integrados. |

La fuente tiene cuatro salidas originales: arriba a SORCERY, abajo a DRAGON, izquierda a GYPSY y derecha a ROGUE. Las dos laterales estaban sin conectar en la entrega anterior; su falta de respuesta no era el comportamiento original.

El recorrido contrastado de estas cuatro ramas parte de las instantáneas reales de la población. Compara 6.113 casos/pasos, nueve composiciones y 368 selecciones en cuatro estados, incluidos el posadero y los regresos. Las ramas de prueba no fabrican inventario ni banderas para forzar entradas. Fuentes: 19C1–19DE para GYPSY; 541C–547F para el mostrador; entradas 29/30/33/41 y retornos 37/40/45/50 del cargador.

## Pendientes localizados desde las vistas disponibles

| Origen      | Salida | Destino DOS | Trabajo pendiente                                                    |
| ----------- | ------ | ----------- | -------------------------------------------------------------------- |
| CHANGE (25) | 74     | Escena 43   | Continuación posterior del disfraz; aún sin integrar.                |
| GYPSY (20)  | 52     | Escena 35   | Continuación posterior todavía sin integrar.                         |
| MAGE (22)   | 72     | Escena 31   | WISE31 ya integrado desde EDGE; falta contrastar esta rama del mago. |

Las rutas 19→24 por35, 20→26 por38 y 20→27 por39 ya están conectadas, con regresos46/48/49. La comparación de tiendas incluye saludo, entrega de dinero a la adivina, fases de oferta de Benn y su rutina de partículas. Se conservan los recursos compartidos entre escenas23/27 con sus estados independientes. Véase [el tramo de tiendas](tiendas_poblacion.md).

EDGE30 ya tiene recorrido persistente hasta WISE31 por72, con regreso del loro a DERELICT27 por73. El contraste incluye los puzles del acantilado y el saludo inicial del loro; no equivale a probar todos los estados de las visitas posteriores. Véase [el alcance de EDGE](borde_del_mundo.md). Las escenas fuera del catálogo siguen pendientes de reconstrucción; el informe de 78 tablas permite localizarlas sin contarlas como jugables.

Las escenas iniciales hasta COUNTRY2 conservan sus recorridos y auditorías anteriores. Esto no demuestra exhaustivamente todos los estados de una partida, retornos de capítulos posteriores, opciones de inventario posibles ni equivalencia completa de cadencia. Los errores de disco DOS, macOS y WebAssembly continúan pendientes de validación. Las mejoras gráficas son opcionales y están separadas de estas garantías del motor.

El probador25 ya conecta36/27. Se contrasta1A74 y los manejadores del vestido; el recorrido posterior del valle sigue pendiente. Véase [el alcance del probador](probador_benn.md).

La auditoría de32 vistas detecta también WISE31→64 por112, correspondiente a la continuación posterior. Junto con52/72/74 suma cuatro enlaces pendientes desde el catálogo; no se ocultan los destinos de capítulos posteriores.
