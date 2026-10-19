# El dinosaurio y el monstruo del valle

## DINO: pelo, estornudo, sello y bolígrafo

`pnpm comparar:dinosaurio --muestra` continúa desde el regreso real de CELESTE a POST. Recoge el pelo110 acercándose con clics550/100 y550/140; conserva sus canales615C y la entrada persistente. Rodea la vegetación por los pies140–145 y baja hasta la salida57 por850/199. No intercambiar esa ruta por una asignación de coordenadas.

En DINO32, el recorrido se acerca por250/170 y340/140, inserta el pelo110 en el monstruo102 y espera256 pasos. El estornudo retira el pelo y cambia su actor al fotograma29, conservando también la posición y las entidades invisibles que deja el EXE. No forzar el actor2C8E a cero: el original conserva tipo1 con ese fotograma. Se recogen el sello109 y el bolígrafo101 y se regresa a POST por58, bajando hasta250/199. El otro acceso54 se contrasta desde la instantánea independiente de CELESTE, sin heredar los objetos de la primera rama.

La rutinaC5E0–C713 conserva umbrales, actores, puerta e inventarios. Se comparan1.152 variantes aisladas,54 de saludo372A y144 de A81A (ambos órdenes, distancias/cámara/puntos y operandos32767/32768/FFFF). Las comparaciones después deSUB conservan el desbordamiento firmado de x86: comparar sus operandos firmados evita confundirlo con mirar solo el signo de la resta envuelta. Las recogidas99/100/101/109/110 tienen108 variantes cada una (540 en total);99 y100 no se conceden a la partida real. La recogida99 conserva5314=90.

Resultado: **8.587 casos/pasos,1.596 selecciones, seis estados de menú auditados y seis composiciones**, comparando DS completo y azar en los pasos reales. Además se vuelve a ejecutar comparar:ramas-valle para comprobar que las recogidas108/111/112 y las primeras rutas siguen coincidiendo.

**DINO y el valle siguen fuera de SDL/F5/F6.** No están reconstruidos todos sus puzles: faltan los intercambios105/106→98, combinar111 con109 (AA6C), insertar114 con112 (AAA3), combinar100 con107 (A952), grabaciónABD5 y destinos64/68/53/59/71 de DINO, además de las demás ramas de CELESTE/POST. Estas combinaciones se declaran pendientes expresamente, sin sustituirlas por rechazos genéricos. Las variantes aisladas de puerta no acreditan haber obtenido sus cuatro objetos argumentales. No aumentar la cifra de pantallas jugables ni activar destinos solo porque ya carguen sus archivos.

Una exploración del regreso hacia60 después de recoger el pelo no completó la transición en la prueba corta, aunque todos sus pasos coincidían; la continuación validada usa57. Conservar la incidencia como investigación, sin afirmar una barrera permanente del juego. Los decorados ampliados permanecen aparcados; contador, sonido, marcos y monitor no cambian.

## Integración SDL

`pnpm comprobar:dinosaurio` prepara la referencia nativa8587 casos/pasos desde las instantáneas comunes de comprobar:poste y comprobar:celeste. Reproduce desde el inicio las dos ramas de entrada57/54, pelo110, estornudo, sello109/bolígrafo101 y regreso58. Seis imágenes de64000 píxeles coinciden; guardar/restaurar cada hito y72 cambios deF5/F6 entre36 vistas correctos. La recogida del mando tras el estornudo y el resto del valle están contrastados en el núcleo y esperan prolongación de las pruebasSDL. No equivale a agotar todos los estados argumentales deDINO.
