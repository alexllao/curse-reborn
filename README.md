# Curse Reborn

**Volver a Enchantia. Recuperar el juego original. Darle una nueva vida.**

Curse Reborn es un port creado por **Àlex Llaó**. Nace de un recuerdo muy concreto: _Curse of Enchantia_ fue el primer juego al que jugó en un **PC 286**. Este proyecto es una forma de regresar a aquella aventura, entender cómo funcionaba y construir una base que permita disfrutarla y remasterizarla con cuidado.

La primera meta es **desensamblar `CURSE.EXE`, entender sus rutinas y trasladar su lógica a C++ con SDL**. Los recursos originales acompañarán esa reconstrucción. Sobre esa base queremos desarrollar una remasterización que conserve su identidad y abra la puerta a mejoras gráficas, sonoras y de uso.

## El proyecto que queremos construir

El trabajo se plantea en dos etapas:

1. **Reconstrucción del original desde su código.** Desensamblar el ejecutable, seguir sus llamadas y saltos, identificar sus rutinas y trasladarlas progresivamente a C++ con SDL. Los formatos de datos se investigarán siguiendo las rutinas originales de carga.
2. **Remasterización.** Incorporar nuevos recursos y mejoras sobre un motor cuyo funcionamiento ya se haya comprobado frente al original.

La fidelidad será el punto de partida: escenarios, personajes, animaciones, inventario y puzles deben entenderse antes de transformarse. La intención es poder distinguir el comportamiento original de las mejoras que se introduzcan más adelante.

Cada parte trasladada deberá conservar una referencia a su dirección en el ejecutable, una explicación de su función y las comprobaciones realizadas. SDL sustituirá las dependencias de DOS y del hardware una vez identificado el comportamiento que deben conservar.

## Estado actual

**Core, título, introducción, mazmorra con su ruta inicial, corredor parcial y caída hasta el lago, reconstruidos en C++ y mostrados en SDL. Los efectos PC Speaker tienen salida audible. Los demás puzles, el juego completo y el resto de dispositivos de sonido siguen pendientes.**

El propietario ha confirmado el [generador de valores acotados](documentacion/generador_aleatorio.md) en **18.816 llamadas**, la [preparación integrada de resultados](documentacion/preparacion_resultados.md) en **2.048 operaciones** y el [avance de acciones y movimiento](documentacion/avance_acciones_movimiento.md) en **10.930 casos**. También ha confirmado los **3.028 casos** de [creación y actualización de entidades](documentacion/entidades.md). El propietario ha confirmado también los **8.924 casos** de [secuencias de animación](documentacion/secuencias_animacion.md).

El traslado de [operaciones de sonido y modelos](documentacion/operaciones_secuencias.md) selecciona órdenes para las cinco modalidades observadas y permite atender sonido y carga mediante servicios explícitos. El propietario ha confirmado sus **1.285 casos**. La mazmorra ya aporta la carga de modelos desde archivos; el audio sigue desactivado.

El [reproductor del altavoz del PC](documentacion/altavoz_pc.md) ya traslada arranque, parada y avance de notas, incluyendo su conexión con el selector y la animación. El propietario ha confirmado sus **3.312 casos**. Los efectos ya generan PCM y se escuchan mediante una salida SDL compartida; la música de zona [MT-32](documentacion/musica_mt32.md) ya está integrada mediante Munt; AdLib conserva la presentación y SB16 digital sigue pendiente.

Las [copias gráficas y la paleta](documentacion/video_planos.md) ya están trasladadas a C++ y el propietario ha confirmado sus **1.183 casos** de comparación amplia. Se conserva la primera imagen de diagnóstico obtenida con la descompresión original como referencia.

El [descompresor RNC de método uno](documentacion/descompresion_rnc.md) ya está trasladado a C++ y el propietario ha confirmado sus **142 casos** de comparación amplia. **`pnpm titulo` genera la pantalla de título íntegramente con el núcleo C++**, sin ejecutar `CURSE.EXE` ni Unicorn. La imagen coincide con la referencia anterior.

