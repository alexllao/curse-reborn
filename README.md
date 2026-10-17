# Curse Reborn

**Volver a Enchantia. Recuperar el juego original. Darle una nueva vida.**

Curse Reborn nace de un recuerdo muy concreto: *Curse of Enchantia* fue el primer juego al que su creador jugó en un ordenador. Este proyecto es una forma de regresar a aquella aventura, entender cómo funcionaba y construir una base que permita disfrutarla y remasterizarla con cuidado.

La primera meta es reconstruir su funcionamiento en C++ con SDL, utilizando los recursos de la versión original para DOS. Sobre esa base queremos desarrollar una remasterización que conserve su identidad y abra la puerta a mejoras gráficas, sonoras y de uso.

## El proyecto que queremos construir

El trabajo se plantea en dos etapas:

1. **Reconstrucción del original.** Investigar el ejecutable y los archivos de datos, documentar sus formatos y reproducir el comportamiento del juego con un motor propio basado en SDL.
2. **Remasterización.** Incorporar nuevos recursos y mejoras sobre un motor cuyo funcionamiento ya se haya comprobado frente al original.

La fidelidad será el punto de partida: escenarios, personajes, animaciones, inventario y puzles deben entenderse antes de transformarse. La intención es poder distinguir el comportamiento original de las mejoras que se introduzcan más adelante.

## Estado actual

**Fase inicial de investigación y documentación. Todavía no hay una versión jugable ni un motor SDL implementado.**

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
- [ ] Descomprimir los recursos RNC y comprobar su integridad.
- [ ] Recuperar y visualizar la pantalla de título con su paleta.
- [ ] Documentar los formatos de escenarios, sprites y animaciones.
- [ ] Crear un primer visor de recursos con SDL.
- [ ] Mostrar un escenario con sus personajes y animaciones.
- [ ] Reconstruir los controles, el movimiento y las interacciones.
- [ ] Reconstruir el inventario, los eventos y los puzles.
- [ ] Incorporar sonido, temporización y partidas guardadas.
- [ ] Comprobar el desarrollo completo de la aventura frente al original.
- [ ] Definir y desarrollar las mejoras de la remasterización.

**Primer hito visual:** mostrar la pantalla de título original a partir de sus archivos de datos. El siguiente será un escenario real con sus sprites.

## Organización del repositorio

El proyecto se organiza como un **monorepositorio**: el motor, las herramientas de investigación, la documentación y las pruebas compartirán este repositorio y su historial. Las carpetas iniciales contienen un archivo `.gitkeep` para conservar su estructura en Git; todavía no incluyen implementaciones. Las instrucciones de compilación se añadirán cuando exista la primera implementación.

```text
curse-reborn/
├── motor/              # Motor del juego basado en SDL
├── herramientas/       # Análisis, extracción y conversión de recursos
├── documentacion/      # Formatos, investigación y decisiones del proyecto
├── pruebas/            # Comprobaciones del motor y las herramientas
└── datos_originales/   # Copia local del juego; solo se versiona .gitkeep
```

| Archivo | Contenido |
| --- | --- |
| [README.md](README.md) | Propósito, estado y hoja de ruta |
| [AGENTS.md](AGENTS.md) | Instrucciones para asistentes que trabajen en el repositorio |
| [.gitignore](.gitignore) | Exclusiones de archivos locales y recursos originales |

### Dónde colocar el juego original

Coloca los archivos de tu copia para DOS dentro de `datos_originales/`, de modo que el ejecutable y los datos queden así:

```text
datos_originales/
├── .gitkeep
├── CURSE.EXE
├── CURSE.CFG
├── SETUP.EXE
└── DATA/
    └── ...
```

En el ZIP examinado, esos archivos están dentro de `CURSE/`: copia **el contenido de esa carpeta** a `datos_originales/`. Conserva los nombres originales de los archivos y las subcarpetas.

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
