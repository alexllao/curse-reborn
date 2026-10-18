# El borde del mundo: rutina de escena

Esta tanda traslada **C093–C4DB** al núcleo C++, con carga de entidades de EDGE y servicios comunes. La llegada SDL a EDGE30 continúa detenida, sin incorporarla todavía a F5/F6 ni contar tiempo jugado en ella. No representa una nueva pantalla jugable completa.

## Contratos reconstruidos

- El modelo20 y fotograma90 preparan los canales B0ED/B0F6/B0FD.
- Los obstáculos usan distancias y contactos originales antes de saltar al rescate C4DC, ya compartido con el comportamiento0411. El pájaro ajusta su altura, asocia la posición de Brad a su tabla y devuelve sus canales al alcanzar el umbral175.
- Las dos franjas de proximidad conservan sus condiciones y coordenadas de C1D7/C3D7.
- El temporizador ADCE reduce, limita a180 y activa DEF en las ventanas21–40; la orden de sonido6 pasa por el servicio común. Las ráfagas crean tres entidades y mantienen cuatro consumos explícitos de azar por intento, además de los que requiere8B6F.
- El objeto que cae conserva sus canales y la rama que incorpora el identificador94, incluso con inventario lleno; la animación no se sustituye por un objeto entregado artificialmente.
- La creación final conserva probabilidad, descriptor, coordenada y desplazamiento aleatorio originales. El agotamiento de registros no evita las escrituras previas ni altera los consumos.

## Comprobación ejecutada

`pnpm comparar:borde --muestra` compara **1.244 casos**, los65.536 bytes de DS, las cuatro palabras del generador y **20 peticiones sonoras** con las instrucciones del EXE. Incluye carga8AA6 de EDGE, llegada, activación del modelo, rescate, contactos de ambos obstáculos, temporizador0–255, ventanas20/21/40/41, registros agotados, inventario lleno, proximidad y fronteras con signo.

El banco EDGE.SPR rebasa el espacio que los comparadores anteriores reservaban antes de SMALL.BRD. Esta comparación aislada dispone solo el banco de escena real y comprueba que cabe en su reserva: C093 no lee el modelo ni dibuja. No se oculta el solapamiento para afirmar que el bucle o la imagen ya están contrastados. Antes de comparar el recorrido y su composición habrá que disponer ambos recursos en regiones independientes y conservar las referencias originales de las entidades.

Los resultados locales quedan en trabajo/comparacion_borde/resultado.json. El código está en motor/entidades/borde.cpp y las anotaciones del EXE se conservan en mapa_ejecutable.json.

## Pendiente antes de activar la escena

Contrastar la secuencia de entrada y el movimiento completo, composición original, rescates persistentes, recogida y uso de los guantes, roca y salto, botones1–3–4–3–2, puente, cuerda, personaje eléctrico, tramos de rocas, caramelo y piedra, abismo y palabras de apertura. Comprobar después los menús ofrecidos y el enlace con la habitación del loro. Seguir la guía local y cada manejador original; no introducir requisitos que no aparezcan en el EXE.

Esta comparación no prueba cadencia, audio físico, SDL, todas las posiciones o todos los estados. Los marcos y el monitor se conservan; los decorados ampliados permanecen aparcados.

## Entrada y composición con bancos separados

El comparador completo conserva EDGE.SPR en4000, sitúa el modelo sustituible en5800 y mantiene STD en6400. Las regiones no se solapan; se verifican los bytes originales después de disponerlas. La referencia del modelo se comparte entre entrada, secuencias, cargaF8, consulta de dimensiones y composición. Los puentes anteriores mantienen su ABI y5400 como valor predeterminado. No se alteran instrucciones ni se normalizan resultados para esconder direcciones distintas.

`pnpm comparar:recorrido-borde --muestra` retoma el transporte real del mago y contrasta730 casos/pasos, incluida entrada47, carga de entidades y256 pasos de EDGE. Cinco composiciones coinciden con EBBD usando simultáneamente los bancos completos. La secuencia termina con el control habilitado para Brad pequeño.

`pnpm comprobar:entrada-borde` prepara las referencias y compara la composición de llegada SDL con el EXE. Se ha comprobado esa captura de64.000 píxeles, reutilizando el ejecutable ya compilado y completando secuencialmente las fases después de un bloqueo de recompilación por una biblioteca en uso. La regresión del probador mantiene sus3.025 casos/pasos y dos composiciones. No ejecutar dos comparaciones que recompilen tabla_acciones.dll al mismo tiempo en Windows.

