# Monorepositorio de Curse Reborn

El workspace de pnpm contiene tres paquetes privados. Una sola instalación y un solo `pnpm-lock.yaml` fijan las dependencias. Las herramientas comunes permanecen en la raíz; no se añaden Nx, Turborepo ni otro sistema de tareas.

| Ubicación           | Responsabilidad                                                                                                         |
| ------------------- | ----------------------------------------------------------------------------------------------------------------------- |
| `apps/juego/`       | Motor C++, SDL, dependencias propias de la plataforma y manifiesto de fuentes. Nativo integrado; WebAssembly pendiente. |
| `apps/web/`         | Espacio reservado para la futura web; todavía sin sitio ni framework.                                                   |
| `packages/assets/`  | Imágenes y otros recursos propios compartidos, incluyendo habitaciones y marcos.                                        |
| `herramientas/`     | Compilación, análisis del EXE, generación de catálogos y calidad.                                                       |
| `pruebas/`          | Comparadores del motor frente al EXE, puentes C++ y pruebas del analizador.                                             |
| `documentacion/`    | Contratos, mapa de ensamblador, recorridos y decisiones.                                                                |
| `licencias/`        | Avisos comunes de dependencias.                                                                                         |
| `datos_originales/` | Juego y ROM locales, excluidos de Git salvo `.gitkeep`.                                                                 |
| `trabajo/`          | Dependencias descargadas, entorno Python, ejecutables e informes locales.                                               |

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

Las herramientas resuelven la raíz desde su ubicación, así que también se pueden ejecutar mediante `pnpm --filter` sin depender del directorio actual. `herramientas/rutas_proyecto.mjs` comparte la ubicación de fuentes y recursos.

El CMake de la raíz delega en `apps/juego/CMakeLists.txt`. También se admite configurar directamente `apps/juego`. En ambos casos el ejecutable queda en la raíz del directorio de compilación, con las DLL y recursos relativos que necesita.

## Rutas de desarrollo y distribución

El código se traslada sin modificar sus rutinas. El manifiesto `apps/juego/fuentes_motor.json` conserva rutas relativas al paquete; SDL, el puente de comparación y CMake compilan esa única lista. El mapa del desensamblado referencia ahora `apps/juego/motor/...`.

Los PNG y sus metadatos viven en `packages/assets/`. Al construir, las herramientas copian los recursos junto al ejecutable como `assets/original/...`; esa es una ruta de distribución, no otra copia de fuentes en el repositorio. Los maestros se conservan sin convertir ni comprimir con pérdida.

Los datos originales, las ROM, los perfiles, el entorno Python y resultados existentes permanecen en sus carpetas locales de la raíz. No hace falta volver a copiarlos por esta reorganización.

`pnpm monorepo:comprobar` revisa manifiestos, includes relativos, recursos y referencias del ensamblador sin necesitar los datos comerciales. La calidad y las comprobaciones SDL/nativas validan los consumidores reales de estas rutas.

## Siguientes tareas

- Crear la web y una compilación WebAssembly con el mismo motor cuando se aborden esas plataformas.
- Preparar `pnpm empaquetar:beta`: ejecutable, dependencias, recursos propios, versión y controles en un ZIP, con optimización de copias de distribución. Todavía no implementado.
- Integrar sonidos de presentación del ordenador: botón, ventilador/disco y recuento de RAM; deben terminar antes de la música del juego y respetar M/volumen. No son efectos del juego DOS.
