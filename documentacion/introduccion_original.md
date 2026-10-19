# Introducción de la bruja

La ventana incorpora **Core → título → introducción → mazmorra**. El bloque original de la introducción es `0000:28B2`–`0000:2F4F`, llamado en `0671` antes de preparar la primera escena. La versión anterior omitía esta llamada; ver Core y el título no significaba que estuviera integrada la introducción narrativa.

## Traslado

[apps/juego/motor/video/introduccion.cpp](../apps/juego/motor/video/introduccion.cpp) enumera los eventos originales: controles de entrada, fundidos, cargas, selección de página, paletas, esperas y limpieza. [apps/juego/plataformas/sdl/introduccion.cpp](../apps/juego/plataformas/sdl/introduccion.cpp) sustituye los archivos, el retrazo y el CRTC por servicios de plataforma. No ejecuta instrucciones DOS ni incorpora las imágenes al repositorio.

Los nombres están en `DS:1938`–`DS:1A2C`:

| Tramo           | Recursos                                                     | Recorrido                                                                                                                    |
| --------------- | ------------------------------------------------------------ | ---------------------------------------------------------------------------------------------------------------------------- |
| Bruja y caldero | `CAULDRN.PAL`, `CAULDRN1.DAT`–`CAULDRN4.DAT`                 | `2951`–`2A24`: 39 selecciones de páginas 1/2/3/0, ocho retrazos explícitos por selección.                                    |
| Balcón          | `BALCONY.PAL`, `BALCONY1.DAT`–`BALCONY5.DAT`                 | `2A98`–`2B9E`: 29 selecciones de páginas 1/2/0, diez retrazos por selección; después se cargan las imágenes cuarta y quinta. |
| Béisbol         | `BASEBALL.PAL`, `BASEBAL1.DAT`–`BASEBAL6.DAT`, `BASEBAT.DAT` | `2C8C`–`2F16`: conserva las cargas y la repetición particular de páginas, incluidos los últimos planos sobrescritos.         |

`2F50`–`2F88` llama al descompresor RNC2 `67BC` y a la subida `7636`, con ancho y paso de **320 píxeles**, equivalentes a **80 bytes por plano**. Las cuatro páginas se separan por `4000h` bytes por plano. Este paso difiere del de 112 bytes usado para Core y título; reutilizarlo sin cambiarlo produciría una imagen incorrecta.

Se conservan los cuatro fundidos de 32 pasos: salida del título, cambio a balcón, cambio a béisbol y salida de la introducción. Las tres paletas se envían completas, sin añadir fundidos de entrada.

## Controles y cadencia

Una tecla o el botón izquierdo en el título inicia la introducción. Otra pulsación durante ella queda pendiente y se consume en el siguiente control original `2F8E`, seguido del fundido de salida. Escape sigue cerrando la ventana. F5/F6 retroceden/avanzan entre las cuatro vistas de desarrollo y permiten omitir la introducción directamente, sin resolver puzles ni guardar partidas.

El recorrido sin entrada tiene **1.005 retrazos explícitos**, incluidos los fundidos. SDL usa la referencia nominal de 70 Hz existente. Se precargan los archivos para evitar bloquear la reproducción; el tiempo de lectura y descompresión en un 286 y la sincronización física de un monitor no se reproducen. Las pulsaciones se adaptan a eventos SDL; no se emula el estado de los botones de los controladores DOS. El audio continúa aparcado.

## Comprobación

```sh
pnpm comparar:introduccion
pnpm comprobar:introduccion
```

La muestra ejecutada contrasta **259 eventos** del recorrido y **16 cargas de imágenes**, una en cada página seleccionada para la muestra. El comparador sigue las instrucciones originales, simula los servicios DOS y cuenta las esperas; compacta únicamente las esperas contiguas. Para las imágenes ejecuta `2F50` completo, incluyendo la descompresión y subida originales, y compara los cuatro planos completos y los 64.000 índices recuperados.

La comparación amplia prepara **64 cargas**, cada una de las dieciséis imágenes en las cuatro páginas, además de los mismos 259 eventos. Queda pendiente de ejecutar por el propietario. Las imágenes e informes locales se guardan en `apps/juego/trabajo/comparacion_introduccion/`.

La comprobación SDL recorre los retrazos de forma determinista, contrasta **cinco capturas de 64.000 píxeles** y verifica que el final natural ocurre en el retrazo 1.005 y que una pulsación en el retrazo 34 provoca la salida en el 73. Caldero, balcón y béisbol coinciden con sus referencias originales; terminar u omitir conduce al estado inicial de la mazmorra. Los resultados quedan en `apps/juego/trabajo/comprobacion_introduccion_sdl/`. La reproducción visible y su cadencia requieren comprobación manual.
