# Monorepositorio de Curse Reborn

El workspace de pnpm contiene tres paquetes privados. Una sola instalación y un solo `pnpm-lock.yaml` fijan las dependencias. Todo lo específico del port pertenece a `apps/juego`: motor, herramientas, documentación, licencias, pruebas y datos locales. La raíz conserva los comandos y configuraciones generales. No se añaden Nx, Turborepo ni otro sistema de tareas.

| Ubicación                      | Responsabilidad                                                                                                         |
| ------------------------------ | ----------------------------------------------------------------------------------------------------------------------- |
| `apps/juego/`                  | Motor C++, SDL, dependencias propias de la plataforma y manifiesto de fuentes. Nativo integrado; WebAssembly pendiente. |
| `apps/web/`                    | Espacio reservado para la futura web; todavía sin sitio ni framework.                                                   |
| `packages/assets/`             | Imágenes y otros recursos propios compartidos, incluyendo habitaciones y marcos.                                        |
| `apps/juego/herramientas/`     | Compilación, análisis del EXE, generación de catálogos y calidad.                                                       |
| `apps/juego/pruebas/`          | Comparadores del motor frente al EXE, puentes C++ y pruebas del analizador.                                             |
| `apps/juego/documentacion/`    | Contratos, mapa de ensamblador, recorridos y decisiones.                                                                |
| `apps/juego/licencias/`        | Avisos comunes de dependencias.                                                                                         |
| `apps/juego/datos_originales/` | Juego y ROM locales, excluidos de Git salvo la guía y `.gitkeep`.                                                       |
| `apps/juego/trabajo/`          | Dependencias descargadas, entorno Python, ejecutables e informes locales.                                               |

## Comandos

Desde la raíz, los comandos anteriores siguen vigentes: `pnpm preparar`, `pnpm ventana`, `pnpm sdl:compilar`, `pnpm comparar:*`, `pnpm comprobar:*` y `pnpm calidad`.

Además:

```text
pnpm install --frozen-lockfile
pnpm monorepo:comprobar
pnpm --filter @curse-reborn/juego compilar
pnpm --filter @curse-reborn/juego dev
pnpm --filter @curse-reborn/juego comprobar
```

Las herramientas resuelven la raíz desde su ubicación, así que también se pueden ejecutar mediante `pnpm --filter` sin depender del directorio actual. `apps/juego/herramientas/rutas_proyecto.mjs` comparte la ubicación de fuentes y recursos.

La compilación se configura desde `apps/juego/CMakeLists.txt`; no hay un CMake adicional en la raíz. `.clang-format` y `ruff.toml` también están en `apps/juego`, junto al código que configuran. Los comandos de pnpm se siguen ejecutando desde la raíz; el ejecutable queda en el directorio de compilación, con las DLL y recursos relativos que necesita.

## Rutas de desarrollo y distribución

El código se traslada sin modificar sus rutinas. El manifiesto `apps/juego/fuentes_motor.json` conserva rutas relativas al paquete; SDL, el puente de comparación y CMake compilan esa única lista. El mapa del desensamblado referencia ahora `apps/juego/motor/...`.

Los PNG y sus metadatos viven en `packages/assets/`. Al construir, las herramientas copian los recursos junto al ejecutable como `assets/original/...`; esa es una ruta de distribución, no otra copia de fuentes en el repositorio. Los maestros se conservan sin convertir ni comprimir con pérdida.

Los datos originales y las ROM permanecen en `apps/juego/datos_originales/`, con su guía de colocación. El entorno Python, dependencias, ejecutables, referencias e informes locales están en `apps/juego/trabajo/`, excluido de Git; las pruebas versionadas están en `apps/juego/pruebas/`. No hace falta volver a copiar los originales.

`pnpm monorepo:comprobar` revisa manifiestos, includes relativos, recursos y referencias del ensamblador sin necesitar los datos comerciales. La calidad y las comprobaciones SDL/nativas validan los consumidores reales de estas rutas.

Al actualizar una copia que aún tenga `trabajo/` en la raíz, los comandos del proyecto y calidad lo trasladan a `apps/juego/trabajo/` antes de usarlo. También trasladan `CURSE/` y `ROMS/` desde los antiguos `datos_originales/` a `apps/juego/datos_originales/`, conservando la guía incluida en Git. Se conserva el contenido, sin descargar de nuevo las dependencias. Si un destino ya existe, no se mezcla ni sobrescribe automáticamente. La carpeta antigua de datos se retira únicamente si queda vacía.

## Siguientes tareas

- Crear la web y una compilación WebAssembly con el mismo motor cuando se aborden esas plataformas.
- `pnpm empaquetar:beta` ya genera carpeta y ZIP para Windows x64, con recursos propios optimizados, dependencias, licencias, versión e integridad. `--con-datos` incluye la copia local del juego y ejecuta las pruebas ocultas desde el paquete. Véase [Distribución](distribucion.md).
- Los sonidos de presentación del ordenador ya están integrados: botón, ventilador/disco y recuento de RAM. Terminan antes de la música del juego y respetan M/volumen. No son efectos del juego DOS.
