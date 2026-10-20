# Partidas, paneles originales e importación DOS

El octavo icono abre Cargar, Guardar y Borrar. Los paneles reproducen la placa negra de128×100, la posición96/50, los textos del EXE y FONT.SPR. No hay botones añadidos, lista coloreada, números de ranura ni selector de archivos dentro del panel. Se conservan los rótulos originales de esta edición, en inglés. El décimo icono mantiene sus cuatro fichas originales.

## Uso

Ratón o flechas arriba/abajo eligen entre **ocho ranuras**; clic izquierdo o Intro ejecutan la selección. Las flechas se detienen en los extremos. Cargar una ranura vacía vuelve al juego, como E251/E2CC. Borrar una ocupada actúa al elegirla, sin segunda confirmación. Clic derecho cancela; Escape cancela la selección de ranura.

Guardar abre primero el selector y después el teclado original de trece caracteres. Las flechas recorren sus letras, Insertar pulsa la tecla señalada y el ratón permite elegirla directamente. Espacio y Retroceso editan el nombre; Intro lo guarda. El teclado contiene las tres acciones originales: espacio, borrar y aceptar. Los huecos entre teclas no escriben caracteres. Clic derecho cancela el editor; Escape no lo cierra, conforme a E4A3. No se añade Ctrl+A ni una confirmación de reemplazo. La entrada de texto SDL se convierte a mayúsculas representables por la fuente; sustituye el controlador físico de teclado DOS y no pretende emular todos sus códigos de exploración.

Los paneles pausan la lógica y mantienen la música. El cursor usa MOUSE.SPR y sus coordenadas originales. Los errores de plataforma se registran en consola; aún no están reproducidas todas las pantallas de error de disco DOS ni las diferencias de repetición y temporización de los dispositivos originales.

La importación se conserva como integración de escritorio mediante arrastrar un CURSE.SAV a la ventana. Añade sus partidas en ranuras libres y conserva archivo de origen y partidas existentes. No añade botones al menú original. El perfil usa SDL_GetPrefPath(AlexLlao/CurseReborn); las modificaciones conservan CURSE.SAV.anterior y utilizan un temporal. Las pruebas aíslan el perfil mediante CURSE_REBORN_PARTIDAS.

## Formato reconstruido

| Campo             | Tamaño y significado                                     |
| ----------------- | -------------------------------------------------------- |
| Cabecera          | Un byte: ocho bits de ocupación                          |
| Ranuras           | Ocho registros, con desplazamiento `1 + indice × 10.164` |
| Nombre            | Trece bytes, rellenados con espacios                     |
| Estado            | 10.151 bytes desde DS:2C0A hasta DS:53B0                 |
| Longitud completa | 81.313 bytes                                             |

Se admiten archivos DOS que no contienen las ranuras finales, siempre que todos los registros activos estén completos. La versión anterior del port admitía siete ranuras; sus archivos continúan siendo válidos y se amplían al escribir. E997 recorre ocho descripciones y E89D permite ocho filas, lo que corrige el límite anterior. El formato original no guarda la semilla de azar de CS:78DE: no se inventa ese campo en el archivo compatible. Los nombres antiguos se conservan byte a byte. Para los perfiles creados por el panel SDL anterior, las minúsculas se muestran como mayúsculas y los índices inexistentes en FONT.SPR como interrogaciones; no se modifica el archivo por dibujarlo. El teclado DOS original solo produce las letras mayúsculas de su matriz.

El formato puro está en `apps/juego/motor/estado/partidas.*`, sin acceso al disco. Se reconstruyen E718–E745 (escritura), E253–E27A (lectura) y E824–E84D (borrado). Borrar conserva los datos del registro. Se mantiene la peculiaridad de E831: aplica XOR7F a la máscara; en la octava ranura no borra su bit. Esta diferencia también está comparada contra el EXE y no se corrige silenciosamente en el modo fiel.

## Restauración

El adaptador prepara un estado temporal y comprueba sus recursos antes de sustituir la sesión activa. Conserva los objetos, animaciones, inventario, puntuación y control del personaje guardados. Sigue la rama AL=1 del cargador: **513F–5143 salta a 5185**, por lo que no debe ejecutar el reinicio de 5145 ni recrear entidades mediante 8AA6. E28B–E2B3 adapta los segmentos de los actores y superposiciones a las referencias de los recursos portables; el cursor y Brad reciben sus bancos correspondientes.

Se reconocen las75 escenas reales integradas (0–73 y77). Los índices74–76 son reservas, no habitaciones jugables adicionales. Un archivo inválido o con recursos no disponibles no sustituye la partida viva.

## Comprobaciones

`pnpm comparar:partidas` contrasta **303 casos** frente a las instrucciones originales: ocho ranuras, nombres, máscaras, conservación de otros registros, lecturas completas y límites con centinelas. Con una instantánea real del recorrido genera además `apps/juego/trabajo/comparacion_partidas/original_dos.sav` usando las escrituras del EXE para probar la importación.

