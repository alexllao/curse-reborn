# Primeras acciones del lago

La ruta continúa la partida real desde la mazmorra y el corredor. No crea objetos ni desbloquea entidades para abreviar la solución. El motor conserva la misma memoria, azar, colisiones, cámara, menús y servicios compartidos de audio.

| Original DOS               | Acción reconstruida                                                                                              |
| -------------------------- | ---------------------------------------------------------------------------------------------------------------- |
| 3CC0–3CCC, 9994–9A47       | Ponerse la pecera: retira pecera y agua del inventario, activa sus seis canales y los de Brad; suma cinco puntos |
| 3B93–3BA0, 9A48–9AA7       | Empujar los barrotes: exige que sigan activos, prepara aproximación, animación y liberación; suma cinco puntos   |
| 38A4–38D2                  | Examinar la arena: prepara la búsqueda y la aparición del gusano                                                 |
| 57F1–5848, 615C–61A6, 623D | Recogidas de gusano, concha, objeto eléctrico y moneda, con retirada persistente y límite de inventario          |
| 5807–581D                  | Rechaza conservar la concha mientras sus canales siguen activos                                                  |
| 3E65–3E79, 9AA8–9B85       | Dar el gusano a Mr. Fish: retira el objeto y programa la respuesta y sus entidades; suma cinco puntos            |

Fuentes: `motor/acciones/lago.cpp`, despacho compartido en `motor/acciones/menu.cpp`. Los canales y recursos conservan las etiquetas y los bytes del original; SDL sustituye únicamente los servicios de plataforma.

`pnpm comparar:lago --muestra` ejecuta la partida previa, compara cada selección y paso con el EXE mediante Unicorn y contrasta DS completo y generador. Añade ramas aisladas sobre copias de una llegada real: inventarios vacíos, parciales y llenos, puntuación desbordada y concha animada o quieta. Guarda imágenes en `trabajo/comparacion_lago/`.

`pnpm comprobar:lago` añade dos capturas SDL: pecera colocada y final de la entrega del gusano. Cada captura se compara por sus 64.000 píxeles con la composición del mismo recorrido. La prueba omite las esperas físicas del monitor y no valida por sí sola la equivalencia acústica.

La muestra ejecutada contiene **5.353 casos/pasos coincidentes**, incluidas las regresiones heredadas de mazmorra y corredor. Las dos capturas SDL coinciden en sus **64.000 píxeles**. También pasan el formato y la revisión estática del proyecto.

La ruta comprobada llega hasta Mr. Fish. Las recogidas posteriores tienen comparación de bloque, pero todavía no acreditan una ruta completa: faltan concha, tortuga, anguilas, picana, tiburones, salto de la almeja y salida por el tapón. La supervivencia completa con el temporizador de oxígeno requiere su propia validación del bucle y del reloj originales.

[Recorrido de prueba](recorridos/03_lago.txt). F5/F6 permiten cambiar entre las vistas disponibles y reinician la sesión; para conservar la pecera hay que llegar por las salidas normales.
