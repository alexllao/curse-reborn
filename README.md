# Curse Reborn

**Volver a Enchantia. Recuperar el juego original. Darle una nueva vida.**

Curse Reborn nace de un recuerdo muy concreto: *Curse of Enchantia* fue el primer juego al que su creador jugó en un ordenador. Este proyecto es una forma de regresar a aquella aventura, entender cómo funcionaba y construir una base que permita disfrutarla y remasterizarla con cuidado.

La primera meta es **desensamblar `CURSE.EXE`, entender sus rutinas y trasladar su lógica a C++ con SDL**. Los recursos originales acompañarán esa reconstrucción. Sobre esa base queremos desarrollar una remasterización que conserve su identidad y abra la puerta a mejoras gráficas, sonoras y de uso.

## El proyecto que queremos construir

El trabajo se plantea en dos etapas:

1. **Reconstrucción del original desde su código.** Desensamblar el ejecutable, seguir sus llamadas y saltos, identificar sus rutinas y trasladarlas progresivamente a C++ con SDL. Los formatos de datos se investigarán siguiendo las rutinas originales de carga.
2. **Remasterización.** Incorporar nuevos recursos y mejoras sobre un motor cuyo funcionamiento ya se haya comprobado frente al original.

La fidelidad será el punto de partida: escenarios, personajes, animaciones, inventario y puzles deben entenderse antes de transformarse. La intención es poder distinguir el comportamiento original de las mejoras que se introduzcan más adelante.

Cada parte trasladada deberá conservar una referencia a su dirección en el ejecutable, una explicación de su función y las comprobaciones realizadas. SDL sustituirá las dependencias de DOS y del hardware una vez identificado el comportamiento que deben conservar.

## Estado actual

**Primera fase de desensamblado estático. Todavía no hay una versión jugable ni rutinas trasladadas a SDL.**

Ya existe una herramienta propia que lee la estructura DOS MZ y sigue llamadas y saltos desde el punto de entrada, tres manejadores identificados y cinco tablas de destinos anotadas. La pasada actual alcanza **12.553 instrucciones candidatas y 104 destinos de llamada directa**, y señala **tres transferencias indirectas pendientes**. Estas cifras se obtienen por análisis estático; no representan el porcentaje completado del motor ni una ejecución verificada.

El [mapa inicial del ejecutable](documentacion/analisis_ejecutable.md) recoge el arranque y las reubicaciones. El [análisis del bucle central](documentacion/bucle_principal.md) documenta su recorrido, la selección de escenas y las tablas indirectas. La herramienta tiene 18 comprobaciones con programas sintéticos que no requieren los archivos originales.

Se ha realizado una primera inspección estática de una copia para DOS:

| Elemento | Resultado comprobado |
| --- | --- |
| Archivos examinados | 196, incluidos el juego, el instalador y los datos |
| Ejecutable principal | `CURSE.EXE`, formato DOS MZ de 16 bits, 111.606 bytes |
| Recursos separados | Archivos `.MAP`, `.SPR`, `.BRD`, `.DAT`, `.PAL` y `.SFX`, entre otros |
| Compresión identificada | Tres archivos con cabecera RNC de método 1 |
| Pantalla de título | `TITLE.DAT` declara 64.000 bytes descomprimidos; su imagen aún no se ha recuperado |

Estas observaciones permiten comenzar el trabajo, pero todavía falta determinar la estructura de buena parte de los recursos y reconstruir las rutinas del juego. Las extensiones y los nombres de archivo son pistas; su significado debe confirmarse mediante análisis.

Desensamblar el ejecutable no devuelve el código fuente original. Será necesario interpretar instrucciones, seguir referencias y contrastar cada parte reconstruida con el comportamiento del juego en DOSBox.

## Hoja de ruta

- [x] Inspeccionar inicialmente el ejecutable y el inventario de archivos.
- [x] Crear el repositorio y establecer la rama `development`.
- [x] Crear una herramienta de desensamblado por flujo y validación de cabeceras MZ.
- [x] Documentar el primer mapa de arranque y los manejadores de interrupción identificados.
- [x] Incorporar cinco tablas de saltos anotadas y vinculadas a la huella del ejecutable.
- [x] Localizar y documentar el recorrido estático del bucle central y la selección de escenas.
- [ ] Completar los destinos indirectos restantes y las estructuras de estado del juego.
- [ ] Validar el recorrido identificado durante la ejecución del original.
- [ ] Seguir las rutinas originales de carga y descompresión de recursos.
- [ ] Documentar los formatos de escenarios, sprites y animaciones a partir de esas rutinas.
- [ ] Trasladar las primeras rutinas a C++ con referencias a sus direcciones originales.
- [ ] Sustituir el acceso gráfico al hardware por SDL y comprobar el resultado.
- [ ] Trasladar los controles, el movimiento y las interacciones.
- [ ] Trasladar el inventario, los eventos y los puzles.
- [ ] Trasladar el sonido, la temporización y las partidas guardadas.
- [ ] Comprobar el desarrollo completo de la aventura frente al original.
- [ ] Definir y desarrollar las mejoras de la remasterización.

