# Auditoría de rutas y pantallas

Una vista F5/F6 no equivale a una habitación terminada. La auditoría distingue enlaces entre habitaciones, interacciones ofrecidas por el menú, variantes de progreso y capítulos posteriores.

## Revisión integral del 20–21 de octubre

Àlex solicita continuar los pendientes y recibir un balance cada hora. Los porcentajes se separan para evitar confundir accesibilidad y revisión de comportamiento:

| Indicador                                                       | Cobertura comprobada | Porcentaje | Límite                                                      |
| --------------------------------------------------------------- | -------------------- | ---------- | ----------------------------------------------------------- |
| Escenas accesibles en SDL                                       | 75/75                | 100 %      | No acredita todas sus variantes.                            |
| Escenas con continuación tras cargar                            | 73/73 con menú       | 100 %      | Reposo, rutas y estados citados;2/68 automáticas excluidas. |
| Escenas con muestras de consecuencias en esta revisión ampliada | 35/73 con menú       | 47,9 %     | Muestras, no todos los estados ni fidelidad1:1 completa.    |

El último indicador cuenta las escenas0–35 salvo2, que es automática. Es una medida de la revisión adicional, no del porcentaje de aventura implementada. Los comparadores anteriores de cada puzle siguen siendo evidencias complementarias y no se suman como escenas nuevas de esta tanda.

Se ha ejecutado el recorrido SDL desde la liberación de Brad hasta THE END, sobre la misma partida y usando las entradas del jugador. Las restauraciones de comprobación usan una segunda instancia; no sustituyen el estado del recorrido principal. El resultado final coincide píxel a píxel con la composición contrastada del EXE. La prueba acelera las esperas de presentación: no sirve para medir cuánto tarda un jugador.

El recorrido principal pasa por 72 escenas, incluida la mazmorra inicial. Se han ejecutado además tres ramas con regreso: tienda abandonada23 por33/45, adivina26 por38/48 y posada28 por41/50. Sus imágenes de regreso coinciden con las referencias contrastadas. Entre el recorrido y estas ramas se visitan las75 escenas reales; esto acredita visitas, no todas las combinaciones posibles de acciones.

`pnpm comparar:opciones --todas` amplía la auditoría a las instantáneas de todas las carpetas locales de comparación. Conserva el modo anterior sin `--todas`, separa el azar de cada rama y restaura sus recursos para que una alternativa no contamine otra. Deduplica memoria y semilla iguales y publica el detalle por escena en `apps/juego/trabajo/auditoria_opciones/resultado.json`.

Resultado ejecutado: **59.470 selecciones coincidentes, 429 estados distintos y 73 escenas con menú**, sin manejadores pendientes en las listas recorridas. Las escenas2 (caída) y68 (puente) son secuencias automáticas sin estado de menú auditable. Se descartaron131 instantáneas sin menú habilitado o fuera del catálogo. De los429 estados,392 conservan su semilla guardada;37 referencias antiguas se comparan con la semilla inicial del comparador, sin afirmar que reproducen su azar histórico.

La comprobación del vestíbulo69 se amplía con tres clics centrales (170/175/180,131): en cada rama se contrasta la caída original54F0–5638, la recuperación del control y la continuación hasta solicitar120 hacia el estudio. El bloque completo suma3.909 casos/pasos, incluidas444 selecciones, cuatro composiciones y tres caídas recuperadas. Esta ampliación se comprueba en el núcleo contra el EXE; no añade una nueva prueba visual SDL de las caídas.

### Continuación después de cargar y puertas alternativas

`pnpm comprobar:continuidad` ejecuta seis recorridos SDL con las mismas entradas: primero sin recargas y después guardando y cargando en la misma partida viva. No sustituye la continuación por una instancia nueva que solo dibuja la imagen. Cada punto comprueba escena, cámara, 64.000 píxeles y paleta inmediatamente después de cargar; las acciones posteriores deben alcanzar el mismo resultado final que la ejecución de control.

La tanda ejecutada comprende el recorrido hasta THE END (313 recargas en 70 escenas), los regresos33/38/41 (102/102/98 recargas) y las dos puertas de60 con regreso110 (270 recargas en cada recorrido). Son1.155 operaciones en seis recorridos que repiten el tramo inicial, no1.155 estados independientes. En conjunto hay continuación tras cargar en73 escenas;2 y68 son secuencias automáticas sin punto de menú. Los archivos y resultados quedan en `trabajo/continuidad_*`, fuera de Git.

Las dos puertas de60 se recorren también en el núcleo desde el mismo estado y semilla reales, atravesando108/109 hasta61/62 y110 de vuelta a58, sin cambiar inventario ni forzar azar. El bloque laberinto pasa a3.381 casos/pasos y17 imágenes. Las dos imágenes de regreso SDL coinciden píxel a píxel con esas referencias contrastadas contra el EXE, tanto con recargas como sin ellas.

### Consecuencias de las interacciones iniciales

`pnpm comparar:consecuencias` reutiliza la exploración de los menús reales y prolonga las selecciones que los cierran (STC). Solo admite las referencias iniciales con semilla persistida; no sustituye una semilla ausente por una inventada. Deduplica el estado y azar resultantes, compara DS completo y azar contra el EXE durante al menos128 pasos, o hasta una petición de salida anterior. Si al terminar ese plazo sigue una acción sin reposo, la prolonga hasta512 pasos y la registra como pendiente si no termina. No carga el destino solicitado ni agota animaciones posteriores a esa observación.

El cubo y las ofertas del pozo permiten acciones con el movimiento bloqueado: el reposo se identifica por menú habilitado y ausencia de camino y secuencia de Brad, sin exigir3594. Dos selecciones del corredor (2FCA/32DE) conservan un canal de caminar anterior; una cambia únicamente el cursor y la otra no inicia una secuencia nueva. Se observan128 pasos y se identifican aparte, sin confundirlas con una acción nueva bloqueada. Esta clasificación no elimina bytes de las comparaciones contra el EXE, incluido el cursor.

Resultado ejecutado:551 ramas únicas resultantes de2.786 selecciones,17 estados con semilla real en las escenas1/5/6/7/9/12/14 y70.925 pasos coincidentes.548 ramas llegan al reposo observado y una solicita una salida coincidente; las otras dos conservan la secuencia previa del corredor. El primer informe amplio dejó2FCA señalado por plazo agotado, sin diferencia de DS ni azar; revisado su manejador y el cambio de cursor, se repitieron selectivamente ambas ramas:256 pasos coincidentes. Se conservan el informe amplio y `resultado_filtrado.json`, sin reescribir el resultado histórico. `--estado comparacion_final_cuevas/corredor_izquierda.ds.bin` reproduce esa revisión selectiva. No se ha repetido la tanda amplia después de refinar esta clasificación.

