# Pruebas del motor

| Carpeta         | Contenido                                                                         |
| --------------- | --------------------------------------------------------------------------------- |
| `motor/`        | Estado, acciones comunes, azar, secuencias, partidas y puente C++ de comparación. |
| `recorridos/`   | Puzles, personajes, salidas y recorridos persistentes de las salas.               |
| `audio/`        | Sonido digital, altavoz, música y sintetizadores.                                 |
| `video/`        | Composición, paneles, gráficos, compresión y escaladores.                         |
| `distribucion/` | Pruebas Node de imágenes, integridad y exclusiones del builder beta.              |

Las órdenes `pnpm comparar:*` no cambian. `ejecutar.py` es su entrada común: dispone las importaciones compartidas y ejecuta el comparador solicitado. Para una ejecución manual se usa `python apps/juego/pruebas/ejecutar.py comparar_aleatorio.py --biblioteca <biblioteca>` desde la raíz del repositorio.

`python apps/juego/pruebas/ejecutar.py --comprobar-importaciones` revisa las importaciones de las cuatro áreas sin ejecutar los recorridos. Los resultados y las cachés no se guardan aquí: siguen en `../trabajo/`, fuera de Git. Las comprobaciones específicas de SDL están en `../plataformas/sdl/pruebas/`.
