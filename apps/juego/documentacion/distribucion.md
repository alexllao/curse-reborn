# Paquetes beta para Windows

Desde la raíz del monorepositorio:

```text
pnpm empaquetar:beta
pnpm empaquetar:beta --con-datos
pnpm empaquetar:beta --con-datos --sin-perdida
```

El primer comando prepara el port y los recursos propios; el receptor coloca su copia de `CURSE.EXE` y `DATA` siguiendo el `LEEME.txt` del paquete. `--con-datos` incorpora la copia local de `apps/juego/datos_originales/CURSE`, después de comprobar la huella del ejecutable. No se descargan datos del juego. Las ROM, partidas, configuración DOS y archivos de trabajo nunca se incorporan.

Cada ejecución crea una carpeta nueva y un ZIP con versión, fecha e identificador en `apps/juego/dist/`. No sobrescribe entregas anteriores ni modifica maestros. El ZIP lleva una huella SHA-256 independiente. No se versionan esas salidas. Esta primera implementación está validada para Windows x64; macOS y WebAssembly continúan pendientes.

Dentro del ZIP la carpeta se llama `CurseReborn-<versión>`, sin repetir la fecha y el identificador: así se evitan rutas internas innecesariamente largas al extraerlo. En `dist/` cada carpeta queda separada por fecha e identificador.

## Compilación y contenido

Se compila en `apps/juego/trabajo/aplicacion_beta`, separada de desarrollo, con `O2`, `NDEBUG`, secciones independientes y eliminación/unificación de código mediante `OPT:REF` y `OPT:ICF`. El subsistema de Windows evita abrir una consola al hacer doble clic. No se desactivan controles o puzles del port.

El paquete contiene el ejecutable, SDL3, Munt, el runtime x64 redistribuible de Visual Studio, licencias, controles y las imágenes realmente referenciadas por los catálogos compilados. No arrastra capturas, cabeceras generadas, comparadores, maestros descartados o decorados aparcados.

Tras extraer todo el ZIP, se puede abrir `curse-reborn.exe` directamente: busca `datos_originales/CURSE/DATA` junto a sí mismo, independientemente del directorio de trabajo. `Jugar-es.cmd` y `Jugar-en.cmd` seleccionan el idioma. Las partidas y el tiempo siguen guardándose en el perfil del usuario.

## Optimización de imágenes

El perfil predeterminado, `equilibrado`, comprime los PNG y permite una paleta de hasta 256 colores en las fotografías opacas de habitaciones y presentación. Esta reducción puede alterar ligeramente el color. Los marcos, el icono y las imágenes transparentes reciben compresión sin pérdida, contrastando los píxeles decodificados. Solo se sustituye una imagen si el resultado ocupa menos.

`--sin-perdida` exige los mismos píxeles en todas las imágenes. No cambia las dimensiones en ninguno de los perfiles: las coordenadas del cristal, botones y pilotos permanecen alineadas con el catálogo. No se cambia a JPEG/WebP ni se reduce la resolución de una textura sin adaptar la geometría; el cargador SDL utiliza PNG y la máscara depende de sus dimensiones originales.

`paquete.json` registra versión, revisión Git, cambios locales, perfil, comprobaciones ejecutadas y ahorro de cada imagen, además del tamaño y SHA-256 de cada archivo. La huella de los metadatos de marco se actualiza únicamente en la copia distribuida.

## Verificación

```text
pnpm comprobar:builder
pnpm empaquetar:beta --verificar "C:\ruta\a\la\carpeta\extraida"
```

Las pruebas del builder cubren transparencia, píxeles sin pérdida, conservación de dimensiones, rechazo de rutas externas/enlaces y detección de alteraciones, archivos sobrantes y partidas. La verificación del manifiesto se realiza antes de crear el ZIP; puede repetirse después de extraerlo y antes de añadir datos opcionales.

Con `--con-datos` se ejecutan además desde la copia empaquetada, con ventanas ocultas, las pruebas de habitaciones/monitor y el recorrido hasta THE END. Sus capturas y perfiles temporales permanecen en `apps/juego/trabajo/`, fuera del paquete. Estas pruebas no acreditan todos los equipos Windows ni cierran la revisión pendiente de alternativas y continuidad de partidas.

Requisitos para construir: dependencias de pnpm, entorno del proyecto preparado y herramientas C++/SDK y redistribuibles x64 de Visual Studio. `sharp` es una dependencia de desarrollo fijada en el lockfile; no hace falta instalar Node, Python o pnpm en el equipo receptor.
