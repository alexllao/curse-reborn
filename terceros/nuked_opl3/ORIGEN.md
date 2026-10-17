# Núcleo OPL externo

Nuked OPL3 1.8, de Nuke.YKT, obtenido de https://github.com/nukeykt/Nuked-OPL3 en la revisión `765ec962e473aeb767e4cba74ffdc8f588ffbfe8`.

`opl3.c`, `opl3.h` y `LICENSE` se conservan sin modificaciones. Sus huellas están en `herramientas/dependencias_audio.json`. Esta carpeta queda fuera del formato del código propio.

El núcleo tiene licencia LGPL 2.1 o posterior; su texto íntegro está en `LICENSE`. Esa licencia corresponde a esta dependencia y no asigna una licencia al resto del proyecto. Al compilar, se copia junto al ejecutable como `LICENCIA_NUKED_OPL3.txt`.

`plataformas/sdl/opl.cpp` compila esta fuente con el compilador C++ del proyecto. Para reconstruir o volver a enlazar la aplicación con una versión modificada de esta dependencia, conservar sus avisos y ejecutar `pnpm sdl:compilar`, o la compilación CMake documentada en el repositorio. No se distribuyen archivos del juego ni ROM como parte de esta dependencia.
