# Paquetes beta para Windows

## Cierre de entrega del 21 de octubre

Por decisión posterior de Àlex, el fondo y la moldura se muestran sin filtros. F11 conserva los efectos del juego y su cursor. Verificados cincuenta fotogramas de marcos: bandas idénticas en los seis modos, F1 reversible y animación simétrica; las seis imágenes del juego siguen siendo distintas. Regresión oculta de habitaciones:58 capturas correctas. La entrega debe recompilarse para incorporar este cambio, además de la corrección de su prueba descrita abajo.

La comprobación de habitaciones del paquete detectó otra incidencia reproducible: el criterio antiguo rechazaba cualquier cambio a más de dos píxeles del borde, incluyendo el relieve autorizado de la nueva moldura. Se corrige la prueba para usar la misma envolvente que `comprobar:marcos`: seis píxeles lógicos en el borde y26 en las esquinas, escalados a la zona física del juego. El resto del interior debe coincidir; al desactivar el marco no puede quedar ningún píxel distinto y las bandas deben recibir imagen cuando existan. El diagnóstico distingue bandas vacías, invasión del interior y restos después de desactivar.

La aplicación anterior reproduce el error con los maestros sin comprimir. Tras recompilar con la corrección, `--comprobar-habitaciones` pasa con cinco diseños, cinco proporciones y ventana vertical,58 capturas y perfiles aislados. Las seis pruebas del builder también pasan. Evidencia ignorada en `trabajo/fallo_builder_marcos/`: `original.log`, `corregido.log` y capturas. No se cambia el dibujo ni se elimina la comprobación. La prueba de compilación se lanzó directamente con las herramientas C++ configuradas porque Node mantiene `spawnSync vswhere.exe EPERM` en esta sesión. Esto no acredita todavía un ZIP nuevo: hay que repetir `pnpm empaquetar:beta --con-datos` con el código actualizado.

La base comprobada y publicada es `1f2de0b`. El recorrido SDL hasta THE END, sus regresos alternativos y la continuidad tras cargar están registrados en `auditoria_pantallas.md`. Las tandas posteriores han cerrado los estados concretos de recaídas11 con actor activo y las marcas reales de puertas de los seis inventarios del palacio. Las notas históricas anteriores no vuelven a abrir esos pendientes.

En esta sesión, `pnpm comprobar:builder` se detiene en el arranque del proceso de pruebas con `spawn EPERM`. Las mismas seis pruebas se han ejecutado dentro del proceso de Node y pasan:

```text
node --test --experimental-test-isolation=none apps/juego/pruebas/distribucion/builder.test.mjs
```

`pnpm empaquetar:beta --con-datos` se ha intentado desde la copia limpia, pero termina en `spawnSync git EPERM`, antes de compilar. No se ha creado ni verificado una beta nueva de esta revisión. Los paquetes del20 de octubre descritos abajo siguen siendo históricos, sin atribuirles los cambios posteriores.

El siguiente paso de entrega es ejecutar ese empaquetado, comprobar sus pruebas ocultas desde la carpeta distribuida y verificar de nuevo el ZIP extraído. Después se podrán registrar ruta, revisión, tamaño y SHA-256 de la nueva beta. No hace falta repetir las tandas de interacciones ya cerradas para reintentar el builder. Los logs de esta sesión son `trabajo/builder_cierre.log` y `trabajo/empaquetado_cierre.log`, excluidos de Git.

Desde la raíz del monorepositorio:

```text
pnpm empaquetar:beta
pnpm empaquetar:beta --con-datos
pnpm empaquetar:beta --con-datos --sin-perdida
pnpm empaquetar:beta --con-datos --habitacion todas
pnpm empaquetar:beta --con-datos --habitacion 03-lucasarts
```

El primer comando prepara el port y los recursos propios; el receptor coloca su copia de `CURSE.EXE` y `DATA` siguiendo el `LEEME.txt` del paquete. `--con-datos` incorpora la copia local de `apps/juego/datos_originales/CURSE`, después de comprobar la huella del ejecutable. No se descargan datos del juego. Las ROM, partidas, configuración DOS y archivos de trabajo nunca se incorporan.