`pnpm comprobar:partidas` compila SDL y prueba guardar, volver a una sesión inicial, restaurar inventario y escena, rechazar una zona pendiente sin cambiar la partida, cargar desde el panel, borrado directo e importar la referencia DOS en otra ranura sin sobrescribir la existente. Tras importar se verifica también que Brad conserva el control. El teclado se prueba con clics reales en letras, espacio, borrado y aceptación. El panel final se captura en un BMP local.

`pnpm comparar:panel-partidas` compara **23 composiciones de64.000 píxeles** con E997, E89D y E46D/D87A: cargar, guardar, borrar, las ocho posiciones y el teclado vacío, parcial y completo. Otros cuatro casos comprueban la adaptación visual de nombres del panel anterior: se comparan sus minúsculas con la representación en mayúsculas de la fuente original, sin afirmar que sean entradas DOS idénticas. `pnpm comprobar:partidas` agrupa ambos comparadores y la integración SDL. No equivale a validar todos los estados ni todos los errores de disco.

La compatibilidad comprobada corresponde al EXE con la huella del proyecto y a archivos escritos por sus rutinas. No se ha validado una colección de partidas de otras ediciones ni el recorrido de ida y vuelta con una sesión interactiva DOSBox.

### Continuar jugando después de cargar

`pnpm comprobar:continuidad` compila SDL y compara seis recorridos sin recargas frente a las mismas entradas con recargas intermedias reales. `--sin-compilar` reutiliza expresamente el ejecutable ya compilado; `--solo-laberinto` limita la tanda a las dos puertas alternativas de60 y sus regresos. Los perfiles de prueba y SAV permanecen en una carpeta nueva de `trabajo/`, separados del perfil del usuario.

El comprobador guarda al cambiar escena, puntuación, progreso o inventario, únicamente con menú habilitado y sin camino, tecla ni secuencia activa de Brad. Carga ese registro en la misma sesión y continúa el recorrido. Comprueba escena, cámara, imagen y paleta al cargar y exige la misma imagen final que el control. El azar y CS permanecen vivos: el formato DOS no los guarda.

Ejecutados313 puntos hasta el desenlace en70 escenas, tres regresos laterales con102/102/98 puntos y ambas puertas de60 con270 puntos cada una. En conjunto son1.155 operaciones y73 escenas; los recorridos repiten estados iniciales.2 y68 son secuencias automáticas sin punto de menú. Esto acredita la continuación de estas rutas, sin afirmar todas las animaciones, partidas de otras ediciones ni restauración del azar tras reiniciar el proceso.

## Información del último icono

Un clic izquierdo pasa a la página siguiente; el quinto vuelve al juego. Intro, Intro numérico y Espacio también avanzan, como comodidad de plataforma. Escape, cualquier otra tecla o clic derecho cierran la información directamente. La repetición automática de una tecla no salta páginas. Cada apertura comienza por la primera ficha y registra la página en consola. La consulta no abre ni crea el archivo de partidas.

E160–E19E recorre la tabla DS:B74E: B758, B804, B8BA y B90B. Se conservan los créditos originales, distinguiéndolos de la autoría de Curse Reborn. La puntuación procede de DS:35C7 y el porcentaje se calcula mediante división entera por 478. El formato decimal de tres posiciones sigue 7970–79B7. La composición reconstruye D623 hasta D6A5: placa negra centrada y texto a ocho píxeles por carácter y diez por línea, mediante D87A y la tabla DS:B6B4. No se versionan los textos extraídos ni FONT.SPR.

`pnpm comparar:informacion` compara cuarenta casos con diez puntuaciones entre 0 y 478: números, posiciones y los 64.000 píxeles de cada ficha con las instrucciones originales. `pnpm comprobar:informacion` añade la apertura desde la décima casilla real, navegación, cierres, conservación del estado guardable, ausencia de escritura en el perfil y cuatro capturas SDL en `apps/juego/trabajo/comprobacion_informacion_sdl/`. La adaptación de teclado está descrita arriba; no se afirma identidad de todas las entradas con DOS.

El [tiempo efectivo de juego](tiempo_juego.md) se conserva por separado en el perfil. No añade bytes a CURSE.SAV, no cambia sus menús ni se rebobina al cargar una ranura. Su asociación a futuros estados de guardado propios sigue pendiente.

## Créditos propios del port

Después de las cuatro fichas DOS se añade una quinta, por petición de Àlex: «Port por Àlex Llaó», versión del paquete apps/juego/package.json y fecha real de compilación. La capa SDL usa FONT.SPR y su placa negra, con tildes y separadores propios; no modifica los textos ni las cuatro composiciones originales. La consulta conserva la partida y no crea CURSE.SAV. La versión se incorpora tanto en la compilación Node como en CMake; la fecha procede de **DATE**, formateada como día/mes/año.

El ejecutable Windows incorpora el icono CR propio, en siete tamaños16–256. La ventana SDL utiliza su PNG; la fuente SVG y los dos formatos están en packages/assets/lanzador/icono. El logo de la web se decidirá aparte. comprobar:informacion conserva40 comparaciones DOS y recorre/captura las cinco fichas SDL. Verificados también el icono extraído del EXE y la compilación CMake con Clang y Windows SDK.
