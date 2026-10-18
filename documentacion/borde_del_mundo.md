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
