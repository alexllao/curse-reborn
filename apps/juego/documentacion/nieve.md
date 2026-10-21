# Las cuatro zonas nevadas

La continuación se obtiene con el vestido real de Benn, salida74 y SNOWAST4/43. El puzle de134 ya está disponible en SDL. El recorrido de SNOWAST3/42, SNOWAST2/41 y SNOWAST1/40 se contrasta en una única partida: desodorante132 y Llevar, saludo a la morsa, tablón125, agujero y pez128, entrega a126, caña127, amigo del hielo, cenizas131 y regreso a la cornisa. Los nombres de desarrollo son propios del port.

Las rutinas CBD0–CF25 pasan a apps/juego/motor/entidades/nieve.cpp. Conservan sus comparaciones firmadas y sin signo, los límites con envoltura y los campos que no escriben. Los manejadores B1F8–B576 y5EAB–5F64 viven en apps/juego/motor/acciones/nieve.cpp y utilizan los servicios comunes de sonido, inventario y planificación. No se añaden diálogos escritos ni condiciones de la guía que no estén en el EXE.

El desodorante conserva OR128 en35C9. La entrega exige esa bandera y la postura17 del esquimal; prepara distintos rodeos según la posición del personaje. El tablón y los personajes conservan las mutaciones de sus descriptores para los regresos. La caña se consume al romperse, aparece131 y6323 vuelve a ofrecer el rescate solo después de las liberaciones necesarias. El saludo correcto a la morsa es el tema1, casilla0 del submenú de voz; la ayuda es el tema0, casilla1. Se comprueban sus efectos persistentes además de las coincidencias de memoria.

El rescate requiere alcanzar la cornisa. Desde la aproximación165/114, la entrada original baja8 pasos, va a la izquierda4 y sube16;CBD0 activa la subida y128 pasos sin dirección completan el movimiento. Después, pedir ayuda inicia34D9 y el guion solicita81 haciaICELAKE/44. La llegada al lago se valida en un bloque posterior.

## Límite explícito de la comparación F6

F6 escribe temporalmente el identificador de salida como imagen antes de cargar otra habitación. Algunas cabeceras quedan fuera del archivoBRD. DOS puede leer entonces memoria ajena al recurso, incluida la envoltura deES; la API portable acotada utiliza0/0 para esa imagen que no llega a dibujarse. Este comportamiento ya estaba documentado en secuencias.cpp.

El comparador registra la ejecución de85DB. Solo cuando se solicita efectivamente la transiciónF6 y el lector acotado confirma que la cabecera no existe, normaliza los cuatro bytes de dimensiones del estado esperado a cero. Todos los demás bytes, azar, resultado, siguiente carga y composición se comparan con el original. Las imágenes normales fuera de rango siguen siendo errores. No se atribuye identidad a esos cuatro bytes de RAM indeterminada ni se oculta una diferencia de fotograma visible.

## Comprobaciones y alcance

`pnpm comparar:morsa` recorre las cuatro zonas desde la liberación real del ser azul. Incluye auditorías de las listas disponibles antes y después de cambios persistentes, variantes aisladas de las rutinas y de los rodeos, y composiciones originales. Los resultados locales quedan en apps/juego/trabajo/comparacion_morsa. No se versionan recursos, instantáneas ni imágenes comerciales.

`pnpm comprobar:morsa` verifica enSDL la entrada80, el desodorante, su uso y el saludo a la morsa, con seis imágenes y guardar/restaurar. La liberación posterior del hielo y los puzles de las otras zonas tienen su continuación nativa; sus comprobacionesSDL se añaden al habilitar sus conexiones y vistas. El accesoF5/F6 sigue creando una sesión independiente.

`pnpm comprobar:rescate-nieve` completa las comprobacionesSDL de la vista44 y de los regresos de41/42: catorce imágenes adicionales, restauración en cada hito y88 cambiosF5/F6. Sumadas a las seis de morsa y la entrada del esquimal, son21 imágenes del recorrido posterior a la bola. SNOWAST1.MAP utiliza SNOWAST3.SPR; el cargadorSDL lee ambos nombres de la tabla784E para conservar esa asociación original. El último hito termina justo después de pedir ayuda: el destino81/ICELAKE aún se integra por separado.

## Revisión ampliada de consecuencias del21 de octubre

Regenerados ser-azul (3.049 casos/pasos,732 selecciones, cinco composiciones) y morsa (19.841/2.796/21), con `--muestra`. Morsa conserva7.877 pasos jugables y las mismas DS; la diferencia frente al informe anterior19.706/2.661 corresponde a135 selecciones contabilizadas, no más escenas ni nuevos pasos jugables. Se mantiene el límiteF6 documentado.

Las31 DS locales previas siguen idénticas porSHA256;27 corresponden a los hitos reales de ambos generadores y guardan contexto5800/fase0. Se añade expresamente el contexto de rescate_solicitud81 después de comprobar la petición, sin otra imagen ni una carga del lago. Las cuatro copias investigacion_cornisa2–5 conservan su falta de contexto y quedan fuera de esta revisión.

Las consecuencias se observan en nieve_bola_recogida43, morsa_ante_saludo42, esquimal_entrada41, rescate_ante_tablon40 y morsa_cenizas42. Suman187 observaciones,23.936 pasos,90 ramas CLC y677 selecciones coincidentes en DS/azar/fase; todas llegan a reposo con menú en128 pasos. Los recorridos positivos mantienen además512 pasos del ser azul y1024 del hielo. No se agotan la entrega del pez en otras posturas, los estados de las liberaciones, las condiciones del rescate o todos los inventarios tardíos.

La [auditoría vigente](auditoria_pantallas.md) contiene los comandos, informes separados y cobertura actual. Las limitaciones antiguas sobre catálogo SDL de las tandas iniciales son históricas; esta ampliación no modifica el motor ni añade capturas SDL.
