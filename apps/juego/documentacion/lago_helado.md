# El lago helado

ICELAKE/44, vista45, continúa la petición81 del rescate sin preparar inventario. La entrada utiliza SMALL20 y conserva131. CF26 limita el desplazamiento al llegar la criatura a la imagen34. Lanzar131 ejecuta B5D4–B68A: consume las cenizas, suma5, programa cinco actores y transporta automáticamente hasta solicitar82. Saltar sobre135 utilizaB579, suma2 y devuelve el control conservando131; se contrasta como rama separada de la misma partida.

`pnpm comparar:lago-helado` compara2475 casos/pasos, seis composiciones, menús de entrada y orilla,15 variantesCF26 y variantes de ambos manejadores con inventario ausente/presente, puntuación envolvente y campos diferidos centinela. La rama del salto usa entrada real y512 pasos; conserva cenizas y control. La partida principal llega a82 sin asignar ningún objeto.

`pnpm comprobar:lago-helado` ejecuta la partida desde el inicio y reproduce entrada81, control, orilla, salto recuperado, cenizas y criatura a64 pasos. Las seis imágenes de64000 píxeles, guardar/restaurar y90 cambios entre45 vistas coinciden. La imagen inmediatamente después de la carga81 también se contrasta. El palacio tras82 todavía se integra por separado; no se acredita su puzle por haber alcanzado la petición.

Los archivos originales y los resultados quedan fuera del historial. Se conservan sonido, contador, marcos y monitor; los decorados ampliados siguen aparcados.

## Revisión ampliada de consecuencias del21 de octubre

Regenerado `pnpm comparar:lago-helado --muestra`:2.475 casos/pasos,218 selecciones y seis composiciones coincidentes. Las siete DS previas mantienen su SHA256 y guardan contexto5800/fase0 real, incluida ahora expresamente lago_solicitud82 sin otra imagen ni cargar45 al guardarla.

Las alternativas de lago_ante_cenizas y lago_salto135 suman67 observaciones,8.631 pasos,26 ramas CLC y205 selecciones coincidentes en DS/azar/fase.65 llegan a reposo con menú; lanzar131 por casillas0-based2/5/2 solicita82 al paso149 desde la orilla y al162 tras el salto. Se termina la observación en la petición. El salto sigue conservando131; no se fabrica otro inventario para el segundo estado.

Comandos, informes separados y cobertura actual en [auditoría vigente](auditoria_pantallas.md). No se agotan las posiciones, fases de la criatura ni los inventarios posteriores. Los pendientes de catálogo de las tandas iniciales son históricos; no hay cambios del motor ni capturas SDL nuevas en esta ampliación.
