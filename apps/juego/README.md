# Juego de Curse Reborn

Paquete `@curse-reborn/juego`: contiene el único motor C++ del port, las plataformas y sus dependencias de terceros. SDL sigue siendo la plataforma integrada. La futura compilación WebAssembly usará este mismo motor; todavía no está implementada.

Desde la raíz:

- `pnpm --filter @curse-reborn/juego dev`: compilar y abrir el juego.
- `pnpm --filter @curse-reborn/juego compilar`: compilar sin abrir ventanas.
- `pnpm --filter @curse-reborn/juego comprobar`: comprobar las habitaciones con ventanas ocultas.

Los comandos anteriores `pnpm ventana`, `pnpm sdl:compilar` y `pnpm comprobar:*` se conservan en la raíz. El manifiesto `fuentes_motor.json` es relativo a este paquete y lo comparten CMake y los compiladores de comparación/SDL.

Los recursos propios están en `../../packages/assets/`. Los originales y las ROM siguen en `../../datos_originales/`; las dependencias descargadas, compilaciones e informes siguen en `../../trabajo/`, sin versionar.

Más detalles: [organización del monorepositorio](../../documentacion/monorepo.md).