La llegada SDL sigue detenida. Estas pruebas no resuelven las interacciones ni acreditan el recorrido completo del acantilado. Los resultados nuevos quedan en trabajo/comparacion_recorrido_borde/ y trabajo/comprobacion_entrada_borde_sdl/.

## Primer sendero y contratos de acciones

El recorrido completo amplía ahora su prueba a **1.142 casos/pasos y seis composiciones**, incluida una aproximación real a la primera roca en coordenada global441. La secuencia de teclas sigue el paso estrecho sin modificar posiciones, inventario ni banderas. Se sueltan las flechas antes de abrir el menú, como en la partida normal; el original bloquea recoger/manipular mientras Brad mantiene su secuencia de caminar.

`pnpm comparar:acciones-borde --muestra` contrasta **3.318 casos/pasos**, **2.543 selecciones**, **dos estados de menú auditados** y **dos composiciones**. Desde la aproximación real, empuja la roca con la opción original, espera su caída y salta sobre ella. Coinciden DS completo, estado aleatorio y las dos imágenes posteriores; la roca pasa de altura76 a95 y Brad llega al otro lado con el mismo control y puntuación que el EXE.

Las variantes separadas incluyen108 recogidas (guantes86, caramelo87 y cuerda95),88 usos/llevadas de guantes,44 llevadas de caramelo,2.048 pulsaciones de botón y12 casos para cada verbo de la roca. Se revisan capacidad, iconos, modelo pequeño/normal, puntuación envolvente, distancias y todos los bits de35C9. Cada botón invierte su bit; la condición del puente usa el patrón B. Esto no sustituye todavía el recorrido real hasta los botones.

El código de acciones está en motor/acciones/borde.cpp. Conserva los mismos servicios de sonido y planificación que las demás escenas. No se entrega ningún objeto en la partida por haber pasado una variante aislada.

Quedan pendientes la continuación real por los obstáculos temporizados, las recogidas del recorrido y la resolución persistente del puente y del resto del acantilado. La cuerda, lanzamiento de piedra, salto del abismo e inscripción devuelven pendiente explícitamente cuando no están trasladados. EDGE sigue detenida en SDL; estos contratos no se presentan como una pantalla completa.

## Obstáculo, rescate y puente desde la partida real

`pnpm comparar:acciones-borde --muestra` amplía el contraste a **6.537 casos/pasos y cinco composiciones**. Después del salto inicial se comprueban dos ramas independientes. La primera entra en contacto con el obstáculo y el pájaro devuelve a Brad al umbral global175. La segunda se detiene en585 mientras cae el obstáculo, continúa por el suelo original, recoge los guantes86 y pulsa1–3–4–3–2. Cada selección usa las listas reales y cada paso contrasta DS completo y azar con el EXE; no se escriben coordenadas ni banderas para franquear el sendero. El puente termina activo con fotograma47 y bits de combinación58h.

Se auditan además los menús al alcanzar los guantes, antes de los botones y después de desplegar el puente. Las cinco capturas incluyen la roca empujada, el salto, el rescate, los guantes recogidos y el puente. Es un recorrido parcial reproducible, no una garantía de todos los estados posibles. No cambia la parada deliberada de EDGE en SDL: siguen pendientes cuerda, personaje eléctrico y el resto del acantilado.

## Recorrido de EDGE hasta la salida72

La reconstrucción de acciones incorpora A599–A7BB (piedra y cinco fragmentos), A7BC–A819 (lanzar cuerda),3280–32BE (salto),3AFD–3B3D (examinar inscripción) y3809–3848 (palabras de apertura). Conserva retiradas, puntuación, canales, demoras relativas de886B y sonido19 por el servicio común. El servicio de creación compartido conserva los consumos de azar y el registro3510 cuando no quedan huecos. Las esperas de los cinco fragmentos deben asignarse en orden inverso de creación, como los POP del EXE: varios fallos pueden devolver el mismo3510 y el orden cambia el resultado.