La primera extensión por `--escenas` admitía0–24, salvo2 automática. La tanda del probador añade25 con su banco y CS:1BA2 registrados; una referencia25 sin ese contexto se rechaza. El ámbito no supone que sus escenas se hayan probado. Ejecutadas dos muestras nuevas: `--escenas 15 --estado comparacion_campo/campo_160.ds.bin` contrasta21 ramas/2.688 pasos; `--escenas 16 --estado comparacion_asaltante/asaltante_dinero.ds.bin`,12 ramas/1.536 pasos. Ambas coinciden en DS y azar y terminan en reposo observado. Sus informes `resultado_15_filtrado.json` y `resultado_16_filtrado.json` no sobrescriben el histórico inicial. Son33 ramas nuevas y4.224 pasos, no una revisión completa de las visitas aCOUNTRY1/2.

### Población y respuestas que mantienen el menú abierto

`--incluir-clc` observa también los verbos32DE/563A/2FCA/3143/384A/3B53 cuando devuelven CLC. Las ramas parten del estado posterior a la selección y representan la continuación después de cerrar con clic derecho: en el adaptador ese cierre solo cambia `menu_visible`, sin alterar DS ni azar. No simula pasos mientras el menú está abierto ni cuenta los simples cambios de submenú como acciones. La selección y cada paso posterior siguen comparándose con el EXE completo; no se excluyen bytes de cursor ni de menú de esa comparación. No añade en esta tanda una prueba visual SDL del gesto de cierre.

Tanda ejecutada:304 observaciones en diez estados reales,38.887 pasos y130 ramas CLC, todas coincidentes en DS y azar. Las ocho referencias de escenas17–24 son `comparacion_asaltante/poblacion_fuente`, `comparacion_asaltante/poblacion_maravillas`, `comparacion_calles/calle_29`, `comparacion_calles/calle_30`, `comparacion_poblacion/poblacion_guardias`, `comparacion_poblacion/poblacion_mago`, `comparacion_calles/calle_33` y `comparacion_tiendas/tienda_35`. Cada una aporta24 ramas/3.072 pasos salvo el mago22, con42 ramas/5.351 pasos y una petición de salida contrastada. Son210 observaciones y26.855 pasos en esta primera visita.

Dos referencias posteriores amplían estados ya existentes: `comparacion_regreso_benn/benn_frente_vendedor` aporta52 ramas/6.656 pasos (18 CLC) y `comparacion_regreso_loro/regreso_mago`,42 ramas/5.376 pasos (14 CLC). Conservan el inventario y progreso reales del recorrido; no se fabrica un estado de prueba. No se cuentan estas dos visitas como escenas adicionales.

Se reproducen con `pnpm comparar:consecuencias --escenas <índice> --estado <carpeta/nombre.ds.bin> --incluir-clc`. Los informes están en `trabajo/auditoria_consecuencias/resultado_<índice>_filtrado_clc.json`; para22/24 se conservaron copias separadas `resultado_<índice>_visita_inicial_clc.json` y `resultado_<índice>_regreso_clc.json` antes de reutilizar el nombre filtrado. Las selecciones de inventario al entrar en una calle no equivalen a agotar todos sus personajes, posiciones o visitas posteriores.

`--solo-clc` permite ampliar las referencias anteriores sin repetir las observaciones STC ya contrastadas. Comprobado selectivamente en fuente17:12 ramas/1.536 pasos, todas CLC y coincidentes. Esta repetición valida el filtro y no aumenta el porcentaje ni se suma a las304 observaciones nuevas. Su informe usa el sufijo `_solo_clc.json`.

### Probador: banco y fase externos de las referencias

Las referencias regeneradas incorporan un archivo propio `.contexto.json`, versión1, con `referencia_modelo` y, cuando el comparador dispone de ella, `fase_probador`. Se registra la fase de la sesión contrastada; no se deduce de DS ni del inventario ni se añade al formato DOS de partidas. Para25 se exige una fase entera válida; se ha verificado el rechazo de contexto ausente y de una fase booleana, así como la conservación de fase1 y banco5800. Las referencias antiguas0–24 sin este archivo conservan el banco inicial compatible5400 y no ejecutan la rutina25.

La auditoría utiliza el puente ya contrastado de `ComparacionRegresoBenn`, con banco explícito y comparación de CS:1BA2 después de cada paso. Cada rama restaura su fase inicial y deduplica incluyendo fase y banco. También comprueba que la selección de menú no modifique ese byte: el puente actual de selección no recibe una fase y detectaría ese contrato pendiente en lugar de darlo por válido.

Se regeneraron `comparar:probador --muestra` (3.025 casos/pasos, dos imágenes) y `comparar:regreso-benn --muestra` (10.109 casos/pasos, siete imágenes). El segundo registra además DS, semilla y contexto en los cambios de fase de la secuencia real del vestido, sin fabricar79 ni generar imágenes nuevas.

| Referencia real de25                              | Ramas STC/CLC | Pasos coincidentes | CLC |
| ------------------------------------------------- | ------------- | ------------------ | --- |
| `comparacion_probador/probador_entrada`           | 24            | 3.072              | 12  |
| `comparacion_regreso_benn/probador_regreso_valle` | 37            | 4.736              | 20  |
| `comparacion_regreso_benn/probador_vestido_real`  | 30            | 3.840              | 16  |

Total nuevo:91 observaciones de interacción y11.648 pasos, incluidas48 CLC, sin diferencias de DS, azar ni fase externa. Son tres estados de una misma escena; solo aumentan en una la cobertura por escenas. Se reproducen con `pnpm comparar:consecuencias --escenas 25 --estado <referencia.ds.bin> --incluir-clc`. Los informes separados son `resultado_25_visita_inicial_clc.json`, `resultado_25_probador_regreso_valle_clc.json` y `resultado_25_probador_vestido_real_clc.json`; el nombre filtrado genérico conserva el último ejecutado.

`--reanudar` exige una referencia explícita y continúa su secuencia sin abrir ni elegir menús. Probado con `--escenas 25 --estado comparacion_regreso_benn/probador_fase_1.ds.bin --reanudar`: fase real1, menú bloqueado, banco5800;128 pasos idénticos al EXE y llegada a reposo con fase0. Es una prueba de reanudación, no una interacción adicional ni un nuevo estado de guardado DOS. Informe `resultado_25_filtrado_reanudar.json`. No se afirma que los SAV originales conserven CS. La regresión selectiva anterior del corredor sigue coincidiendo:dos selecciones STC/256 pasos; no se ha repetido la tanda amplia del inicio.

Esta ampliación compara el núcleo y el contexto de las referencias; no añade capturas SDL ni agota todos los verbos, inventarios o visitas posibles. El ámbito del comando sigue limitado a0–25 salvo2; los capítulos siguientes se incorporarán con referencias completas.

### Respuestas CLC de las cuevas

