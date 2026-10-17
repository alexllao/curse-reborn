# Primer mapa del ejecutable

## Objetivo y alcance

El trabajo parte del código de `CURSE.EXE`: identificar sus rutinas y estructuras, documentar su comportamiento y trasladarlo a C++ con SDL. La lectura de los recursos se reconstruirá siguiendo el código original que los utiliza.

Esta primera entrega es estática. No se ha ejecutado el juego, validado su comportamiento en DOSBox ni traducido ninguna rutina al motor nuevo. Un destino de llamada es una entrada candidata, no una función completamente comprendida.

## Copia analizada

| Propiedad                                | Valor                                                              |
| ---------------------------------------- | ------------------------------------------------------------------ |
| Archivo local                            | `datos_originales/CURSE/CURSE.EXE`                                 |
| SHA-256                                  | `f2c0046e07b25f408516edae5d6a66c06107971fb95c55ed770fa52a6b4ecd8d` |
| Tamaño                                   | 111.606 bytes                                                      |
| Cabecera DOS MZ                          | 512 bytes                                                          |
| Imagen almacenada tras la cabecera       | 111.094 bytes                                                      |
| Entrada relativa CS:IP                   | `0000:001A`                                                        |
| Posición de entrada en el archivo        | `0x021A`                                                           |
| Pila inicial relativa SS:SP              | `1B20:0400`                                                        |
| Reubicaciones                            | 28                                                                 |
| Bytes posteriores al tamaño MZ declarado | 0                                                                  |

Las 28 reubicaciones contienen inicialmente el valor `0F31`. El arranque utiliza ese segmento relativo para inicializar DS. Los datos correspondientes comienzan en la posición de archivo `0xF510`.

Las direcciones de este documento son relativas a la imagen MZ antes de que DOS la reubique. Para una dirección `segmento:desplazamiento`, la posición del archivo es `512 + segmento × 16 + desplazamiento`. El segmento real en ejecución depende de dónde cargue DOS el programa; no se conoce a partir del archivo por sí solo.

## Recorrido inicial identificado

| Dirección relativa | Evidencia y función observada                                                                                                                                                                                                   |
| ------------------ | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `0000:001A`        | Inicio: calcula el tamaño del bloque de memoria y solicita su ajuste mediante `INT 21h`, función `4Ah`.                                                                                                                         |
| `0000:0027`        | Guarda referencias de segmento en `CS:0000` y `CS:0002`; inicializa DS con el segmento reubicable `0F31`.                                                                                                                       |
| `0000:003C`        | Abre la cadena `curse.cfg`, situada en `DS:0DFD`, para lectura. Si falla, salta a `0000:007E`.                                                                                                                                  |
| `0000:0048`        | Solicita ocho bytes para `DS:99BA`, seguidos de cuatro lecturas de un byte para `DS:0AD3` a `DS:0AD6`; cierra el archivo. Aún no se han asignado nombres definitivos a esos campos.                                             |
| `0000:0081`        | Selecciona el modo BIOS `13h` y después modifica registros de VGA. La organización final de la imagen debe deducirse de esa configuración y de las rutinas gráficas; el modo BIOS inicial no basta para asumir un búfer lineal. |
| `0000:00EC`        | Consulta el vector `33h`, inspecciona su entrada y, si procede, llama a la interfaz del ratón.                                                                                                                                  |
| `0000:011D`        | Cambia al directorio `DATA`, cuya cadena está en `DS:0083`.                                                                                                                                                                     |
| `0000:0124`        | Llama a `0000:8C26`, que instala un manejador del vector `08h`.                                                                                                                                                                 |
| `0000:012F`        | Llama a `0000:6A4E`, que instala un manejador del vector `09h`.                                                                                                                                                                 |
| `0000:0132`        | Selecciona rutas de inicialización según campos de configuración. Las funciones exactas de cada modalidad siguen pendientes.                                                                                                    |
| `0000:0189`        | Conserva el vector `08h` actual e instala el manejador `0000:0238`. Programa el temporizador con el divisor `4250h`.                                                                                                            |
| `0000:01C0`        | Llama a `0000:0276`, una entrada importante para continuar el análisis del juego. Todavía no se identifica toda esa rutina como el bucle principal.                                                                             |
| `0000:01C3`        | Tras regresar, comienza la restauración del temporizador y los manejadores.                                                                                                                                                     |
| `0000:0227`        | Cambia al directorio `..`, restaura el modo de texto `03h` y termina mediante `INT 21h`, función `4Ch`.                                                                                                                         |

Las cadenas mencionadas se han contrastado con las posiciones calculadas desde DS. Las llamadas relativas cuyo destino Capstone expresa con extensión de signo se normalizan a IP de 16 bits: por ejemplo, `0xFFFF8C26` corresponde aquí a `0000:8C26`.

## Manejadores y referencias comprobadas

