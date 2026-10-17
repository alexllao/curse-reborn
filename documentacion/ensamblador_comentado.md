# Ensamblador como referencia del traslado a C/C++

El resultado buscado es un motor en C/C++ que pueda portarse. El listado de ensamblador permite revisar qué hace cada tramo original y a qué bloque C++ corresponde.

## Generación

```sh
pnpm ensamblador
```

Es un nombre alternativo del análisis. Tanto ese comando como `pnpm analizar` generan:

- `trabajo/analisis_ejecutable/desensamblado.asm`: instrucciones candidatas con direcciones relativas, posiciones en el archivo, bytes y comentarios propios en castellano.
- `trabajo/analisis_ejecutable/analisis.json`: instrucciones, enlaces, tablas, incidencias y anotaciones estructuradas.

Los comentarios se conservan en [mapa_ejecutable.json](mapa_ejecutable.json), vinculados al SHA-256 del ejecutable. Actualmente hay 157 anotaciones sobre entradas, bucle, recursos, manejadores y tramos trasladados. Las anotaciones de los bloques reconstruidos incluyen su archivo C++ correspondiente. Regenerar el listado conserva esos comentarios. Las direcciones anotadas que no se alcanzan se señalan al final del listado.

## Alcance

El listado es una reconstrucción para lectura y análisis. No es el fuente original perdido ni un fichero preparado para recompilar con un ensamblador. Aún faltan símbolos, separación exhaustiva de código y datos, definición de estructuras y otros detalles para poder reconstruir un ejecutable ensamblado completo.

Los comentarios explican las partes estudiadas; no se presenta el resto del listado como código ya comprendido. Los mnemónicos, los registros y los identificadores técnicos de x86 mantienen sus nombres.

El listado completo derivado del ejecutable queda en `trabajo/`, junto a los originales locales excluidos de Git. Se versionan las herramientas, las anotaciones propias y el C++ trasladado. Cada nuevo traslado debe ampliar las referencias y las comprobaciones, de modo que el ensamblador siga siendo una ayuda verificable para el código portable.
