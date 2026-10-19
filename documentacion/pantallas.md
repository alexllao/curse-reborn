# Nombres de las pantallas

Nombres descriptivos propios del port, en castellano; no se presentan como títulos oficiales del juego original. Se comparten en la consola, los cambios F5/F6 y el título de la ventana, también al atravesar una salida normal o cargar una partida.

El número de vista sigue el orden de F5/F6. El índice de escena es el identificador estable del ejecutable DOS y se conserva para las partidas y las futuras pistas. Core, título e introducción quedan fuera de las52 vistas de desarrollo.

| Vista | Escena DOS | Nombre                           | Recurso  |
| ----- | ---------- | -------------------------------- | -------- |
| 1     | 0          | La mazmorra                      | DUNGEON  |
| 2     | 1          | El corredor de las estatuas      | CORRIDOR |
| 3     | 2          | La caída al lago                 | DROP     |
| 4     | 3          | El fondo del lago                | LAKEBED  |
| 5     | 8          | La poza de las cuevas            | POOL     |
| 6     | 4          | Primer corredor de las cuevas    | CAVECOR  |
| 7     | 5          | Segundo corredor de las cuevas   | CAVECOR  |
| 8     | 6          | Tercer corredor de las cuevas    | CAVECOR  |
| 9     | 9          | El pozo de los deseos            | WELL     |
| 10    | 10         | La cueva del picapedrero         | CAVERBMI |
| 11    | 7          | Cuarto corredor de las cuevas    | CAVECOR  |
| 12    | 11         | La cueva de la catapulta         | SEESAW   |
| 13    | 13         | La cueva del monitor             | CAVE     |
| 14    | 14         | La cueva de los cuatro agujeros  | CAVE     |
| 15    | 12         | El pozo del cubo                 | SHAFT    |
| 16    | 15         | La salida al campo               | COUNTRY1 |
| 17    | 16         | El camino del asaltante          | COUNTRY2 |
| 18    | 21         | La entrada de Paso del Dragón    | DRAGON   |
| 19    | 29         | La tienda de Frank               | FRANKS   |
| 20    | 17         | La plaza de la fuente            | XROADS   |
| 21    | 18         | La calle de las maravillas       | SORCERY  |
| 22    | 22         | La tienda del mago               | MAGE     |
| 23    | 19         | La calle de los disfraces        | ROGUE    |
| 24    | 20         | La calle de la adivina           | GYPSY    |
| 25    | 23         | La tienda abandonada             | DERELICT |
| 26    | 28         | La posada Red Dragon             | INNSIDE  |
| 27    | 24         | La tienda de disfraces de Benn   | BENN     |
| 28    | 26         | La tienda de la adivina          | SALLY    |
| 29    | 27         | La casa abandonada               | DERELICT |
| 30    | 25         | El probador de Benn              | CHANGE   |
| 31    | 30         | El borde del mundo               | EDGE     |
| 32    | 31         | La habitación del loro sabio     | WISE     |
| 33    | 35         | La encrucijada del valle         | POST     |
| 34    | 34         | El mando del valle               | REMOTE   |
| 35    | 33         | El barco del valle               | CELESTE  |
| 36    | 32         | El valle del dinosaurio          | DINO     |
| 37    | 38         | La nube del valle                | CLOUD    |
| 38    | 37         | La cueva del detergente          | GHETTO   |
| 39    | 36         | El interior del barco            | HOLD     |
| 40    | 39         | La plataforma eléctrica          | ELECTRO  |
| 41    | 43         | La llanura del ser azul          | SNOWAST4 |
| 42    | 42         | La morsa y el amigo del hielo    | SNOWAST3 |
| 43    | 41         | El agujero del pez y el esquimal | SNOWAST2 |
| 44    | 40         | La cornisa de los amigos         | SNOWAST1 |
| 45    | 44         | El lago helado                   | ICELAKE  |
| 46    | 45         | La entrada del palacio de hielo  | ICEPAL   |
| 47    | 46         | La sala de los dados             | ICEROOM  |
| 48    | 47         | El corredor de las poleas        | ICECORR  |
| 49    | 48         | El segundo corredor de hielo     | ICECORR  |
| 50    | 49         | El corredor de la ventana        | ICECORR  |
| 51    | 50         | La habitación del gato mecánico  | ICEROOM  |
| 52    | 51         | La habitación de la lupa         | ICEROOM  |

