# Menú, primeras interacciones y depuración

La mazmorra ya permite abrir el menú original, navegar por sus submenús y ejecutar las primeras interacciones trasladadas. El motor común prepara listas, inventario, destinos y canales; la lógica de cada puzle sigue necesitando su traslado. La [integración posterior de movimiento](movimiento_y_colisiones.md) permite caminar con clic y flechas tras la liberación, con BOXDET; todavía no hay una mazmorra completa. Las acciones pendientes se conservan sin aplicar un efecto aproximado.

## Uso

```sh
pnpm ventana
```

En la mazmorra, **Escape abre/cierra el menú**, como en el recorrido original `0706`–`075C`. El botón derecho ofrece el mismo acceso como adaptación SDL. El menú espera a que el guion permita usarlo (`DS:3595`). Fuera de la mazmorra, Escape conserva el cierre de la ventana; en la mazmorra se cierra con el botón de la ventana.

El ratón señala una de las diez casillas y el botón izquierdo la ejecuta. También funcionan izquierda/derecha y Enter. Se conserva la posición superior o inferior que el original elige según la mitad de pantalla en que se abre. Las coordenadas del ratón pasan por la transformación lógica de SDL, incluidas escala y bandas negras.

Para iniciar la primera secuencia: abre el menú, elige **el bocadillo de conversación, quinto icono**, y después **la segunda casilla**, correspondiente al tema original cero. El estado y las animaciones avanzan desde los guiones del EXE. No hay audio todavía. El primer tema, uno, solo solicita sonido en esta escena bajo la condición inicial; con sonido desactivado no produce una reacción audible.

También están trasladadas la recogida de los objetos originales uno y tres y la combinación de los identificadores cero y uno, con los contratos originales de inventario e inicio. Su disponibilidad la decide la proximidad y el estado originales: no se añaden objetos ni se completa un puzle mediante los atajos.

## Rutinas

| Original                       | Traslado y alcance                                                                                                                        |
| ------------------------------ | ----------------------------------------------------------------------------------------------------------------------------------------- |
| `DB02`–`DB4B`                  | Limpieza de nodos temporales, listas y ubicación del panel.                                                                               |
| `DE96`–`DEF4`                  | Diez entradas iniciales; bloqueo de recoger y temas mientras existe secuencia del personaje.                                              |
| `DFD3`–`E15F`                  | Inventario, objetos cercanos, acciones secundarias, temas y opciones. Se reutilizan `DF15` y `891E`.                                      |
| `E07F`, `3B53` y `3D10`–`3D2E` | Selección de objetos y primera combinación cero/uno. Los demás efectos específicos quedan pendientes.                                     |
| `32DE`–`337A`                  | Tema cero de la escena cero: programa los canales de la secuencia inicial.                                                                |
| `567E`–`56C8`                  | Recogida de objetos uno y tres, precedida de la inserción original en inventario.                                                         |
| `989E`–`98FF`                  | Combinación inicial, cambios de guiones, puntuación y fases según orientación.                                                            |
| `096D`–`0A58`                  | Pasos de movimiento de acciones, nodos enlazados y activación de sus canales al llegar. No decide caminos libres ni sustituye colisiones. |
| `DEFB`–`DFCF`                  | Fondo `MENU.DAT` mediante RNC1 y sprites de `ICONS.SPR`; los objetos consultan `DS:524E`.                                                 |
| `DB99`–`DBF5`                  | Marco de selección de 26×26, color 255.                                                                                                   |

El banco de iconos tiene más de 128 entradas; su dibujo usa la referencia en párrafos de su propia tabla, sin el límite de siete bits de los fotogramas de entidades. Las máscaras de 24×24 se leen de `DS:BC10`. Los canales de estado del cursor también necesitan `MOUSE.SPR`, aunque la flecha visible sigue siendo la del sistema. Las imágenes comerciales permanecen locales.

El menú detiene el avance de la escena. Se recompone en memoria lineal, sustituyendo las copias de páginas y las aperturas/cierres deslizantes del original. El marco usa la tabla original de 24 colores; su cadencia se aproxima con el reloj SDL. Las transiciones deslizantes del panel, el cursor gráfico original, la integración de las demás acciones, guardado/carga y las transiciones a otras escenas siguen pendientes. No se presenta la interfaz entera como temporalmente idéntica al original.

