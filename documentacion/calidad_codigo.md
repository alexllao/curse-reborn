# Formato y revisión del código

Las herramientas se ejecutan desde la raíz y solo revisan código y documentación propios. Los datos originales, `apps/juego/trabajo/`, los binarios y los avisos de licencia de terceros quedan excluidos.

## Preparación

```sh
pnpm install --frozen-lockfile
pnpm preparar
```

Se necesita Node 22.13 o posterior para ESLint 10; la versión de Node usada por el propietario es 22.23.3. Las dependencias JavaScript y Ruff están fijadas en sus respectivos archivos de dependencias. El formateo de C++ requiere `clang-format` 21 en la ruta de ejecución, o su ruta completa en la variable `CLANG_FORMAT`.

## Comandos

```sh
pnpm formatear
pnpm formato:comprobar
pnpm lint
pnpm calidad
```

- `formatear` aplica Prettier a JavaScript, JSON, YAML y Markdown; clang-format al C++ propio; y Ruff a Python.
- `formato:comprobar` comprueba esos mismos formatos sin modificar archivos.
- `lint` ejecuta las reglas recomendadas de ESLint para Node y las comprobaciones de errores de Ruff para Python.
- `calidad` agrupa las comprobaciones de formato y los analizadores.
- `lint:corregir` permite las correcciones automáticas de ESLint y después revisa Python; no aplica correcciones Python que puedan cambiar el comportamiento.

Las configuraciones están en `.prettierrc.json`, `.prettierignore`, `eslint.config.mjs`, `.clang-format` y `ruff.toml`. ESLint utiliza su configuración plana y desactiva reglas de estilo que pueden entrar en conflicto con Prettier, siguiendo las [indicaciones de integración de Prettier](https://prettier.io/docs/install.html) y la [configuración oficial de ESLint](https://eslint.org/docs/latest/use/configure/configuration-files).

El formateador de C++ no es un analizador semántico. El núcleo y la ventana siguen compilándose con los avisos del compilador tratados como errores. Los comandos de calidad no sustituyen las comparaciones contra el ejecutable original ni ejecutan una partida.

## Comentarios de funciones

Cada función nueva debe llevar encima un comentario breve en castellano que explique su propósito. Las rutinas reconstruidas deben incluir la dirección del original y sus contratos o límites importantes. Los comentarios deben ayudar a entender el código sin repetirlo línea por línea.
