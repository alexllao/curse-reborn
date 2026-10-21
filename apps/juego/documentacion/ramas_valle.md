# Primeras ramas del valle

## Primeras ramas nativas del valle

`pnpm comparar:ramas-valle --muestra` retoma la instantánea real de POST obtenida después del mago y del guardia. Recorre las salidas60 (POST35→REMOTE34),56 (REMOTE34→CELESTE33) y61 (CELESTE33→POST35), con planificación, colisiones, cámara, pasos, recursos y composición comparados contra el EXE. El sendero rodea la vegetación por los pies145 antes de descender hacia la salida60: mantener izquierda por otra altura también se detiene en el original.

C714–C736 actualiza los umbrales de CELESTE salvo fotograma19; C737–C793 inicia tres canales de REMOTE cuando Brad entra en sus límites originales. Se comparan324 variantes aisladas de fotogramas, canal, coordenadas límite y cámara envolvente. Los modelos REMOTE22 y LOST23 usan el banco5800 independiente, sin solapar escena ni STD.

La partida real presencia la reacción de REMOTE y el rechazo de recoger108 mientras el actor2C4C sigue activo. Después vuelve por CELESTE y recoge111 desde su oferta original, conservándolo en el inventario. Se comparan108 variantes por recogida108/111/112 (324 en total): capacidad0/9/10, modelo20/FF, iconos normal/1F/22, puntos0/FFFF y actor0/1/3. Las variantes no conceden objetos a la partida persistente. Las entradas alternativas55 y71 se comprueban sobre copias como contratos de carga, sin afirmar un recorrido hasta sus umbrales.

Resultado: **5.968 casos/pasos,858 selecciones, cinco estados de menú auditados y ocho composiciones**. Todos los pasos comparan65.536 bytes de DS y el estado aleatorio; las rutinas aisladas sin azar comparan DS completo. La prueba parte de una instantánea contrastada o prepara el comparador anterior si no existe.

**Las escenas32–38 siguen fuera del catálogo SDL.** No están completados todos los puzles del valle: quedan ABD5 (grabación con108), AAA3, interacciones con nuevos objetos, rutas hacia DINO/CLOUD y los demás destinos de CELESTE. Una exploración adicional desde la recogida111 se detiene cerca de global463/pies122 y ofrece nuevas acciones aún pendientes; no acredita el encuentro C794 ni otra salida. Conservar esa limitación y seguir el terreno original, sin asignar posiciones ni retirar actores. El contador, marcos, monitor y sonido común no cambian; decorados ampliados aparcados.

## Auditoría posterior de los enlaces de CELESTE (19 de octubre)

`pnpm comparar:enlaces-valle --muestra` contrasta **13.589 casos/pasos, 616 selecciones, tres estados de menú y seis composiciones**. Retoma el regreso real del barco para el umbral70 de CELESTE33:330/170,450/180,530/190,530/199. El detector original conserva el destino70 hacia DINO32 o elige52 hacia POST35 con un consumo de azar de límite2. No se fuerza la semilla: el primer destino procede del estado del barco; el segundo aparece tras tres pasos reales en POST y el acceso71 a CELESTE. Esperar en CELESTE sin otra fuente de azar no cambia esa decisión.

Se habilitan en el cargador70 desde33,52 desde33 además de20, y71 desde32 además de35. Las tablas originales de destino/entrada permanecen intactas. Ambos destinos regresan realmente por71: DINO por480/190,480/199; POST por750/145,750/110,720/100,700/95,685/90. También se contrasta el acceso inferior55 a REMOTE y su regreso56, antes comprobado solo como carga aislada. Cada paso compara DS completo, azar y orden de dibujo; las seis imágenes coinciden con el EXE.

Este bloque completa esos enlaces del núcleo; no incrementa aún las32 vistas SDL ni acredita todos los estados futuros de estas salas. Se mantienen los puzles ya documentados del valle y el pendiente de integración SDL. La lista inicial de pendientes de este documento es histórica; consultar las ampliaciones de AGENTS.md y los documentos de nube, barco y plataforma para el estado posterior.

## REMOTE en SDL

`pnpm comprobar:remoto` utiliza las referencias comunes preparadas con `pnpm comprobar:poste` y repite el contraste nativo de ramas-valle. Reproduce desde el comienzo del juego los mismos clics hastaREMOTE y compara la entrada, la reacción y el rechazo del mando: tres capturas de64000 píxeles. Comprueba guardar/restaurar cada estado y68 cambios deF5/F6 entre34 vistas. La prueba SDL todavía no incluye el retorno56 ni la recogida108 después del estornudo; sus contratos nativos ya están documentados. POST yREMOTE son las primeras dos salas del valle activadas enSDL.

## CELESTE en SDL

`pnpm comprobar:celeste` prolonga la misma partida por56 y61 y recoge111 al volver aPOST. Compara tres imágenes originales y guardar/restaurar;70 cambios deF5/F6 entre35 vistas. CELESTE incorpora55/61 y los accesos56/71; se admite la alternativa52 haciaPOST. DINO yHOLD aún no están activos enSDL en esta revisión. Las dos decisiones aleatorias y los regresos completos siguen contrastados en el núcleo por enlaces-valle, sin presentarlos todavía como comprobados enSDL.

## Regreso posterior con grabación y correo enviado

`pnpm comparar:regreso-remote` retoma106 y los consumos reales de correo, cierra60/56/61 con1.360 pasos y conserva el inventario. Espera el control antes de enviar cada clic durante las reacciones del personaje; no anula animaciones ni cambia el motor. Tres referencias nuevas con DS/semilla/contexto5800/fase0. Véase el bloque del21 de octubre en [la auditoría](auditoria_pantallas.md).