Los cuatro corredores de las cuevas son escenas distintas aunque comparten CAVECOR. Las cuevas del monitor y de los cuatro agujeros comparten CAVE, pero conservan nombres e índices independientes. Un recurso reutilizado no determina el nombre de una escena.

Las doce vistas de la población cubren el trayecto de comida y transporte del mago, las dos calles de la fuente, la tienda abandonada y la posada con sus regresos. Benn, la adivina y la segunda casa abandonada ya tienen entrada y regreso. El probador de Benn ya permite entrar y regresar; las continuaciones posteriores siguen pendientes. El número de vistas no representa pantallas completadas al 100 %. Véanse el [alcance de la población](puzles_poblacion.md) y la [auditoría de rutas](auditoria_pantallas.md).

EDGE ya tiene recorrido contrastado hasta abrir la puerta y entrar en WISE31. El saludo inicial del loro y el regreso73 a DERELICT27 están reconstruidos. Ambas escenas se ofrecen en F5/F6; estos atajos no conservan el inventario ni representan todos los capítulos posteriores. Véase [el alcance de EDGE y WISE](borde_del_mundo.md).

El catálogo está en `plataformas/sdl/vistas.hpp`. Al añadir una escena se incorpora allí su nombre y se actualiza esta tabla; los nombres no se utilizan como claves de partidas. El futuro sistema de pistas podrá referirse al índice de escena sin depender del texto visible.

Cuando exista el HUD propio, mostrará brevemente el nombre de la pantalla cada vez que se acceda a ella, también al regresar a una ya visitada. Reutilizará este catálogo. La duración y el aspecto quedan pendientes de decidir; este aviso todavía no está implementado.

El título de la ventana incluye el número de vista, por ejemplo «Curse Reborn — 1: La mazmorra» y «Curse Reborn — 4: El fondo del lago». Se actualiza al entrar, volver y cargar una partida; los números DOS del archivo de guardado no cambian.

POST35 se incorpora por la salida52 real de la calle de la adivina, después del segundo pago al mago y del guardia. El acceso F5/F6 conserva su papel de sesión nueva de desarrollo. Sus puzles del núcleo ya están documentados; los destinos del valle se activarán en SDL conforme se contrasten sus adaptadores. No confundir esta entrada con la integración SDL de las ocho escenas del valle.

REMOTE34 permite entrar por60 desdePOST. Se contrasta en SDL la llegada con el modelo22, su reacción y el rechazo original de recoger108 mientras su protector permanece activo. La recogida tras el estornudo deDINO y su uso posterior están contrastados en el núcleo; esa continuidad SDL se añadirá con las salas siguientes. F5/F6 inicia una visita nueva y no elimina al protector.

CELESTE33 enlaza conREMOTE por55/56 y conPOST por61/71; el destino52 de su umbral aleatorio también está catalogado. Las otras rutas se activan con sus salas. Se contrasta la llegada56, el regreso61 y la recogida111 después de volver, con restauración y navegaciónF5/F6. Los enlaces haciaDINO yHOLD todavía no se presentan como integrados enSDL.

DINO32 incorpora las entradas57 desdePOST,54/70 desdeCELESTE y los regresos58/53/71; su camino59 también enlaza conREMOTE. El núcleo permite todos esos enlaces. La comprobaciónSDL reproduce el pelo110, su inserción en102, el estornudo, sello109 y bolígrafo101, el regreso58 y una segunda entrada54 desde el barco. El interiorGHETTO37 y la plataformaELECTRO39 esperan todavía su adaptadorSDL.

