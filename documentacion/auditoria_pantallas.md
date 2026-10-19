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
| MAGE (22)   | 72     | Escena 31   | Carga22→31 contrastada; faltan condiciones argumentales posteriores. |

Las rutas 19→24 por35, 20→26 por38 y 20→27 por39 ya están conectadas, con regresos46/48/49. La comparación de tiendas incluye saludo, entrega de dinero a la adivina, fases de oferta de Benn y su rutina de partículas. Se conservan los recursos compartidos entre escenas23/27 con sus estados independientes. Véase [el tramo de tiendas](tiendas_poblacion.md).

EDGE30 ya tiene recorrido persistente hasta WISE31 por72, con regreso del loro a DERELICT27 por73. El contraste incluye los puzles del acantilado y el saludo inicial del loro; no equivale a probar todos los estados de las visitas posteriores. Véase [el alcance de EDGE](borde_del_mundo.md). Las escenas fuera del catálogo siguen pendientes de reconstrucción; el informe de 78 tablas permite localizarlas sin contarlas como jugables.

Las escenas iniciales hasta COUNTRY2 conservan sus recorridos y auditorías anteriores. Esto no demuestra exhaustivamente todos los estados de una partida, retornos de capítulos posteriores, opciones de inventario posibles ni equivalencia completa de cadencia. Los errores de disco DOS, macOS y WebAssembly continúan pendientes de validación. Las mejoras gráficas son opcionales y están separadas de estas garantías del motor.

El probador25 ya conecta36/27. Se contrasta1A74 y los manejadores del vestido; el recorrido posterior del valle sigue pendiente. Véase [el alcance del probador](probador_benn.md).

La auditoría de32 vistas detecta también WISE31→64 por112, correspondiente a la continuación posterior. Junto con52/72/74 suma cuatro enlaces pendientes desde el catálogo; no se ocultan los destinos de capítulos posteriores.

La revisión siguiente incorpora el adaptador MAGE22→WISE31 por72, contrastado como contrato aislado del cargador. No prueba todavía su activación argumental posterior. Quedan tres enlaces desde las vistas disponibles (52/74/112). POST35 tiene entrada y rutina contrastadas solo en el núcleo; sus caminos60/57/66/71 y puzles permanecen pendientes. Véase [el regreso del loro](regreso_loro.md).

Las tandas nativas siguientes contrastan60/56/61, cargas alternativas55/71 y el recorrido57→DINO→58 con pelo, estornudo, sello y bolígrafo;54 se comprueba desde otra rama deCELESTE. No se añaden vistas SDL: quedan puzles y conexiones tardías pendientes. Alcance y pruebas en [ramas del valle](ramas_valle.md) y [DINO](dinosaurio.md).

## Activación SDL de POST

La salida52 desdeGYPSY20 ya está conectada aPOST35. `comprobar:poste` recorre la partida hasta esa entrada, compara64000 píxeles y comprueba guardar/restaurar y66 cambios entre33 vistas F5/F6. El núcleo del resto del valle ya está reconstruido hastaELECTRO39 y regreso16; su activación SDL se realiza por sala y sus rutas siguen pendientes en la plataforma hasta comprobarlas. Las anotaciones anteriores sobre52 y las32 vistas corresponden a revisiones previas.

## Estado vigente tras integrar el valle

Las40 vistasSDL cubren las escenas0–39. La auditoría de las78 tablas deja dos enlaces pendientes desde ese catálogo: CHANGE25/salida74→43 y WISE31/salida112→64. Todas las rutasBOXDET del valle32–39 están catalogadas, incluida70; su alternativa52 y los retornos71 se contrastan por separado con enlaces-valle. Los destinos de guiones66/67/69 están ejercitados en los recorridosSDL. Esto describe enlaces y casos revisados, no todas las combinaciones de estados posteriores del juego.

Las secciones anteriores conservan la historia de ampliaciones; sus recuentos de32 vistas o52 pendiente quedan superados por esta revisión. Las38 capturasSDL añadidas en las ocho salas del valle y80 cambiosF5/F6 están contrastados. Sigue pendiente el capítulo posterior al ventilador, empezando por el regreso aBenn y su probador.
