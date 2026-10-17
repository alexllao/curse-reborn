# Copias de vídeo, paleta y referencia visual del título

El código de [planos.cpp](../motor/video/planos.cpp) traslada cuatro rutinas gráficas originales. La muestra de **83 casos** coincide con el ejecutable en RAM, los cuatro planos y las operaciones de puerto. Hay **1.183 casos** preparados para la comparación amplia.

También se ha recuperado y comprobado visualmente la pantalla de título como imagen de diagnóstico. **No es una partida ni una ventana SDL.** Esta referencia inicial utiliza la rutina original de `CURSE.EXE` en Unicorn. El [traslado posterior de RNC](descompresion_rnc.md) ya permite generar la misma imagen con C++ mediante `pnpm titulo`, sin ejecutar esa rutina original.

## Recorrido desde el arranque

Las instrucciones de `0584`–`0612` leen `TITLE.PAL` y `TITLE.DAT`, llaman a `64E0`, suben la imagen con `7636` y envían la paleta con `787E`. Preparan una imagen de 320 × 200, con un paso de destino de 448 píxeles, equivalente a 112 bytes por plano. La presentación completa incluye además la selección de página, esperas y entrada; ese conjunto aún no está trasladado.

| Rutina original | Traslado C++ | Operación |
| --- | --- | --- |
| `7636`–`7674` | `subir_imagen` | Lee cuatro índices consecutivos de una imagen lineal y escribe uno en cada plano de vídeo. |
| `7679`–`76B7` | `recuperar_imagen` | Selecciona cada plano de lectura y reconstruye los cuatro índices lineales. |
| `76BC`–`770D` | `copiar_planos` | Copia mediante los cuatro cerrojos VGA en modo de escritura uno, con pasos independientes. |
| `787E`–`788D` | `enviar_paleta` | Escribe el índice inicial en `3C8h` y los componentes RGB consecutivos en `3C9h`. |

## Memoria por planos

El C++ recibe cuatro planos contiguos de 65.536 bytes. La posición y los pasos de sus funciones se expresan en bytes de cada plano; en las entradas originales AX, CX y DX se expresan en píxeles y se desplazan dos bits.

El número de grupos es `ancho >> 2`: los restos de uno a tres píxeles se descartan, como hacen las instrucciones originales. Dentro de cada grupo, la subida selecciona máscaras `1`, `2`, `4` y `8`; la recuperación selecciona planos `0`, `1`, `2` y `3`. Se conservan el orden y el tamaño de las operaciones de puerto.

La copia usa el modo de escritura uno. Cada lectura carga simultáneamente los cuatro cerrojos y la escritura los copia al destino con máscara `0Fh`. El traslado mantiene ese avance de byte en byte, incluyendo solapamientos. No usa una copia con dirección elegida para evitar solapamientos: eso cambiaría lo que hace el original.

`PuertosVideo` sustituye las instrucciones de entrada y salida por funciones del llamador. El código nunca toca el hardware del ordenador anfitrión. Las matrices de planos son estado explícito y todavía deben conectarse a una textura y una ventana SDL.

La API acepta anchos de al menos cuatro píxeles, alturas no nulas, pasos no menores que el número de grupos, imágenes lineales de hasta 65.536 bytes y regiones completas dentro de cada plano. Rechaza las geometrías que cruzarían su final. Esos límites evitan atribuir al traslado los efectos de contadores nulos, bucles enormes y acarreos de direcciones que no están cubiertos por esta comparación. Origen lineal y planos deben ser búferes separados.

La paleta admite entre uno y 256 colores y conserva los bytes que recibe, sin reducirlos ni expandirlos. La posición de lectura envuelve a dieciséis bits, como SI. La expansión de componentes de seis a ocho bits se hace únicamente al escribir la imagen PNG de diagnóstico.

## Modelo de vídeo de las pruebas

La comparación ejecuta las cuatro rutinas en Unicorn. Un modelo mínimo registra las máscaras de escritura, plano de lectura, registro de modo y cerrojos de cuatro planos. Las lecturas y escrituras de memoria VGA se aplican mediante esos registros. Esto permite comparar los **262.144 bytes** de vídeo, además de los **65.536 bytes** de RAM y la traza de puertos.

El modelo implementa las operaciones que utilizan estas rutinas. No es una emulación completa de VGA ni valida modos de vídeo ajenos a este recorrido. Las escrituras de palabra en un puerto se conservan como una operación de dos bytes, aunque el modelo distribuya internamente índice y valor.

## Pantalla de título como referencia

La prueba ejecuta `64E0` con `TITLE.DAT` como origen y un destino separado, comprueba su retorno y recoge los 64.000 índices descomprimidos. Después compara la subida C++ con `7636`, la recuperación C++ con `7679` y la paleta con `787E`. Los índices recuperados tienen que ser idénticos a los descomprimidos antes de generar el PNG.

Se guardan localmente:

- `trabajo/vista_titulo/titulo.png`: referencia visual de 320 × 200.
- `trabajo/vista_titulo/procedencia.json`: huellas del ejecutable, archivo comprimido e imagen indexada, y separación entre descompresión original y copias trasladadas.

La paleta del título contiene 768 bytes con componentes entre cero y 63. Para el PNG se convierten mediante `componente × 255 / 63`, con división entera. La imagen se ha inspeccionado: aparecen el título y el castillo completos. No se han inventado gráficos ni sustituido los recursos originales. Los archivos derivados permanecen fuera de Git.

El [descompresor RNC de método uno](descompresion_rnc.md) ya está trasladado y conserva su comparación propia. El cargador real y la presentación interactiva continúan pendientes. Este conjunto gráfico sigue usando la descompresión original como referencia independiente.

## Comparación agrupada

```sh
pnpm comparar:video
```

| Grupo | Comparación amplia preparada | Muestra comprobada |
| --- | ---: | ---: |
| Subida de imagen | 378 | 24 |
| Recuperación de imagen | 378 | 24 |
| Copia de planos y solapamientos | 360 | 24 |
| Envío de paleta | 64 | 8 |
| Título original: subida, recuperación y paleta | 3 | 3 |
| **Total** | **1.183** | **83** |

Se comprueban también el retorno y el equilibrio de pila. La muestra se ejecuta añadiendo `--muestra`. Los informes separados se guardan en `trabajo/comparacion_video/muestra.json` y `resultado.json`. Este comando requiere también los archivos locales `DATA/TITLE.DAT` y `DATA/TITLE.PAL`.
