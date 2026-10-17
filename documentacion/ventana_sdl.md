# Presentación reconstruida en una ventana SDL

La presentación nativa está en [principal.cpp](../plataformas/sdl/principal.cpp). Lee Core, título y los recursos de la mazmorra y los muestra mediante el núcleo reconstruido. **Hay tres vistas de desarrollo; aún faltan la presentación animada completa, el control del personaje y la partida. El audio queda aparcado.** El [análisis del arranque](arranque_escena.md) documenta los bloques originales y las comprobaciones.

## Abrir la ventana

Desde la raíz del repositorio:

```sh
pnpm ventana
```

En Windows de 64 bits, la primera ejecución prepara SDL si falta, compila y abre Core. Las siguientes utilizan el paquete descargado y vuelven a compilar la aplicación. Una tecla o el botón izquierdo inicia el fundido hacia la siguiente vista: Core, título y mazmorra. F6 avanza directamente y F5 retrocede directamente para acelerar las pruebas. Los atajos no modifican el progreso de una partida. Escape y el botón de cierre terminan el programa. La terminal permanece ocupada mientras la ventana está abierta.

La imagen lógica es de 320 × 200. La ventana inicial mide 960 × 600. Al redimensionar o maximizar, la imagen se ajusta al espacio disponible manteniendo su proporción de 8:5, con bandas negras donde sobra espacio. Usa vecino más próximo, sin suavizado ni efectos visuales añadidos. Por ahora presenta píxeles cuadrados; no reproduce el aspecto físico de un monitor CRT. Los marcos y la evaluación de filtros se dejan para más adelante.

La aplicación carga los recursos originales directamente. No usa el PNG generado por Python, no necesita `CURSE.EXE` y no ejecuta Unicorn. El núcleo no incluye cabeceras SDL. [titulo.cpp](../motor/recursos/titulo.cpp) agrupa descompresión y copias por planos; la capa de plataforma abre archivos, adapta la paleta a RGBA y gestiona ventana, textura y eventos.

Core y título corresponden a los recorridos de `0498` y `0584`, con los bloques contrastados de `64E0`, `7636` y `7679`. Los fundidos conservan las paletas de `77E4`; la capa SDL usa una frecuencia nominal de 70 Hz, pendiente de contrastar en una ejecución completa. La vista inicial incorpora el montaje de `50D2` y el sprite de `6D8C`, pero aún no el guion inicial ni el bucle de juego completo.

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

`pnpm comprobar:escena` comprueba además el avance por F6 desde el título, la lectura del MAP y del banco del personaje y los 64.000 píxeles de la mazmorra. La comparación amplia del nuevo código está agrupada en `pnpm comparar:presentacion`; queda pendiente de confirmación del propietario.

## Otras plataformas

[CMakeLists.txt](../CMakeLists.txt) prepara la compilación del mismo código con CMake 3.21 o posterior y una instalación de SDL3 3.4.18 o compatible con su configuración CMake. En macOS y Linux, `pnpm sdl:compilar` utiliza esa ruta. No se ha comprobado todavía en esos sistemas. El paquete automático de Windows no sirve para ellos.

La compilación WebAssembly y la adaptación del bucle de presentación al navegador siguen pendientes. El núcleo se conserva independiente de la ventana para poder realizar ese paso después.