CLOUD38 conserva el movimiento del modeloLOST, el balanceo, la recogida121 y la salida inferior67. El acceso66 desdePOST exige el pase original. El acceso directoF5/F6 sigue siendo una sesión de desarrollo que no fabrica ese pase. Las pruebas de continuidad incluyen la grabación106 y el correo antes de solicitar la entrada66.

GHETTO37 enlaza64/65 conDINO. La petición original de ayuda provoca el derrumbe y permite recoger120; se conserva la bolsa121 obtenida enCLOUD. El catálogo de desarrollo permite entrar sin resolver las visitas anteriores y mantiene todos sus requisitos originales.

HOLD36 incorpora la entrada62 después de apartar al guardia, los dos tablones117/118, su colocación para construir el puente, el trapo119 y la salida63. Llevar un tablón utiliza el modeloFE del banco de la escena. El accesoF5/F6 no derrota al guardia de la partida anterior: abre una sesión independiente.

ELECTRO39 conserva el acceso68 con los cuatro objetos reales, la limpieza, el botón, el cruce con canicas y bandeja, el ventilador124 y la salida69 al campo. Las caídas a ambos lados recuperan al personaje dentro de la misma escena. Las cuarenta vistas cubren0–39 del EXE, con orden de desarrollo propio; no acreditan todos los estados posteriores de las salas reutilizadas.

SNOWAST4/43, vista41: entrada74 desde el vestido real, bola134, lanzamiento y transformación. Salida80 hacia42 pendiente hasta integrar su destino. comprobar:ser-azul compara cuatro imágenes y guardar/restaurar; el núcleo incluye variantes y auditorías de sus opciones.

La vista42 incorpora SNOWAST3 y los enlaces80/79 con43. Seis capturasSDL verifican desodorante y saludo; el recorrido nativo continúa por las zonas41/40 hasta cenizas y rescate. Las conexiones78/77 hacia41 se activan con la siguiente vista.

La vista43 activa78/77 entre42 y41. Su entrada perfumada coincide enSDL y restaura la partida; los puzles del tablón, pez y caña ya se contrastan en el núcleo y se comprueban enSDL al habilitar40, donde está el tablón.

La vista44 habilita76/75 entre41 y40 y completa enSDL los puzles de las cuatro zonas nevadas hasta pedir81. catorce capturas de tablón, pez, entrega, caña, hielo, cenizas y cornisa coinciden; MAP1 ySPR3 se seleccionan por la tabla original784E. La llegada a44/ICELAKE sigue pendiente.

La vista45 habilita81 desde la cornisa, el salto135 y el transporte mediante131. Seis imágenesSDL y restauración coinciden;82 hacia el palacio permanece como continuación pendiente.

La vista46 habilita82, reproduce los cuatro carámbanos y abre83 según el original. Ocho imágenesSDL incluyen error y aciertos; el interior de83 permanece pendiente de integración.

La vista47 incorpora83, escoba141, dados140 y su puerta con dos tiradas persistentes contrastadas. Nueve imágenesSDL; los corredores después de84 siguen como siguiente ampliación.

Vista48: entrada84, inspección143 y regreso85 comprobados enSDL; los laterales del palacio siguen pendientes, véase corredores_palacio.md.

Vista49: segundo corredor, enlaces86/87 y regreso persistente a las poleas. Las marcas de las puertas continúan siendo las elegidas por los dados originales.

Vista50: tercer corredor con inspección146, salto rechazado mientras esté cerrado y regreso89. El catálogo llega a50 vistasSDL; las habitaciones laterales y los útiles del puzle posterior continúan pendientes.

Vista51: sala del gato149, entrada91 elegida por dados y regreso92. Se comprueban recogida, inventario y restauración; los demás útiles del palacio continúan por habitaciones independientes.

Vista52: lupa150, entrada93 y regreso94 con inventario persistente. Se conservan las tiradas de los dados y se revisan104 cambiosF5/F6. Las otras cuatro habitaciones laterales siguen pendientes.
