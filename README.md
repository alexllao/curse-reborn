# Curse Reborn

**Volver a Enchantia. Recuperar el juego original. Darle una nueva vida.**

Curse Reborn es un port creado por **Àlex Llaó**. Nace de un recuerdo muy concreto: _Curse of Enchantia_ fue el primer juego al que jugó en un **PC 286**. Este proyecto es una forma de regresar a aquella aventura, entender cómo funcionaba y construir una base que permita disfrutarla y remasterizarla con cuidado.

La primera meta es **desensamblar `CURSE.EXE`, entender sus rutinas y trasladar su lógica a C++ con SDL**. Los recursos originales acompañarán esa reconstrucción. Sobre esa base queremos desarrollar una remasterización que conserve su identidad y abra la puerta a mejoras gráficas, sonoras y de uso.

**[Consultar todas las teclas y controles](documentacion/controles.md)**: juego, volumen, sonido, cambios de pantalla y depuración.

## El proyecto que queremos construir

El trabajo se plantea en dos etapas:

1. **Reconstrucción del original desde su código.** Desensamblar el ejecutable, seguir sus llamadas y saltos, identificar sus rutinas y trasladarlas progresivamente a C++ con SDL. Los formatos de datos se investigarán siguiendo las rutinas originales de carga.
2. **Remasterización.** Incorporar nuevos recursos y mejoras sobre un motor cuyo funcionamiento ya se haya comprobado frente al original.

La fidelidad será el punto de partida: escenarios, personajes, animaciones, inventario y puzles deben entenderse antes de transformarse. La intención es poder distinguir el comportamiento original de las mejoras que se introduzcan más adelante.

Cada parte trasladada deberá conservar una referencia a su dirección en el ejecutable, una explicación de su función y las comprobaciones realizadas. SDL sustituirá las dependencias de DOS y del hardware una vez identificado el comportamiento que deben conservar.

## Estado actual

Los **decorados ampliados están aparcados** por decisión de Àlex: Frank y el pozo conservan sus gráficos originales. Los PNG se guardan como referencia para una revisión futura; el catálogo activo está vacío. Mayús+F1 informa de esta situación y no añade ilustraciones. Los marcos exteriores F1 y el monitor continúan disponibles. Detalles en [Recursos propios](assets/README.md); `pnpm comprobar:decorados` verifica que las escenas permanezcan originales.

**Recorrido reconstruido en C++ y SDL desde la mazmorra, lago y cuevas hasta el asaltante, la compra de comida y el transporte del mago de Paso del Dragón. Hay veintiséis vistas de desarrollo; se han conectado las dos calles de la fuente, la tienda abandonada y la posada. Otras ramas y el resto del juego siguen pendientes. Véase la [auditoría de rutas y pendientes](documentacion/auditoria_pantallas.md). Música y efectos comparten el sistema de audio común.**

El propietario ha confirmado el [generador de valores acotados](documentacion/generador_aleatorio.md) en **18.816 llamadas**, la [preparación integrada de resultados](documentacion/preparacion_resultados.md) en **2.048 operaciones** y el [avance de acciones y movimiento](documentacion/avance_acciones_movimiento.md) en **10.930 casos**. También ha confirmado los **3.028 casos** de [creación y actualización de entidades](documentacion/entidades.md). El propietario ha confirmado también los **8.924 casos** de [secuencias de animación](documentacion/secuencias_animacion.md).

El traslado de [operaciones de sonido y modelos](documentacion/operaciones_secuencias.md) selecciona órdenes para las cinco modalidades observadas y permite atender sonido y carga mediante servicios explícitos. El propietario ha confirmado sus **1.285 casos**. La mazmorra ya aporta la carga de modelos desde archivos; los servicios de audio están conectados a SDL.

El [reproductor del altavoz del PC](documentacion/altavoz_pc.md) ya traslada arranque, parada y avance de notas, incluyendo su conexión con el selector y la animación. El propietario ha confirmado sus **3.312 casos**. Los efectos ya generan PCM y se escuchan mediante una salida SDL compartida; la música de zona [MT-32](documentacion/musica_mt32.md) ya está integrada mediante Munt; AdLib mantiene su tema continuo original y los [efectos digitales Sound Blaster](documentacion/efectos_digitales.md) ya están integrados.

Las [copias gráficas y la paleta](documentacion/video_planos.md) ya están trasladadas a C++ y el propietario ha confirmado sus **1.303 casos** de comparación amplia. Se conserva la primera imagen de diagnóstico obtenida con la descompresión original como referencia.

El [descompresor RNC de método uno](documentacion/descompresion_rnc.md) ya está trasladado a C++ y el propietario ha confirmado sus **142 casos** de comparación amplia. **`pnpm titulo` genera la pantalla de título íntegramente con el núcleo C++**, sin ejecutar `CURSE.EXE` ni Unicorn. La imagen coincide con la referencia anterior.

**`pnpm ventana` recorre Core → título → introducción original → mazmorra inicial**. Los fundidos de salida conservan los 32 pasos de paleta del original. Una tecla o el botón izquierdo inicia el fundido; F6 avanza y F5 retrocede directamente entre estas vistas de desarrollo. Escape cierra fuera de la mazmorra; dentro abre/cierra el menú. La imagen se ajusta al redimensionar o maximizar conservando sus proporciones, con bandas negras. **Alt + Enter** alterna ventana y pantalla completa sin bordes; **F1** muestra u oculta el marco decorativo de Àlex con un zoom y un fundido suave de 250 ms; comienza desactivado. Arranca sin filtros y con vecino más próximo; **F11** alterna original, suavizado, líneas de escaneo, CRT suave, xBRZ 2x y 2xSaI. **F12** muestra u oculta un contador pequeño de FPS en la esquina superior derecha; comienza visible.

El [montaje de fondos y sprites](documentacion/arranque_escena.md), incluido RNC de método dos, coincide en una muestra de **102 casos**; los [fundidos originales](documentacion/arranque_escena.md) coinciden en **64 paletas** adicionales. La nueva [carga, composición y selección de objetos](documentacion/objetos_mazmorra.md) tiene confirmados por el propietario sus **773 casos/pasos** de comparación amplia, incluidas 257 composiciones y 256 pasos persistentes. La comprobación SDL compara los **64.000 píxeles** del estado inicial y de los objetos tras 32 pasos. Ya están conectados el [menú y las primeras interacciones](documentacion/menu_e_interacciones.md). Ya se puede [caminar en la mazmorra](documentacion/movimiento_y_colisiones.md) tras la liberación, con clic o flechas y colisiones originales. Ya están conectados [el corredor y el regreso](documentacion/corredor_y_camara.md), y [la caída con llegada al agua](documentacion/caida_y_agua.md). La ruta continúa por [la salida al campo](documentacion/salida_al_campo.md); el [camino del asaltante](documentacion/camino_del_asaltante.md) y los [puzles de comida y mago](documentacion/puzles_poblacion.md) continúan por Paso del Dragón. La cadencia en un monitor real y el dibujante acelerado quedan para comprobación manual.

