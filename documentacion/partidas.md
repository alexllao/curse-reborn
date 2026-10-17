# Partidas, menús e importación DOS

El octavo icono del menú de acciones abre Cargar, Guardar y Borrar. El décimo y último icono abre las cuatro fichas de información del original, con puntuación, porcentaje y créditos. Todos estos menús pausan la lógica y mantienen la música. Los paneles de partidas son una adaptación de plataforma en castellano; las fichas informativas utilizan los textos y la tipografía originales del EXE y FONT.SPR.

## Uso

Elegir una ranura con ratón o flechas arriba/abajo y confirmar con Intro o Aceptar. Escape y Cancelar devuelven al juego. Guardar permite editar un nombre de hasta trece caracteres ASCII; Ctrl+A limpia y Retroceso borra. Reemplazar o borrar una ranura exige una segunda confirmación. Mantener Intro pulsado no confirma la operación por repetición de teclado.

Desde Cargar, **Importar DOS** permite seleccionar un CURSE.SAV. También se puede arrastrar ese archivo sobre el panel. Sus partidas se añaden en ranuras libres; si no hay suficientes, no se importa ninguna. El archivo de origen y las ranuras que ya existían se conservan. Después de importar hay que elegir la nueva ranura para cargarla.

El archivo del usuario se guarda en la carpeta devuelta por SDL_GetPrefPath para AlexLlao/CurseReborn y su ruta se registra en consola. Cada modificación conserva `CURSE.SAV.anterior` y escribe primero un temporal junto al destino. Las pruebas usan perfiles aislados dentro de `trabajo/`, mediante CURSE_REBORN_PARTIDAS, y no modifican el perfil normal ni los archivos comerciales.

## Formato reconstruido

| Campo             | Tamaño y significado                                      |
| ----------------- | --------------------------------------------------------- |
| Cabecera          | Un byte: siete bits de ocupación                          |
| Ranuras           | Siete registros, con desplazamiento `1 + indice × 10.164` |
| Nombre            | Trece bytes, rellenados con espacios                      |
| Estado            | 10.151 bytes desde DS:2C0A hasta DS:53B0                  |
| Longitud completa | 71.149 bytes                                              |

Se admiten archivos DOS que no contienen las ranuras finales, siempre que todos los registros activos estén completos. El formato original no guarda la semilla de azar de CS:78DE: no se inventa ese campo en el archivo compatible. Los nombres antiguos se conservan byte a byte; los caracteres que no admite la fuente ASCII del primer panel pueden mostrarse como interrogaciones.

El formato puro está en `motor/estado/partidas.*`, sin acceso al disco. Se reconstruyen E718–E745 (escritura), E253–E27A (lectura) y E824–E84D (borrado). Borrar limpia el bit de ocupación y conserva los datos del registro, igual que DOS.

## Restauración

El adaptador prepara un estado temporal y comprueba sus recursos antes de sustituir la sesión activa. Conserva los objetos, animaciones, inventario, puntuación y control del personaje guardados. Sigue la rama AL=1 del cargador: **513F–5143 salta a 5185**, por lo que no debe ejecutar el reinicio de 5145 ni recrear entidades mediante 8AA6. E28B–E2B3 adapta los segmentos de los actores y superposiciones a las referencias de los recursos portables; el cursor y Brad reciben sus bancos correspondientes.

Se reconocen las zonas integradas, incluida la salida al campo COUNTRY1 y la entrada a COUNTRY2; los puzles de esta última siguen pendientes. Una partida de una zona posterior puede importarse y conservarse, pero no se carga hasta implementar esa zona. Un archivo inválido o con recursos no disponibles no sustituye la partida viva.

## Comprobaciones

`pnpm comparar:partidas` contrasta **183 casos** frente a las instrucciones originales: siete ranuras, nombres, máscaras, conservación de otros registros, lecturas completas y límites con centinelas. Con una instantánea real del recorrido genera además `trabajo/comparacion_partidas/original_dos.sav` usando las escrituras del EXE para probar la importación.

`pnpm comprobar:partidas` compila SDL y prueba guardar, volver a una sesión inicial, restaurar inventario y escena, rechazar una zona pendiente sin cambiar la partida, cargar desde el panel, confirmar el borrado e importar la referencia DOS en otra ranura sin sobrescribir la existente. Tras importar se verifica también que Brad conserva el control. El panel final se captura en un BMP local.

La compatibilidad comprobada corresponde al EXE con la huella del proyecto y a archivos escritos por sus rutinas. No se ha validado una colección de partidas de otras ediciones ni el recorrido de ida y vuelta con una sesión interactiva DOSBox.

## Información del último icono

Un clic izquierdo pasa a la página siguiente; el cuarto vuelve al juego. Intro, Intro numérico y Espacio también avanzan, como comodidad de plataforma. Escape, cualquier otra tecla o clic derecho cierran la información directamente. La repetición automática de una tecla no salta páginas. Cada apertura comienza por la primera ficha y registra la página en consola. La consulta no abre ni crea el archivo de partidas.

E160–E19E recorre la tabla DS:B74E: B758, B804, B8BA y B90B. Se conservan los créditos originales, distinguiéndolos de la autoría de Curse Reborn. La puntuación procede de DS:35C7 y el porcentaje se calcula mediante división entera por 478. El formato decimal de tres posiciones sigue 7970–79B7. La composición reconstruye D623 hasta D6A5: placa negra centrada y texto a ocho píxeles por carácter y diez por línea, mediante D87A y la tabla DS:B6B4. No se versionan los textos extraídos ni FONT.SPR.

`pnpm comparar:informacion` compara cuarenta casos con diez puntuaciones entre 0 y 478: números, posiciones y los 64.000 píxeles de cada ficha con las instrucciones originales. `pnpm comprobar:informacion` añade la apertura desde la décima casilla real, navegación, cierres, conservación del estado guardable, ausencia de escritura en el perfil y cuatro capturas SDL en `trabajo/comprobacion_informacion_sdl/`. La adaptación de teclado está descrita arriba; no se afirma identidad de todas las entradas con DOS.