**`pnpm ventana` recorre Core → título → introducción original → mazmorra inicial**. Los fundidos de salida conservan los 32 pasos de paleta del original. Una tecla o el botón izquierdo inicia el fundido; F6 avanza y F5 retrocede directamente entre estas vistas de desarrollo. Escape cierra fuera de la mazmorra; dentro abre/cierra el menú. La imagen se ajusta al redimensionar o maximizar conservando sus proporciones, con bandas negras y vecino más próximo, sin filtros añadidos.

El [montaje de fondos y sprites](documentacion/arranque_escena.md), incluido RNC de método dos, coincide en una muestra de **102 casos**; los [fundidos originales](documentacion/arranque_escena.md) coinciden en **64 paletas** adicionales. La nueva [carga, composición y selección de objetos](documentacion/objetos_mazmorra.md) tiene confirmados por el propietario sus **773 casos/pasos** de comparación amplia, incluidas 257 composiciones y 256 pasos persistentes. La comprobación SDL compara los **64.000 píxeles** del estado inicial y de los objetos tras 32 pasos. Ya están conectados el [menú y las primeras interacciones](documentacion/menu_e_interacciones.md). Ya se puede [caminar en la mazmorra](documentacion/movimiento_y_colisiones.md) tras la liberación, con clic o flechas y colisiones originales. Ya están conectados [el corredor y el regreso](documentacion/corredor_y_camara.md), y [la caída con llegada al agua](documentacion/caida_y_agua.md). Siguen pendientes los demás puzles y habitaciones. La cadencia en un monitor real y el dibujante acelerado quedan para comprobación manual.

La mazmorra lee las tablas y guiones del EXE como datos, con huella verificada, y ejecuta la lógica traducida a C++. No ejecuta instrucciones DOS. Los originales siguen siendo archivos locales excluidos de Git.

La prioridad de jugabilidad es completar las acciones del corredor y del lago siguiendo la guía aportada por Àlex. La música MT-32 y los efectos PC Speaker comparten dispositivo. AdLib conserva el tema de presentación; sus arreglos de zona y SB16 digital siguen pendientes. Más adelante se estudiarán marcos y filtros visuales.

Ya existe una herramienta propia que lee la estructura DOS MZ y sigue llamadas y saltos desde el punto de entrada, tres manejadores identificados, seis tablas estáticas y las fuentes de una tabla dinámica de acciones. La pasada actual alcanza **18.744 instrucciones candidatas y 113 destinos de llamada directa**, y señala **dos transferencias indirectas pendientes**, que encadenan vectores de DOS. Estas cifras se obtienen por análisis estático; no representan el porcentaje completado del motor ni una ejecución verificada del juego.

El [mapa inicial del ejecutable](documentacion/analisis_ejecutable.md) recoge el arranque y las reubicaciones. El [análisis del bucle central](documentacion/bucle_principal.md) documenta su recorrido y la selección de escenas. El constructor de la [tabla dinámica de acciones](documentacion/tabla_acciones.md), la selección de coincidencias y la interpretación del [retorno de acciones](documentacion/contrato_acciones.md) pasan **1.120 comparaciones contra las instrucciones originales** ejecutadas en un emulador de CPU. Además, hay **25 comprobaciones** del analizador con programas sintéticos que no necesitan el juego.