La mazmorra lee las tablas y guiones del EXE como datos, con huella verificada, y ejecuta la lógica traducida a C++. No ejecuta instrucciones DOS. Los originales siguen siendo archivos locales excluidos de Git.

El lago permite ponerse la pecera, liberar al pez, recoger la moneda, entregar el gusano a Mr. Fish, recoger la concha, cruzar las anguilas sobre la tortuga y recoger y usar la picana contra el tiburón. Hablar cerca de Mr. Fish activa su reacción original. La ruta continúa con la almeja, el tapón y la puerta secreta de la poza hacia las cuevas. La auditoría de opciones incluye las listas disponibles en sus estados de referencia. La música MT-32 y los efectos digitales Sound Blaster o PC Speaker comparten dispositivo. AdLib conserva el tema continuo del ejecutable; los [efectos digitales Sound Blaster](documentacion/efectos_digitales.md) ya están integrados. F11 permite probar filtros visuales opcionales. F1 utiliza los [marcos propios de estilo pixel art](assets/README.md), separados de los futuros recursos del remaster.

Las [piedras y el picapedrero](documentacion/cuevas_y_piedras.md) ya permiten obtener la cuerda desde la partida inicial: doce recogidas, dos entregas y regresos persistentes. La muestra compara **16.204 casos y pasos** con el EXE; cuatro capturas SDL coinciden hasta la cuerda. Se conservan las cuatro piedras de cada tamaño que pide este ejecutable. El pozo y el resto de este tramo continúan en el bloque final de las cuevas. El [recorrido en castellano](documentacion/recorridos/05_piedras_y_cuerda.txt) explica la ruta.

Ya existe una herramienta propia que lee la estructura DOS MZ y sigue llamadas y saltos desde el punto de entrada, tres manejadores identificados, seis tablas estáticas y las fuentes de una tabla dinámica de acciones. La pasada actual alcanza **18.798 instrucciones candidatas y 113 destinos de llamada directa**, y señala **dos transferencias indirectas pendientes**, que encadenan vectores de DOS. Estas cifras se obtienen por análisis estático; no representan el porcentaje completado del motor ni una ejecución verificada del juego.

El [mapa inicial del ejecutable](documentacion/analisis_ejecutable.md) recoge el arranque y las reubicaciones. El [análisis del bucle central](documentacion/bucle_principal.md) documenta su recorrido y la selección de escenas. El constructor de la [tabla dinámica de acciones](documentacion/tabla_acciones.md), la selección de coincidencias y la interpretación del [retorno de acciones](documentacion/contrato_acciones.md) pasan **1.120 comparaciones contra las instrucciones originales** ejecutadas en un emulador de CPU. Además, hay **25 comprobaciones** del analizador con programas sintéticos que no necesitan el juego.

El objetivo es ampliar ese C/C++ hasta disponer de un motor portable. Se conserva un [listado de ensamblador comentado](documentacion/ensamblador_comentado.md) como referencia, con 243 anotaciones propias y enlaces a los bloques trasladados. El núcleo se mantiene independiente de SDL. La [capa SDL](documentacion/ventana_sdl.md) se ha compilado y comprobado en Windows; macOS y la futura compilación web mediante Emscripten siguen pendientes de validar e integrar.

Se ha realizado una primera inspección estática de una copia para DOS:

| Elemento                | Resultado comprobado                                                                                                |
| ----------------------- | ------------------------------------------------------------------------------------------------------------------- |
| Archivos examinados     | 196, incluidos el juego, el instalador y los datos                                                                  |
| Ejecutable principal    | `CURSE.EXE`, formato DOS MZ de 16 bits, 111.606 bytes                                                               |
| Recursos separados      | Archivos `.MAP`, `.SPR`, `.BRD`, `.DAT`, `.PAL` y `.SFX`, entre otros                                               |
| Compresión identificada | RNC de métodos 1 y 2; método 1 trasladado y comparado con `CORE.DAT`, `MENU.DAT` y `TITLE.DAT`                      |
| Pantalla de título      | Imagen de 320 × 200 generada por C++ y contrastada con la descompresión original, las copias por planos y la paleta |

Estas observaciones permiten comenzar el trabajo, pero todavía falta determinar la estructura de buena parte de los recursos y reconstruir las rutinas del juego. Las extensiones y los nombres de archivo son pistas; su significado debe confirmarse mediante análisis.

Desensamblar el ejecutable no devuelve el código fuente original. Será necesario interpretar instrucciones, seguir referencias y contrastar cada parte reconstruida con el comportamiento del juego en DOSBox.

## Hoja de ruta

