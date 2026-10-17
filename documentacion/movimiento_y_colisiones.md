# Movimiento de la mazmorra y corrección de la puerta

La primera mazmorra admite **clic izquierdo para proponer un destino** y **flechas para caminar** después de la liberación. Se respeta la bandera original `DS:3594`: las entradas no permiten caminar mientras Brad siga colgado o una secuencia bloquee el control. Las paredes se leen de `BOXDET.DAT`; no se sustituye el suelo por una geometría inventada.

## Puerta desplazada tras quitarse las esposas

La integración anterior llamaba a `183F` como si perteneciera a la mazmorra inicial. Esa atribución era incorrecta: el despacho `1791`–`17A2` consulta la tabla **CS:17A3**, cuya entrada **cero vale cero** y cuya entrada **cinco vale 183F**. La mazmorra no debe ejecutar ese manejador.

Al terminar la liberación, `3594` se activa y aquella llamada indebida movía la entidad `DS:2CF1`, la puerta, de `(30,83)` a unas coordenadas calculadas para otra escena. Ahora `actualizar_rutina_escena` respeta las escenas integradas: cero sin manejador, cinco con `actualizar_escena_cinco`. Los demás manejadores particulares siguen pendientes y no se ejecutan por aproximación.

Las comparaciones anteriores de 183F demostraban su traducción aislada; no demostraban que correspondiera a la escena cero. Las primeras muestras de avance tampoco llegaban al control habilitado posterior a quitarse las esposas. La nueva regresión recorre los menús originales, llama al guardia, recoge la llave, la combina con las esposas, termina la animación y comprueba que la puerta permanece en `(30,83)`.

## Bloques reconstruidos

| Original      | Alcance                                                                                                                                                                                  |
| ------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `504F`–`50CC` | Índice por escena y bloque de rectángulos de `BOXDET.DAT`. La escena cero tiene catorce registros de diez bytes.                                                                         |
| `89DA`–`8AA5` | Nodos de camino `siguiente/x/y` en `DS:9978`; conserva cajas y rutas cuando están activadas. El cargador original deja `DS:0DF2=0` en la entrada de la mazmorra y el destino es directo. |
| `083D`–`086D` | Cuatro bits de dirección: izquierda 8, derecha 4, arriba 2, abajo 1. Se reutiliza `0BF8`.                                                                                                |
| `08C8`–`08DA` | Clic, construcción de camino y planificación inicial mediante `0F5F`.                                                                                                                    |
| `096D`–`0A58` | Incrementos fraccionarios y activación de canales al terminar un nodo.                                                                                                                   |
| `254A`–`28B1` | Intersección de rectángulos, paredes, condiciones y retornos especiales de la escena.                                                                                                    |
| `0A6E`–`0BA0` | Márgenes de los pies, ajuste de un píxel, restauración de ejes y detección de un paso bloqueado.                                                                                         |
| `0BA6`–`0BC0` | Historial de diez posiciones horizontales consumido por entidades.                                                                                                                       |
| `1791`–`17A2` | Despacho de la rutina particular de la escena.                                                                                                                                           |

El paso común está en [motor/estado/paso_escena.cpp](../motor/estado/paso_escena.cpp), independiente de SDL. Conserva posición anterior, entrada, nodos, guiones, colisiones, historial, comportamientos y despacho, en ese orden. Los lectores y detectores están en [motor/movimiento/geometria.cpp](../motor/movimiento/geometria.cpp).

El punto de destino no teletransporta al personaje. Las colisiones corrigen cada paso como en el bucle original. Los campos de coordenadas conservan palabras de dieciséis bits y comparaciones con signo. Las API limitan las lecturas al recurso y reservan hasta nueve puntos intermedios para no invadir otras estructuras DS; no aceptan rutas malformadas que el original pudiera recorrer fuera del bloque.

## Controles y depuración

- **Clic izquierdo**, con el menú cerrado: propone un destino en coordenadas de juego.
- **Flechas**: mantienen la dirección mientras estén pulsadas y se detienen al soltarlas. En el menú, izquierda/derecha siguen seleccionando casillas.
- **Escape o botón derecho**: abre/cierra el menú cuando el guion lo permite.
- **F7–F10**: conservan los interruptores de depuración existentes.

F10 añade los rectángulos reales de `BOXDET`: **amarillo para paredes, azul para zonas de transición y rojo para zonas de finalización**, además del encuadre y los umbrales anteriores. Son líneas de diagnóstico; no modifican la geometría ni el estado. Las cajas de entidades y las zonas de proximidad de F8/F9 siguen siendo categorías distintas.

La entrada en otras escenas, los servicios de las zonas especiales y el resto de puzles siguen pendientes. Al detectar una transición aún no integrada, SDL conserva el último punto de la habitación y detiene ese recorrido; no simula la siguiente escena ni permite continuar fuera del mapa. Las otras rutinas específicas deberán incorporarse al despacho al integrar sus escenas. La cámara de esta vista continúa usando el fondo fijo de la mazmorra.

## Comprobaciones

```sh
pnpm comparar:jugabilidad
pnpm comprobar:jugabilidad
```

La muestra tiene **658 casos/pasos**, incluidos 513 pasos jugables persistentes. La comparación amplia ejecutada tiene **1.018**, incluidos 873 pasos. Ambas comparan DS completo y el estado aleatorio con las instrucciones originales; prueban el despacho de escenas cero/cinco, los catorce rectángulos reales, condiciones, correcciones de ejes, rutas enlazadas sintéticas, historial, menús, liberación, direcciones y cuatro destinos del ratón. Además de la igualdad, se exige que el control esté habilitado, que el personaje se desplace y que la puerta mantenga su posición.

La referencia recorre `096D`–`0BC1`, no solo animaciones aisladas. Las regiones de recursos del comparador permanecen separadas. Las transiciones que requieren otro servicio quedan fuera de ese recorrido de partida, aunque el detector devuelve su decisión tipada.

SDL comprueba dos capturas de 64.000 píxeles: liberación y resultado de caminar con flechas y ratón; ambas coinciden con las composiciones contrastadas con el original. El recorrido selecciona los menús y usa el adaptador de movimiento, sin escribir directamente inventario o posición. Los resultados quedan en `trabajo/comparacion_jugabilidad/` y `trabajo/comprobacion_jugabilidad_sdl/`. La sensación de control y la selección con ratón en una ventana visible maximizada siguen requiriendo comprobación manual.
