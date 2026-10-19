# Descompresión RNC de método uno en C++

[rnc.cpp](../apps/juego/motor/recursos/rnc.cpp) reconstruye el método uno estudiado en `0000:64E0`–`0000:67AC`. No incorpora una biblioteca de descompresión ajena. La muestra de **38 casos** coincide con las instrucciones originales. El propietario ha confirmado también los **142 casos** de RNC y los **1.183 casos gráficos** anteriores, **1.325 casos** en conjunto.

El comando `pnpm titulo` ya genera el título con descompresión, subida a cuatro planos, recuperación y paleta en C++. **No carga `CURSE.EXE` ni ejecuta Unicorn.** Ese comando produce una imagen fija PNG. La [presentación SDL](ventana_sdl.md) ya permite mostrarla directamente desde los recursos originales mediante `pnpm ventana`; todavía no hay una partida.

## Relación con las instrucciones originales

| Original                     | Traslado                                                                                                                                                  |
| ---------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `64EE`–`6515`, `6676`–`6691` | Identificación `RNC`/método uno, longitudes de cabecera y lectura de palabras grandes.                                                                    |
| `6518`–`6580`                | El original protege ciertos solapamientos trasladando la entrada por párrafos. La API C++ utiliza búferes separados y evita esa dependencia de segmentos. |
| `6581`–`65BE`                | Inicialización de ventana de bits y tablas de literales, distancias y repeticiones.                                                                       |
| `65C4`–`65E8`                | Copia una repetición hacia delante; permite que los bytes recién escritos sean origen de los siguientes.                                                  |
| `65EA`–`661A`                | Copia literales y reconstruye la ventana de bits después de los bytes sin comprimir.                                                                      |
| `661E`–`665C`                | Repite grupos y bloques; los ajustes de segmentos se sustituyen por posiciones lineales explícitas.                                                       |
| `6692`–`66CD`                | Busca el código en la tabla y lee los bits adicionales del valor.                                                                                         |
| `66CE`–`672D`                | Ventana de dos palabras con consumo por bits bajos.                                                                                                       |
| `672E`–`67AC`                | Construye códigos canónicos, invierte el orden de bits y conserva la tabla anterior si el recuento es cero.                                               |

La ventana conserva dos palabras, el número de bits disponibles y la posición de lectura. Los literales comparten el flujo con palabras adelantadas: después de copiarlos, se preservan los bits todavía pendientes y se combinan con las palabras siguientes. No basta con alinear el lector al siguiente byte.

Los valores de símbolos cero y uno son directos. Para los restantes se leen `símbolo − 1` bits adicionales y se incorpora el bit superior. Las distancias se incrementan en uno; las cantidades de repetición, en dos. Esas transformaciones se toman de las instrucciones originales.

## API portable y límites

La función recibe origen y longitud, destino y capacidad, y devuelve estado y cantidad escrita. No asigna memoria ni necesita segmentos DOS. **Origen y destino deben ser búferes separados.** Los ajustes de la implementación DOS para descomprimir sobre su entrada no se reproducen como movimientos de memoria adicionales.

Se distinguen éxito, ausencia de firma, método no admitido y entrada inválida. Esta API corresponde al método uno; el método dos dispone ahora de su traslado separado en `rnc2.cpp`, descrito más abajo. La API del método uno valida cabecera, capacidad, referencias de repetición, tamaños de tablas y longitud final. Las tablas admitidas tienen hasta 16 símbolos, de acuerdo con el espacio de tablas y longitudes estudiado. Los tres recursos reales comparados pertenecen a ese dominio.

La lectura adelantada puede consultar palabras incompletas al final del flujo; el C++ aporta ceros para esa anticipación sin leer fuera del búfer del llamador. Los literales reales deben quedar dentro de la longitud empaquetada declarada. Una entrada inválida puede dejar un resultado parcial: el retorno no promete reversión de escrituras.

No se comprueban los CRC de la cabecera, que tampoco se utilizan en el tramo original estudiado. Las comprobaciones nuevas de seguridad no se atribuyen al ejecutable para entradas corruptas. El estado del generador aleatorio no participa en este algoritmo.

