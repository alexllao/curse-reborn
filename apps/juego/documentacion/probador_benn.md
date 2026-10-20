# El probador de Benn

La escena25, CHANGE, se alcanza desde Benn24 por la salida36 y regresa por27. Su entrada normal, entidades, colisiones, sonido común, regreso y restauración DOS están integrados. F5/F6 incluye la vista30, «El probador de Benn», sin transferir inventario entre sesiones de desarrollo.

## Rutinas originales

1A74–1BA1 crea siete entidades cuando el actor alcanza la postura12. Durante CHANGE.BRD (modelo19), el fotograma de Brad y el byte CS:1BA2 deciden si emitir16 o22 partículas. Se conserva el orden de creación, las escrituras de los descriptores y todos los consumos de azar, incluso sin registros libres.

CS:1BA2 se almacena explícitamente fuera del segmento DS, mediante RecursosEntidades y un byte por instancia de Mazmorra. Empieza a cero con una sesión nueva y se conserva entre pasos y cargas DOS. Los guardados originales no incluyen ese byte; no se amplía CURSE.SAV ni se inventa un lugar dentro de él para persistirlo.

La recogida del vestido79 sigue563A–5679 y5CAF–5CF3, incluido rechazo por inventario lleno. Llevar79 despacha3CDF/430F, rechaza fuera del probador y prepara sus canales, retirada de inventario y puntuación. Las demoras se calculan desde AL de886B; no son tiempos fijos añadidos por SDL.

## Comprobación y límites de la integración inicial

`pnpm comparar:probador --muestra` contrasta DS completo, generador, decisiones y CS:1BA2 con las instrucciones del EXE. Parte de la entrada real de Benn de comparar:tiendas, camina hasta CHANGE, espera y regresa. Audita los menús ofrecidos en dos estados y compara sus dos composiciones.

La tanda ejecutada suma **3.025 casos y pasos**, **215 selecciones**, **dos estados auditados** y **dos composiciones**.

Además compara80 variantes de1A74 (modelo normal/19, fotogramas0–4, fase0/1, postura0/12 y registros libres/agotados), recogida con capacidad e iconos distintos, rechazo de llevar fuera de25 y512 pasos posteriores a llevar79 en una copia aislada. Esa copia aporta el objeto solo para verificar el manejador: no representa haber completado el capítulo del valle ni forma parte de la partida persistente.

`pnpm comprobar:probador` prepara las referencias con el mismo orden inicial de dibujo, compara entrada y regreso en SDL y recorre las30 vistas F5/F6, incluida restauración en memoria del probador. No toca perfiles ni partidas del usuario. Los resultados quedan en apps/juego/trabajo/comparacion_probador/ y apps/juego/trabajo/comprobacion_probador_sdl/.

La visita temprana no fabrica la oferta de Benn, el vestido ni las condiciones del capítulo posterior. La salida74 desde CHANGE hacia la escena43 y los destinos posteriores siguen fuera del alcance integrado. EDGE30 continúa detenido hasta reconstruir y contrastar C093. Esta tanda no declara el puzle entero del disfraz ni todos los estados del juego completos.

## Continuación posterior al valle

El vestido ya se obtiene en la misma partida que vuelve con el ventilador124: pagar aBenn, recoger79, entrar36, vestir y solicitar74. La comparación nueva distingue este recorrido de las variantes aisladas preparadas anteriormente. Incluye10.109 casos/pasos nativos, siete composiciones y seis capturasSDL con guardar/restaurar. No carga la sala43 ni aumenta las40 vistas. Alcance detallado en [regreso aBenn](regreso_benn.md).

## Revisión integral posterior

El catálogo actual ya incluye75 escenas reales y el desenlace; los destinos pendientes citados arriba describen la integración histórica. La revisión adicional contrasta91 observaciones STC/CLC en tres estados reales de25,11.648 pasos y el banco y CS:1BA2 externos. Se registra el contexto propio de cada referencia y se comprueba también continuar128 pasos desde una fase1 real de la animación hasta fase0/reposo. No modifica el formato CURSE.SAV. El [alcance de la auditoría](auditoria_pantallas.md) detalla comandos, informes y límites: estas muestras no agotan todas las interacciones.