- [x] Inspeccionar inicialmente el ejecutable y el inventario de archivos.
- [x] Crear el repositorio y establecer la rama `development`.
- [x] Crear una herramienta de desensamblado por flujo y validación de cabeceras MZ.
- [x] Documentar el primer mapa de arranque y los manejadores de interrupción identificados.
- [x] Incorporar cinco tablas de saltos anotadas y vinculadas a la huella del ejecutable.
- [x] Localizar y documentar el recorrido estático del bucle central y la selección de escenas.
- [x] Reconstruir la construcción de la tabla dinámica de acciones y anotar sus fuentes de destinos.
- [x] Trasladar el primer bloque a C++ con referencia a su dirección y comparar sus escrituras con las instrucciones originales.
- [x] Trasladar la selección de seis resultados y la decisión del retorno de acciones; contrastar los tres bloques en 1.120 casos.
- [x] Conservar comentarios de ensamblador y referencias a C++ al regenerar el análisis.
- [x] Conectar la acción completa con su planificación y trasladar listas, direcciones y avance de movimiento; muestra de 421 casos coincidente.
- [x] Confirmar con el propietario la coincidencia de los 10.930 casos de acciones y movimiento.
- [x] Trasladar creación, catorce comportamientos y recorrido de entidades; muestra de 374 casos coincidente.
- [x] Confirmar con el propietario los 3.028 casos de entidades.
- [x] Trasladar el avance de tres canales de secuencias, dimensiones y retirada; muestra de 462 casos coincidente.
- [x] Confirmar con el propietario los 8.924 casos de secuencias.
- [x] Trasladar selección de sonido y servicios opcionales de las secuencias; muestra de 245 casos coincidente.
- [x] Confirmar con el propietario los 1.285 casos de operaciones.
- [x] Trasladar arranque, parada y temporizador del altavoz del PC; muestra de 310 casos coincidente.
- [x] Confirmar con el propietario los 3.312 casos del altavoz.
- [x] Trasladar copias por planos y envío de paleta; muestra de 83 casos y referencia visual del título comprobadas.
- [x] Confirmar con el propietario los 1.303 casos de vídeo.
- [x] Trasladar RNC de método uno y generar el título con C++; muestra de 38 casos coincidente.
- [x] Confirmar con el propietario los 142 casos de RNC.
- [x] Conectar el título a SDL y comprobar sus 64.000 píxeles con dibujante de software y cierre por Escape.
- [x] Trasladar RNC de método dos, el montaje de fondos MAP y los sprites con transparencia, espejo y recorte; muestra de 102 casos coincidente.
- [x] Trasladar los fundidos de paleta del arranque; 64 pasos contrastados contra el DAC original.
- [x] Conectar Core, título y vista inicial con F5/F6; conservar los fundidos originales en el avance normal.
- [ ] Confirmar la comparación amplia de la presentación con el propietario.
- [x] Cargar la lista original de entidades y superposiciones y conectarla a SDL.
- [x] Integrar secuencias, comportamientos y rutina de mazmorra; comprobar 32 pasos y sus composiciones.
- [x] Trasladar la selección de objetos por proximidad y compatibilidad de acción.
- [x] Confirmar con el propietario los 773 casos/pasos y las animaciones de objetos.
- [x] Conectar el menú original, las primeras interacciones y la depuración F7–F10; muestra de 91 casos/pasos.
- [x] Contrastar los 307 casos/pasos de menú tras corregir la disposición de bancos del comparador.
- [ ] Confirmar con el propietario la prueba visible del menú y sus controles.
- [x] Conectar clic, flechas, BOXDET, corrección de colisiones e historial y corregir el despacho que desplazaba la puerta.
- [x] Conectar mazmorra y corredor con regreso persistente, cámara y caída al agua.
- [x] Recoger la pecera y atravesar el corredor hasta la caída, conservando el inventario.
- [ ] Completar los puzles del lago y las demás habitaciones.
- [x] Preparar un exportador de referencias WAV sin pérdida para el futuro remaster sinfónico con IA, conservando las versiones originales.
- [ ] Confirmar manualmente ventana visible, maximización y cadencia de los fundidos.
- [ ] Completar los destinos indirectos restantes y las estructuras de estado del juego.
- [ ] Validar el recorrido identificado durante la ejecución del original.
- [ ] Seguir las rutinas originales de carga y descompresión de recursos.
- [ ] Documentar los formatos de escenarios, sprites y animaciones a partir de esas rutinas.
- [ ] Ampliar el traslado a rutinas completas, con sus contratos de entrada y salida documentados.
- [ ] Sustituir el acceso gráfico al hardware por SDL y comprobar el resultado.
- [ ] Trasladar los controles, el movimiento y las interacciones.
- [ ] Trasladar el inventario, los eventos y los puzles.
- [x] Conectar efectos digitales Sound Blaster y PC Speaker con música AdLib y MT-32.
- [ ] Completar las demás temporizaciones y las partidas guardadas.
- [ ] Comprobar el desarrollo completo de la aventura frente al original.
- [ ] Definir y desarrollar las mejoras de la remasterización.
- [ ] Al crear el HUD propio, mostrar brevemente el nombre de la pantalla cada vez que se acceda a ella, incluidos los regresos. Usar los nombres del catálogo de pantallas; duración y presentación visual pendientes de decidir.
- [ ] Reutilizar el estilo del HUD propio en las opciones dentro del juego: placas y botones con bordes metálicos, relieve, grises y detalles rojizos, acordes con la interfaz original. Compartir el estilo con los avisos de entrada a pantalla; distribución y comportamiento de las opciones pendientes de diseñar.
- [ ] Diseñar una ayuda opcional con pistas progresivas por pantalla: orientación, pista concreta y solución bajo petición. Consultar el estado de los puzles y el inventario para evitar pistas ya resueltas; basarse en la guía aportada y contrastarla con la lógica original. Pendiente de definir su acceso y aspecto dentro del juego; todavía sin implementar.

**Siguiente hito de reconstrucción:** conectar el menú original y los efectos de sus acciones sobre objetos y personaje, conservando las condiciones del puzle para pasar de la escena animada a una mazmorra jugable.

## Organización del repositorio

El proyecto se organiza como un **monorepositorio**: el motor, las herramientas de investigación, la documentación y las pruebas comparten este repositorio y su historial. Los bloques aislados e integrados conservan contratos, referencias al ejecutable y comparaciones propias. Las carpetas vacías conservan un archivo `.gitkeep`. Todavía no hay una compilación del juego completo; los comandos de comparación compilan únicamente los bloques trasladados.

```text
curse-reborn/
├── motor/              # Núcleo C++ reconstruido, independiente de SDL
├── plataformas/        # Presentación y acceso a recursos mediante SDL
├── herramientas/       # Análisis del ejecutable y apoyo a su reconstrucción
├── documentacion/      # Mapa de código, formatos y decisiones del proyecto
├── pruebas/            # Comprobaciones del motor y las herramientas
├── licencias/          # Avisos de las dependencias, sin asignar licencia al código propio
└── datos_originales/   # Copia local del juego; solo se versiona .gitkeep
```

| Archivo                          | Contenido                                                    |
| -------------------------------- | ------------------------------------------------------------ |
| [README.md](README.md)           | Propósito, estado y hoja de ruta                             |
| [AGENTS.md](AGENTS.md)           | Instrucciones para asistentes que trabajen en el repositorio |
| [.gitignore](.gitignore)         | Exclusiones de archivos locales y recursos originales        |
| [package.json](package.json)     | Gestión del proyecto y comandos de pnpm                      |
| [pnpm-lock.yaml](pnpm-lock.yaml) | Archivo de bloqueo de pnpm                                   |

## Puesta en marcha con pnpm

Se utiliza **Node 22.13 o posterior y pnpm 11.18.0**. La versión de Node está indicada en `.nvmrc` y el gestor en `package.json`. El análisis utiliza además Python 3.9 o posterior, disponible como `py`, `python` o `python3`. Si necesitas seleccionar un ejecutable concreto, indica su ruta en la variable de entorno `PYTHON`.

