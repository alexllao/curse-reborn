# Preparación integrada de resultados en C++

El bloque [preparar_resultados.cpp](../apps/juego/motor/acciones/preparar_resultados.cpp) integra el generador y la selección de coincidencias. Corresponde a `0000:B740`–`0000:B939`, con la auxiliar `0000:88F3`–`0000:891D`. **El propietario confirmó la coincidencia en las 2.048 operaciones integradas**, incluido DS completo y el estado del generador. La ejecución no se ha repetido íntegramente en el entorno del asistente.

## Recorrido trasladado

1. Busca la primera aparición de `008Ch` en las diez palabras de `DS:5225`. Si existe, desplaza las posteriores y termina la lista con `FFFFh`. La búsqueda no se detiene al encontrar otras marcas `FFFFh`.
2. Limpia las catorce marcas que seleccionará el bloque de coincidencias.
3. Genera un primer resultado entre 0 y 5 y guarda su representación más uno en `DS:2C8F`.
4. Genera un valor entre 0 y 99. Si es menor que 50, reutiliza el primer resultado; en caso contrario genera otro entre 0 y 5. Lo guarda más uno en `DS:2CB0`.
5. Repite esa decisión para el tercero, reutilizando otra vez **el primero**, y lo guarda más uno en `DS:2CD1`.
6. Llama al bloque C++ de coincidencias y prepara los registros que las instrucciones siguientes utilizan para continuar el estado y la animación.

Cada operación consume entre tres y cinco llamadas al generador. Su orden y la decisión de reutilizar el primer resultado forman parte del comportamiento original; cambiar cualquiera de ellos altera las secuencias posteriores.

El traslado conserva los campos de los registros de estado que el original no escribe. Termina antes de `B93A`, que salta a `886B`: no ejecuta esa rutina compartida ni sus dependencias. El parámetro de datos representa un segmento DS completo y el estado aleatorio se recibe de forma explícita, sin depender de las direcciones DOS en ejecución.

## Comparación preparada

```sh
pnpm comparar:resultados
```

El comando compila la biblioteca y ejecuta **solo esta comparación integrada**. Unicorn recorre el tramo original con sus llamadas reales a la retirada de la lista y al generador, sin sustituirlas por implementaciones de prueba.

Se preparan 64 semillas, dos estados iniciales de memoria y 16 operaciones consecutivas por secuencia: **2.048 operaciones previstas**. Se incluyen listas con el identificador en cada posición, sin él y con duplicados situados después de una marca `FFFFh`.

Después de cada operación se compara el segmento DS completo y las cuatro palabras del estado aleatorio; también se comprueba que las llamadas originales dejan la pila equilibrada. Las modificaciones se conservan entre operaciones para detectar desajustes acumulados. El informe se guarda en `apps/juego/trabajo/comparacion_resultados/resultado.json`.

No se comparan todos los registros ni indicadores de la CPU, el dibujo posterior ni una partida completa. La ampliación que continúa por `886B` hasta el retorno está en [avance_acciones_movimiento.md](avance_acciones_movimiento.md); el propietario ha confirmado la coincidencia de sus 10.930 casos.
