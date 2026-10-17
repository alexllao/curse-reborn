# Puzle de la mazmorra

La ruta reconstruida parte del estado inicial y pasa por la conversación, primera llave, liberación, acción sobre la pared, clip y apertura de la puerta. El comparador ejecuta las mismas selecciones del menú que el adaptador SDL, sin introducir objetos en el inventario ni colocar a Brad directamente en los destinos.

## Bloques trasladados

| Original                                   | Comportamiento                                                                                  | Fuente                         |
| ------------------------------------------ | ----------------------------------------------------------------------------------------------- | ------------------------------ |
| 563A–5678                                  | Puntuación por icono 1F/22; inserción y rechazo del inventario lleno                            | `motor/acciones/menu.cpp`      |
| 56CB–56DB, 615C–61A6                       | Recogida del objeto cinco, modelo normal/especial y contador de pasos + 6                       | `motor/acciones/menu.cpp`      |
| 623D–625F                                  | Baja en la lista persistente de objetos                                                         | `motor/acciones/menu.cpp`      |
| 3B53–3D68                                  | Despacho de empujar la pared y desbloqueo con pareja simétrica 3/4; rechazos del ámbito inicial | `motor/acciones/menu.cpp`      |
| 9900–9953                                  | Condición de fotograma de la entidad, secuencias, activación del clip y puntuación              | `motor/acciones/menu.cpp`      |
| 9954–9993                                  | Aproximación y apertura de la puerta mediante canales diferidos                                 | `motor/acciones/menu.cpp`      |
| 2FCA, 3143, 384A                           | Para identificadores 0–5: rechazo de los dos primeros; retorno sin cambio ni acarreo de 384A    | `motor/acciones/menu.cpp`      |
| 4B05–4B0E, 4C43–4C98, 4D2B–4D40, 5145–5181 | Preparación de las entradas 0/2 entre celda y corredor con STD.BRD                              | `motor/estado/inicializar.cpp` |
| BEA8–BEFC                                  | Rutina de la escena uno según ventana, estado de Brad y entidad                                 | `motor/entidades/mazmorra.cpp` |

## Comparaciones

Verificados localmente: **1.834 casos/pasos** en la comparación amplia y **64.000 píxeles RGB** en la captura SDL. La muestra actual contiene 1.582 casos/pasos. También pasó la regresión de menú de 91 casos y la revisión de formato y lint.

- `pnpm comparar:puzle`: compara DS completo, azar en los pasos de juego y composición final contra instrucciones del EXE. Incluye 336 combinaciones de acción, objeto previo y destino; inventario lleno; las 256 fases posibles de la entidad; 1.161 pasos persistentes; dos fragmentos de entrada y 18 estados del corredor. La opción `--muestra` reduce las fases de la entidad a cuatro conservando el recorrido entero.
- `pnpm comprobar:puzle`: compila SDL, ejecuta la muestra anterior y recorre los mismos clics desde el adaptador de plataforma. Compara los 64.000 píxeles RGB de la puerta abierta con la composición original. La ejecución de comprobación avanza sin esperar tiempo real.

La apertura debe dejar el fotograma de DS:2CB0 en 10 hexadecimal. Esto habilita el detector original de salida cero; no se fuerza ese fotograma desde el menú ni desde la plataforma.

La comparación del cargador es por fragmentos de estado: no ejecuta ni valida sus reservas de memoria DOS, lectura completa de recursos, fundido o cambio de habitación en SDL. Los estados sintéticos del corredor no son una partida jugable en el corredor. En Windows, Unicorn necesita detener explícitamente los fragmentos en el gancho de instrucciones cuando sus destinos ya forman parte de bloques traducidos de llamadas anteriores.

## Guía y alcance

[Recorrido de la mazmorra en texto](recorridos/00_mazmorra.txt), con casillas contadas desde uno. Este comparador termina en la puerta abierta. El comparador posterior de [corredor y cámara](corredor_y_camara.md) prolonga la misma partida hasta entrar y regresar; [DROP y la entrada al agua](caida_y_agua.md) tienen su propia comparación. Los demás puzles, guardado/carga y audio siguen pendientes. No afirmar fidelidad del juego completo a partir de estas comprobaciones.