## Elegir las habitaciones incluidas

Por defecto se incluye **solo la habitación indicada en `inicial`** dentro de `packages/assets/lanzador/habitaciones/habitaciones.json`. Actualmente es `02-buhardilla`. Se conservan todas las resoluciones y proporciones de ese diseño para adaptar el monitor a la ventana.

| Parámetro                                    | Resultado                                                           |
| -------------------------------------------- | ------------------------------------------------------------------- |
| Sin `--habitacion`, o `--habitacion inicial` | Solo la inicial del JSON.                                           |
| `--habitacion todas`                         | Todos los diseños, conservando como inicial el indicado en el JSON. |
| `--habitacion 03-lucasarts`                  | Solo ese diseño, que se convierte en el inicial del paquete.        |

Los identificadores disponibles son `01-misterio`, `02-buhardilla`, `03-lucasarts`, `04-sierra` y `05-ciencia-ficcion`. Se consulta el catálogo fuente al generar; los identificadores desconocidos o parámetros repetidos se rechazan antes de compilar.

La selección afecta tanto al catálogo C++ compilado como al JSON y las imágenes entregadas. Ctrl+H recorre únicamente los diseños incluidos; no puede seleccionar uno ausente. Los catálogos y maestros del repositorio permanecen intactos y la compilación normal de desarrollo conserva todos los diseños. `paquete.json` registra la habitación inicial y la lista incluida.

Cada ejecución crea una carpeta nueva y un ZIP con versión, fecha e identificador en `apps/juego/dist/`. No sobrescribe entregas anteriores ni modifica maestros. El ZIP lleva una huella SHA-256 independiente. No se versionan esas salidas. Esta primera implementación está validada para Windows x64; macOS y WebAssembly continúan pendientes.

Dentro del ZIP la carpeta se llama `CurseReborn-<versión>`, sin repetir la fecha y el identificador: así se evitan rutas internas innecesariamente largas al extraerlo. En `dist/` cada carpeta queda separada por fecha e identificador.

## Preparar una entrega

1. Instalar las dependencias con `pnpm install --frozen-lockfile` y preparar el entorno del proyecto con `pnpm preparar` si es una copia nueva. Se requieren Node22.13 o superior, pnpm11, LLVM/Clang y las herramientas C++/Windows SDK de Visual Studio.
2. Revisar la versión de los paquetes y los cambios que se van a entregar. El builder lee la versión de `apps/juego/package.json`; esa misma versión se muestra en Información. La versión coordinada actual es0.9.0, anterior al cierre de la revisión integral. El builder no incrementa versiones automáticamente.
3. Ejecutar `pnpm calidad` y `pnpm comprobar:builder`. Son comprobaciones previas; el empaquetador no las ejecuta automáticamente.
4. Generar el paquete con el comando correspondiente, usando `--con-datos` para una beta local que ya contenga el juego. Añadir `--sin-perdida` si se necesitan los colores exactos de todos los maestros.
5. Extraer el ZIP completo en otra carpeta y ejecutar `pnpm empaquetar:beta --verificar "C:\ruta\extraida\CurseReborn-0.9.0"` antes de añadir ROM u otros archivos opcionales. Abrir después el ejecutable desde esa carpeta para comprobar la entrega extraída.
6. Entregar el ZIP y su archivo `.sha256`. Conservar `paquete.json` para identificar la versión, el perfil de imágenes y el contenido probado.

La verificación de carpetas detecta cambios y archivos sobrantes respecto del manifiesto. La huella del ZIP permite contrastar la copia transferida, por ejemplo con `Get-FileHash -Algorithm SHA256 "C:\ruta\paquete.zip"` en PowerShell. No es una firma digital del autor.

## Estructura de salida