El objetivo es ampliar ese C/C++ hasta disponer de un motor portable. Se conserva un [listado de ensamblador comentado](documentacion/ensamblador_comentado.md) como referencia, con 174 anotaciones propias y enlaces a los bloques trasladados. El núcleo se mantiene independiente de SDL. La [capa SDL](documentacion/ventana_sdl.md) se ha compilado y comprobado en Windows; macOS y la futura compilación web mediante Emscripten siguen pendientes de validar e integrar.

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
- [x] Confirmar con el propietario los 1.183 casos de vídeo.
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
- [ ] Completar los puzles del corredor y del lago, y las demás habitaciones.
- [ ] Confirmar manualmente ventana visible, maximización y cadencia de los fundidos.
- [ ] Completar los destinos indirectos restantes y las estructuras de estado del juego.
- [ ] Validar el recorrido identificado durante la ejecución del original.
- [ ] Seguir las rutinas originales de carga y descompresión de recursos.
- [ ] Documentar los formatos de escenarios, sprites y animaciones a partir de esas rutinas.
- [ ] Ampliar el traslado a rutinas completas, con sus contratos de entrada y salida documentados.
- [ ] Sustituir el acceso gráfico al hardware por SDL y comprobar el resultado.
- [ ] Trasladar los controles, el movimiento y las interacciones.
- [ ] Trasladar el inventario, los eventos y los puzles.
- [ ] Trasladar el sonido, la temporización y las partidas guardadas.
- [ ] Comprobar el desarrollo completo de la aventura frente al original.
- [ ] Definir y desarrollar las mejoras de la remasterización.

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
| `pnpm comparar:video`       | Compara 1.183 casos de copias por planos y paleta, confirmados por el propietario. Admite `--muestra` con 83 casos. Genera la referencia `trabajo/vista_titulo/titulo.png`. |
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

La [introducción de la bruja](documentacion/introduccion_original.md) ya recorre `CAULDRN`, `BALCONY` y `BASEBAL`/`BASEBAT` desde el bloque original `28B2`. Conserva las cargas, páginas, paletas, controles de omisión y fundidos. Una pulsación en el título inicia la introducción; otra durante ella solicita omitirla en el siguiente control original. Al terminar entra en la mazmorra. F5/F6 recorren Core, título, introducción, mazmorra, corredor, caída y lago como vistas de desarrollo. La música MT-32 comienza con CURSE.RS5 antes de Core y cambia según la tabla original de cada escena; los efectos PC Speaker están activos en las escenas jugables.

`pnpm comparar:introduccion` contrasta el recorrido y las imágenes contra el ejecutable; `pnpm comprobar:introduccion` compara capturas SDL del caldero, balcón, béisbol y llegada a la mazmorra al terminar y al omitir. Las esperas usan retrazos nominales a 70 Hz; la duración de la E/S DOS y la sincronización física de monitor no se reproducen.

### Menú y depuración

En la mazmorra, Escape o botón derecho abre/cierra el menú. El ratón y el botón izquierdo seleccionan; izquierda/derecha y Enter también funcionan. Para poner en marcha la primera secuencia, elige el bocadillo de conversación (quinto icono) y la segunda casilla de su submenú. Se mantienen las condiciones originales de disponibilidad.

**F7** activa/desactiva la depuración. **F8** alterna cajas de entidades; **F9**, zonas de interacción; **F10**, encuadre, umbrales y rectángulos BOXDET. Verde: objetos; cian: Brad; magenta: zonas disponibles por afinidad; naranja: proximidad de los pies; amarillo: límites del encuadre. Son datos de diagnóstico, sin alterar la partida. F10 muestra además las paredes de BOXDET en amarillo y las zonas de salida en azul. Tras quitarse las esposas se camina con clic izquierdo o flechas; la puerta conserva su posición original.

`pnpm comparar:jugabilidad` contrasta el movimiento y la liberación; `pnpm comprobar:jugabilidad` comprueba sus dos capturas SDL.

`pnpm comparar:menu` agrupa las comparaciones de menú; `pnpm comprobar:menu` verifica panel, primera interacción y colores de depuración en SDL. Consulta [rutinas, alcance y controles](documentacion/menu_e_interacciones.md).

### Recorrido de la mazmorra

Ya se puede empujar la pared, recoger el clip y desbloquear la puerta con él. Se mantienen las condiciones y los rechazos originales, incluida la recogida del objeto de puntuación. La puerta permite entrar al corredor y regresar con el estado conservado. Faltan la recogida de la pecera y el recorrido completo entre estatuas y guardias.

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

`pnpm comprobar:corredor` compara entrada, mecanismo y regreso en SDL, además de estados de cámara y sus 18 posiciones gráficas originales. El recorrido de partida conserva la barrera de la entidad del corredor; sus cambios de cámara se contrastan en estados de prueba separados, sin fingir que su puzle ya esté resuelto.

