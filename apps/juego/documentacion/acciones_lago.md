# Acciones del lago y salida a las cuevas

La ruta continúa la partida real desde la mazmorra y el corredor. No crea objetos ni desbloquea entidades para abreviar la solución. El motor conserva la misma memoria, azar, colisiones, cámara, menús y servicios compartidos de audio.

| Original DOS               | Acción reconstruida                                                                                                                                   |
| -------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------- |
| 3558–3587, 3545/37FF       | Hablar: reacción de Mr. Fish con el tema uno a menos de 64 píxeles; ambos temas conservan su efecto de sonido                                         |
| 3CC0–3CCC, 9994–9A47       | Ponerse la pecera: retira pecera y agua del inventario, activa sus seis canales y los de Brad; suma cinco puntos                                      |
| 3B93–3BA0, 9A48–9AA7       | Empujar los barrotes: exige que sigan activos, prepara aproximación, animación y liberación; suma cinco puntos                                        |
| 38A4–38D2                  | Examinar la arena: prepara la búsqueda y la aparición del gusano                                                                                      |
| 57F1–5848, 615C–61A6, 623D | Recogidas de gusano, concha, objeto eléctrico y moneda, con retirada persistente y límite de inventario                                               |
| 5807–581D                  | Rechaza conservar la concha mientras sus canales siguen activos                                                                                       |
| 3E65–3E79, 9AA8–9B85       | Dar el gusano a Mr. Fish: retira el objeto y programa la respuesta y sus entidades; suma cinco puntos                                                 |
| 3E7C–3E86, 9B86–9BB7       | Dar la concha a la tortuga: consume el objeto, repone el valor original 160, activa comportamiento 040B y suma cinco puntos                           |
| 040B, 104A, 1111           | Transporte y descenso originales: canales de Brad y tortuga, cámara y recuperación del control                                                        |
| 2FD3–300A, 040C            | Usar la picana: rechaza sin tiburón activo; secuencia 1AB2 y comprobación de modelo y fotograma para activar 11C6 y retirar su descriptor persistente |

Fuentes: `apps/juego/motor/acciones/lago.cpp`, despacho compartido en `apps/juego/motor/acciones/menu.cpp`. Los canales y recursos conservan las etiquetas y los bytes del original; SDL sustituye únicamente los servicios de plataforma.

El recorrido se prolonga desde el tiburón hasta la almeja y el tapón. `314C–3193` exige que la almeja muestre el modelo cerrado 12 y programa el salto desde la casilla de combate. `3D72–3D89` acepta insertar picana y tapón en ambos órdenes; `9BB8–9C72` consume los identificadores 23 y uno, sincroniza cinco canales con el acercamiento y suma cinco puntos. El guion solicita después la salida cuatro; se carga `EMERGE.BRD` y la escena ocho de `POOL`, sin resolver acciones de las cuevas.

`pnpm comparar:lago --muestra` ejecuta la partida previa, compara cada selección y paso con el EXE mediante Unicorn y contrasta DS completo y generador. Incluye la recogida de la concha, su entrega al acercarse la tortuga, 192 pasos de transporte y descenso, la picana y una descarga que alcanza el intervalo original del tiburón. Añade ramas aisladas sobre copias de una llegada real: inventarios vacíos, parciales y llenos, puntuación desbordada y concha animada o quieta, tiburón ausente o activo y orientaciones de Brad. Guarda imágenes en `apps/juego/trabajo/comparacion_lago/`.

`pnpm comprobar:lago` añade doce capturas SDL: pecera colocada, entrega del gusano, reacción al hablar, concha recogida, viaje sobre la tortuga, descenso, picana recogida, tiburón aturdido, salto de la almeja terminado, apertura del tapón, entrada a la cueva y aparición de Brad. Cada captura se compara por sus 64.000 píxeles con la composición del mismo recorrido. La prueba omite las esperas físicas del monitor y no valida por sí sola la equivalencia acústica.

Las comprobaciones conservan también un volcado local de DS junto a cada captura (`.ds.bin`) para localizar divergencias; no es un formato de partida guardada. La referencia y SDL consumen un desplazamiento de cámara antes de cada paso lógico, incluido el transporte rápido. La cadencia y las repeticiones de cámara del bucle EAD5 completo necesitan una validación temporal adicional.