```text
apps/juego/dist/
├── <fecha>-<identificador>/
│   └── CurseReborn-0.9.0/
│       ├── curse-reborn.exe
│       ├── SDL3.dll, mt32emu.dll y runtime de Visual C++
│       ├── assets/
│       ├── datos_originales/
│       │   ├── CURSE/CURSE.EXE y DATA/   (solo con --con-datos)
│       │   ├── ROMS/                   (vacía)
│       │   └── LEEME.txt
│       ├── Jugar-es.cmd y Jugar-en.cmd
│       ├── LEEME.txt y CONTROLES.md
│       ├── BIBLIOTECAS.txt y licencias
│       └── paquete.json
├── curse-of-enchantia-reborn-<versión>-win-x64-<fecha>-<id>.zip
└── curse-of-enchantia-reborn-<versión>-win-x64-<fecha>-<id>.zip.sha256
```

Si una compilación o comprobación falla, el comando termina con error. No se debe entregar una carpeta parcial ni un ZIP sin completar la verificación. Los mensajes y las capturas de las pruebas permiten investigar el fallo; los resultados locales permanecen fuera de Git. El ZIP solo se crea después de verificar la carpeta.

## Compilación y contenido

Se compila en `apps/juego/trabajo/aplicacion_beta`, separada de desarrollo, con `O2`, `NDEBUG`, secciones independientes y eliminación/unificación de código mediante `OPT:REF` y `OPT:ICF`. El subsistema de Windows evita abrir una consola al hacer doble clic. No se desactivan controles o puzles del port.

El paquete contiene el ejecutable, SDL3, Munt, el runtime x64 redistribuible de Visual Studio, licencias, controles y las imágenes realmente referenciadas por los catálogos compilados. No arrastra capturas, cabeceras generadas, comparadores, maestros descartados o decorados aparcados.

Tras extraer todo el ZIP, se puede abrir `curse-reborn.exe` directamente: busca `datos_originales/CURSE/DATA` junto a sí mismo, independientemente del directorio de trabajo. `Jugar-es.cmd` y `Jugar-en.cmd` seleccionan el idioma. Las partidas y el tiempo siguen guardándose en el perfil del usuario.

Los archivos persistentes se llaman `CURSE.SAV` y `tiempo_juego.json`. Se sitúan mediante `SDL_GetPrefPath("AlexLlao", "CurseReborn")`, habitualmente bajo `%APPDATA%\AlexLlao\CurseReborn` en Windows. Por ello, borrar o sustituir la carpeta de una beta no borra las partidas del perfil. Las variables de pruebas `CURSE_REBORN_PARTIDAS` y `CURSE_REBORN_TIEMPO` permiten aislarlos; el builder las utiliza para no tocar los archivos del usuario.

## Optimización de imágenes

El perfil predeterminado, `alta`, convierte a JPEG de calidad92 los fondos opacos de habitaciones, presentación y Cielo encantado. Se conserva la resolución y se usa crominancia4:4:4 para proteger los detalles de color. `equilibrada` usa calidad85 y `compacta`,75. JPEG implica pérdida: conviene revisar el resultado a pantalla completa. Solo se sustituye una imagen si ocupa menos. Los maestros de `packages/assets` nunca se modifican.

Logos, iconos, molduras, pixel art y cualquier fondo con píxeles transparentes conservan PNG sin pérdida, con igualdad de píxeles comprobada. Un canal alfa completamente opaco sí permite JPEG. `--sin-perdida` (equivalente a `--imagenes sin-perdida`) conserva los píxeles de todas las imágenes. Las dimensiones, máscara del cristal, botones y pilotos se mantienen en todos los perfiles.

```text
pnpm empaquetar:beta --con-datos
pnpm empaquetar:beta --con-datos --imagenes equilibrada
pnpm empaquetar:beta --con-datos --imagenes compacta
pnpm empaquetar:beta --con-datos --sin-perdida
```