| Rutina      | Cómo se obtiene su dirección                              | Comportamiento observado                                                                                                                                                                       |
| ----------- | --------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `0000:0238` | Instalación de `INT 08h` entre `0000:0199` y `0000:01A4`. | Atiende rutas condicionadas por configuración, incrementa un contador, acumula `4250h` y encadena el manejador anterior o confirma la interrupción. Termina mediante salto indirecto o `IRET`. |
| `0000:6A8C` | Instalación de `INT 09h` en `0000:6A69` a `0000:6A6F`.    | Lee el puerto `60h`, distingue pulsación y liberación y actualiza una tabla de estado. Confirma la interrupción y termina con `IRET`.                                                          |
| `0000:8C51` | Instalación de `INT 08h` en `0000:8C36` a `0000:8C40`.    | Actualiza campos de entrada bajo una condición de configuración y encadena el manejador anterior. La semántica completa de esos campos sigue pendiente.                                        |

Estas entradas se incluyen explícitamente en el análisis porque las llamadas DOS que instalan vectores no equivalen a instrucciones `CALL` hacia el manejador.

## Resultado de la primera pasada

Las cifras de esta sección corresponden a la pasada inicial sin tablas anotadas. La ampliación actual y el bucle central se documentan en [bucle_principal.md](bucle_principal.md).

Con el punto de entrada y los tres manejadores anteriores como raíces:

- 9.850 instrucciones candidatas alcanzadas por el recorrido estático.
- 101 destinos distintos de llamada directa.
- Ocho transferencias indirectas señaladas para revisión: `0000:0264`, `0000:0C0E`, `0000:0D0F`, `0000:17A0`, `0000:52AD`, `0000:8C81`, `0000:DE2B` y `0000:EB48`.

Dos transferencias identificadas, `0000:0264` y `0000:8C81`, encadenan vectores anteriores guardados en memoria. No debe asumirse que todos los demás destinos indirectos pertenecen al ejecutable: algunos podrían depender de memoria cargada o de servicios externos. Hace falta seguir cada caso.

## Cómo reproducirlo con pnpm

Se necesitan Node 22 o posterior, pnpm 11.18.0, Python 3.9 o posterior y la copia original en `datos_originales/`. Desde la raíz del repositorio:

```sh
pnpm install --frozen-lockfile
pnpm preparar
pnpm analizar
pnpm comprobar
```

`pnpm preparar` crea el entorno Python local e instala las dependencias fijadas. `pnpm analizar` aplica `documentacion/mapa_ejecutable.json`, con las raíces `0000:0238`, `0000:6A8C` y `0000:8C51` y las tablas anotadas. Se comprueba el SHA-256 antes de aplicarlo. El punto de entrada siempre se obtiene de la cabecera MZ. Para añadir otras entradas confirmadas, usa `pnpm analizar --raiz segmento:desplazamiento`.

Para reproducir la pasada inicial sin el mapa, en Windows se puede invocar directamente `trabajo/entorno/Scripts/python.exe herramientas/analizar_ejecutable.py --raiz 0000:0238 --raiz 0000:6A8C --raiz 0000:8C51`. En otros sistemas, el ejecutable del entorno está en `trabajo/entorno/bin/python`.

La herramienta escribe `analisis.json` y `desensamblado.asm` en `trabajo/analisis_ejecutable/`, que está excluida de Git. El JSON conserva la cabecera, las reubicaciones, las instrucciones candidatas, sus enlaces y las incidencias. El listado incluye posiciones de archivo y direcciones relativas. El ejecutable original se lee sin modificarlo.

La dependencia usada es Capstone 5.0.7, mediante su [interfaz oficial para Python](https://www.capstone-engine.org/lang_python). No se incorpora su código al repositorio. Las pruebas usan imágenes MZ e instrucciones sintéticas: comprueban cabeceras, reubicaciones, archivos truncados, saltos de 16 bits, bucles, destinos indirectos, solapamientos y terminación del recorrido.

## Límites del método

- Se recorren las dos ramas de las condiciones sin evaluar registros ni indicadores; algunas rutas podrían no ejecutarse en la práctica.
- Se supone provisionalmente que una llamada puede regresar. Las rutinas que no regresen deben marcarse tras estudiarlas.
- Los destinos indirectos se anotan sin adivinarlos. No se descubren automáticamente todas las tablas de saltos, interrupciones ni entradas activadas por datos.
- Las reubicaciones se documentan; no se simula la carga DOS ni se ejecutan instrucciones. Las llamadas lejanas que dependan de segmentos reubicados necesitan interpretación adicional.
- El código generado, modificado o cargado durante la ejecución queda fuera de esta primera pasada.
- Capstone decodifica x86 de 16 bits; eso no demuestra que cada instrucción alcanzada pertenezca al procesador mínimo del juego.
- Las instrucciones alcanzadas no equivalen a lógica comprendida. Las cifras no miden la cobertura total del juego.

## Siguiente trabajo

El recorrido desde `0000:0276`, el bucle y cinco tablas indirectas ya se han estudiado en [la ampliación del análisis](bucle_principal.md). La [construcción de la tabla dinámica de `0000:DE2B`](tabla_acciones.md), la [selección de `0000:B7F2` y el retorno de acciones](contrato_acciones.md) están documentados y trasladados como bloques a C++. El siguiente trabajo es ampliar el C++ con el generador de valores acotados y continuar el traslado de los manejadores.
