# La sala de los dados

ICEROOM/46, vista47, continúa la puerta83 del palacio. Se conserva la escoba141 y el puzle de los dados140. Recogerlos cierra la puerta de paso hasta Y89; lanzarlos vuelve a abrirla hasta Y30 y decide las marcas de las puertas de los corredores con el generador compartido de la partida. El límite CFA9 recorta únicamente cuando está fuera de30..89 y detiene el canal vertical al hacerlo.

La acción reutiliza ejecutar_accion_resultados, que ya reconstruyeB740–B939 y el salto común886B. No hay otro generador ni una tirada preparada para abrir puertas concretas. La segunda tirada real vuelve a coger140 acercándose a la mesa, cierra la puerta y la abre con el avance del mismo azar. La rama de comparación se restaura antes de continuar la partida principal hacia84.

`pnpm comparar:dados-palacio` contrasta5607 casos/pasos, nueve composiciones, seis auditorías y681 selecciones. Incluye diez límitesCFA9,36 variantes por recogida y24 semillas del selector completo. El contraste preexistente del bloque aleatorio se conserva como referencia independiente.

`pnpm comprobar:dados-palacio` verifica las nueve imágenes de64000 píxeles, guardar/restaurar y94 cambiosF5/F6 entre47 vistas: entrada, control, escoba, mesa, dados, primera tirada, nueva recogida y segunda tirada. La continuidad principal solicita84; sus corredores y puzles posteriores se incorporan por separado. No se presenta esta sala como el palacio completo.

## Revisión ampliada de consecuencias del21 de octubre

Regenerado `pnpm comparar:dados-palacio --muestra`:5.607 casos/pasos,681 selecciones y nueve composiciones coincidentes. Las diez DS anteriores mantienen su SHA256 y guardan contexto5800/fase0 real; se conserva ahora expresamente el contexto de dados_solicitud84, sin otra imagen ni cargar el corredor al guardar la petición.

Las alternativas de dados_recogidos y dados_segunda_tirada suman55 observaciones,7.040 pasos,28 ramas CLC y210 selecciones coincidentes en DS/azar/fase; todas terminan en reposo con menú en128 pasos. Se conservan la puerta cerrada tras recoger140 y su reapertura con otra tirada y avance real del azar, sin fijar marcas. El generador mantiene además512 pasos de cada tirada; las24 semillas aisladas no equivalen a24 partidas recorridas.

Comandos, informes separados y cobertura actual en [auditoría vigente](auditoria_pantallas.md). No se agotan todas las tiradas, marcas, inventarios posteriores o regresos. Los pendientes de catálogo de las tandas iniciales son históricos; no hay cambios del motor ni capturas SDL nuevas en esta ampliación.