La entrega contiene `.jpg` reales y omite su PNG maestro. Los catálogos conservan las rutas lógicas `.png`: el cargador intenta primero ese PNG y, si no existe, su alternativa `.jpg`. La decodificación JPEG usa WIC del sistema en Windows, sin otra DLL distribuida; la generación de paquetes sigue limitada a Windows x64. Las plataformas futuras necesitarán su adaptador JPEG; los maestros PNG de desarrollo siguen siendo utilizables. Si existe un PNG corrupto se informa del fallo, sin ocultarlo con una alternativa.

`paquete.json` registra versión, revisión Git, cambios locales, perfil, comprobaciones ejecutadas y ahorro de cada imagen, además del tamaño y SHA-256 de cada archivo. Cada imagen identifica `ruta` entregada, `ruta_maestro`, método y las dos huellas. La huella de los metadatos de marco se actualiza únicamente en la copia distribuida, aunque su nombre lógico siga terminando en `.png.json`.

Validación del23/10/2026: compilación nativa O2/avisos como errores y ocho pruebas del builder correctas. Copia aislada con34 JPEG reales de calidad92 (25 variantes de habitación, cinco presentaciones y cuatro fondos Cielo encantado), sin sus PNG en esas rutas: `--comprobar-habitaciones` pasa58 capturas y `--comprobar-menu-port`,22 capturas en seis proporciones. Se conservan PNG del logo, icono y moldura. Evidencia local ignorada en `trabajo/fondos_jpeg`; el ahorro medido antes con diez fondos de la habitación inicial y presentación es52.588.222→13.258.214bytes al92,9.708.598al85 y7.265.239al75. No son tamaños de ZIP. ESLint, Prettier, formato C++ y monorepo pasan directamente. `pnpm calidad` sigue abortando al comprobar dependencias/registro sin TTY, y el intento directo de empaquetado termina en `spawnSync git EPERM` antes de compilar. No se ha generado un ZIP nuevo en esta sesión.

Si `cambios_locales` es `true`, la revisión Git identifica la base del árbol utilizado, no un commit que contenga necesariamente todos los cambios compilados. Para futuras entregas reproducibles desde el historial, generar después de publicar los cambios y comprobar que este campo sea `false`. La fecha se almacena en UTC, con sufijo `Z`.

## Verificación

```text
pnpm comprobar:builder
pnpm empaquetar:beta --verificar "C:\ruta\a\la\carpeta\extraida"
```

Las ocho pruebas del builder cubren selección de diseños y perfiles, JPEG en tres niveles, transparencia real y alfa opaco, píxeles sin pérdida, conservación de dimensiones, rechazo de rutas externas/enlaces y detección de alteraciones, archivos sobrantes y partidas. La verificación del manifiesto se realiza antes de crear el ZIP; puede repetirse después de extraerlo y antes de añadir datos opcionales.

Con `--con-datos` se ejecutan además desde la copia empaquetada, con ventanas ocultas, las pruebas de habitaciones/monitor y el recorrido hasta THE END. Sus capturas y perfiles temporales permanecen en `apps/juego/trabajo/`, fuera del paquete. Estas pruebas no acreditan todos los equipos Windows ni cierran la revisión pendiente de alternativas y continuidad de partidas.

Requisitos para construir: dependencias de pnpm, entorno del proyecto preparado y herramientas C++/SDK y redistribuibles x64 de Visual Studio. `sharp` es una dependencia de desarrollo fijada en el lockfile; no hace falta instalar Node, Python o pnpm en el equipo receptor.

## Primera beta comprobada: 20 de octubre de 2026

El builder se incorporó en `0ee2cef`. La primera beta se generó durante su preparación e incluía los cinco diseños, antes de añadir el selector: sus cifras equivalen ahora a usar `--habitacion todas`. Su manifiesto conserva la base `476d7a2` y `cambios_locales: true`. No se ha sustituido ese dato por una revisión posterior.