**Siguiente hito de reconstrucción:** completar las estructuras de estado y la tabla dinámica de llamadas que quedan pendientes, y preparar una primera rutina para su traslado verificable a C++.

## Organización del repositorio

El proyecto se organiza como un **monorepositorio**: el motor, las herramientas de investigación, la documentación y las pruebas comparten este repositorio y su historial. Ya hay una herramienta de análisis y sus comprobaciones; el motor sigue pendiente. Las carpetas vacías conservan un archivo `.gitkeep`. Las instrucciones de compilación del motor se añadirán cuando exista su primera implementación.

```text
curse-reborn/
├── motor/              # Motor del juego basado en SDL
├── herramientas/       # Análisis del ejecutable y apoyo a su reconstrucción
├── documentacion/      # Mapa de código, formatos y decisiones del proyecto
├── pruebas/            # Comprobaciones del motor y las herramientas
└── datos_originales/   # Copia local del juego; solo se versiona .gitkeep
```

| Archivo | Contenido |
| --- | --- |
| [README.md](README.md) | Propósito, estado y hoja de ruta |
| [AGENTS.md](AGENTS.md) | Instrucciones para asistentes que trabajen en el repositorio |
| [.gitignore](.gitignore) | Exclusiones de archivos locales y recursos originales |
| [package.json](package.json) | Gestión del proyecto y comandos de pnpm |
| [pnpm-lock.yaml](pnpm-lock.yaml) | Archivo de bloqueo de pnpm |

## Puesta en marcha con pnpm

Se utiliza **Node 22 o posterior y pnpm 11.18.0**. La versión de Node está indicada en `.nvmrc` y el gestor en `package.json`. El análisis utiliza además Python 3.9 o posterior, disponible como `py`, `python` o `python3`. Si necesitas seleccionar un ejecutable concreto, indica su ruta en la variable de entorno `PYTHON`.

Desde la raíz del repositorio:

```sh
pnpm install --frozen-lockfile
pnpm preparar
pnpm estado
pnpm analizar
pnpm comprobar
```

| Comando | Qué hace |
| --- | --- |
| `pnpm preparar` | Crea un entorno Python en `trabajo/entorno/` e instala la versión fijada del desensamblador. Necesita acceso a la red la primera vez. |
| `pnpm estado` | Muestra la disponibilidad del ejecutable, el entorno local y las dependencias. Si falta preparar el entorno, termina con un aviso y un código de error. |
| `pnpm analizar` | Analiza `datos_originales/CURSE/CURSE.EXE`; guarda los resultados en `trabajo/analisis_ejecutable/`. Requiere la copia original. |
| `pnpm comprobar` | Ejecuta las comprobaciones con programas sintéticos, sin necesitar los archivos del juego. |

La gestión general del proyecto se hace con Node y pnpm; Python queda integrado para el análisis con Capstone. No hace falta activar manualmente el entorno Python. La preparación es explícita: instalar con pnpm no descarga dependencias de Python ni ejecuta el juego.

Puedes pasar opciones a la herramienta de análisis, por ejemplo `pnpm analizar --raiz 0000:0276`. Los comandos de gestión contemplan Windows, Linux y macOS; por ahora se han comprobado en Windows. Todavía no existe un comando para compilar o arrancar el motor, porque el motor SDL está pendiente de implementar.

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

## Recursos originales y autoría

*Curse of Enchantia* es una obra de Core Design. Curse Reborn es un proyecto independiente y no oficial, y reconoce el trabajo de quienes crearon el juego original.

El repositorio no incluye el ejecutable ni los archivos comerciales del juego. Para las pruebas con recursos originales será necesario aportar una copia propia. Los archivos del juego, los recursos extraídos y las partidas deben permanecer fuera del historial del repositorio.

La licencia del código nuevo está pendiente de definir. Antes de incorporar código de terceros se revisarán su licencia y sus requisitos de atribución.

---

Este proyecto empieza con el deseo de volver al primer juego de ordenador de su creador y darle un futuro sin perder aquello que lo hizo especial.