Para mantener el código uniforme: `pnpm formatear` aplica Prettier, clang-format 21 y Ruff; `pnpm calidad` comprueba el formato y ejecuta ESLint y Ruff. Consulta la [preparación y el alcance de estas herramientas](documentacion/calidad_codigo.md). Cada función nueva llevará un comentario breve sobre su propósito y, cuando proceda, su dirección en el ejecutable original.

Desde la raíz del repositorio:

```sh
pnpm install --frozen-lockfile
pnpm preparar
pnpm estado
pnpm analizar
pnpm comprobar
```

| Comando                     | Qué hace                                                                                                                                                                    |
| --------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `pnpm preparar`             | Crea un entorno Python en `trabajo/entorno/` e instala la versión fijada del desensamblador. Necesita acceso a la red la primera vez.                                       |
| `pnpm estado`               | Muestra la disponibilidad del ejecutable, el entorno local y las dependencias. Si falta preparar el entorno, termina con un aviso y un código de error.                     |
| `pnpm analizar`             | Analiza `datos_originales/CURSE/CURSE.EXE`; guarda los resultados en `trabajo/analisis_ejecutable/`. Requiere la copia original.                                            |
| `pnpm ensamblador`          | Regenera el mismo análisis y su listado `desensamblado.asm`, conservando los comentarios del mapa.                                                                          |
| `pnpm comprobar`            | Ejecuta las comprobaciones con programas sintéticos, sin necesitar los archivos del juego.                                                                                  |
| `pnpm comparar:acciones`    | Compila la biblioteca C++ y compara tres bloques de acciones con las instrucciones originales en 1.120 casos. Requiere el juego y Clang con LLD en Windows de 64 bits.      |
| `pnpm comparar:aleatorio`   | Compara 18.816 llamadas al generador; el propietario ya ha confirmado su coincidencia.                                                                                      |
| `pnpm comparar:resultados`  | Compara las 2.048 operaciones del tramo integrado de preparación; el propietario ha confirmado su coincidencia.                                                             |
| `pnpm comparar:avance`      | Compara 10.930 casos de acciones completas y movimiento; el propietario ha confirmado su coincidencia. Admite `--muestra` con 421 casos.                                    |
| `pnpm comparar:entidades`   | Compara 3.028 casos de creación y comportamientos de entidades; el propietario ha confirmado su coincidencia. Admite `--muestra` con 374 casos.                             |
| `pnpm comparar:secuencias`  | Compara 8.924 casos de canales y retirada; el propietario ha confirmado su coincidencia. Admite `--muestra` con 462 casos.                                                  |
| `pnpm comparar:operaciones` | Compara 1.285 casos de selección de sonido y servicios de modelos; el propietario ha confirmado su coincidencia. Admite `--muestra` con 245 casos.                          |
| `pnpm comparar:altavoz`     | Compara 3.312 casos del reproductor del altavoz; el propietario ha confirmado su coincidencia. Admite `--muestra` con 310 casos.                                            |
| `pnpm comparar:video`       | Compara 1.303 casos de copias por planos y paleta, confirmados por el propietario. Admite `--muestra` con 83 casos. Genera la referencia `trabajo/vista_titulo/titulo.png`. |
| `pnpm comparar:rnc`         | Compara 142 casos del descompresor de método uno, confirmados por el propietario. Admite `--muestra` con 38 casos.                                                          |
| `pnpm titulo`               | Genera `trabajo/vista_titulo/titulo_cpp.png` con descompresión, copias y paleta en C++, sin ejecutar `CURSE.EXE` ni Unicorn. No abre una ventana ni inicia una partida.     |
| `pnpm comparar:graficos`    | Compila una vez y agrupa los 1.325 casos de vídeo y RNC, confirmados por el propietario. Con `--muestra` ejecuta 121 casos.                                                 |
| `pnpm ventana`              | Prepara SDL en Windows si falta, compila y abre el título calculado por C++. Escape o cerrar terminan la aplicación.                                                        |
| `pnpm sdl:preparar`         | Descarga y verifica el paquete oficial SDL3 fijado para Windows de 64 bits.                                                                                                 |
| `pnpm sdl:compilar`         | Compila la presentación sin abrir una ventana.                                                                                                                              |
| `pnpm comprobar:ventana`    | Captura el título dibujado por SDL en software, compara los 64.000 píxeles y comprueba el cierre por Escape.                                                                |

La gestión general del proyecto se hace con Node y pnpm; Python queda integrado para el análisis con Capstone. No hace falta activar manualmente el entorno Python. La preparación es explícita: instalar con pnpm no descarga dependencias de Python ni ejecuta el juego.

El nuevo comando `pnpm comparar:presentacion` agrupa RNC2, fondos, sprites y fundidos originales. `pnpm comparar:escena` agrupa RNC2, fondos y sprites; `pnpm comparar:rnc2` los comprueba por separado. `pnpm comprobar:escena` contrasta además los píxeles de la mazmorra dibujados por SDL. Consulta [el alcance de estos comandos y sus límites](documentacion/arranque_escena.md).

`pnpm comparar:objetos` agrupa la carga de entidades, rutina particular, orden, selección, composición y avance persistente. Su comparación amplia de **773 casos/pasos** está confirmada por el propietario. `pnpm comprobar:escena` utiliza ahora la muestra de objetos y compara también su animación. Consulta [los datos necesarios y el alcance actual](documentacion/objetos_mazmorra.md).

Para comprobar los bloques trasladados, ejecuta `pnpm preparar` y después `pnpm comparar:acciones`. El entorno incorpora Unicorn para ejecutar los tramos originales aislados. Consulta [los requisitos de compilación](documentacion/tabla_acciones.md) y [el alcance de las comparaciones](documentacion/contrato_acciones.md).

Puedes pasar opciones a la herramienta de análisis, por ejemplo `pnpm analizar --raiz 0000:0276`. Los comandos de gestión contemplan Windows, Linux y macOS; por ahora se han comprobado en Windows. La primera pantalla SDL se compila con `pnpm sdl:compilar` y se abre con `pnpm ventana`. Consulta [sus requisitos y el alcance de la comprobación](documentacion/ventana_sdl.md). Todavía no inicia una partida.

### Introducción original

La [introducción de la bruja](documentacion/introduccion_original.md) ya recorre `CAULDRN`, `BALCONY` y `BASEBAL`/`BASEBAT` desde el bloque original `28B2`. Conserva las cargas, páginas, paletas, controles de omisión y fundidos. Una pulsación en el título inicia la introducción; otra durante ella solicita omitirla en el siguiente control original. Al terminar entra en la mazmorra. F5/F6 recorren Core, título, introducción, mazmorra, corredor, caída y lago como vistas de desarrollo. La música MT-32 comienza con CURSE.RS5 antes de Core y cambia según la tabla original de cada escena; los efectos digitales Sound Blaster están activos en las escenas jugables y F3 permite volver a PC Speaker.

