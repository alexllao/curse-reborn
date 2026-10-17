# Quince escenas y salida de las cuevas

Este tramo comprende las escenas originales 0–14. Las vistas F5/F6 se han ampliado después a dieciséis con COUNTRY1. La ruta jugable ya llega por las salidas normales hasta COUNTRY1, escena 15, cuyo recorrido posterior está documentado en [salida al campo](salida_al_campo.md). Las quince escenas incluyen la caída y corredores que comparten recursos; no son quince fondos diferentes. Core, título e introducción no cuentan en esas quince.

## Bloques reconstruidos

| Zona                     | Acciones y estados principales                                                                                      |
| ------------------------ | ------------------------------------------------------------------------------------------------------------------- |
| Mazmorra y corredor      | Liberación, pared, clip, cerradura, regreso, pecera, mecanismo, estatuas y caída                                    |
| Lago y poza              | Pecera, pez, gusano, tienda, moneda, concha, tortuga, picana, tiburón, almeja, tapón, algas, botón y puerta secreta |
| Corredores y picapedrero | Piedras persistentes, entregas, cartel y cordel                                                                     |
| Catapulta y monitor      | Tablón, apoyo, monitor, imán, bajada de la cornisa y alambre                                                        |
| Cuatro agujeros          | Examinar 65–68, expulsar las criaturas, descubrir y recoger ramitas 64                                              |
| Trampa                   | Instalar alambre 63 en los aros 36, reacción 183F y recogida del barro 37                                           |
| Pozo                     | Lanzar moneda 47, aparición de ofertas, dinero 51, mujer 52 y casco 53, incluida su recogida posterior del suelo    |
| Desprendimientos y cubo  | Ponerse casco, cruzar el peligro, entrar en el cubo 60, seis recetas de máscara y salida 24                         |

La opción Atacar del original sirve también para bajar de la cornisa y entrar en el cubo. No se sustituye por una acción inventada. En el agujero del monitor funcionan tanto Lanzar el imán con cordel como Insertar ambos objetos, en los dos órdenes que admite 3D8C–3DA5. El picapedrero rechaza objetos ajenos a sus piedras, incluido el cordel que acaba de entregar.

## Fuentes del traslado

- `3934–3AFC`: los cuatro agujeros y sus condiciones; `5BDE–5C2D`: ramitas y reposición de ofertas.
- `9C73–9CBD`: instalación de la trampa. `183F`: captura del monstruo mediante sus secuencias originales. `58E2–590A`: recogida del barro.
- `59A4–59B4`: moneda del cuarto corredor. `9E21–9F10`: lanzamiento al pozo. `9F11–9F38`: creación de fragmentos con sus tablas de posición, profundidad y tipo. `59CC–5B5F`: los tres premios y recogida del casco del suelo.
- `9CBE–9DE3`: casco y paso de desprendimientos. `320B–324B`: cubo. `A1D4–A382`: seis recetas y uso de la máscara. `4AF0` y entrada 24: COUNTRY1 con el modelo LOOKHOLE y sus dimensiones originales.

El núcleo de acciones está repartido en `motor/acciones/cuevas.cpp`, `catapulta.cpp`, `agujeros.cpp`, `pozo.cpp` y `mascara.cpp`. El menú solicita la creación de fragmentos mediante un servicio de la plataforma; la implementación común sigue siendo `crear_entidad`, con las dimensiones del recurso original. No se escriben banderas para dar por resuelto el puzle en los recorridos de prueba.

## Evidencia y alcance

`pnpm comparar:final-cuevas --muestra` conserva la partida desde el comienzo y contrasta **32.629 casos y pasos**: DS completo, azar, guiones, menús, colisiones, entradas y composición. El comparador descarta traducciones en caché de Unicorn entre referencias y variantes; esto no modifica el estado del juego. También conserva instantáneas y semillas para revisar el tramo sin reconstruir estados a mano.

`pnpm comprobar:final-cuevas` añade **dieciséis capturas SDL de 64.000 píxeles**, todas coincidentes, desde la moneda hasta salir del cubo. `pnpm comprobar:catapulta` cubre otras seis capturas. `pnpm comprobar:vistas` verifica **treinta cambios F5/F6** por las quince vistas y su regreso.

`pnpm comparar:opciones` recorre las listas reales de inventario, verbos y destinos de **47 estados**, con **5.642 selecciones coincidentes** y sin manejadores pendientes en las listas exploradas. Excluye momentos en que el original no permite abrir el menú. Guardar, cargar, borrar e información se atienden en la plataforma y se comprueban por separado.

Estas pruebas cubren el recorrido y las opciones ofrecidas en sus estados y posiciones. No acreditan de manera exhaustiva todos los tiempos, posiciones, partidas posibles o plataformas. La comparación visual se refiere a las imágenes originales; los filtros, marcos y paneles añadidos son opciones de la plataforma. La continuación de COUNTRY1 ya está integrada; los puzles posteriores a la explanada siguen pendientes.

Véanse [el recorrido de esta parte](recorridos/07_final_cuevas.txt) y [las partidas](partidas.md).