Tanda selectiva posterior ejecutada con `--solo-clc`, sin repetir la observación STC anterior ni fabricar objetos. Se restauran las instantáneas y semillas reales de `comparacion_final_cuevas/`; cada selección y sus128 pasos de continuación se contrastan con el EXE, incluidos DS completo, azar y fase externa del comparador.

| Referencia real           | Escena | Ramas CLC | Pasos coincidentes |
| ------------------------- | ------ | --------- | ------------------ |
| `final_casco.ds.bin`      | 9      | 24        | 3.072              |
| `final_mascara.ds.bin`    | 12     | 14        | 1.792              |
| `final_agujero_68.ds.bin` | 14     | 23        | 2.944              |
| `final_trampa.ds.bin`     | 5      | 22        | 2.816              |

Total:83 observaciones nuevas y10.624 pasos coincidentes. Las83 llegan al reposo con menú habilitado; no se han encontrado diferencias ni un manejador pendiente en estas ramas. No agota todas las posiciones ni los inventarios posibles de estas escenas.

Reproducción: `pnpm comparar:consecuencias --escenas <índice> --estado comparacion_final_cuevas/<referencia.ds.bin> --solo-clc`. Informes locales preservados como `resultado_9_final_casco_solo_clc.json`, `resultado_12_final_mascara_solo_clc.json`, `resultado_14_final_agujero_68_solo_clc.json` y `resultado_5_final_trampa_solo_clc.json`, en `trabajo/auditoria_consecuencias/`. Los nombres filtrados genéricos también conservan la ejecución de cada escena.

Se revisó además `corredor_izquierda.ds.bin` de escena1 con el mismo filtro:24 selecciones inmediatas coincidentes, pero ninguna rama CLC observada (cero pasos). Su informe `resultado_1_filtrado_solo_clc.json` no se cuenta como avance de consecuencias ni acredita ausencia de CLC en otros estados del corredor.

Las cuatro escenas ya estaban incluidas entre las18/73 con muestras, por lo que el porcentaje se mantiene en24,7%. No hay nuevas capturas ni nuevas salas SDL en esta tanda. Siguen pendientes más estados iniciales, referencias antiguas sin semilla real y capítulos posteriores al ámbito0–25 del comando.

### Pozo y cubo: otros cuatro estados CLC

Ampliación separada sin repetir las cuatro referencias anteriores:

| Referencia real en `comparacion_final_cuevas/` | Escena | Ramas CLC | Pasos coincidentes |
| ---------------------------------------------- | ------ | --------- | ------------------ |
| `final_casco_suelo.ds.bin`                     | 9      | 20        | 2.560              |
| `final_ofertas_pozo.ds.bin`                    | 9      | 24        | 3.072              |
| `final_cubo.ds.bin`                            | 12     | 22        | 2.816              |
| `final_dentro_cubo.ds.bin`                     | 12     | 22        | 2.816              |

Son88 observaciones nuevas y11.264 pasos: todas llegan al reposo con menú habilitado y coinciden con el EXE en DS completo, azar y fase externa. El segundo estado del cubo permite menú con movimiento bloqueado; la prueba conserva esa condición real. Los informes separados son `resultado_9_final_casco_suelo_solo_clc.json`, `resultado_9_final_ofertas_pozo_solo_clc.json`, `resultado_12_final_cubo_solo_clc.json` y `resultado_12_final_dentro_cubo_solo_clc.json`. Se reproducen con el mismo comando anterior cambiando referencia e índice; no se fabrican objetos ni semillas.

Estas dos escenas ya estaban incluidas en18/73. La ampliación añade estados comprobados, no nuevas salas ni un porcentaje global de fidelidad. Quedan otros estados iniciales y los capítulos posteriores pendientes del contraste explícito de bancos y fases.

### Semillas reales y consecuencias de cuatro regresos del pueblo

`ComparacionCalles.captura` conserva ahora la semilla real de `self.semilla_menu`, que es el mismo objeto usado como `sem` durante este recorrido, incluso cuando `recursos_menu` está desactivado. Antes las cuatro entradas guardaban manualmente su semilla, pero los regresos solo guardaban DS. No se sustituye por una semilla inicial inventada.

Regenerado `comparar_calles.py --muestra`:6.113 casos/pasos, nueve composiciones y368 selecciones coincidentes. Las nueve instantáneas DS resultantes son idénticas por SHA-256 a las anteriores; se recuperan semillas persistentes para `regreso_29`, `regreso_30`, `regreso_33` y `regreso_41`. La rama de la posada conserva también la reacción real del mostrador antes del regreso.

Con esas referencias se ejecutó `--incluir-clc`: cada uno de los cuatro regresos aporta24 observaciones y3.072 pasos, incluidas12 CLC. Total nuevo:96 observaciones,12.288 pasos y48 ramas CLC; todas llegan al reposo con menú y coinciden en DS/azar/fase. Las escenas de destino son17,17,18 y21; ya estaban cubiertas, por lo que permanece18/73 (24,7%).

Comandos: `pnpm comparar:calles --muestra` y `pnpm comparar:consecuencias --escenas <17|18|21> --estado comparacion_calles/regreso_<29|30|33|41>.ds.bin --incluir-clc`. Informes preservados como `resultado_17_regreso_29_clc.json`, `resultado_17_regreso_30_clc.json`, `resultado_18_regreso_33_clc.json` y `resultado_21_regreso_41_clc.json`. Los informes genéricos filtrados quedan actualizados con el último estado ejecutado de cada escena; las visitas iniciales descritas antes siguen siendo reproducibles con sus referencias originales.

Esto no vuelve a ejecutar ni reemplaza el balance global histórico de59.470 selecciones; sus392 semillas reales y37 referencias antiguas describen aquella ejecución. Quedan más referencias antiguas sin azar persistente, además de estados e interacciones posteriores pendientes.

### Semillas del recorrido inicial completo

El comparador del corredor conserva ahora el mismo objeto de semilla que recibe `despues_puerta` para la partida real. El registro se limita al mismo objeto DS recibido en ese recorrido: una copia sintética, aunque tenga bytes idénticos, no hereda automáticamente su azar. Cada captura guarda sus cuatro palabras antes de componer la imagen; no lee el azar de una rutina de dibujo ni lo reconstruye a partir de DS. Las subclases de lago, poza, cuevas y catapulta heredan esa conservación. Los estados posteriores de `ComparacionFinalCuevas` mantienen también su registro existente de semilla y contexto.

Ejecutado `pnpm comparar:final-cuevas --muestra`:32.933 casos/pasos coincidentes con el EXE. Las58 instantáneas DS de `comparacion_final_cuevas/` son idénticas por SHA-256 a las anteriores y todas conservan semilla; las29 referencias que antes carecían de ella en esa carpeta quedan regeneradas desde el recorrido, sin adjuntar a un estado antiguo un azar supuesto. Los resultados locales están en `trabajo/semillas_inicio_regeneracion.log`, `trabajo/semillas_inicio_antes.json` y `comparacion_final_cuevas/muestra.json`.

