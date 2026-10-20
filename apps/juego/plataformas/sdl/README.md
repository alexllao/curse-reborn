# Plataforma SDL

| Carpeta     | Contenido                                            |
| ----------- | ---------------------------------------------------- |
| `audio/`    | Salida de sonido y sintetizadores musicales.         |
| `interfaz/` | Partidas, idiomas, fichas, avisos y contador de FPS. |
| `lanzador/` | Habitaciones, presentación y sonidos del ordenador.  |
| `video/`    | Filtros, escaladores, cursor y marcos.               |
| `pruebas/`  | Comprobaciones de interfaz y recorridos SDL.         |

En este nivel quedan el arranque, la introducción, el adaptador del motor, el catálogo de vistas, el registro y el contador de tiempo efectivo. La lógica del juego continúa en `../../motor/`.

La compilación por pnpm y CMake utiliza estas mismas fuentes. No hay copias de compatibilidad en las ubicaciones anteriores. Las comprobaciones siguen disponibles mediante sus órdenes `pnpm comprobar:*`.
