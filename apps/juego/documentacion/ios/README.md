# Primera integración de iPad

El destino iOS pertenece a `apps/juego/plataformas/ios`. Comparte las19 fuentes de SDL mediante `plataformas/sdl/fuentes.cmake` y las62 fuentes del motor mediante `fuentes_motor.json`. No hay otro paquete JavaScript ni una copia del juego. Herramientas en `herramientas/ios`, pruebas de preparación en `pruebas/ios`; los proyectos Xcode y los recursos generados quedan ignorados bajo `trabajo/ios`.

## Preparar en el Mac

Se necesita Xcode completo con el SDK de iOS, Node22.13 o posterior, pnpm11, CMake3.24 o posterior y FFmpeg para comprimir las cinco canciones. Si se usa Homebrew, `brew install node cmake ffmpeg` proporciona estas herramientas; pnpm debe estar disponible aparte. En Xcode, Settings/Locations permite seleccionar las herramientas de línea de comandos de esa instalación.

Actualizar la copia del repositorio en el Mac y colocar los recursos locales originales en `apps/juego/datos_originales/CURSE`: `CURSE.EXE` y su carpeta `DATA`. No llegan mediante Git; se transfieren desde la copia del usuario. No se descargan datos del juego ni se incorporan partidas o ROM.

Desde la raíz:

```sh
git pull
pnpm install
pnpm ios
```

El preparador comprueba la huella del EXE, genera catálogos, copia únicamente la habitación inicial y prepara música MP3 y fondos JPEG de calidad92. Los maestros y los catálogos de origen se conservan. CMake descarga las fuentes oficiales de SDL3.4.18, verifica su SHA256 y compila una biblioteca estática para el SDK elegido; no usa el framework macOS ni la DLL Windows. El proyecto se abre en Xcode al finalizar la configuración.

En Xcode, seleccionar el esquema `curse-reborn`, el iPad conectado y el equipo personal en Signing & Capabilities; después Run. La firma queda en manos de Xcode. El preparador no instala certificados, no selecciona una cuenta Apple y no sube nada a TestFlight o App Store.

```sh
pnpm ios --simulador
pnpm ios --equipo ABC1234567
pnpm ios --datos "/ruta/a/CURSE"
pnpm ios --imagenes equilibrada
pnpm ios --imagenes sin-perdida
pnpm ios --musica wav
pnpm ios --sin-abrir
```

El simulador usa arm64 en Mac Apple Silicon y x86_64 en Intel. El dispositivo usa arm64. Sus proyectos se generan en carpetas separadas. `--musica wav` evita FFmpeg a cambio de una aplicación más grande. Cada preparación crea una carpeta nueva de recursos, sin borrar resultados anteriores. No deben compartirse esas carpetas ni la app con los datos originales incluidos sin la autorización correspondiente.

## Primeros controles

- Un dedo mueve el cursor; soltar produce el clic izquierdo.
- Mantener500ms antes de soltar produce el clic derecho del juego original.
- Un segundo dedo abre o cierra el menú del port mediante su recorrido existente de Mayús+Esc. Cancela los clics de ambos dedos.
- El ratón o teclado conectado conserva el manejo de escritorio. Se desactiva el ratón táctil automático de SDL para evitar acciones duplicadas.

Los eventos táctiles se convierten en coordenadas de ventana y pasan por la misma transformación al monitor/juego que el ratón. La integración usa el bucle SDL existente y no cambia la lógica de los puzles.

## Recursos y dependencias de Apple

- El icono procede de `packages/assets/lanzador/icono/curse-reborn.png`, el mismo PNG del ejecutable. Se generan diez entradas para iPad/marketing, con nueve tamaños físicos, incluida la imagen1024. El exterior transparente se compone sobre azul oscuro; no se rediseña. Xcode compila `Assets.xcassets` como catálogo y aplica la máscara del sistema.
- PNG y transparencias siguen en SDL. Los JPEG usan ImageIO/CoreGraphics; hay que comprobar orientación y color en el dispositivo.
- La música remaster, OPL2/OPL3 y efectos comparten su implementación. `CURSE_REBORN_SIN_MT32` evita depender de la biblioteca dinámica de Munt en este primer destino. El perfil MT-32 no se ofrece si no está disponible. Windows conserva Munt.
- Partidas y preferencias ya usan `SDL_GetPrefPath`; los recursos son de solo lectura dentro del bundle. La primera app está configurada para iPad, horizontal, iOS15 o posterior. El nombre y la versión proceden del paquete del juego.

## Validación y pendientes

Pruebas de preparación sin Xcode:

```sh
pnpm comprobar:ios
```

La opción `--solo-recursos` permite ejecutar la preparación en otra plataforma. El ejecutable SDL ofrece `--comprobar-entrada-tactil` para comprobar los gestos con eventos reproducibles, sin pantalla táctil.

Comprobado en Windows el23/10/2026: tres pruebas de preparación iOS y ocho del builder, generación local completa de17 imágenes/cinco canciones WAV/192 archivos del juego y catálogo de iconos. Compilación SDL O2 con avisos como errores, sintaxis del adaptador sin Munt sin incluir sus cabeceras, prueba de gestos y22 capturas de regresión del menú correctas. ESLint, Prettier, formato C++ y contrato del monorepo pasan directamente. `pnpm calidad` sigue abortando durante la comprobación de dependencias/registro sin TTY; no se declara aprobado el agregado. Resultados ignorados en `trabajo/ios`. La preparación MP3 en este entorno sigue limitada por el lanzamiento de FFmpeg desde Node, por lo que esta prueba de recursos usó `--musica wav`.

Esta integración inicial todavía requiere compilar con el SDK de Apple y probar en un iPad real. No se ha generado una IPA ni se afirma que ya funcione en iPad. Las comprobaciones Windows no validan UIKit, ImageIO, la firma o la incorporación del catálogo de iconos. También quedan por validar el paso a segundo plano, interrupciones de audio, presión de memoria, zonas táctiles pequeñas y la tipografía: Apple utiliza de momento el respaldo SDL, no la fuente GDI de Windows. Guardar/cargar nombres puede necesitar ajustes del teclado virtual. No es todavía una entrega móvil cerrada.

Referencias oficiales: [SDL3 en iOS](https://wiki.libsdl.org/SDL3/README-ios), [SDL3 con CMake](https://wiki.libsdl.org/SDL3/README-cmake). La disponibilidad para instalar localmente con Xcode es independiente de los derechos necesarios para distribuir el juego.
