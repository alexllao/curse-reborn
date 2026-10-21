# Los dos cementerios

Las vistas66 y67 corresponden a las escenas65 (GRAVEYD1) y77 (GRAVEYD2). Ambas usan GRAVEYD.SPR y GRAVEYD.SFX según la tabla original784E. La salida113 continúa la excavación del ataúd con el modelo26;114 comunica los dos fondos y115 permite regresar. El destino66 de la verja116 queda para el siguiente bloque.

## Encuentros y objetos

| Objeto      | Identificador | Contrato contrastado                                                          |
| ----------- | ------------- | ----------------------------------------------------------------------------- |
| Bandeja     | 167           | Recogida, inventario y respuestas del menú.                                   |
| Pala        | 168           | Recogida; Atacar en fase1 derriba al vampiro y crea cuatro fragmentos.        |
| Platillos   | 169           | Atacar en fase5, retirada y tres canales auxiliares.                          |
| Pan con ajo | 170           | Comer en fase3, aproximación y cinco fragmentos.                              |
| Aspiradora  | 171           | Recogida en el segundo fondo; su presencia interviene en el último encuentro. |
| Piedra      | 172           | Usar en fase2 la desplaza y permite avanzar.                                  |
| Cruz        | 173           | Atacar en fase4, retirada y siete fragmentos.                                 |

CS:1BA3–20CC controla los murciélagos, las partículas de las tumbas y las fases0–6. El primer vampiro cae al rodear la tumba vacía. Después siguen pala4555, piedra4605, pan4680, cruz4730 y platillos47D6. Los ataques y la comida conservan el consumo aunque el servicio de retirada informe de ausencia: es el comportamiento del EXE, sin requisitos añadidos.

La fase2 conserva un detalle del código original: al continuar por1EF4, AL contiene el byte bajo de447−cámara y no la fase anterior. El último vampiro aparece en77 cuando se lleva171 y la cámara alcanza la ventana8; sus secuencias abren la verja. BOXDET permite116 cuando2CD1 deja de ser22 y2C4C queda inactivo.

Las seis recogidas pasan por60E1–6140/615C: respetan capacidad, iconos de puntuación, baja del descriptor persistente y recogida baja de los modelos normal/20. Los movimientos y el sonido conservan sus servicios comunes.

## Colisiones y recuperación

El recorrido rodea las tumbas y los pequeños obstáculos de BOXDET; no cambia sus límites para facilitar la prueba. Se reconstruyen54F0–5638 para los tropiezos según orientación, conservando la diferencia de intervalo entre los identificadores9 y10 y el contrato compartido11. Los casos de esta ruta no acreditan todas las orientaciones ni todos los estados de11.

Hay atajos que producen una caída y solicitan113 en el original. Se conserva el enlace65→65 para la recuperación. Las pruebas de recorrido publicadas siguen el camino seguro; no se afirma haber agotado todas las maneras de morir.

## Comprobaciones

`pnpm comparar:cementerio --muestra` retoma la partida real del ataúd:17.568 casos/pasos,17 imágenes y14 estados con sus listas de acciones auditadas. Añade480 variantes directas de la rutina de vampiros y196 variantes de acciones/recogidas. Compara los65.536 bytes de DS y el estado del azar con el EXE. Las variantes de menú están incluidas en el recuento de selecciones; las480 llamadas directas se contabilizan aparte.

GRAVEYD.SPR ocupa112.144 bytes. El comparador sitúa el modelo DOS en9000 para separar los bancos de escena, STD, ratón y menú; verifica los intervalos ordenados por dirección. No normaliza diferencias de los actores.

`pnpm comprobar:cementerio` compara17 capturas SDL, comprueba restauración en memoria y134 cambios F5/F6 de las67 vistas. Los resultados locales quedan en `apps/juego/trabajo/comparacion_cementerio/` y `apps/juego/trabajo/comprobacion_cementerio_sdl/`.

La petición116 se conserva en una instantánea para seguir con el bosque del castillo. Cargar ese destino y resolver las habitaciones finales sigue pendiente. Véase el [recorrido propio](recorridos/29_el_cementerio.txt).

## Comer el ajo antes de cruzar114

La revisión ampliada21/10 reproduce una suspensión del original: desde `cementerio_piedra`, fase3 en escena65, Comer170 (casillas2/3/4) pasa a fase4, consume el ajo y prepara destino78/177.4680–472F comprueba fase, pero no exige escena77. En65 Brad termina en64/130, sin menú ni control; la planificación queda vacía y no devuelve la secuencia pendiente. Se conservan esas condiciones, sin convertir la acción en rechazo ni corregir el EXE dentro del port fiel.

`pnpm comparar:ajo-primera-zona` compara4.096 pasos contra el EXE, con DS completo, azar y fase. Exige la misma posición, destino, fase4, menú/control0 y canales de Brad/camino0 en512/1024/2048/4096; tres selecciones reales y ningún objeto o posición asignado por la prueba. Registra el resultado en `trabajo/comparacion_ajo_primera_zona/resultado.json`. No afirma un ciclo infinito: describe la suspensión observada durante ese plazo.

El informe amplio `resultado_65_cementerio_piedra_clc.json` se conserva con su aviso de plazo agotado,72 ramas/9.600 pasos/218 selecciones; no se reescribe para aparentar reposo.71 ramas alcanzan reposo, y la restante queda contrastada por la prueba selectiva anterior. La ruta positiva usa Comer170 después de114 en77, como indica la guía; sus fases posteriores y el regreso115 se comprueban por separado.