Esto habilita nuevas comprobaciones de consecuencias con semilla real. No implica que se hayan recuperado automáticamente las semillas de todas las copias históricas en otras carpetas ni sustituye el balance global de la auditoría antigua. La preservación de la referencia por sí sola no aumenta el número de escenas con consecuencias observadas. La guardia de identidad se comprobó selectivamente con una referencia, una copia y la actualización del mismo objeto de semilla; es una prueba de metadatos, sin composición ni casos jugables añadidos. La regeneración completa citada precede a esa guardia adicional.

### Nuevas muestras del inicio: siete escenas

Después de regenerar las referencias se ejecutaron las consecuencias con `--incluir-clc`, tanto STC como verbos CLC tras el cierre lógico del menú:

| Referencia real en `comparacion_final_cuevas/` | Escena | Observaciones | Pasos coincidentes | CLC |
| ---------------------------------------------- | ------ | ------------- | ------------------ | --- |
| `mazmorra_regreso.ds.bin`                      | 0      | 32            | 4.096              | 10  |
| `lago_hablar.ds.bin`                           | 3      | 32            | 4.096              | 10  |
| `cuevas_corredor.ds.bin`                       | 4      | 18            | 2.304              | 8   |
| `poza_puerta.ds.bin`                           | 8      | 31            | 3.968              | 10  |
| `cuevas_cuerda.ds.bin`                         | 10     | 41            | 5.248              | 14  |
| `catapulta_tablon.ds.bin`                      | 11     | 56            | 7.168              | 19  |
| `catapulta_monitor.ds.bin`                     | 13     | 30            | 3.840              | 16  |

Total nuevo:240 observaciones,30.720 pasos y87 ramas CLC, todas reposo con menú habilitado y coincidentes en DS completo, azar y fase externa del comparador. No se han encontrado omisiones ni diferencias en estas referencias. Reproducción: `pnpm comparar:consecuencias --escenas <índice> --estado comparacion_final_cuevas/<referencia.ds.bin> --incluir-clc`.

Los informes se preservan como `resultado_<escena>_<referencia>_clc.json` en `trabajo/auditoria_consecuencias/`, además de los nombres genéricos filtrados. Los logs locales son `consecuencias_inicio_<referencia>.log`. No se modifican ni publican las instantáneas comerciales.

Las siete escenas no estaban incluidas en el indicador adicional: este pasa de18/73 a25/73 (34,2%). La franja0–25 dispone ahora de muestras de consecuencias salvo la caída automática2, pero no se acreditan todos sus estados. El regreso de la mazmorra no sustituye todas las condiciones de su inicio, y un estado de cada tramo no agota variantes de inventario ni visitas posteriores. No se han añadido capturas SDL en esta ampliación; siguen75/75 salas accesibles y73/73 escenas con continuación tras cargar dentro del alcance descrito.

### Consecuencias de la adivina, la segunda casa y la posada

Se amplía el ámbito del comando hasta28, conservando la exclusión de2 y la exigencia de contexto registrado para25–28. Las referencias emplean el banco real5400; no se deduce desde el inventario. El puente común contrasta DS completo y azar, y comprueba que CS:1BA2 no cambie. La fase externa del probador solo se exige explícita para25.

Se regeneró `pnpm comparar:tiendas --muestra`:7.261 casos/pasos,681 selecciones y ocho composiciones coincidentes, con contexto y semilla guardados por el recorrido real. La posada usa las referencias de `comparacion_calles` regeneradas en la tanda anterior. El contraste original conserva35DC–35F8 para el saludo,420F–430C para las respuestas al oro según35C9 y541C–547F para el posadero; no se han modificado condiciones del motor.

| Escena | Referencia                         | Observaciones STC/CLC | Pasos | CLC |
| ------ | ---------------------------------- | --------------------- | ----- | --- |
| 26     | comparacion_tiendas/tienda_38      | 25                    | 3.200 | 12  |
| 26     | comparacion_tiendas/adivina_saludo | 42                    | 5.577 | 14  |
| 27     | comparacion_tiendas/tienda_39      | 24                    | 3.072 | 12  |
| 28     | comparacion_calles/calle_41        | 24                    | 3.072 | 12  |

Total:115 observaciones,14.921 pasos y50 ramas CLC, con389 selecciones de despacho. Todas terminan en reposo con menú habilitado y coinciden con el EXE. Reproducción: `pnpm comparar:consecuencias --escenas <26|27|28> --estado <referencia.ds.bin> --incluir-clc`. Informes separados `resultado_<escena>_<referencia>_clc.json` en `trabajo/auditoria_consecuencias`; los nombres genéricos filtrados conservan el último estado de cada escena.

Dos referencias tenían el menú bloqueado y se comprobaron mediante `--reanudar`, sin contar nuevas selecciones: `comparacion_tiendas/adivina_respuesta` recupera reposo a los137 pasos; `comparacion_calles/posadero` solicita salida50 a los16 pasos. Son153 pasos adicionales coincidentes; el comparador no carga el destino de esa petición. Informes `resultado_26_adivina_respuesta_reanudar.json` y `resultado_28_posadero_reanudar.json`; logs `trabajo/consecuencias_tiendas_<referencia>.log`.

Se añaden tres escenas al indicador:28/73 (38,4%). Estas muestras no agotan las cinco respuestas de la adivina con progresos posteriores ni todos los inventarios y posiciones de la posada. No hay capturas SDL nuevas:75/75 accesibles y continuidad73/73 siguen con el alcance anterior. Próximo bloque: referencias completas y bancos de29–31, además de estados tardíos pendientes.

### Frank, primera roca de EDGE y saludo del loro

El ámbito del comando se amplía hasta31 y sigue exigiendo contexto registrado para25–31. Se regeneraron tres recorridos originales: `pnpm comparar:poblacion --muestra` (8.855 casos/pasos,1.084 selecciones,12 composiciones), `pnpm comparar:recorrido-borde --muestra` (1.142 casos/pasos y seis composiciones) y `pnpm comparar:loro --muestra` (1.484 casos/pasos,442 selecciones y seis composiciones). Ninguna DS previa de sus carpetas cambia de SHA256; solo los hitos realmente emitidos por cada generador reciben contexto nuevo. No se atribuye contexto a las pruebas exploratorias antiguas de EDGE.

Los bancos conservados son5400 para FRANKS29 y5800 para SMALL en EDGE30 y WISE31. El comparador verifica que los recursos no se solapen y mantengan sus bytes originales. CS:17A3 no añade rutina específica en29/31;30 ejecutaC093–C4DB. Se conservan los contratos35FB–3617 del saludo a Frank,41DA–420C del pago,419E–41D7 del consumo y361A–3727 del saludo del loro. La guía local confirma el orden del trayecto; las condiciones y resultados se contrastan con el EXE.