## Comparación agrupada

```sh
pnpm comparar:rnc
```

| Grupo                                                                  | Comparación amplia confirmada | Muestra comprobada |
| ---------------------------------------------------------------------- | ----------------------------: | -----------------: |
| `CORE.DAT`, `MENU.DAT` y `TITLE.DAT`                                   |                            24 |                  6 |
| Literales, repeticiones solapadas y reutilización de tablas sintéticas |                           104 |                 18 |
| Ausencia de firma y método diferente                                   |                             6 |                  6 |
| Cabeceras truncadas, capacidad y tamaños inválidos de la API           |                             8 |                  8 |
| **Total**                                                              |                       **142** |             **38** |

Unicorn ejecuta la rutina original con origen/destino separados y también con el mismo segmento, ejercitando la protección original del solapamiento. El C++ utiliza siempre búferes separados. Se comparan todos los bytes descomprimidos, se comprueba que la cola nativa permanezca intacta y se verifican retorno, pila y registros conservados del original. Con segmentos separados se compara también el búfer original completo de salida.

Los programas sintéticos son generados por un escritor propio: incluyen valores de literales hasta 32.767, repeticiones de un byte hacia delante y hasta tres bloques que reutilizan tablas. Las cabeceras inválidas se comprueban solo contra el contrato seguro del C++; no se ejecutan como si el original garantizase su rechazo. Los informes guardan las huellas de cada recurso y de su salida.

La muestra se ejecuta añadiendo `--muestra`. Los informes locales se guardan por separado en `apps/juego/trabajo/comparacion_rnc/muestra.json` y `resultado.json`.

Para agrupar esta comparación con los 1.183 casos gráficos, usa `pnpm comparar:graficos`: compila una vez y ejecuta los dos conjuntos, **1.325 casos** en total, conservando sus informes separados. Con `--muestra` ejecuta 121 casos.

## Imagen del título sin ejecutar el EXE

```sh
pnpm titulo
```

El comando compila el núcleo y utiliza únicamente `datos_originales/CURSE/DATA/TITLE.DAT` y `TITLE.PAL`. Genera `apps/juego/trabajo/vista_titulo/titulo_cpp.png` y un JSON de procedencia. Node gestiona la compilación; Python llama a las funciones C++ y escribe el PNG. No interpreta lógica del juego ni efectúa la descompresión en Python.

La imagen C++ generada se ha contrastado con la referencia obtenida anteriormente mediante el ejecutable. **Ambos PNG tienen el mismo SHA-256**, y los índices descomprimidos del título tienen la huella `e9bd143ea644c93e43330b763b007913cf604e5865c39437ae432e7d908ea714`.

La creación del PNG aplica la misma expansión de componentes de paleta que la referencia visual. No sustituye el dibujo de entidades, el desplazamiento de vista, la entrada ni la cadencia del bucle del juego. La [capa de presentación SDL](ventana_sdl.md) conecta las vistas reconstruidas y conserva separado el núcleo.

## Método dos y fondos de escena

`apps/juego/motor/recursos/rnc2.cpp` traslada `67BC`–`69BD`. Su lector consume dos bits iniciales y después bits de mayor a menor peso, intercalados con literales. Conserva las longitudes cortas, literales agrupados, dieciséis prefijos de distancia, referencias solapadas y terminadores de bloque con continuación. A diferencia de DOS, la API requiere entrada y salida separadas. Comprueba tamaños y referencias; no verifica CRC.

`pnpm comparar:rnc2` compara los dieciséis DAT de método dos y las diez tiras de la mazmorra con el original, además de programas sintéticos. La muestra comprobada de **67 casos** cubre dos DAT, tres tiras, códigos cortos/largos, continuación y límites de cabecera. La comparación amplia está preparada y pendiente de ejecutar por el propietario. Los informes quedan en `apps/juego/trabajo/comparacion_rnc2/`.

`pnpm comparar:presentacion` agrupa esta comparación con fondos, sprites y fundidos originales. El [recorrido de arranque y escena](arranque_escena.md) documenta su alcance y sus límites.
