# Acciones del lago hasta el tiburón

La ruta continúa la partida real desde la mazmorra y el corredor. No crea objetos ni desbloquea entidades para abreviar la solución. El motor conserva la misma memoria, azar, colisiones, cámara, menús y servicios compartidos de audio.

| Original DOS               | Acción reconstruida                                                                                                                                   |
| -------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------- |
| 3CC0–3CCC, 9994–9A47       | Ponerse la pecera: retira pecera y agua del inventario, activa sus seis canales y los de Brad; suma cinco puntos                                      |
| 3B93–3BA0, 9A48–9AA7       | Empujar los barrotes: exige que sigan activos, prepara aproximación, animación y liberación; suma cinco puntos                                        |
| 38A4–38D2                  | Examinar la arena: prepara la búsqueda y la aparición del gusano                                                                                      |
| 57F1–5848, 615C–61A6, 623D | Recogidas de gusano, concha, objeto eléctrico y moneda, con retirada persistente y límite de inventario                                               |
| 5807–581D                  | Rechaza conservar la concha mientras sus canales siguen activos                                                                                       |
| 3E65–3E79, 9AA8–9B85       | Dar el gusano a Mr. Fish: retira el objeto y programa la respuesta y sus entidades; suma cinco puntos                                                 |
| 3E7C–3E86, 9B86–9BB7       | Dar la concha a la tortuga: consume el objeto, repone el valor original 160, activa comportamiento 040B y suma cinco puntos                           |
| 040B, 104A, 1111           | Transporte y descenso originales: canales de Brad y tortuga, cámara y recuperación del control                                                        |
| 2FD3–300A, 040C            | Usar la picana: rechaza sin tiburón activo; secuencia 1AB2 y comprobación de modelo y fotograma para activar 11C6 y retirar su descriptor persistente |

Fuentes: `motor/acciones/lago.cpp`, despacho compartido en `motor/acciones/menu.cpp`. Los canales y recursos conservan las etiquetas y los bytes del original; SDL sustituye únicamente los servicios de plataforma.

`pnpm comparar:lago --muestra` ejecuta la partida previa, compara cada selección y paso con el EXE mediante Unicorn y contrasta DS completo y generador. Incluye la recogida de la concha, su entrega al acercarse la tortuga, 192 pasos de transporte y descenso, la picana y una descarga que alcanza el intervalo original del tiburón. Añade ramas aisladas sobre copias de una llegada real: inventarios vacíos, parciales y llenos, puntuación desbordada y concha animada o quieta, tiburón ausente o activo y orientaciones de Brad. Guarda imágenes en `trabajo/comparacion_lago/`.

`pnpm comprobar:lago` añade siete capturas SDL: pecera colocada, entrega del gusano, concha recogida, viaje sobre la tortuga, descenso, picana recogida y tiburón aturdido. Cada captura se compara por sus 64.000 píxeles con la composición del mismo recorrido. La prueba omite las esperas físicas del monitor y no valida por sí sola la equivalencia acústica.

Las comprobaciones conservan también un volcado local de DS junto a cada captura (`.ds.bin`) para localizar divergencias; no es un formato de partida guardada. SDL respeta un desplazamiento de cámara por ciclo antes de continuar la lógica, como EAD5, incluido el transporte rápido.

La muestra ejecutada coincide en **6.291 comprobaciones**. Las **siete capturas SDL** nuevas y las **ocho capturas anteriores del corredor** coinciden en sus 64.000 píxeles. También pasan el formato, ESLint y Ruff.

La ruta comprobada llega hasta el primer tiburón aturdido. Las capturas del transporte muestran a Brad sobre la tortuga antes de devolverle el control. La descarga conserva la condición de modelo y fotograma del ejecutable: pulsar demasiado pronto o tarde puede fallar; no se sustituye por un impacto garantizado.

Quedan por comprobar el resto del recorrido de tiburones, el salto de la almeja y la salida por el tapón. La supervivencia completa con el temporizador de oxígeno requiere su propia validación del bucle y del reloj originales.

[Recorrido de prueba](recorridos/03_lago.txt). F5/F6 permiten cambiar entre las vistas disponibles y reinician la sesión; para conservar la pecera hay que llegar por las salidas normales.