| Escena | Referencia                                | Observaciones STC/CLC | Pasos | CLC |
| ------ | ----------------------------------------- | --------------------- | ----- | --- |
| 29     | comparacion_poblacion/poblacion_frank     | 25                    | 3.200 | 12  |
| 29     | comparacion_poblacion/poblacion_comida    | 53                    | 6.784 | 18  |
| 29     | comparacion_poblacion/poblacion_consumida | 42                    | 5.376 | 14  |
| 30     | comparacion_recorrido_borde/borde_roca    | 41                    | 5.248 | 14  |
| 31     | comparacion_loro/loro_entrada             | 25                    | 3.201 | 12  |

Total:186 observaciones,23.809 pasos,70 ramas CLC y554 selecciones de despacho, sin diferencias de DS completo, azar ni fase externa.185 observaciones terminan en reposo con menú; el saludo del loro solicita salida73 al paso129. El comparador de consecuencias no carga ese destino. Además, `comparacion_loro/loro_saludo_64` se reanuda sin seleccionar menú y solicita73 después de65 pasos coincidentes; se registra aparte, sin contarlo como nueva interacción. Total con esa reanudación:23.874 pasos.

Reproducción: `pnpm comparar:consecuencias --escenas <29|30|31> --estado <referencia.ds.bin> --incluir-clc`; para la secuencia ya iniciada, `--escenas 31 --estado comparacion_loro/loro_saludo_64.ds.bin --reanudar`. Informes locales `resultado_<escena>_<referencia>_clc.json` y `resultado_31_loro_saludo_64_reanudar.json`; logs `consecuencias_franks_edge_<referencia>.log`. Las guardas de ámbito rechazan32 y2;25–31 rechazan contexto ausente, y se comprueba la lectura de5400/5800 sin sumar casos jugables.

La cobertura por muestras pasa a31/73 (42,5%), escenas0–31 salvo2. No se han encontrado diferencias en esta tanda ni se acredita que estén agotados todos sus estados. En particular, esta muestra de EDGE es la aproximación a la primera roca; falta extender consecuencias a los demás puzles y al rescate con sus contextos reales. También quedan visitas posteriores al loro y más inventarios. SDL75/75 y continuidad73/73 no cambian; no hay nuevas capturas SDL. Siguiente: regenerar contextos de los hitos persistentes de `comparar:acciones-borde`, comprobar sus consecuencias y avanzar32+ por bloques.

### Más consecuencias en EDGE: rescate, guantes, puente, piedra e inscripción

Se regeneró `pnpm comparar:acciones-borde --muestra` para registrar los contextos reales de sus quince hitos:12.229 casos/pasos,4.476 selecciones y quince composiciones coincidentes. Ninguna DS previa de la carpeta cambia de SHA256. Solo los hitos emitidos reciben contexto5800; las instantáneas exploratorias antiguas no lo heredan. El recorrido conserva el rescate alternativo por el pájaro, los botones y el ciclo ADCE real, sin fabricar posiciones, inventario ni fases para los estados persistentes. Las variantes aisladas del generador siguen separadas del recorrido.

Se amplían las consecuencias STC y CLC tras cierre lógico de menú en seis referencias distintas de la primera aproximación ya revisada:

| Referencia de comparacion_acciones_borde | Observaciones | Pasos | CLC | Selecciones |
| ---------------------------------------- | ------------- | ----- | --- | ----------- |
| borde_roca_empujada                      | 41            | 5.248 | 14  | 113         |
| borde_rescate                            | 24            | 3.072 | 12  | 92          |
| borde_guantes                            | 30            | 3.840 | 16  | 118         |
| borde_puente                             | 64            | 8.192 | 21  | 234         |
| borde_caramelo_puesto                    | 37            | 4.736 | 20  | 148         |
| borde_inscripcion                        | 42            | 5.376 | 13  | 114         |

Total nuevo:238 observaciones,30.464 pasos,96 ramas CLC y819 selecciones. Todas llegan a reposo con menú y coinciden en DS completo, azar y fase externa del comparador. La reanudación separada de `borde_abismo_saltado`, cuyo menú está bloqueado en la captura, añade128 pasos coincidentes hasta reposo:30.592 pasos incluyendo esa secuencia, sin contar una selección nueva. No se cambia el motor ni se añaden condiciones de puzles.

Reproducción: `pnpm comparar:consecuencias --escenas 30 --estado comparacion_acciones_borde/<referencia>.ds.bin --incluir-clc`; para el salto ya iniciado, `--estado comparacion_acciones_borde/borde_abismo_saltado.ds.bin --reanudar`. Informes locales `resultado_30_<referencia>_clc.json` y `resultado_30_borde_abismo_saltado_reanudar.json`; logs `consecuencias_edge_<referencia>.log` y `consecuencias_edge_abismo_reanudado.log`. Los nombres filtrados genéricos conservan la última ejecución de su tipo.

Esta tanda profundiza en una escena ya contada, por lo que mantiene31/73 (42,5%) con muestras,75/75 accesibles en SDL y73/73 con continuidad tras cargar. No se añade ninguna captura SDL ni se agotan todas las posiciones, inventarios o ciclos temporizados. Quedan otras referencias persistentes de EDGE: roca_saltada, cuerda, electrico, caramelo, roca_grande, cuerda_lanzada y puerta_abierta, además de estados posteriores y capítulos32+. `borde_salida` conserva una petición ya emitida: no tratarla como una nueva interacción ni como carga del destino. No repetir los seis estados de esta tabla ni la reanudación sin un cambio que lo justifique.

### Entradas y regresos alternativos del valle32–35

Se amplía el ámbito de consecuencias hasta35, siempre con contexto obligatorio25–35. Se regeneró `pnpm comparar:enlaces-valle --muestra`:13.589 casos/pasos,616 selecciones y seis composiciones coincidentes, sin cambiar ninguna DS previa de la carpeta. Los seis hitos registran su banco real5800. La decisión70→DINO32 surge del estado real del barco; la alternativa52→POST35 aparece tras tres pasos reales en POST y el retorno71 a CELESTE, sin asignar semillas para forzarla. Se conservan también55 haciaREMOTE y56 de regreso.

Las rutinasC5E0–C713 (DINO),C714–C736 (CELESTE),C737–C793 (REMOTE) yC794–C8B0 (POST), sus actores y sus umbrales siguen en los servicios comunes. El estado aleatorio se conserva con cada hito y el comparador contrasta DS completo, azar y la fase externa que controla el puente; no se deduce el banco desde el inventario.

| Escena | Referencia de comparacion_enlaces_valle | Observaciones | Pasos | CLC | Selecciones |
| ------ | --------------------------------------- | ------------- | ----- | --- | ----------- |
| 32     | celeste_azar_70                         | 54            | 6.912 | 32  | 262         |
| 33     | celeste_regreso_70                      | 54            | 6.912 | 32  | 262         |
| 34     | celeste_remote_55                       | 54            | 6.912 | 32  | 262         |
| 35     | celeste_azar_52                         | 24            | 3.072 | 12  | 92          |