`pnpm comprobar:caida` compara dos momentos de `DROP` y la entrada sumergida en `LAKEBED`. Desde la mazmorra, **F6 abre el corredor** y otro F6 abre la caída; F5 retrocede por las escenas integradas. Cada salto prepara una sesión nueva. La caída avanza sola hasta el agua. Estos atajos no resuelven puzles de una partida ni conservan la sesión anterior.

[Guía del corredor](documentacion/recorridos/01_corredor.txt) · [Guía de la caída](documentacion/recorridos/02_caida.txt). La siguiente prioridad acordada es el **audio compartido**, empezando por PC Speaker y continuando con AdLib y MT-32. Los efectos PC Speaker ya siguen las órdenes, prioridades y secuencias originales, con una salida compartida entre escenas. La música de zona [MT-32](documentacion/musica_mt32.md) ya está integrada mediante Munt; AdLib conserva la presentación y SB16 digital sigue pendiente.

## Recursos originales y autoría

El creador del port **Curse Reborn** es **Àlex Llaó**. Contacto: [alexllao@me.com](mailto:alexllao@me.com).

_Curse of Enchantia_ es una obra de Core Design. Curse Reborn es un proyecto independiente y no oficial, y reconoce el trabajo de quienes crearon el juego original.

El repositorio no incluye el ejecutable ni los archivos comerciales del juego. Para las pruebas con recursos originales será necesario aportar una copia propia. Los archivos del juego, los recursos extraídos y las partidas deben permanecer fuera del historial del repositorio.

La licencia del código nuevo está pendiente de definir. Antes de incorporar código de terceros se revisarán su licencia y sus requisitos de atribución.

---

Este proyecto empieza con el deseo de volver al primer juego de ordenador de su creador y darle un futuro sin perder aquello que lo hizo especial.

La solución de referencia y el seguimiento por tramos están en [referencia de solución](documentacion/referencia_solucion.md), a partir de la guía de Sophie Cheshire aportada por Àlex.

## Sonido y vistas de desarrollo

`pnpm ventana` activa los efectos originales de PC Speaker. **M** alterna el silencio; los guiones y el temporizador siguen avanzando. Los sonidos usan las secuencias del EXE, también con el menú abierto o durante un fundido. Si no hay dispositivo, la ventana continúa sin audio. Para desactivarlo desde el arranque: `pnpm ventana --sin-sonido`.

`pnpm comprobar:audio --muestra` agrupa la comparación original del altavoz, la nueva señal PCM y la apertura/alimentación de SDL con un dispositivo de prueba. `pnpm comparar:audio` amplía la comparación. El WAV de prueba se guarda localmente en `trabajo/comparacion_audio/efecto_original.wav`. Esta prueba no escucha ni valida los altavoces físicos. Véanse [detalles y límites](documentacion/altavoz_pc.md).

`pnpm comprobar:vistas` introduce ocho cambios F5/F6 y verifica el orden mazmorra → corredor → caída → lago y el regreso. Estos atajos reinician la sesión; para conservar el inventario hay que atravesar las salidas normales del juego.

La [música MT-32](documentacion/musica_mt32.md) reproduce los cinco bancos CURSE.RS y consulta las 78 referencias originales de escena. Con las ROM locales válidas se selecciona al arrancar. **F4** recorre PC Speaker, AdLib y MT-32, con efectos PC Speaker; omite MT-32 si no puede prepararlo. La [música AdLib](documentacion/musica_adlib.md) conserva el tema de presentación y sus nueve canales; los arreglos AdLib de zona y SB16 digital siguen pendientes. La consola describe la opción efectiva. **M** silencia toda la mezcla. Se registran también las acciones del jugador, selecciones, inventario y cambios de escena, sin repetir mensajes por fotograma.

`pnpm comprobar:musica --muestra` compara el reproductor musical original y comprueba ocho segundos de síntesis y salida SDL de prueba. La comparación amplia queda disponible en `pnpm comparar:musica`.