| Dato                                     | Resultado comprobado                                                                     |
| ---------------------------------------- | ---------------------------------------------------------------------------------------- |
| Versión y perfil                         | 0.9.0, equilibrado, con datos locales del juego.                                         |
| Archivos del paquete                     | 257; sin ROM, partidas ni configuración DOS.                                             |
| Imágenes activas                         | 32 PNG.                                                                                  |
| Tamaño original de esas imágenes         | 137.833.429 bytes (131,4 MiB).                                                           |
| Tamaño optimizado                        | 61.223.186 bytes (58,4 MiB), un55,6 % menos.                                             |
| ZIP corregido con carpeta interior corta | 69.600.713 bytes, aproximadamente66,4 MiB.                                               |
| Pruebas del builder                      | Cuatro pruebas Node correctas.                                                           |
| Pruebas SDL desde el paquete             | 52 capturas de habitaciones y recorrido hasta THE END correctos.                         |
| Comprobación tras extraer                | 257 archivos verificados; habitaciones y recorrido final con salida correcta.            |
| Imagen final contrastada                 | 64.000 píxeles coincidentes con la referencia del EXE.                                   |
| Arranque sin argumentos                  | Correcto desde otro directorio, con PATH limitado al sistema y pantalla/audio ficticios. |
| Revisiones de código                     | Calidad y comprobación del monorepositorio correctas.                                    |

La primera extracción detectó que un nombre interior demasiado largo impedía cargar el banner desde una ruta profunda. Se corrigió usando `CurseReborn-0.9.0` como carpeta interior y se repitieron las comprobaciones desde el ZIP corregido. El ensayo de arranque sin argumentos comprobó el inicio de la aplicación; no equivale a una sesión manual completa.

La copia de entrega se llama `Curse-of-Enchantia-Reborn-0.9.0-beta-Windows.zip`. Su SHA-256 es `4a8f2812f1621dd5d732ac6c0bb2cdbc467fee1a8161766cd3946c07ca5c54a6`. Las cifras corresponden a esa entrega y no garantizan el tamaño de futuros paquetes. El perfil sin pérdida tiene pruebas unitarias; no se generó una segunda beta completa con ese perfil en esta tanda.

## Beta reducida con la habitación inicial

Por petición posterior de Àlex, el comportamiento predeterminado pasa a incluir únicamente la inicial del JSON. Se ha ejecutado `pnpm empaquetar:beta --con-datos` con `02-buhardilla`, sin modificar el catálogo maestro.

| Dato                                   | Resultado comprobado                                                                                        |
| -------------------------------------- | ----------------------------------------------------------------------------------------------------------- |
| Diseños incluidos e inicial            | Solo `02-buhardilla`.                                                                                       |
| Variantes de habitación                | Cinco PNG, con todas sus proporciones.                                                                      |
| Imágenes activas totales               | 12 PNG, incluyendo presentación, marco e icono.                                                             |
| Archivos del paquete                   | 237, sin los20 PNG de los otros cuatro diseños.                                                             |
| Imágenes antes y después de comprimir  | 55.892.294 →24.568.425 bytes (53,3 →23,4 MiB).                                                              |
| ZIP                                    | 32.819.895 bytes, aproximadamente31,3 MiB; cerca de un53 % menos que la primera beta con todos los diseños. |
| Pruebas Node                           | Seis correctas, incluidas selección, conservación del maestro y parámetros inválidos.                       |
| Pruebas SDL desde la copia distribuida | 28 capturas del diseño incluido y recorrido hasta THE END correctos.                                        |
| Integridad                             | 237 archivos verificados; JSON y catálogo compilado contienen solo la selección.                            |

Se han generado además los catálogos JSON/C++ para `todas` y `03-lucasarts`, comprobando el número de diseños y la inicial. En esta ampliación no se generaron dos ZIP adicionales para esas opciones. El desarrollo normal conserva todos los diseños y las pruebas SDL usan ahora la cantidad real de diseños compilados.

La copia reducida de entrega se llama `Curse-of-Enchantia-Reborn-0.9.0-beta-Buhardilla-Windows.zip`. Su SHA-256 es `43daf4ebb7528117f48a70211243ed338c8cae879edaa0167c50d1a7be5b26c1`. La primera beta con cinco diseños se mantiene como resultado histórico, no como el tamaño esperado del comando predeterminado actual.