Total nuevo:186 observaciones,23.808 pasos,108 ramas CLC y878 selecciones. Todas llegan a reposo con menú y coinciden con el EXE; no se encontraron diferencias en estas referencias. Reproducción: `pnpm comparar:consecuencias --escenas <32|33|34|35> --estado comparacion_enlaces_valle/<referencia>.ds.bin --incluir-clc`. Informes locales `resultado_<escena>_<referencia>_clc.json` y logs `consecuencias_valle_<referencia>.log`. Los nombres filtrados genéricos conservan la ejecución de cada escena. Las guardas de ámbito rechazan2/36 y contexto ausente en25–35; esta comprobación de argumentos no suma pasos jugables.

La cobertura por muestras pasa de31/73 a35/73 (47,9%), escenas0–35 salvo2. Estas son entradas y regresos concretos con inventarios del recorrido del barco, no todas las fases de los puzles de esas salas: no sustituyen las consecuencias pendientes del pelo y estornudo, grabación, mando, buzón ni posteriores visitas. En REMOTE se contrasta la entrada55, sin acreditar automáticamente todas las entradas60/59. No se ha modificado el motor ni se añaden capturas SDL: accesibilidad75/75 y continuidad73/73 mantienen el alcance anterior. Próximo: más estados significativos de32–35, restantes referencias deEDGE y capítulos36+ con contexto registrado.

### Consecuencias del estornudo y regreso con sello y bolígrafo

La revisión adicional de DINO32 y POST35 añade un checkpoint sin imagen ante el monstruo: `dinosaurio_antes_estornudo`. Se escribe desde el estado y la semilla reales después de acercarse por250/170 y340/140, antes de las variantes aisladas y de insertar el pelo. Conserva su contexto5800 mediante el servicio común, sin asignar inventario, posición ni banderas. No altera la ruta ni añade una captura SDL.

Se regeneró `pnpm comparar:dinosaurio --muestra`:8.587 casos/pasos,1.596 selecciones, seis estados auditados y seis composiciones coincidentes. Ninguna DS previa de su carpeta cambia de SHA256; el checkpoint nuevo permite observar las alternativas en el punto del puzle y no solo desde la entrada. Se conservan A81A–A8AB para insertar110/102,372A–3763 para el saludo cercano y C5E0–C713 para umbrales y actores. La secuencia sigue consumiendo el pelo, conservando el fotograma29 del monstruo y permitiendo recoger109/101 antes del regreso58 según el EXE.

| Escena | Referencia de comparacion_dinosaurio | Observaciones | Pasos | CLC | Selecciones |
| ------ | ------------------------------------ | ------------- | ----- | --- | ----------- |
| 32     | dinosaurio_antes_estornudo           | 61            | 7.808 | 20  | 204         |
| 32     | dinosaurio_estornudo                 | 47            | 6.016 | 15  | 138         |
| 32     | dinosaurio_sello_boligrafo           | 36            | 4.608 | 20  | 148         |
| 35     | dinosaurio_regreso_post              | 36            | 4.608 | 20  | 148         |

Total nuevo:180 observaciones,23.040 pasos,75 ramas CLC y638 selecciones. Todas llegan a reposo con menú y coinciden en DS completo, azar y fase externa del comparador. Reproducción: `pnpm comparar:consecuencias --escenas <32|35> --estado comparacion_dinosaurio/<referencia>.ds.bin --incluir-clc`. Informes `resultado_<escena>_<referencia>_clc.json` en `trabajo/auditoria_consecuencias/`; logs `consecuencias_dinosaurio_<referencia>.log`. El nombre genérico32 queda en sello_boligrafo, pero las tres ejecuciones se conservan por separado.

No hay diferencias nuevas detectadas ni cambios de puzles. Se profundiza en escenas ya incluidas: la cobertura mantiene35/73 (47,9%), SDL75/75 y continuidad73/73. Las combinaciones y respuestas ofrecidas en estos cuatro estados no agotan todas las posiciones, fases o inventarios; el checkpoint ante el monstruo no sustituye otros accesos a DINO ni las fases posteriores de grabación, mando o buzón. Próximo: estados significativos de grabación y REMOTE, más regresos, capítulos36+ y restantes hitos de EDGE.

### Consecuencias de grabación, combinaciones de correo y REMOTE tras el estornudo

Se regeneró `pnpm comparar:grabacion-completa --muestra`:13.171 casos/pasos,1.260 selecciones, seis estados auditados y ocho composiciones coincidentes. Las ocho referencias conservan ahora contexto5800 real; ninguna DS previa de la carpeta cambia de SHA256. La partida conserva el mando108 recogido en REMOTE desde59 después del estornudo, la cinta105, la grabación106, sello109 y carta111, sin concederlos desde otra rama. La espera de768 pasos tras empujar el mando y la secuencia de256 del envío continúan contrastadas en ese recorrido completo.

Los contratos AB60–ABD2,ABD5–AC5B,AA6C–AAA0 yAAA3–AB5D conservan las pruebas firmadas, consumos, oferta de la grabación, puntuación y campos ajenos del EXE. La revisión adicional observa las opciones ofrecidas en cuatro estados:

| Escena | Referencia de comparacion_grabacion_completa | Observaciones | Pasos | CLC | Selecciones |
| ------ | -------------------------------------------- | ------------- | ----- | --- | ----------- |
| 34     | mando_recogido                               | 42            | 5.376 | 24  | 182         |
| 35     | cinta_insertada                              | 72            | 9.216 | 26  | 218         |
| 35     | grabacion_y_carta                            | 50            | 6.400 | 28  | 220         |
| 35     | grabacion_y_correo_enviado                   | 61            | 7.808 | 22  | 179         |

Total nuevo:225 observaciones,28.800 pasos,100 ramas CLC y799 selecciones, todas en reposo con menú y coincidentes en DS completo, azar y fase externa del comparador. El informe de cinta_insertada incluye Empujar108 desde inventario por las casillas0-based2/2/0/5. El de grabacion_y_carta contiene Combinar109→111 y111→109 por2/7/3/0/6 y2/7/6/0/3. Las respuestas negativas y los otros verbos ofrecidos se observan restaurando la referencia y el azar entre ramas; no se acumulan sobre la solución principal.

Reproducción: `pnpm comparar:consecuencias --escenas <34|35> --estado comparacion_grabacion_completa/<referencia>.ds.bin --incluir-clc`. Informes locales `resultado_<escena>_<referencia>_clc.json` y logs `consecuencias_grabacion_<referencia>.log`. Los informes genéricos filtrados conservan el último estado de cada escena. Los128 pasos de cada observación y el reposo lógico no constituyen una medición de audio o cadencia; la ruta positiva larga permanece respaldada por la regeneración completa citada.

