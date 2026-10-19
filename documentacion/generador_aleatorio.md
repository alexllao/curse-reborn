# Generador de valores acotados en C++

Se ha trasladado la rutina `0000:788E`–`0000:78DD` a [apps/juego/motor/aleatorio/generador.cpp](../apps/juego/motor/aleatorio/generador.cpp). **El propietario ha confirmado la comparación en su equipo: las 18.816 llamadas coinciden tanto en las secuencias como en las cuatro palabras del estado.** Esta ejecución no se ha repetido en el entorno del asistente.

## Contrato observado

- Entrada: AX contiene un divisor distinto de cero.
- Estado persistente: cuatro palabras en `CS:78DE`, `CS:78E0`, `CS:78E2` y `CS:78E4`.
- Salida: AX contiene un valor entre cero y el divisor menos uno.
- El original conserva BX, CX y DX mediante la pila y retorna con `RET` cercano.

El traslado usa `EstadoAleatorio`, con cuatro palabras de 16 bits, y recibe el estado de forma explícita. No depende de una dirección DOS, del reloj del sistema ni de una biblioteca de números aleatorios. Con la misma semilla y los mismos límites debe producir la misma secuencia que el original; esa es la propiedad que comprueba el comparador preparado.

## Operaciones trasladadas

1. Lee las cuatro palabras en AX, BX, CX y DX.
2. Desplaza DX una posición y realiza tres parejas de rotaciones CX/DX a través del acarreo.
3. Resta siete a CX y propaga el préstamo a DX.
4. Combina AX con CX, y BX con DX, mediante XOR.
5. Guarda CX, BX, AX y DX como las cuatro nuevas palabras, en ese orden.
6. Divide el nuevo BX por el límite y devuelve el resto.

Se conservan los desbordamientos de 16 bits. Las rotaciones no se sustituyen por desplazamientos independientes porque transmiten bits entre las dos palabras a través del acarreo.

El C++ rechaza un límite cero devolviendo `false`, sin modificar el estado ni el resultado. Es una protección de la nueva API. En el original, el estado se actualiza y después `DIV BX` provoca una excepción si el divisor es cero; no se afirma equivalencia para esa entrada inválida.

## Comparación preparada

```sh
pnpm comparar:aleatorio
```

El comando utiliza las dependencias y el compilador ya empleados para las acciones. Compila la biblioteca de los bloques C++ y ejecuta **solo** la comparación del generador, sin repetir las 1.120 comparaciones de acciones ni regenerar el ensamblador.

Se preparan 84 semillas: la inicial del ejecutable, valores extremos, un patrón mixto, cada uno de los 64 bits del estado por separado y 16 estados generados con semilla fija. Para cada semilla se prueban siete límites y 32 llamadas consecutivas: **18.816 llamadas previstas**.

Unicorn ejecuta la rutina original completa, incluido su retorno, con una pila de prueba. Se compara el valor devuelto y las cuatro palabras del estado después de cada llamada; también se comprueba la conservación de BX, CX y DX y el ajuste esperado de la pila del original. El rechazo del límite cero se prueba por separado como contrato exclusivo de C++.

Si todo coincide, el informe se guarda en `trabajo/comparacion_aleatorio/resultado.json`, con las huellas del ejecutable y de la biblioteca. La prueba no valida todos los indicadores de CPU ni el comportamiento completo del juego.