La comparación ampliada conserva **doce capturas SDL** de 64.000 píxeles. La captura final espera 96 pasos para cubrir la recuperación del control, que el recorrido original alcanza en el paso 86. Añade 24 variantes del estado de presentación y la semilla para contrastar las partículas de `1963–1992`. Las ocho capturas anteriores del corredor también se habían contrastado. La prueba omite las esperas físicas de vídeo.

Las capturas del transporte muestran a Brad sobre la tortuga antes de devolverle el control. La descarga conserva la condición de modelo y fotograma del ejecutable: pulsar demasiado pronto o tarde puede fallar; no se sustituye por un impacto garantizado. El recorrido ampliado continúa hasta emerger en la cueva; no demuestra todas las posibles secuencias de ataque del tiburón.

Quedan pendientes la revisión exhaustiva de opciones y rechazos de manipulación del lago y las acciones de las cuevas. La supervivencia completa con el temporizador de oxígeno requiere su propia validación del bucle y del reloj originales. No se considera terminada toda la escena por haber validado una solución.

[Recorrido de prueba](recorridos/03_lago.txt). F5/F6 permiten cambiar entre las vistas disponibles y reinician la sesión; para conservar la pecera hay que llegar por las salidas normales.

## Revisión de opciones del menú

La reacción al hablar de la tienda estaba desensamblada y faltaba en el despacho C++. La corrección compara los dos temas a ambos lados del límite de proximidad (63, 64 y 65 píxeles), cerca y lejos, y conserva la aritmética de 16 bits. Añade una rama desde la llegada real a Mr. Fish con su imagen tras ocho pasos; el recorrido del gusano continúa desde una copia anterior a esa conversación opcional.

Se contrastan también los retornos comunes de examinar sin una respuesta especial y los rechazos de usar o atacar objetos sin una acción aplicable. La almeja añade modelos 0, 11, 12, 13 y 255 y puntuación con desbordamiento; el tapón cubre ambos órdenes de selección, inventarios con y sin los identificadores retirados y puntuación con desbordamiento. Siguen pendientes otras combinaciones de manipulación. La casilla E126 ofrece las operaciones de partida (E19F/E325/E793), por lo que no debe confundirse con un salto genérico.

## Poza y puerta secreta

La escena ocho `POOL` continúa la misma partida. `59B5–59C9` recoge las algas (identificador 49), retira su descriptor persistente y asigna el identificador 48 al botón. Conserva el límite de diez objetos, el ajuste de pies de nueve píxeles y las secuencias de recogida normal o de Brad pequeño. Examinar la zona clara (50) retorna sin cambios por `3B4F`: en este ejecutable DOS la habilitación del botón procede de recoger las algas; no se añade una condición tomada de la guía de Amiga.

Empujar el botón entra en `9DE4–9E20`. Solo acepta el modelo cerrado seis de la puerta, programa la aproximación y sus canales, sincroniza la apertura con AL+12 y suma cinco puntos con aritmética de 16 bits. Tras abrirla, las colisiones originales solicitan la salida 17 hacia la escena cuatro `CAVECOR`. La entrada utiliza `STD.BRD`; la lista persistente, inventario, música y servicios de sonido continúan compartidos. La rutina particular de la escena cuatro es nula en `CS:17A3`.

`pnpm comparar:poza --muestra` prolonga el recorrido completo desde la mazmorra. Se han contrastado **7.996 casos y pasos**: memoria DS completa y generador, recogida, apertura, inventario lleno, puntuación desbordada, puerta cerrada o abierta, conversación, examen, uso, combate y combinaciones de manipulación antes y después de abrir. No se introducen objetos para resolver la ruta. Las variantes aisladas modifican copias expresamente para comprobar sus límites.

`pnpm comprobar:poza` compila y agrupa esa comparación con **cuatro capturas SDL**: algas recogidas, puerta abierta, entrada al corredor y devolución del control. Sus 64.000 píxeles RGB coinciden con la composición de referencia. Se han ejecutado la comparación completa y las capturas por separado para reutilizar los binarios compilados. F5/F6 incluyen el corredor y se han comprobado sus doce cambios de vista.

Los puzles posteriores del corredor y del resto de las cuevas siguen pendientes. No se da por terminada la zona de cuevas. [Recorrido de la poza](recorridos/04_poza.txt).