La cobertura por escenas se mantiene35/73 (47,9%), SDL75/75 y continuidad73/73. No hay cambios del motor ni capturas SDL nuevas. Se comprueban más inventarios y consecuencias en escenas ya contadas, no todas las fases del valle. Siguen pendientes en esta revisión ampliada la entrega105/106 a98 y el pase, otras fases de grabación y REMOTE, el momento previo al envío de la carta, más regresos y capítulos36+. El estado posterior al envío no sustituye todas las condiciones posibles de inserción en el buzón.

### Entrega al destinatario, pase y salida automática66

Se añade `destinatario_antes_entrega`, un checkpoint sin imagen guardado después de acercarse realmente por150/165 y antes de las variantes aisladas. Conserva DS, semilla y contexto5800 desde la partida con grabación106 y correo enviado. No se asignan objetos, puntuación ni fotograma del destinatario para crear ese punto.

`pnpm comparar:entrega-grabacion --muestra` vuelve a coincidir en6.286 casos/pasos,1.522 selecciones, tres estados auditados y cuatro composiciones. Ninguna DS previa cambia de SHA256. Se conservan A8DF–A94F para entregar106 y obtener99 con destinatario en fotograma4, y AC5E–AC8C para usar99 con116. La ruta mantiene las esperas positivas de512 pasos de entrega y hasta256 en la puerta; el guion solicita66 sin consumir el pase ni sumar puntos al abrir.

| Escena | Referencia de comparacion_entrega_grabacion | Observaciones | Pasos | CLC | Selecciones |
| ------ | ------------------------------------------- | ------------- | ----- | --- | ----------- |
| 32     | destinatario_antes_entrega                  | 62            | 7.936 | 22  | 179         |
| 32     | pase_recibido                               | 36            | 4.608 | 20  | 148         |
| 35     | puerta_antes_del_pase                       | 61            | 7.705 | 22  | 179         |

Total nuevo:159 observaciones,20.249 pasos,64 ramas CLC y506 selecciones coincidentes en DS completo, azar y fase externa del comparador.158 llegan a reposo con menú; la inserción del pase por las casillas0-based2/1/4/0 solicita66 al paso25. El comparador de consecuencias termina en esa petición y no carga CLOUD38. `puerta_solicitud66`, ya emitida por el generador, no se vuelve a contar como otra interacción.

Reproducción: `pnpm comparar:consecuencias --escenas <32|35> --estado comparacion_entrega_grabacion/<referencia>.ds.bin --incluir-clc`. Informes locales `resultado_<escena>_<referencia>_clc.json`; logs `consecuencias_entrega_<referencia>.log`. Los informes filtrados genéricos conservan la última ejecución de cada escena, con los dos estados32 preservados por separado.

La revisión mantiene35/73 (47,9%) con muestras,75/75 accesibles en SDL y73/73 con continuidad tras cargar. No se modifica el motor ni se añaden capturas SDL. Las864 variantes aisladas del generador cubren el despacho de105/106 con varios estados, pero no equivalen a observar una entrega negativa105 desde un recorrido real ni todas sus consecuencias. Queda pendiente esa rama real con cinta vacía y sin correo, más estados del valle y capítulos36+. Una ruta reproducible puede partir de cinta_y_mando de grabacion_completa, pasar por61/57 y acercarse al destinatario conservando105 real; no fabricar105 ni cambiar el fotograma para forzar el rechazo.

### Límites que siguen abiertos

- La auditoría global recorre opciones realmente ofrecidas en esas posiciones y estados y contrasta su despacho inmediato, DS y azar. La observación de consecuencias solo cubre las referencias citadas, incluidas las ramas CLC de la población; quedan otros capítulos, más estados CLC y las situaciones no representadas en las instantáneas.128 pasos coincidentes tampoco acreditan una animación indefinida completa.
- El recorrido hasta el final y las ramas citadas no agotan todos los regresos con inventarios posteriores ni todas las condiciones de progreso.
- Las recargas nuevas se realizan en reposo con menú habilitado, sin camino, tecla ni secuencia activa de Brad. No agotan guardados durante animaciones, todas las variantes de inventario, ni importaciones entre procesos o ediciones DOS. Las tres aproximaciones centrales contrastadas no equivalen a agotar todas las direcciones y posiciones posibles de caída.

No se ha encontrado una implementación ausente en esta tanda. No se declara todavía cerrada la revisión de todas las alternativas. La finalidad de estas comprobaciones es detectar omisiones del port fiel; no calcular la duración de una ruta con desvíos.

`pnpm auditar:rutas` verifica la huella del EXE y recorre los 78 índices de BOXDET.DAT. Cruza sus umbrales de tipo 2 con la tabla DOS de destinos en DS:7482 y la lista única `salidas_integradas` de SDL. El informe completo queda en `apps/juego/trabajo/auditoria_rutas/resultado.json`. No ejecuta puzles, predicados ni destinos indirectos de guiones; un enlace habilitado tampoco acredita todas las opciones de su destino.

## Rutas detectadas en esta revisión

| Origen                       | Salida | Destino                            | Estado                                                   |
| ---------------------------- | ------ | ---------------------------------- | -------------------------------------------------------- |
| Fuente, derecha (17)         | 29     | Calle de los disfraces, ROGUE (19) | Entrada y regreso 37 integrados.                         |
| Fuente, izquierda (17)       | 30     | Calle de la adivina, GYPSY (20)    | Entrada y regreso 40 integrados.                         |
| Calle de las maravillas (18) | 33     | Tienda abandonada, DERELICT (23)   | Entrada y regreso 45 integrados.                         |
| Entrada del pueblo (21)      | 41     | Posada Red Dragon, INNSIDE (28)    | Entrada, reacción del mostrador y regreso 50 integrados. |

La fuente tiene cuatro salidas originales: arriba a SORCERY, abajo a DRAGON, izquierda a GYPSY y derecha a ROGUE. Las dos laterales estaban sin conectar en la entrega anterior; su falta de respuesta no era el comportamiento original.

El recorrido contrastado de estas cuatro ramas parte de las instantáneas reales de la población. Compara 6.113 casos/pasos, nueve composiciones y 368 selecciones en cuatro estados, incluidos el posadero y los regresos. Las ramas de prueba no fabrican inventario ni banderas para forzar entradas. Fuentes: 19C1–19DE para GYPSY; 541C–547F para el mostrador; entradas 29/30/33/41 y retornos 37/40/45/50 del cargador.

## Pendientes localizados desde las vistas disponibles

| Origen      | Salida | Destino DOS | Trabajo pendiente                                                    |
| ----------- | ------ | ----------- | -------------------------------------------------------------------- |
| CHANGE (25) | 74     | Escena 43   | Continuación posterior del disfraz; aún sin integrar.                |
| GYPSY (20)  | 52     | Escena 35   | Continuación posterior todavía sin integrar.                         |
| MAGE (22)   | 72     | Escena 31   | Carga22→31 contrastada; faltan condiciones argumentales posteriores. |

