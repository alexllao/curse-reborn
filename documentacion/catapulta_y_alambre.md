# Catapulta, imán y alambre

La partida continúa desde el cordel del picapedrero por el cuarto corredor (escena 7), SEESAW (11) y la cueva del monitor (13). Son tres vistas nuevas con entradas y regresos normales, cámara, inventario persistente, menú, audio común y marcos opcionales. F5/F6 abren sesiones independientes y no conceden objetos.

## Reconstrucción y condiciones

| Rutina DOS           | Comportamiento                                                                                                                   |
| -------------------- | -------------------------------------------------------------------------------------------------------------------------------- |
| 5B60–5BB0            | Recoger tablón 56, ocultar su afinidad y programar el modelo de recogida                                                         |
| A07E–A0DA            | Combinar primero tablón 56 y después roca 58; prepara el apoyo persistente y profundidad 10                                      |
| A0DB–A1D3            | Lanzar monitor 61: exige apoyo modelo 6, distancia horizontal ≤60 y vertical ≤5; modifica profundidades y sincroniza los canales |
| 5BB1–5BDD            | Recoger imán 57 desde la cornisa y retirarlo de la lista persistente                                                             |
| 3196–320A            | Acción Atacar sobre cornisa 59 para bajar; exige conservar imán o su combinación                                                 |
| 320B–324B            | Acción Atacar sobre cubo 60; el modelo catorce la rechaza. Su escena natural todavía está pendiente                              |
| 5C2E–5C43, 615C–61A6 | Recoger monitor 61, retirar descriptor y activar animación con destino +5/altura−4                                               |
| A383–A3B7            | Combinar imán 57 y cordel 55 en ambos órdenes: consume ambos y crea 70                                                           |
| A3B8–A405            | Lanzar 70 en escena 13, recuperar alambre 63 si sigue en el agujero y sumar cinco puntos una sola vez                            |
| 38D3–3933            | Examinar agujero 62: acercamiento, indicio y marca visual si aún conserva alambre                                                |

El traslado está en motor/acciones/catapulta.cpp y el despacho en motor/acciones/menu.cpp. Se conservan las operaciones de 16 bits, los bloqueos originales y los cambios de profundidad del tablón. El dibujo se contrasta con EBBD: en la fase preparada Brad puede quedar detrás del tablón como en el original.

## Recorrido por el menú

1. Desde el picapedrero, regresar al segundo corredor, entrar al tercero y bajar por la salida al cuarto.
2. Entrar en la cueva del monitor y recogerlo.
3. Regresar al cuarto corredor. Aproximarse a su salida central por el suelo y subir al tercero.
4. Entrar en SEESAW por la salida superior izquierda. Recoger tablón y combinarlo con roca.
5. Lanzar monitor cerca del apoyo; Brad llega automáticamente a la cornisa.
6. Recoger imán y seleccionar Atacar sobre la cornisa para bajar. Rodear la roca por la derecha para regresar al corredor.
7. Volver a la cueva del monitor. Combinar cordel con imán: para elegir el segundo objeto del inventario, usar primero la casilla de inventario del submenú de destinos.
8. Lanzar imán con cordel. El alambre entra en inventario y el objeto del agujero desaparece.

## Comprobaciones y límites

`pnpm comparar:catapulta --muestra` reproduce la partida desde la mazmorra inicial y añade menús, capacidad, puntuación, respuestas negativas, entradas, pasos persistentes y seis composiciones frente al EXE. `pnpm comprobar:catapulta` añade seis capturas SDL y 30 cambios F5/F6. La comparación intermedia desde una instantánea comprobada cubre 5.934 operaciones y pasos; el informe del recorrido completo concreta su total.

El cuarto corredor, la moneda, el monstruo de barro, el pozo, los cuatro agujeros y el cubo continúan en [el final de las cuevas](final_cuevas.md). Hay quince vistas de desarrollo; el estado de la reconstrucción se concreta por los recorridos y opciones contrastados, sin equiparar ese número con una prueba exhaustiva de todos los estados.

## Número de escenas del original

DS:784E contiene 78 registros de ocho bytes con referencias a MAP y SPR (índices 0–77). Se reutilizan fondos: cuatro corredores CAVECOR, dos CAVE y varias habitaciones de hielo, entre otros; además existen transiciones y variantes. La cifra de 78 corresponde a entradas internas y no a 78 ilustraciones diferentes. No incluye Core y título como habitaciones jugables.
