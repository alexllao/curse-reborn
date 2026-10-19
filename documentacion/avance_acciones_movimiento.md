# Acciones completas, listas y movimiento

Esta entrega amplía el C++ hasta el retorno de la acción que empieza en `0000:B740`, incluyendo su salto común a `886B` y las dependencias numéricas. No dibuja todavía una escena ni ejecuta una partida.

Se ha compilado y enlazado la biblioteca. La muestra de **421 casos** coincide con las instrucciones originales. El propietario ha confirmado también la coincidencia de los **10.930 casos** de la comparación amplia en su equipo.

## Código trasladado

| Original                    | C++                                  | Comportamiento                                                                        |
| --------------------------- | ------------------------------------ | ------------------------------------------------------------------------------------- |
| `88BC`–`88D8`               | `acciones/lista_identificadores.cpp` | Consulta un identificador en diez palabras de `DS:5225`.                              |
| `88D9`–`88F2`               | Mismo archivo                        | Inserta en la primera palabra `FFFFh`, permitiendo duplicados.                        |
| `88F3`–`891D`               | Mismo archivo                        | Retira la primera coincidencia, desplaza las siguientes y termina con `FFFFh`.        |
| `0BF8`–`0F5E`               | `movimiento/direccion.cpp`           | Selecciona secuencias de dirección según cuatro bits, estado previo y modalidad.      |
| `0F5F`–`10EB`               | `movimiento/planificar.cpp`          | Calcula deltas, pasos e incrementos de movimiento hacia un nodo.                      |
| `096D`–`0999`               | `movimiento/avanzar.cpp`             | Aplica un paso con acarreo y decide entre las continuaciones `099A` y `0A59`.         |
| `8822`–`886A`               | `acciones/iniciar_accion.cpp`        | Reinicia campos de control y fases.                                                   |
| `88A0`–`88BB`               | Mismo archivo                        | Prepara el estado de rechazo; el original retorna con acarreo activo.                 |
| `886B`–`889F`               | Mismo archivo                        | Reinicia, planifica si hay una acción pendiente, prepara estado del cursor y retorna. |
| `B740` hasta su `RET` común | Mismo archivo                        | Une preparación, generador, coincidencias e inicio común de la acción.                |

Las rutas de C++ de la tabla son relativas a `apps/juego/motor/`. Las funciones trabajan con el estado y un segmento de datos explícitos; no realizan llamadas DOS ni acceden al hardware.

## Detalles que conserva el traslado

- Las listas recorren las diez posiciones incluso si aparece una marca `FFFFh` intermedia. En su API, `true` equivale a acarreo despejado en el original.
- Las direcciones usan las dos tablas originales de 16 combinaciones. Se mantienen los retornos anticipados: hay casos que escriben un campo y dejan otros como estaban. No se reinicializa toda la estructura indiscriminadamente.
- La planificación mantiene la modalidad especial de `DS:8082=24h` y `DS:358C=FEh`, y el tratamiento de `14h`. Los desplazamientos y las sumas envuelven a 16 bits.
- El numerador de la división se construye a partir de la componente secundaria multiplicada por 256, y el cociente se separa en palabras equivalentes a las instrucciones originales.
- El avance suma primero la parte fraccionaria y propaga el acarreo a la coordenada entera. Con contador cero no escribe; al llegar al último paso cambia la continuación.
- La entrada común conserva los campos que el original no reinicia, e inicia la planificación únicamente cuando `DS:0AD7` es distinto de cero.

El listado usa ahora los nombres `CBW` y `CWD` para los opcodes `98h` y `99h` sin prefijo de tamaño en este código de 16 bits. La denominación que devolvía Capstone para esos bytes podía sugerir operaciones de 32 bits; los bytes originales no cambian.

## Contratos y límites

La planificación devuelve el AX observado: el contador de movimiento, con mínimo uno. Su API devuelve `false` si una división sería inválida o su cociente no cabe en 16 bits con signo. El original produciría una excepción; pueden haberse realizado escrituras anteriores a ese punto. La comparación preparada utiliza geometrías del dominio sin esas excepciones.

Las funciones de entrada de acción devuelven AX por parámetro. Un resultado `true` de la API indica que la ejecución trasladada es válida; el acarreo original en el retorno normal de `886B` está activo. No se confunde ese estado con el indicador físico de la CPU que ejecuta C++.

Los bloques que actualizan memoria no prometen reproducir todos los registros o indicadores del procesador. La comparación comprueba DS completo, los retornos declarados, el estado aleatorio en la acción completa y el equilibrio de la pila original. La actualización de entidades de `10EC` se amplía en el [traslado siguiente](entidades.md); el avance temporal se amplía en [secuencias_animacion.md](secuencias_animacion.md), mientras la presentación continúa pendiente.

## Un comando para el conjunto

```sh
pnpm comparar:avance
```

| Grupo                                      | Casos preparados |
| ------------------------------------------ | ---------------: |
| Listas                                     |              210 |
| Direcciones                                |            6.240 |
| Planificación                              |            2.048 |
| Avance de pasos                            |            1.024 |
| Reinicio, rechazo e inicio común           |              384 |
| Acción completa, con semillas persistentes |            1.024 |
| **Total**                                  |       **10.930** |

La muestra ejecutada con `pnpm comparar:avance --muestra` contiene 421 casos. Su informe se escribe en `apps/juego/trabajo/comparacion_avance/muestra.json`; el informe de la comparación completa se guarda por separado en `resultado.json`, para evitar presentar una muestra como validación amplia.

Unicorn recorre el código original y sus llamadas reales. No se sustituyen los manejadores de movimiento ni las auxiliares por implementaciones de prueba. El conjunto incluye modalidades de dirección, imágenes límite, lecturas de nodo que cruzan el final del segmento, listas con duplicados y operaciones sucesivas con el mismo estado.

## Continuación

La [creación y actualización de entidades](entidades.md) amplía este conjunto con las dependencias utilizadas por `0000:10EC`. El [avance de secuencias](secuencias_animacion.md) incorpora los tres canales y la retirada; sus operaciones de sonido y carga todavía deben integrarse.
