# Curse Reborn

Monorepositorio de Àlex Llaó para el port de _Curse of Enchantia_, sus recursos propios y la futura web.

| Paquete                                      | Contenido                                                                      |
| -------------------------------------------- | ------------------------------------------------------------------------------ |
| [apps/juego](apps/juego/README.md)           | Port C++/SDL, herramientas, documentación, licencias, pruebas y datos locales. |
| [apps/web](apps/web/README.md)               | Espacio reservado para la web; todavía sin implementar.                        |
| [packages/assets](packages/assets/README.md) | Habitaciones, marcos y recursos propios compartidos.                           |

Se utiliza **pnpm workspaces**, una instalación y un lockfile común. La versión WebAssembly está pendiente.

Desde la raíz:

```text
pnpm install --frozen-lockfile
pnpm preparar
pnpm ventana
```

Antes de jugar, coloca tu copia del juego y las ROM según la [guía de datos originales](apps/juego/datos_originales/README.md). Los comandos trasladan automáticamente las antiguas carpetas locales cuando el destino aún no existe; conservan las copias existentes.

`pnpm calidad` revisa formato y código; `pnpm monorepo:comprobar` comprueba las rutas. Los comandos anteriores de compilación y comparación siguen disponibles desde la raíz.

Consulta [los controles](apps/juego/documentacion/controles.md), [la documentación del port](apps/juego/README.md) y [la organización del repositorio](apps/juego/documentacion/monorepo.md).

La raíz conserva las configuraciones generales de pnpm, CMake, formato y calidad. El juego original, las ROM, las partidas y los resultados locales no se distribuyen en este repositorio.
