# Filtros de píxeles

F11 permite comparar la imagen original con suavizado, líneas de escaneo, CRT suave, xBRZ 2x y 2xSaI. El modo inicial continúa siendo original. Alt + Enter alterna ventana y pantalla completa sin bordes; la presentación SDL conserva proporciones y bandas negras en ambos casos.

## Implementación

`apps/juego/plataformas/sdl/video/escaladores_pixel.hpp` contiene implementaciones propias RGB de las reglas de escalado. xBRZ 2x compara luminancia y crominancia en una vecindad 5 × 5, distingue diagonales dominantes y pendientes, y mezcla subpíxeles según su cobertura. 2xSaI usa una vecindad 4 × 4, coincidencias de diagonales y promedios por componente de ocho bits. Los bordes prolongan el último píxel válido. Las dos salidas son RGBA opacas de 640 × 400.

Se atribuyen los algoritmos xBRZ a Zenju y 2xSaI a Derek Liauw Kie Fa. Las referencias técnicas consultadas son [xBRZ](https://github.com/atheros/xbrzscale), la [variante 2x de Libretro](https://github.com/libretro/slang-shaders/blob/master/edge-smoothing/xbrz/shaders/2xbrz.slang) y las [reglas 2xSaI conservadas en Snes9x](https://github.com/snes9xgit/snes9x/blob/master/filter/2xsai.cpp). Sus implementaciones tienen sus propias licencias; no se incorpora su código ni se asigna una licencia al proyecto. Esta implementación RGB no se presenta como una reproducción binaria de esas bibliotecas ni de sus versiones RGB565.

Los escaladores reciben únicamente la imagen final. No leen DS, no modifican recursos, animaciones ni coordenadas de entrada y se mantienen fuera del motor reconstruido. El modo original continúa utilizando su textura de 320 × 200. La depuración y el contador de FPS se dibujan después del filtro. El coste de CPU es mayor con xBRZ; el límite de presentación y el reloj lógico siguen siendo independientes.

## Comprobaciones

`pnpm comprobar:filtros` compila una prueba C++ sin SDL para colores planos, dimensiones mínimas, bordes, opacidad, escritura acotada y contornos conocidos. También simula Alt + Enter dos veces y comprueba las banderas SDL de pantalla completa sin modo exclusivo y el regreso a ventana. Después simula seis pulsaciones de F11 y guarda siete capturas SDL a 960 × 600. Comprueba que los seis modos producen imágenes diferentes, que volver al original recupera exactamente la primera captura y que se conservan los 64.000 píxeles originales del título. Los resultados quedan en `apps/juego/trabajo/comprobacion_filtros/`.

Estas pruebas verifican los contratos y la integración visual; no acreditan equivalencia píxel a píxel con todas las versiones de escaladores de otros emuladores. La presentación sin filtro es la referencia para comparar el juego con DOS.
