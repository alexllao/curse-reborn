# Presentación reconstruida en una ventana SDL

La presentación nativa está en [principal.cpp](../apps/juego/plataformas/sdl/principal.cpp). Lee Core, título y los recursos de la mazmorra y los muestra mediante el núcleo reconstruido. La mazmorra incorpora ahora [sus objetos, superposiciones y guiones autónomos](objetos_mazmorra.md), gestionados por [mazmorra.cpp](../apps/juego/plataformas/sdl/mazmorra.cpp). **Ya se conectan el menú y las primeras interacciones; clic y flechas permiten caminar tras la liberación, con BOXDET; el corredor permite entrar y regresar con estado persistente y la caída enlaza con el agua. Siguen pendientes otras acciones y puzles completos; la siguiente prioridad solicitada es el audio común.** El [análisis del arranque](arranque_escena.md) documenta los bloques originales anteriores.

## Abrir la ventana

Desde la raíz del repositorio:

```sh
pnpm ventana
```

En Windows de 64 bits, la primera ejecución prepara SDL si falta, compila y abre Core. Las siguientes utilizan el paquete descargado y vuelven a compilar la aplicación. Una tecla o el botón izquierdo inicia el fundido hacia la siguiente vista: Core, título, introducción y mazmorra. F6 avanza directamente y F5 retrocede directamente para acelerar las pruebas. Desde la mazmorra, F6 abre una sesión nueva del corredor; otro F6 muestra la caída y el siguiente, el lago. F5 retrocede por el mismo orden. Los atajos reinician esas vistas y no resuelven ni guardan puzles de la sesión anterior. Escape abre/cierra el menú dentro de una escena jugable y cierra fuera de ella; el botón de cierre siempre termina el programa. La terminal permanece ocupada mientras la ventana está abierta.

La imagen lógica es de 320 × 200. La ventana inicial mide 960 × 600. Al redimensionar o maximizar, la imagen se ajusta al espacio disponible manteniendo su proporción de 8:5, con bandas negras donde sobra espacio. Usa vecino más próximo, sin suavizado ni efectos visuales añadidos. Por ahora presenta píxeles cuadrados; no reproduce el aspecto físico de un monitor CRT. Los marcos y la evaluación de filtros se dejan para más adelante.

La aplicación carga los recursos originales directamente y no usa el PNG generado por Python ni ejecuta Unicorn. Core y título solo necesitan sus DAT/PAL. **La mazmorra necesita ahora `CURSE.EXE` como fuente de tablas y guiones**, además de sus bancos originales. Node verifica la huella antes de iniciar la escena; C++ lee únicamente datos. El núcleo no incluye cabeceras SDL ni ejecuta instrucciones DOS. [titulo.cpp](../apps/juego/motor/recursos/titulo.cpp) agrupa descompresión y copias por planos; la capa de plataforma abre archivos, adapta la paleta a RGBA y gestiona ventana, textura y eventos.

Core y título corresponden a los recorridos de `0498` y `0584`, con los bloques contrastados de `64E0`, `7636` y `7679`. Los fundidos conservan las paletas de `77E4`; la capa SDL usa una frecuencia nominal de 70 Hz, pendiente de contrastar en una ejecución completa. La mazmorra incorpora el montaje de `50D2`, la carga de `8AA6`, la composición de `EBBD` y el avance de secuencias y comportamientos. El despacho de `1791` deja vacía la rutina particular de la escena cero: `183F` pertenece a la cinco. El menú y las primeras acciones están integrados; el bloque posterior conecta movimiento y colisiones. F5/F6 recorren las vistas y preparan un estado nuevo al volver a entrar en la mazmorra; no escriben partidas.

## Dependencias y compilación en Windows

Se requieren Node 22 o posterior, pnpm 11, Clang de 64 bits y las herramientas C++ de Visual Studio con Windows SDK. Las herramientas C++ pueden proceder de Visual Studio Community o Build Tools. Tener Clang para las comparaciones sin biblioteca estándar no garantiza disponer del SDK necesario para compilar una aplicación de escritorio.

La herramienta localiza Visual Studio con `vswhere`, configura su entorno únicamente para el proceso de compilación y utiliza sus cabeceras y bibliotecas. No cambia la configuración global del equipo. Si no encuentra el SDK, informa del requisito pendiente.