`pnpm comparar:introduccion` contrasta el recorrido y las imágenes contra el ejecutable; `pnpm comprobar:introduccion` compara capturas SDL del caldero, balcón, béisbol y llegada a la mazmorra al terminar y al omitir. Las esperas usan retrazos nominales a 70 Hz; la duración de la E/S DOS y la sincronización física de monitor no se reproducen.

### Menú y depuración

En la mazmorra, Escape o botón derecho abre/cierra el menú. El ratón y el botón izquierdo seleccionan; izquierda/derecha y Enter también funcionan. Para poner en marcha la primera secuencia, elige el bocadillo de conversación (quinto icono) y la segunda casilla de su submenú. Se mantienen las condiciones originales de disponibilidad.

**F7** activa/desactiva la depuración. **F8** alterna cajas de entidades; **F9**, zonas de interacción; **F10**, encuadre, umbrales y rectángulos BOXDET. Verde: objetos; cian: Brad; magenta: zonas disponibles por afinidad; naranja: proximidad de los pies; amarillo: límites del encuadre. Son datos de diagnóstico, sin alterar la partida. F10 muestra además las paredes de BOXDET en amarillo y las zonas de salida en azul. Tras quitarse las esposas se camina con clic izquierdo o flechas; la puerta conserva su posición original.

`pnpm comparar:jugabilidad` contrasta el movimiento y la liberación; `pnpm comprobar:jugabilidad` comprueba sus dos capturas SDL.

`pnpm comparar:menu` agrupa las comparaciones de menú; `pnpm comprobar:menu` verifica panel, primera interacción y colores de depuración en SDL. Consulta [rutinas, alcance y controles](documentacion/menu_e_interacciones.md).

### Recorrido de la mazmorra

Ya se puede empujar la pared, recoger el clip y desbloquear la puerta con él. Se mantienen las condiciones y los rechazos originales, incluida la recogida del objeto de puntuación. La puerta permite entrar al corredor y regresar con el estado conservado. Ya se recoge la pecera y se atraviesan las estatuas y los guardias hasta la puerta izquierda, la caída y el lago.

[Guía paso a paso en texto](documentacion/recorridos/00_mazmorra.txt). Se ampliará al validar cada pantalla. Las instrucciones usan las casillas reales del menú y el mismo orden que la comparación automática.

`pnpm comparar:puzle` contrasta las acciones correctas e incorrectas y el recorrido persistente hasta la puerta abierta; `pnpm comprobar:puzle` comprueba también su captura SDL. [Evidencias, bloques y límites](documentacion/puzle_mazmorra.md).

### Dónde colocar el juego original

Conserva la distribución de tu instalación de DOSBox dentro de `datos_originales/`, con el juego en `CURSE/` y las ROM del MT-32 en `ROMS/`:

```text
datos_originales/
├── .gitkeep
├── CURSE/
│   ├── CURSE.EXE
│   ├── CURSE.CFG
│   ├── SETUP.EXE
│   └── DATA/
│       └── ...
└── ROMS/
    └── ...                  # ROM del MT-32 de tu instalación
```

En el ZIP examinado ya existe la carpeta `CURSE/`: cópiala completa a `datos_originales/`. Si utilizas ROM del MT-32, conserva su carpeta `ROMS/` al mismo nivel. El desensamblado estático no necesita las ROM y todavía no hay integración del MT-32 en el motor. Conserva los nombres originales de los archivos y las subcarpetas.

Git conservará únicamente el archivo vacío `.gitkeep`. El resto del contenido de `datos_originales/`, incluidas sus subcarpetas, queda excluido para que puedas trabajar con tu copia local sin publicarla.

## Desarrollo en castellano

La rama de trabajo es **`development`**.

La documentación, los comentarios del código, los textos propios de las herramientas y los mensajes de los cambios se escribirán en castellano. También se usarán nombres en castellano para los elementos propios del proyecto cuando sea viable. Se conservarán los identificadores exigidos por C++, SDL y otras dependencias, así como los nombres originales necesarios para leer los datos del juego.

Los avances se documentarán con claridad: qué se ha comprobado, qué sigue siendo una hipótesis y qué puede ejecutarse realmente. Cada función reconstruida deberá poder contrastarse con el original; las mejoras de la remasterización se identificarán como tales.

## Corredor, caída y siguiente bloque

`pnpm comprobar:corredor` compara ocho capturas SDL: entrada, mecanismo, regreso, pecera, golpe de estatua, caída y llegada al agua, también con audio activo en un dispositivo de prueba. Contrasta el recorrido con el ejecutable original, incluidos los golpes, la recuperación del control y el inventario persistente; conserva además las pruebas de cámara y sus 18 posiciones gráficas originales.

`pnpm comprobar:lago` prolonga la partida desde la mazmorra hasta ponerse la pecera, liberar al pez azul, recoger la moneda, descubrir y recoger el gusano y entregarlo a Mr. Fish. Continúa con concha, transporte sobre las anguilas, picana, tiburón, salto de la almeja y apertura del tapón hasta emerger en la cueva. Contrasta memoria y guiones con el EXE y doce capturas SDL de 64.000 píxeles, incluida la reacción al hablar junto a la tienda. `pnpm comparar:lago` amplía las ramas heredadas del corredor. Véanse [rutinas y límites del lago](documentacion/acciones_lago.md) y [recorrido](documentacion/recorridos/03_lago.txt). Quedan pendientes otras combinaciones del menú del lago; validar una solución no equivale a completar toda la escena. La poza continúa con algas, botón y puerta secreta hasta el corredor de las cuevas: `pnpm comparar:poza --muestra` contrasta 7.996 casos y pasos, y `pnpm comprobar:poza` añade cuatro capturas SDL. Véase el [recorrido de la poza](documentacion/recorridos/04_poza.txt). El recorrido continúa con [las últimas acciones de las cuevas](documentacion/final_cuevas.md).

`pnpm comprobar:caida` compara dos momentos de `DROP` y la entrada sumergida en `LAKEBED`. Desde la mazmorra, **F6 abre el corredor** y otro F6 abre la caída; F5 retrocede por las escenas integradas. Cada salto prepara una sesión nueva. La caída avanza sola hasta el agua. Estos atajos no resuelven puzles de una partida ni conservan la sesión anterior.