`pnpm comparar:acciones-borde --muestra` contrasta ahora **12.229 casos/pasos,4.476 selecciones,14 estados de menú auditados y15 composiciones**. El recorrido persistente parte de la aproximación real a la primera roca y llega a solicitar salida72: roca, espera del obstáculo, guantes, puente, cuerda, personaje eléctrico, refugio del caramelo, piedra adherida, roca grande, cuerda del abismo, salto, segundo obstáculo, inscripción, palabras y puerta. Una rama separada verifica el rescate del pájaro; después restaura la semilla y los recursos de la rama principal. Las fases de ADCE se esperan con pasos reales, sin asignar el temporizador ni eliminar rocas artificialmente.

Las variantes nuevas revisan40 distancias/cámaras/puntuaciones por lanzamiento94/95 y salto96,16 estados de agotamiento/azar para los fragmentos y ocho casos de inscripción y ocho de palabras. Son límites cercanos a los puntos utilizables; no garantizan el planificador DOS con coordenadas ajenas al mundo, como una distancia8000h. La comparación sigue los65.536 bytes de DS y las cuatro palabras de azar; las imágenes se componen con los bancos originales independientes.

**EDGE sigue detenida en SDL y fuera del catálogo F5/F6 hasta contrastar allí el mismo recorrido.** La prueba llega a la solicitud72, pero no carga ni acredita todavía la habitación del loro31. El siguiente bloque debe activar y comprobar EDGE en SDL y trasladar ese destino antes de presentar la continuidad como integrada. No cambiar el contador de tiempo ni declarar jugabilidad SDL completa por este contraste del núcleo. Los decorados ampliados permanecen aparcados y los marcos/monitor no cambian.

## EDGE y primera visita al loro integradas en SDL

Esta ampliación sustituye la parada anterior de EDGE30. F5/F6 añade EDGE30 por47 desde22 y WISE31 por72 desde30:32 vistas de desarrollo. Las salidas30→31 por72 y31→27 por73 están habilitadas; MAGE22→31 por72 sigue pendiente de contrastar como rama propia. El cargador admite72/73 expresamente, sin habilitar52–71 ni otros orígenes de esos índices. Guardar/cargar admite esos índices y31. El tiempo efectivo ya incluye EDGE y WISE, como las demás escenas jugables.

361A–3727 traslada el saludo de WISE31 en acciones/poblacion.cpp. Primera visita OR1 en35C9 y41EC/41F5=3; conserva siete u ocho actores diferidos y canales de Brad, sin simplificar los intervalos. CS:17A3[31]=0, por lo que actualizar_rutina_escena incluye31 como no-op; WISE.SPR/MAP se identifica por la tabla784E, no por un nombre inventado PARROT. comparar:loro contrasta1.484 casos/pasos,442 selecciones,256 variantes de banderas, dos estados auditados y seis composiciones, incluida entrada72, saludo inicial, solicitud73 y regreso real aDERELICT27. No conceder objetos ni transformar a Brad solo porque lo diga la guía: manda el EXE comprobado.

comprobar:borde recorre desde la partida inicial hasta EDGE y el loro con entradas reales, usando las referencias de población y las comparaciones recorrido-borde/acciones-borde/loro. Compara16 capturas SDL de64.000 píxeles, las ranuras DOS en puente/loro/regreso y64 cambios F5/F6. La captura de salida de EDGE se compara después de la carga31, no contra el estado del EXE que todavía solicita72. La rama alternativa de rescate sigue contrastada en el núcleo; no se incluye entre esas16 capturas SDL. Cada fotograma de cámara/fundido se respeta sin consumir pasos lógicos adicionales. Las rutinas de sonido, marcos y monitor continúan compartidas. Los decorados ampliados siguen aparcados.

La ruta y la primera visita están comprobadas; visitas posteriores, el enlace alternativo del mago72 y los destinos GYPSY52/CHANGE74 no quedan completos por el número de vistas. Próximo bloque: contrastar la continuación de población tras el regreso del loro y esos accesos posteriores. La guía propia está en documentacion/recorridos/12_borde_del_mundo.txt.

La auditoría estática de78 tablas muestra ahora cuatro enlaces pendientes desde vistas disponibles: GYPSY20→35 por52, MAGE22→31 por72, CHANGE25→43 por74 y WISE31→64 por112 (capítulo posterior). Estos últimos destinos no quedan resueltos por validar la primera visita al loro.
