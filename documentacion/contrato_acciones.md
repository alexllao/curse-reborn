# Contratos de acciones y selección de coincidencias

El objetivo final es tener código C/C++ portable. Este documento relaciona dos nuevos bloques C++ con las instrucciones que sustituyen y con sus comprobaciones. El ensamblador comentado sirve para revisar la correspondencia.

## Entrada observada a los manejadores de acciones

En el llamador, entre `0000:DE10` y `0000:DE2B`:

- DS apunta al segmento de estado del juego.
- `DS:B60A` se desplaza cinco bits a la derecha y se copia en BX. Ese valor selecciona la casilla de acción; en el recorrido normal observado está entre 0 y 9.
- SI recibe el doble del índice, para consultar palabras en `DS:B60C`.
- AX recibe el destino de la casilla. Si es cero, no hay llamada. En el resto de los casos se realiza un `CALL AX` cercano.

La tabla de acciones, sus sustituciones y su constructor se documentan en [tabla_acciones.md](tabla_acciones.md). Esta entrada es el contrato observado de este llamador; no implica que cada rutina tenga ese mismo contrato cuando se invoque desde otros lugares.

### Uso del índice en algunos manejadores

`32DE`, `2FCA`, `3143`, `384A`, `3B53` y `563A` empiezan duplicando BX y leyendo el identificador en `DS:B620 + BX×2`. A continuación realizan transformaciones distintas: `32DE` resta `17h`, mientras que las otras entradas mencionadas restan `1Ah`. Esto confirma que se les entrega la **casilla**, y que no se debe sustituir directamente BX por el identificador del objeto o por el destino de la acción.

No se declara que BX, CX, DX, SI o DI se conserven en todos los manejadores. El llamador recarga el estado que necesita en sus rutas posteriores. La auditoría completa de registros y efectos de cada manejador sigue pendiente.

## Retorno consumido por el llamador

El bloque `0000:DE2D`–`0000:DE59` decide la continuación utilizando el acarreo y AX:

| Estado después del manejador           | Primer destino observado | Decisión C++                                                     |
| -------------------------------------- | ------------------------ | ---------------------------------------------------------------- |
| Acarreo despejado                      | `DE2F`                   | Continuar la selección y su presentación.                        |
| Acarreo activo, AX distinto de `8000h` | `DE5B`                   | Finalizar pasando por la llamada de restauración gráfica `D99F`. |
| Acarreo activo, AX igual a `8000h`     | `DE5E`                   | Finalizar saltándose esa llamada previa.                         |

`DE5E` es también la convergencia después de la llamada de `DE5B`; las dos rutas comparten código posterior. El traslado [retorno_accion.cpp](../apps/juego/motor/acciones/retorno_accion.cpp) devuelve una decisión tipada. **No ejecuta** todavía la restauración ni la presentación.

Se han comparado 140 casos con las instrucciones originales: 70 valores de AX, incluidos `7FFFh`, `8000h` y `8001h`, con los dos estados de acarreo. Unicorn ejecuta los saltos originales y se detiene antes de los tres destinos anteriores. La prueba compara el destino realmente alcanzado con la decisión de C++; no sustituye esa ejecución por otra fórmula escrita en Python.

## Tabla de seis coincidencias de `B7F2`

Entre `B78C` y `B7CC` se obtienen tres resultados de 0 a 5. Se llama a `788E` con AX igual a seis, o se copia un resultado ya obtenido. El final del generador divide por el AX de entrada y devuelve el resto: con divisor seis, el resultado está entre cero y cinco. El generador conserva DX y permite mantener los dos primeros valores en DL y DH.

El bloque trasladado empieza en `B7D1`, con DL, DH y AL como primer, segundo y tercer resultado. Termina antes de `B8A3`:

- Si los tres difieren, no escribe en este tramo.
- Si se repiten dos, activa las marcas que corresponden al valor repetido.
- Si los tres son iguales, activa las siete marcas del grupo par o impar, según el bit bajo del resultado.

Las seis palabras de `CS:B7F7` terminan donde empieza el primer destino, `B803`. Los destinos son `B803`, `B815`, `B822`, `B829`, `B83A` y `B84B`. El índice está acotado por el generador y por las copias entre resultados; el mapa incorpora esta tabla al recorrido.

El traslado [seleccionar_coincidencias.cpp](../apps/juego/motor/acciones/seleccionar_coincidencias.cpp) conserva las escrituras originales en DS. No limpia las marcas anteriores: esa limpieza ocurre antes del bloque, en `B746`–`B787`. Tampoco genera los números ni reproduce la animación posterior. La API C++ rechaza sin escribir valores fuera de 0–5; ese rechazo es una comprobación de su contrato, no un comportamiento atribuido al original para entradas inválidas.

### Comprobación de todas las combinaciones válidas

Se ejecutan las 216 combinaciones de tres valores entre 0 y 5, cada una con dos estados iniciales del segmento: los datos originales y un segmento generado con semilla fija. En los **432 casos**, los 65.536 bytes de DS resultantes coinciden entre el bloque x86 y C++. El segundo estado comprueba que el traslado conserva las posiciones y valores que no debe tocar.

La prueba termina antes de `B8A3`, que inicia otra fase del estado y la animación. No atribuye significado visual a los seis resultados ni reproduce una secuencia completa del juego.

## Comparación conjunta

```sh
pnpm comparar:acciones
```

El comando compila los tres bloques C++ y compara:

| Bloque                                              | Casos     |
| --------------------------------------------------- | --------- |
| Construcción de la tabla de acciones, `DF15`–`DF4D` | 548       |
| Selección de coincidencias, `B7D1`–`B8A2`           | 432       |
| Decisión del retorno, `DE2D`–`DE59`                 | 140       |
| **Total**                                           | **1.120** |

El informe local conserva la huella del ejecutable y de la biblioteca compilada. Estas comprobaciones acotan bloques concretos; no validan una partida, la generación aleatoria, todos los registros de la CPU ni la integración con SDL.

## Próximo traslado

La última pasada alcanza 18.744 instrucciones candidatas, 113 destinos de llamada directa y siete tablas o fuentes de destinos anotadas. Las dos transferencias sin candidatos incorporados encadenan vectores anteriores de DOS. Esto no demuestra que todo el código esté descubierto: aún pueden faltar entradas activadas por datos o por estados no estudiados.

El propietario ha confirmado el generador de `788E`, la [preparación integrada de resultados](preparacion_resultados.md), los 10.930 casos de [acciones completas y movimiento](avance_acciones_movimiento.md), los 3.028 de [entidades](entidades.md) y los 8.924 de [secuencias](secuencias_animacion.md). Las [operaciones de sonido y modelos](operaciones_secuencias.md) continúan el traslado con su comparación amplia preparada. La reproducción de audio y el acceso real a archivos todavía deben integrarse.
