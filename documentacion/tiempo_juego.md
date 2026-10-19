# Tiempo efectivo de juego

El contador es una estadística propia del perfil del usuario, separada del motor DOS y de CURSE.SAV. Permite conocer el tiempo acumulado aunque todavía no haya nuevos estados de guardado propios. La consola muestra el acumulado al abrir el juego y las duraciones de sesión y perfil al cerrar; no se añade un HUD ni se modifica la ficha original de información.

## Qué cuenta

- Estar en una escena jugable con la ventana visible y enfocada.
- Caminar, pensar la solución, usar el inventario y escoger acciones en el menú original.
- Animaciones, respuestas de personajes y transiciones que forman parte de la partida.

No cuenta Core, título ni introducción, los paneles de guardar/cargar/borrar e información, segundo plano, minimizado ni el apagado/arranque del ordenador virtual. EDGE30 y WISE31 ya cuentan como juego activo después de integrar sus recorridos. No se exige mover continuamente el ratón: pensar también forma parte del juego.

## Cómo se conserva

`apps/juego/plataformas/sdl/tiempo_juego.*` usa nanosegundos monotónicos de SDL, sin depender de los FPS o del reloj emulado. El archivo `tiempo_juego.json` vive en el mismo perfil SDL que las partidas, bajo `AlexLlao/CurseReborn`, y contiene la versión del formato y `milisegundos_jugados`. La variable `CURSE_REBORN_TIEMPO` permite elegir un archivo aislado para pruebas.

Se escribe cada treinta segundos, al pausar el contador y al cerrar, mediante un temporal en la misma carpeta y sustitución del archivo. Un cierre forzado puede perder lo acumulado desde la última escritura. Los errores de escritura se registran por consola y el tiempo sigue disponible en memoria; los archivos corruptos o de formato desconocido se conservan sin sobrescribirlos.

Para evitar contabilizar suspensión del equipo o una interrupción del proceso, se descartan intervalos entre muestras superiores a cinco segundos. Al cambiar de activo a pausado o viceversa se omite el intervalo de esa muestra: la precisión en esas fronteras es la de un fotograma de presentación. La persistencia redondea hacia abajo a milisegundos; las horas de la consola no vuelven a cero al superar veinticuatro horas.

## Alcance para futuras partidas propias

`milisegundos_totales()` y `milisegundos_sesion()` permiten tomar una instantánea de las estadísticas para futuros metadatos. El acumulado actual pertenece al perfil completo: no es la duración de una aventura ni de cada ranura DOS. Empezar otra sesión, recorrer vistas F5/F6 o cargar una partida antigua no borra las horas ya registradas. Las partidas importadas no aportan horas históricas desconocidas. La asociación de tiempo a nuevos estados de guardado queda pendiente.

## Comprobación

`pnpm comprobar:tiempo` usa relojes simulados y archivos exclusivos de `trabajo/comprobacion_tiempo/`. Contrasta actividad y exclusiones, pausas/reanudación, suspensión, guardado periódico, recuperación entre ejecuciones, perfiles inválidos, conservación de un CURSE.SAV de prueba y diez segundos equivalentes a30/60/144 FPS. No espera a que transcurra tiempo real ni modifica el perfil del usuario.

Las demás comprobaciones automáticas omiten la apertura del contador real para no inflar sus estadísticas.