[Guía del corredor](documentacion/recorridos/01_corredor.txt) · [Guía de la caída](documentacion/recorridos/02_caida.txt). El **audio compartido** ya incluye PC Speaker, efectos digitales Sound Blaster y música AdLib OPL2, OPL3 y MT-32. Los efectos PC Speaker ya siguen las órdenes, prioridades y secuencias originales, con una salida compartida entre escenas. La música de zona [MT-32](documentacion/musica_mt32.md) ya está integrada mediante Munt; AdLib mantiene su tema continuo original y los [efectos digitales Sound Blaster](documentacion/efectos_digitales.md) ya están integrados.

## Recursos originales y autoría

El creador del port **Curse Reborn** es **Àlex Llaó**. Contacto: [alexllao@me.com](mailto:alexllao@me.com).

_Curse of Enchantia_ es una obra de Core Design. Curse Reborn es un proyecto independiente y no oficial, y reconoce el trabajo de quienes crearon el juego original.

El repositorio no incluye el ejecutable ni los archivos comerciales del juego. Para las pruebas con recursos originales será necesario aportar una copia propia. Los archivos del juego, los recursos extraídos y las partidas deben permanecer fuera del historial del repositorio.

La licencia del código nuevo está pendiente de definir. Antes de incorporar código de terceros se revisarán su licencia y sus requisitos de atribución.

---

Este proyecto empieza con el deseo de volver al primer juego de ordenador de su creador y darle un futuro sin perder aquello que lo hizo especial.

La solución de referencia y el seguimiento por tramos están en [referencia de solución](documentacion/referencia_solucion.md), a partir de la guía de Sophie Cheshire aportada por Àlex.

## Exportar música para preparar el remaster

`pnpm exportar:midi` reúne las cinco pistas originales `CURSE.RS1`–`CURSE.RS5` y sus versiones MIDI estándar en `trabajo/referencias_musicales/`. Conserva mensajes, tempos y marcadores `LOOPSTART`/`LOOPEND`; `pistas.json` documenta los tiempos y las huellas, y `LEEME.txt` explica su uso. Los programas son de Roland MT-32: un sintetizador General MIDI puede reproducir instrumentos distintos. No necesita las ROM para esta conversión. Estos archivos son referencias locales para reorquestar y no se publican en Git.

`pnpm exportar:musica` genera nueve WAV locales con **duración tomada de los guiones originales**: cinco bancos MT-32 y dos temas internos FM con OPL2 y OPL3. En MT-32 incluye la entrada y termina en el primer salto de repetición; en FM espera a que todos los canales hayan completado su primer recorrido, porque sus bucles son independientes. Se guardan en `trabajo/exportacion_musica/`, con `pistas.json` que identifica archivos, duración, canales y huellas. Requiere los recursos originales y las ROM locales para MT-32.

Para forzar un corte de prueba: `pnpm exportar:musica --segundos 180` (entre 1 y 600). Si no puede identificar el recorrido, la herramienta informa del error; no lo sustituye por un corte arbitrario.

La herramienta reproduce las pistas con los sintetizadores del port, sin efectos del juego ni el volumen amplificado de la partida. La exportación automática conserva la entrada y un recorrido de sus bucles; la duración forzada con `--segundos` puede acabar a mitad de uno. El tema FM cero se exporta como material auxiliar: el original arranca con el uno. Los originales y las futuras versiones sinfónicas deben conservarse separados. No se incluyen WAV, recursos ni ROM en Git.

## Sonido y vistas de desarrollo

`pnpm comprobar:digital` compara **134 efectos**, **3.442.325 muestras**, **140 programaciones DSP/DMA** y las **78 referencias de bancos**, y después comprueba su integración SDL. Admite `--muestra` con 31 efectos. `pnpm comparar:digital` ejecuta solo el contraste del motor. Los efectos siguen las órdenes originales de las acciones integradas; esta prueba no implementa puzles o habitaciones pendientes. [Funcionamiento y límites](documentacion/efectos_digitales.md).

`pnpm ventana` activa los efectos digitales originales de Sound Blaster. **F3** alterna con PC Speaker sin cambiar la música. **M** alterna el silencio; los guiones y el temporizador siguen avanzando. Los sonidos usan las secuencias del EXE, también con el menú abierto o durante un fundido. Si no hay dispositivo, la ventana continúa sin audio. Para desactivarlo desde el arranque: `pnpm ventana --sin-sonido`.

`pnpm comprobar:audio --muestra` agrupa la comparación original del altavoz, la nueva señal PCM y la apertura/alimentación de SDL con un dispositivo de prueba. `pnpm comparar:audio` amplía la comparación. El WAV de prueba se guarda localmente en `trabajo/comparacion_audio/efecto_original.wav`. Esta prueba no escucha ni valida los altavoces físicos. Véanse [detalles y límites](documentacion/altavoz_pc.md).

El siguiente bloque incorpora cuarto corredor, SEESAW y cueva del monitor: [catapulta, imán y alambre](documentacion/catapulta_y_alambre.md). Hay veintiséis vistas de desarrollo y el recorrido continúa por la salida al campo. `pnpm comparar:catapulta --muestra` contrasta el recorrido persistente y `pnpm comprobar:catapulta` añade seis capturas SDL.

El tramo final añade moneda, cuatro agujeros, ramitas, trampa y barro, premios del pozo, casco, máscara y salida por el cubo. `pnpm comparar:final-cuevas --muestra` contrasta 32.629 casos y pasos hasta COUNTRY1; `pnpm comprobar:final-cuevas` añade dieciséis capturas SDL. La auditoría `pnpm comparar:opciones` comprueba 5.642 selecciones en 47 estados, sin manejadores pendientes en sus listas disponibles. Esto delimita lo comprobado; no demuestra todos los estados posibles. Véase [final de las cuevas](documentacion/final_cuevas.md).

Guardar, cargar y borrar están disponibles desde el octavo icono del menú original; importar es una integración de escritorio por arrastre. Las ocho ranuras usan el formato CURSE.SAV, con 303 casos contrastados frente a sus instrucciones DOS. La importación añade ranuras libres y permite cargar zonas integradas, conservando el archivo de origen. Véanse [partidas y compatibilidad](documentacion/partidas.md). `pnpm comprobar:partidas` utiliza un perfil temporal para validar restauración, importación y borrado inmediato original. El icono de información muestra las cuatro fichas originales de puntuación y créditos.

`pnpm comparar:cuevas --muestra` prolonga la partida hasta la cuerda; `pnpm comprobar:cuevas` añade cuatro capturas SDL y los treinta y cuatro cambios de vista.