Las rutas 19→24 por35, 20→26 por38 y 20→27 por39 ya están conectadas, con regresos46/48/49. La comparación de tiendas incluye saludo, entrega de dinero a la adivina, fases de oferta de Benn y su rutina de partículas. Se conservan los recursos compartidos entre escenas23/27 con sus estados independientes. Véase [el tramo de tiendas](tiendas_poblacion.md).

EDGE30 ya tiene recorrido persistente hasta WISE31 por72, con regreso del loro a DERELICT27 por73. El contraste incluye los puzles del acantilado y el saludo inicial del loro; no equivale a probar todos los estados de las visitas posteriores. Véase [el alcance de EDGE](borde_del_mundo.md). Las escenas fuera del catálogo siguen pendientes de reconstrucción; el informe de 78 tablas permite localizarlas sin contarlas como jugables.

Las escenas iniciales hasta COUNTRY2 conservan sus recorridos y auditorías anteriores. Esto no demuestra exhaustivamente todos los estados de una partida, retornos de capítulos posteriores, opciones de inventario posibles ni equivalencia completa de cadencia. Los errores de disco DOS, macOS y WebAssembly continúan pendientes de validación. Las mejoras gráficas son opcionales y están separadas de estas garantías del motor.

El probador25 ya conecta36/27. Se contrasta1A74 y los manejadores del vestido; el recorrido posterior del valle sigue pendiente. Véase [el alcance del probador](probador_benn.md).

La auditoría de32 vistas detecta también WISE31→64 por112, correspondiente a la continuación posterior. Junto con52/72/74 suma cuatro enlaces pendientes desde el catálogo; no se ocultan los destinos de capítulos posteriores.

La revisión siguiente incorpora el adaptador MAGE22→WISE31 por72, contrastado como contrato aislado del cargador. No prueba todavía su activación argumental posterior. Quedan tres enlaces desde las vistas disponibles (52/74/112). POST35 tiene entrada y rutina contrastadas solo en el núcleo; sus caminos60/57/66/71 y puzles permanecen pendientes. Véase [el regreso del loro](regreso_loro.md).

Las tandas nativas siguientes contrastan60/56/61, cargas alternativas55/71 y el recorrido57→DINO→58 con pelo, estornudo, sello y bolígrafo;54 se comprueba desde otra rama deCELESTE. No se añaden vistas SDL: quedan puzles y conexiones tardías pendientes. Alcance y pruebas en [ramas del valle](ramas_valle.md) y [DINO](dinosaurio.md).

## Activación SDL de POST

La salida52 desdeGYPSY20 ya está conectada aPOST35. `comprobar:poste` recorre la partida hasta esa entrada, compara64000 píxeles y comprueba guardar/restaurar y66 cambios entre33 vistas F5/F6. El núcleo del resto del valle ya está reconstruido hastaELECTRO39 y regreso16; su activación SDL se realiza por sala y sus rutas siguen pendientes en la plataforma hasta comprobarlas. Las anotaciones anteriores sobre52 y las32 vistas corresponden a revisiones previas.

## Estado vigente tras integrar el valle

Las40 vistasSDL cubren las escenas0–39. La auditoría de las78 tablas deja dos enlaces pendientes desde ese catálogo: CHANGE25/salida74→43 y WISE31/salida112→64. Todas las rutasBOXDET del valle32–39 están catalogadas, incluida70; su alternativa52 y los retornos71 se contrastan por separado con enlaces-valle. Los destinos de guiones66/67/69 están ejercitados en los recorridosSDL. Esto describe enlaces y casos revisados, no todas las combinaciones de estados posteriores del juego.

Las secciones anteriores conservan la historia de ampliaciones; sus recuentos de32 vistas o52 pendiente quedan superados por esta revisión. Las38 capturasSDL añadidas en las ocho salas del valle y80 cambiosF5/F6 están contrastados. Sigue pendiente el capítulo posterior al ventilador, empezando por el regreso aBenn y su probador.

## Estado vigente tras los cementerios

Las67 vistas abarcan0–65 y77. Se añaden113 desde64/65,114 desde65 y115 desde77. El recorrido persistente contrasta las fases0–6 de los vampiros y solicita116 tras abrir la verja. El destino66 sigue pendiente; no se activa por haber reconocido su salida. Los recuentos anteriores describen entregas históricas. Véase [Cementerios](cementerio.md).

## Bosque del castillo integrado

Catálogo68 vistas:0–66 y77. Entrada116 desde77, ambos senderos deCOPSE66 hasta117; destino67 pendiente. Native1.797 casos/pasos,296 selecciones en dos estados y tres composiciones. Véase [Bosque](bosque.md).

## Verja del castillo integrada

Catálogo69 vistas:0–67 y77. Entrada117 desde66, apertura174 y recorrido hasta118; CASTLENT68 pendiente. Núcleo2.954 casos/pasos,12 variantes y cinco composiciones. Véase [Verja](verja.md).

## Puente del castillo integrado

Catálogo70 vistas:0–68 y77. Entrada118 desde67 y derrumbe automático hasta119, sin etapa libre de menú. Núcleo180 casos/pasos y cuatro composiciones; CASTHALL69 pendiente. Véase [Puente](puente_castillo.md).

## Vestíbulo del castillo integrado

Catálogo71 vistas:0–69 y77. Entrada119 y dos escaleras hasta120/122; STUDY70 yLIBRARY71 pendientes. Núcleo1.911 casos/pasos,444 selecciones y cuatro composiciones. Véase [Vestíbulo](vestibulo_castillo.md).

## Estudio del castillo integrado

Catálogo72 vistas:0–70 y77. Entrada120, anillo175, regreso121 y segunda visita contrastados. Núcleo2.193 casos/pasos,72 variantes de recogida y seis composiciones. Biblioteca71 sigue pendiente. Véase [Estudio](estudio_castillo.md).

## Biblioteca del castillo integrada

Catálogo73 vistas:0–71 y77. Entrada122, regreso123 y libro176 hasta solicitar124. Núcleo6.524 casos/pasos,48 variantes y ocho composiciones. Batalla72 y desenlace73 pendientes. Véase [Biblioteca](biblioteca_castillo.md).

## Duelo con la bruja integrado

Catálogo74 vistas:0–72 y77. Duelo72 desde124, tres defensas y anillo hasta125; desenlace73 pendiente.1.611 casos/pasos,480 variantes directas, siete composiciones. Véase [Duelo](duelo_bruja.md).

## Desenlace integrado: catálogo completo

75 vistas: escenas0–73 y77. Las74–76 son reservasDUNGEON, no habitaciones adicionales de la aventura. Entrada125, vaciado original de inventario y tres actores finales contrastados:391 casos/pasos y seis composiciones. Sigue pendiente revisar una partida continua con todas sus alternativas. Véase [Desenlace](desenlace.md).