## Depuración

Está desactivada al iniciar. Los interruptores se conservan al recorrer las vistas de desarrollo; solo dibujan en la mazmorra.

| Tecla   | Categoría                                      | Colores                                                                                                            |
| ------- | ---------------------------------------------- | ------------------------------------------------------------------------------------------------------------------ |
| **F7**  | Activa/desactiva toda la capa.                 | Conserva las categorías elegidas.                                                                                  |
| **F8**  | Alterna rectángulos de entidades.              | Verde: entidades activas. Cian: personaje.                                                                         |
| **F9**  | Alterna zonas de disponibilidad.               | Magenta: objetos con afinidad de acción y margen vertical. Naranja: banda de proximidad de los pies del personaje. |
| **F10** | Alterna encuadre y umbrales de desplazamiento. | Amarillo: borde 320×200 y campos `DS:358D/358F`.                                                                   |

F8–F10 activan también la capa general para hacer visible el cambio. Los rectángulos leen posición, tamaño y campos reales de las entidades. Las zonas magenta/naranja corresponden a la selección de proximidad de `891E`, **no a colisiones físicas ni a la detección píxel a píxel del ratón**. F10 también muestra ahora los rectángulos originales de BOXDET: paredes amarillas y salidas azules; se documentan en [movimiento_y_colisiones.md](movimiento_y_colisiones.md). No se inventa una geometría para sustituir esos datos.

La capa se dibuja después de la textura, en coordenadas lógicas y con colores RGB fijos. No escribe en el estado, inventario, paleta ni imagen indexada del motor; desactivarla recupera la vista habitual. No añade filtros a la imagen original.

## Comprobaciones

```sh
pnpm comparar:menu
pnpm comprobar:menu
```

La muestra comprobada contiene **91 casos/pasos**: 38 selecciones, cinco paneles, dos fondos, dos composiciones de interacción, 32 pasos persistentes, cuatro combinaciones y ocho activaciones de canales. La referencia ejecuta los manejadores originales y compara DS completo. El dibujo ejecuta `DEFB`/`DE96` con el modelo VGA contrastado; los pasos persistentes incluyen los cambios de banco solicitados por F8 y consumen los mismos archivos originales.

La comparación amplia de **307 casos/pasos**, con 192 pasos de interacción y 64 activaciones, también se ha ejecutado y coincide. Los resultados están en `apps/juego/trabajo/comparacion_menu/`.

La comprobación SDL contrasta 64.000 píxeles del panel sobre la escena y otros 64.000 tras ejecutar la primera orden y avanzar 32 pasos. Una tercera captura verifica que las cinco categorías de color de depuración se dibujan. Los resultados quedan en `apps/juego/trabajo/comprobacion_menu_sdl/`. El ratón en una ventana visible maximizada, las teclas de depuración y las transiciones se comprueban manualmente.

### Corrección de los bancos de referencia

La primera comparación amplia detectó diferencias en `DS:2D18`–`DS:2D1B` en el paso 90. El comparador colocaba `DUNGEON.SPR` en `40000h` y el modelo en `50000h`; el banco de escena mide 67.504 bytes y el modelo sobrescribía sus últimos 1.968 bytes. La muestra de 32 pasos no alcanzaba los fotogramas afectados. El motor SDL mantiene estos recursos en vectores independientes y no tenía ese solapamiento.

La referencia reserva ahora regiones separadas: escena `40000h`, modelo `54000h`, estándar `64000h`, ratón `74000h` y fondo de menú `76000h`. Las etiquetas del puente nativo se ajustan a esa distribución. Antes de avanzar se comprueba que ningún banco invade el siguiente y que los bytes de todos siguen intactos; los errores de avance indican paso y valores de cada diferencia. El recorrido amplio ya contrasta los 192 pasos, incluido el 90, sin modificar ni ignorar los campos que fallaban.

El margen se ha ampliado para admitir `LAKEBED.SPR` (75.424 bytes), que tampoco cabe en 64 KiB. La geometría de estas comparaciones utiliza `7A000h`. La separación se verifica antes de cada paso.