`pnpm comprobar:marcos` comprueba veintinueve capturas: cinco fases de zoom y fundido de 250 ms, apaisado, vertical, sin bandas, tamaño impar, xBRZ y el marco predeterminado en mazmorra, corredor y lago. F1 recupera la imagen anterior. El remate artístico interior se dibuja completo con su transparencia, más dos píxeles lógicos de solape adicional para dar profundidad; puede superponerse al juego y a la barra.

`pnpm comprobar:fps` prueba mostrar, ocultar y volver a mostrar los FPS a 1000 × 700. Comprueba que el juego y las bandas negras se conservan fuera de la placa. El HUD de mensajes de acciones está previsto para una fase posterior; el registro actual sigue en consola.

`pnpm comprobar:filtros` comprueba los escaladores C++ con colores planos, bordes y contornos conocidos. Simula seis pulsaciones de F11 y captura los seis modos y la vuelta al original a 960 × 600. Comprueba que los modos difieren, que el ciclo es reversible y que los 64.000 píxeles originales se conservan. Resultados locales en `trabajo/comprobacion_filtros/`. Véanse los [detalles de los filtros](documentacion/filtros_pixeles.md).

`pnpm comprobar:vistas` introduce cuarenta y cuatro cambios F5/F6 y verifica las veintidós vistas desde la mazmorra hasta el cuarto corredor, SEESAW, las cuevas del monitor y de los cuatro agujeros, el cubo y la salida al campo, incluido el regreso. Estos atajos reinician la sesión; para conservar el inventario hay que atravesar las salidas normales del juego.

La [música MT-32](documentacion/musica_mt32.md) reproduce los cinco bancos CURSE.RS y consulta las 78 referencias originales de escena. Con las ROM locales válidas se selecciona al arrancar. **F4** recorre sin música, AdLib OPL2, Sound Blaster 16 OPL3 y MT-32; **F3** elige por separado efectos digitales Sound Blaster o PC Speaker; omite MT-32 si no puede prepararlo. La [música AdLib](documentacion/musica_adlib.md) conserva sus nueve canales y el tema uno de forma continua, también durante las escenas; la selección de bancos RS por zona pertenece a MT-32. Los [efectos digitales Sound Blaster](documentacion/efectos_digitales.md) ya están integrados. La consola describe la opción efectiva. **M** silencia toda la mezcla y **+ / −** ajustan entre 0 y 200 % (100 % es el nivel normal). Se registran también las acciones del jugador, selecciones, inventario y cambios de escena, sin repetir mensajes por fotograma.

`pnpm comprobar:musica --muestra` compara el reproductor musical original y comprueba ocho segundos de síntesis y salida SDL de prueba. La comparación amplia queda disponible en `pnpm comparar:musica`.

**Volumen:** `+` y `−` ajustan el nivel general en pasos del 10 %, entre 0 y 200 %. También funcionan `=` y las teclas del teclado numérico. La consola muestra el porcentaje al arrancar y al cambiarlo. `M` silencia y recupera el ajuste elegido; cambiar de perfil no lo reinicia.

El último icono del menú de acciones muestra las cuatro fichas originales de información, puntuación y créditos con FONT.SPR. Clic izquierdo o Intro pasan de página; Escape cierra. Comprobación específica: `pnpm comprobar:informacion`. Consulta [partidas e información](documentacion/partidas.md).

La pantalla dieciséis, [salida al campo](documentacion/salida_al_campo.md), reproduce el izado por el monstruo, la caída de la máscara y el sendero hacia la población. `pnpm comprobar:campo` contrasta 722 casos/pasos y nueve capturas SDL. El [camino del asaltante](documentacion/camino_del_asaltante.md) continúa este recorrido; el [trayecto de comida y mago](documentacion/puzles_poblacion.md) continúa por la población.

Cada escena tiene un nombre en castellano compartido por la consola, el título de ventana y F5/F6. Consulta el [catálogo de pantallas](documentacion/pantallas.md), que distingue el orden de navegación del índice original DOS.

La pantalla diecisiete, [El camino del asaltante](documentacion/camino_del_asaltante.md), incluye tropiezo, recogida y uso de espada, ataque, dinero y sendero hasta DRAGON. `pnpm comprobar:asaltante` contrasta 3.229 casos/pasos y doce capturas SDL; F5/F6 recorre ahora veintidós vistas. El tramo siguiente incorpora los [puzles de la población](documentacion/puzles_poblacion.md).

La [ruta de Paso del Dragón](documentacion/puzles_poblacion.md) incluye guardias, saludo a Frank, compra y consumo de comida, portal, fuente y transporte del mago. `pnpm comprobar:poblacion` agrupa comparación contra el EXE y doce capturas SDL. `pnpm comprobar:calles` añade las dos calles laterales de la fuente, la tienda abandonada y la posada: 6.113 casos/pasos, nueve capturas SDL y 52 cambios F5/F6 entre 26 vistas. `pnpm auditar:rutas` inventaría los umbrales originales y las ramas todavía pendientes. La llegada al borde del mundo está cargada y detenida; las tiendas de las nuevas calles y los capítulos posteriores siguen pendientes. Véanse la [auditoría de pantallas](documentacion/auditoria_pantallas.md) y el [recorrido en castellano](documentacion/recorridos/10_paso_del_dragon.txt).

Los [paneles de partidas](documentacion/partidas.md) conservan ahora el diseño original: placa negra, FONT.SPR, ocho ranuras y teclado de nombres. Se han retirado los botones Aceptar/Cancelar/Importar y las confirmaciones añadidas. `pnpm comprobar:partidas` incluye303 casos del formato,23 composiciones originales y4 casos de nombres anteriores comparados por píxeles y el flujo SDL. La compatibilidad con perfiles anteriores de siete ranuras se conserva.

El marco opcional de monitor de PC 286 se selecciona con **F1**: predeterminado → monitor → sin marco. Incluye una habitación de época detrás y conserva el mismo tamaño máximo del juego que sin marco. La carcasa puede salir de la ventana cuando no cabe; nunca reduce ni tapa la escena. **Mayús+F1** sigue controlando los decorados ampliados por separado. **Ctrl+rueda** o **Ctrl+−/+** permiten alejar el monitor para ver más habitación; **Ctrl+0** recupera el máximo. Comprobación específica: `pnpm comprobar:monitor`.

El botón del monitor apaga el ordenador y lo deja apagado hasta otra pulsación. Al encender reproduce un arranque visual de BIOS 386 y DOS de siete segundos, sin saltarlo, antes de Core. Conserva los ajustes de pantalla y sonido y no modifica las partidas guardadas. Comprobación específica: `pnpm comprobar:reinicio`.

