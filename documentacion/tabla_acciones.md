# Construcción de la tabla de acciones

## Hallazgo

La llamada indirecta de `0000:DE2B` utiliza una tabla de diez palabras en `DS:B60C`. El valor inicial de esa zona en el archivo es cero, pero no representa sus destinos durante el juego: el bloque `0000:DF15`–`0000:DF4D` la reconstruye a partir del estado actual.

Se ha trasladado ese bloque a [apps/juego/motor/acciones/construir_tabla.cpp](../apps/juego/motor/acciones/construir_tabla.cpp). Es la primera pieza C++ reconstruida desde instrucciones del ejecutable. Su contrato consiste en reproducir las escrituras en memoria de ese bloque, no todos los registros e indicadores de la CPU ni la rutina completa de presentación que lo contiene.

## Datos que utiliza

| Dirección relativa en DS | Contenido observado                                                                   |
| ------------------------ | ------------------------------------------------------------------------------------- |
| `B60C`–`B61F`            | Diez destinos de acción, una palabra por casilla.                                     |
| `B620`–`B633`            | Diez identificadores que describen las casillas actuales.                             |
| `B636`                   | Manejador alternativo para identificadores con valor con signo igual o superior a 23. |
| `B686`–`B6B3`            | Tabla base de 23 destinos para los identificadores ordinarios de 0 a 22.              |

El bloque empieza cargando DS y ES desde `CS:0000`. A continuación establece SI en `B620`, DI en `B60C` y CX en 10. Se ha analizado suponiendo el indicador de dirección despejado (`DF=0`), de modo que `LODSW` y `STOSW` avanzan.

## Reglas reconstruidas

Para cada una de las diez posiciones:

1. Lee un identificador de `DS:B620`.
2. Si es `FFFFh`, escribe `DE96h` en esa posición y en todas las restantes, sin seguir leyendo identificadores.
3. Si su valor **con signo** es igual o superior a 23, copia el manejador de `DS:B636`.
4. En los demás casos, duplica el identificador como palabra de 16 bits y lee el destino en `DS:(B686h + índice)`, con envoltura de dirección de 16 bits.

Para los valores normales 0–22, la última regla consulta las 23 entradas de la tabla base. Los valores `8000h`–`FFFEh` son negativos en la comparación original y también siguen esa regla: pueden consultar otras zonas del segmento. El traslado conserva ese detalle y por eso trabaja con un segmento de 65.536 bytes, en vez de sustituirlo por un acceso limitado a un vector de 23 elementos. Esto reproduce las instrucciones; no demuestra que el juego genere esos valores durante una partida.

## Cómo se modifican las acciones

El análisis incorpora las fuentes observadas de los destinos:

- `DE96` inicializa los diez identificadores a 0–9 y reconstruye la tabla. Si se cumple una condición del estado, `DEF1` y `DEF4` sustituyen las casillas 1 y 2 por `DE96`.
- `DFD3`, `E010`, `E039`, `E05B`, `E0E9`, `E0FD`, `E126` y `E14A` preparan listas de identificadores o cambian el manejador alternativo antes de reconstruir.
- `E0B8` sustituye directamente el primer destino por `E0C0` después de reconstruir una de las listas.

| Escritura                | Valor asignado a `DS:B636` |
| ------------------------ | -------------------------- |
| `0000:DFF3`              | `DE96`                     |
| `0000:E007`              | `563A`                     |
| `0000:E030`              | `32DE`                     |
| `0000:E0A1`, `0000:E0E0` | `3B53`                     |
| `0000:E0F4`              | `384A`                     |
| `0000:E11D`              | `2FCA`                     |
| `0000:E155`              | `3143`                     |

La unión de la tabla base y estas sustituciones aporta 23 destinos distintos al recorrido. El mapa los marca como **candidatos de una tabla dinámica**: el índice de la tabla fuente no se confunde con la casilla de la interfaz, y la lista no se presenta como prueba de exhaustividad para todos los estados posibles.

En `DE10`–`DE23`, el consumidor divide entre 32 el campo `DS:B60A`, lo convierte en un índice de palabras y consulta la tabla resultante. Omite una entrada nula; para el resto llama al destino en `DE2B`. Después comprueba el acarreo y, según el resultado, continúa la presentación o toma otra ruta. Todavía hay que documentar el contrato completo de retorno de todos los manejadores.

## Comparación contra las instrucciones originales

Se ha preparado `pnpm comparar:acciones`, que, para este bloque:

1. Compila los bloques C++ disponibles y un pequeño puente de prueba en una biblioteca local.
2. Comprueba la huella del ejecutable original.
3. Carga su imagen MZ en Unicorn, en modo x86 de 16 bits y con base de carga cero.
4. Prepara `CS:0000` como lo deja el arranque y ejecuta desde `DF15` hasta antes de `DF4E`.
5. Llama al bloque C++ con una copia de los mismos datos y compara **todo el segmento DS**, incluidos los bytes que no deben cambiar.

Se han superado **548 casos**: las diez posiciones de la marca de fin, cada identificador de la tabla base, el umbral de comparación, valores con el bit de signo activo, manejadores alternativos y 512 casos generados con semilla fija. Los 65.536 bytes coinciden en todos los casos. Esta comprobación usa las instrucciones reales del ejecutable como referencia, sin escribir otra versión de la fórmula esperada en las pruebas.

El informe local `apps/juego/trabajo/comparacion_acciones/resultado.json` guarda la huella del ejecutable, la de la biblioteca comparada, la versión del emulador y el resultado. No se publican el binario del juego ni el listado completo de instrucciones.

### Requisitos y comando

Además del entorno habitual, se necesita **Clang con LLD** para la comparación en Windows de 64 bits. `clang++` debe estar disponible en la ruta de ejecutables. La variable `CXX` permite indicar la ruta de un compilador Clang concreto. Node y Python deben ser de 64 bits en Windows.

```sh
pnpm preparar
pnpm comparar:acciones
```

La preparación instala Unicorn 2.1.4 junto a Capstone. No es necesario activar el entorno Python. La comparación se ha probado en Windows de 64 bits; las rutas de compilación para otros sistemas todavía no se han validado.

## Alcance pendiente

Este traslado reproduce las escrituras de `DF15`–`DF4D` con `DF=0`. No sustituye los registros, los indicadores, el dibujo posterior, los manejadores de acciones ni la emulación de DOS. El juego completo sigue pendiente de ejecución y contraste.

La primera ampliación con estos candidatos alcanzó 18.724 instrucciones y 113 destinos de llamada directa, con una nueva tabla en `B7F2`. Esa tabla y el contrato de retorno ya están estudiados y trasladados como bloques a C++ en [contrato_acciones.md](contrato_acciones.md). El comando conjunto compara ahora 1.120 casos; los 548 descritos aquí corresponden exclusivamente al constructor de la tabla.