La versión fijada es [SDL 3.4.18](https://github.com/libsdl-org/SDL/releases/tag/release-3.4.18). El paquete de desarrollo oficial para Windows ocupa aproximadamente 17 MB y su SHA-256 está fijado en [dependencias_sdl.json](../herramientas/dependencias_sdl.json):

```text
78d84602ae616cfe26b33a73b7d3b9b6b56e8a6650e53d92c9c6ca938f747acd
```

Se comprueba antes de extraerlo a `trabajo/dependencias/`. La compilación deja el programa, `SDL3.dll` y su aviso de licencia en `trabajo/aplicacion/`. Ninguno de esos binarios se publica en Git. La primera preparación necesita acceso a GitHub; el paquete completo permite recompilar después sin red.

También puedes ejecutar las etapas por separado:

```sh
pnpm sdl:preparar
pnpm sdl:compilar
```

SDL utiliza la licencia zlib. Se conserva su aviso sin modificar en [licencias/SDL3.txt](../licencias/SDL3.txt). La licencia del código propio de Curse Reborn sigue pendiente de decidir por su creador, Àlex Llaó; este aviso no le asigna una licencia al proyecto ni a los recursos comerciales.

## Comprobación de la presentación

```sh
pnpm comprobar:ventana
```

El comando compila la aplicación y genera una referencia mediante `pnpm titulo`. Abre una ventana oculta de 320 × 200 con el dibujante de software de SDL, crea la textura, dibuja, recupera el fotograma y compara **los 64.000 píxeles RGB** con la referencia. Repite la comprobación después de iniciar Core e introducir una pulsación de Intro, esperando a que finalice el fundido normal hacia el título. Ambas capturas deben coincidir con la referencia. Después introduce un evento de Escape en la cola SDL y comprueba que el programa termina por el tratamiento normal de eventos.

La comprobación ha pasado en Windows de 64 bits. Las capturas de Core y del título se han inspeccionado visualmente. Se guardan `trabajo/comprobacion_ventana/fotograma.bmp`, `tras_fundido.bmp` y `resultado.json`; el informe recoge versión, plataforma, tipo de dibujante, huella RGB y alcance.

La aplicación normal no requiere Python. La comprobación utiliza el generador de referencia existente y por ello necesita el entorno preparado con `pnpm preparar`. No vuelve a ejecutar todas las comparaciones diferenciales.

Esta comprobación verifica la ruta de software con ventana oculta y cierre por Escape. La interacción visible, el cambio de tamaño y el dibujante acelerado quedan para la comprobación manual con `pnpm ventana`. No verifica audio, una partida ni compatibilidad con todos los controladores gráficos.

`pnpm comprobar:escena` comprueba el avance por F6 desde el título, la lectura de datos del EXE y los bancos y los 64.000 píxeles de la mazmorra inicial. Repite la captura tras 32 pasos de escena, con sus entidades y animaciones. Ambas coinciden con la referencia contrastada contra el original. La comparación amplia de objetos está agrupada en `pnpm comparar:objetos`; el propietario ha confirmado sus 773 casos/pasos. `pnpm comparar:presentacion` conserva el grupo anterior de fondos, sprites, RNC2 y fundidos.

## Otras plataformas

[CMakeLists.txt](../CMakeLists.txt) prepara la compilación del mismo código con CMake 3.21 o posterior y una instalación de SDL3 3.4.18 o compatible con su configuración CMake. En macOS y Linux, `pnpm sdl:compilar` utiliza esa ruta. No se ha comprobado todavía en esos sistemas. El paquete automático de Windows no sirve para ellos.

La compilación WebAssembly y la adaptación del bucle de presentación al navegador siguen pendientes. El núcleo se conserva independiente de la ventana para poder realizar ese paso después.

La [introducción original](introduccion_original.md) se intercala ahora entre título y mazmorra; también dispone de su comparación y capturas SDL específicas.

El [menú y la depuración F7–F10](menu_e_interacciones.md) tienen comprobación SDL propia. Escape abre/cierra el menú dentro de la mazmorra; fuera mantiene el cierre de la ventana.

El [movimiento con colisiones y la corrección de la puerta](movimiento_y_colisiones.md) disponen de comparación del bucle completo y capturas SDL propias.

`pnpm comprobar:corredor` verifica la entrada, el mecanismo y el regreso en tres capturas; [el alcance de cámara y las barreras originales](corredor_y_camara.md) distingue la partida de sus estados de prueba. `pnpm comprobar:caida` verifica DROP tras 16/32 pasos y la entrada a LAKEBED tras 24 pasos. La [guía de caída](recorridos/02_caida.txt) explica F5/F6 y sus reinicios. La llegada al agua no implica que estén resueltas las interacciones del lago.

`pnpm comprobar:vistas` valida ocho cambios con eventos F5/F6: mazmorra, corredor, caída y lago, con regresos intercalados. El salto desde mazmorra ya pasa por el corredor. F3 permite elegir los [efectos digitales Sound Blaster](efectos_digitales.md), activos por defecto, o PC Speaker. Los efectos PC Speaker y sus comprobaciones se describen en [altavoz del PC](altavoz_pc.md); M permite silenciarlos.

[Referencia completa de teclas y controles](controles.md).