La población amplía su recorrido con Benn (escena24), la adivina (26) y la segunda casa abandonada (27), accesibles desde sus calles y con regreso persistente. F5/F6 ofrece ahora43 vistas. `pnpm comparar:tiendas` compara los nuevos pasos y las respuestas originales; `pnpm comprobar:tiendas` añade las capturas SDL y la navegación. El probador de Benn permite entrar y regresar; sus variantes del vestido están contrastadas, pero el capítulo posterior sigue pendiente; véase [alcance y recorrido](documentacion/tiendas_poblacion.md).

El port contabiliza el **tiempo efectivo de juego del perfil**, excluyendo presentaciones, paneles de partidas/información, segundo plano, minimizado y ordenador virtual apagado. Persiste en `tiempo_juego.json`, separado de las partidas DOS, e informa por consola al iniciar y cerrar. Inventario, decisiones y animaciones de juego cuentan. El total continúa aunque se cargue una partida antigua; todavía no se asigna a una ranura. Véanse [criterios y persistencia](documentacion/tiempo_juego.md). Comprobación: `pnpm comprobar:tiempo`.

El [probador de Benn](documentacion/probador_benn.md) incorpora36/27, rutina1A74 y contratos del vestido79. `pnpm comprobar:probador` contrasta el EXE, las dos capturas SDL y64 cambios F5/F6; la visita temprana no adelanta el progreso del valle.

El [borde del mundo y el loro](documentacion/borde_del_mundo.md) ya continúan en SDL: puzles de EDGE, entrada al loro, saludo inicial y regreso a la casa abandonada. F5/F6 ofrece43 vistas de desarrollo. `pnpm comparar:acciones-borde` contrasta12.229 casos/pasos y15 composiciones; `pnpm comparar:loro`,1.484 casos/pasos y seis composiciones. `pnpm comprobar:borde` recorre el mismo camino y compara16 capturas SDL, además de guardar/restaurar y64 cambios de vista. El [recorrido en texto](documentacion/recorridos/12_borde_del_mundo.txt) explica la solución comprobada. Quedan pendientes las continuaciones posteriores y la rama alternativa del mago; el número de vistas no significa habitaciones completas al100 %.

El [regreso del loro al mago](documentacion/regreso_loro.md) continúa con el segundo pago, el guardia y la salida52 aPOST. Las ocho escenas del valle32–39 están integradas enSDL: POST, REMOTE, CELESTE, DINO, CLOUD, GHETTO, HOLD yELECTRO. El recorrido principal obtiene el ventilador124 y regresa por69 al campo, conservando los cambios persistentes del capítulo. El [catálogo de pantallas](documentacion/pantallas.md) reúne las43 vistas y sus nombres.

La misma partida reproduce el [pelo y el dinosaurio](documentacion/dinosaurio.md), mando y cinta, [grabación y correo](documentacion/grabacion_valle.md), [pase](documentacion/entrega_grabacion.md), [nube y bolsa](documentacion/nube_del_valle.md), [detergente](documentacion/cueva_del_valle.md), [guardia](documentacion/guardia_del_barco.md), [puente y trapo](documentacion/puente_del_barco.md), [bandeja, plataforma y ventilador](documentacion/plataforma_del_valle.md). Las pruebasSDL recorren las acciones desde el comienzo; no preparan inventarios ni posiciones para saltar sus requisitos. Los recorridos de texto13–19 en `documentacion/recorridos/` describen la ruta contrastada.

| Sala o tramo                          | Comprobación SDL                  |
| ------------------------------------- | --------------------------------- |
| Encrucijada POST                      | `pnpm comprobar:poste`            |
| Mando REMOTE                          | `pnpm comprobar:remoto`           |
| Barco CELESTE                         | `pnpm comprobar:celeste`          |
| Dinosaurio DINO                       | `pnpm comprobar:dinosaurio`       |
| Grabación, correo, pase y CLOUD       | `pnpm comprobar:nube-valle`       |
| Cueva GHETTO                          | `pnpm comprobar:cueva-valle`      |
| Guardia y puente HOLD                 | `pnpm comprobar:barco-valle`      |
| Plataforma ELECTRO y regreso al campo | `pnpm comprobar:plataforma-valle` |

`comprobar:poste` prepara las referencias comunes del recorrido. Los modos siguientes vuelven a contrastar sus bloques nativos y reutilizan las referencias anteriores. El conjunto compara38 imágenesSDL del valle, guardar/restaurar en cada hito y80 cambiosF5/F6 entre40 vistas. Incluye restaurar llevando un tablón y recuperar aBrad de las dos caídas deELECTRO. `pnpm comparar:enlaces-valle --muestra` comprueba las dos decisiones originales del umbral70 y los retornos71, además del acceso55/56.

Estas cifras describen cobertura y estados comprobados. Los capítulos siguientes y todos los estados posibles de las salas reutilizadas siguen pendientes de contraste; no equivalen al100% del juego. Las mejoras opcionales se reunirán en un menú y quedarán desactivadas por defecto; esa unificación sigue pendiente. Los decorados ampliados permanecen aparcados.

El [regreso real aBenn](documentacion/regreso_benn.md) ya continúa después del ventilador: pago, vestido79 y probador hasta solicitar74 hacia la nieve. `pnpm comparar:regreso-benn --muestra` contrasta10.109 casos/pasos, siete imágenes y los menús del inventario que vuelve del valle; `pnpm comprobar:regreso-benn` añade seis imágenesSDL y guardar/restaurar. La sala43 no se carga todavía y el catálogo sigue40 vistas. Véase el [recorrido del vestido](documentacion/recorridos/20_vestido_y_puerta_de_la_nieve.txt).

La nieve continúa enSDL con la vista41, SNOWAST4: bola de nieve134 y ser azul. `pnpm comprobar:ser-azul` contrasta cuatro imágenes, restauración y82 cambiosF5/F6. [Alcance y pendientes](documentacion/entrada_nieve.md). La salida80 a42 y su regreso79 ya están habilitados. `pnpm comprobar:morsa` añade seis capturasSDL; [recorrido de nieve y límites](documentacion/nieve.md). Las otras dos vistas del capítulo se activan por separado.

La vista43 añade SNOWAST2, el agujero del pez y el esquimal. `pnpm comprobar:esquimal` verifica su entrada real, restauración y86 cambiosF5/F6; el recorrido completo del puzleSDL continúa al incorporar el tablón de40.
