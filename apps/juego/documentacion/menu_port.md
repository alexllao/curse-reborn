# Menú del port

Menú propio superpuesto a la habitación, fuera del monitor. Reutiliza el logo existente de Reborn y dibuja paneles azul noche, botones y molduras doradas mediante SDL. No sustituye los iconos, inventario, fichas ni partidas del menú original.

## Arranque y partida

Tras el banner aparece la habitación con el ordenador apagado y el menú abierto. Se elimina el encendido automático a los dos segundos. **Jugar** enciende el PC y conserva BIOS/DOS y la presentación original. **Cargar partida** enciende igualmente el equipo y, tras el arranque, abre las ocho ranuras originales directamente. Cancelar esa carga deja una sesión nueva en la mazmorra. Durante una sesión, Jugar cambia a **Continuar** y Cargar abre las ranuras sobre la sesión existente.

Durante la partida o la presentación, **Mayús + Esc** abre/cierra el menú del port. El botón **Menú** en la esquina superior derecha también permite abrirlo con ratón. Se muestra el escritorio completo y se recupera el zoom anterior al continuar. Mientras el ordenador está arrancando se mantienen únicamente los controles de arranque existentes.

El menú pausa la lógica, los fundidos, la cámara, la introducción, el tiempo efectivo y el dispositivo de audio. Se liberan las direcciones retenidas. Al continuar se compensan los relojes absolutos y se conserva la fase sonora, sin reiniciar la canción ni consumir de golpe el tiempo de la pausa. **Esc sin Mayús y el clic derecho mantienen el menú original del juego.**

## Navegación

- Ratón: pasar por una fila la selecciona; clic izquierdo la activa. En un ajuste, la mitad izquierda reduce/retrocede y la derecha aumenta/avanza.
- Arriba/abajo y Tab/Mayús + Tab: recorrer filas, con vuelta entre extremos.
- Intro, espacio o Intro numérico: aceptar.
- Izquierda/derecha: cambiar el valor seleccionado.
- Esc: volver al apartado superior; desde la portada permite continuar si ya hay sesión. Al arrancar conserva el menú abierto.
- Alt + Intro: alternar pantalla completa, también con el panel abierto.

No se entregan al juego los clics o teclas usados en el menú. Salir tiene una pantalla de confirmación.

## Apartados

**Sonido:** volumen 0–400 %, silencio, perfiles musicales disponibles, Sound Blaster/PC Speaker y estéreo posicional. La selección musical omite los dispositivos que no pueden prepararse.

**Pantalla:** pantalla completa, los seis filtros existentes y Cielo encantado. El filtro conserva su comportamiento previo: se aplica al juego ampliado, sin filtrar la habitación, el marco ni este menú.

**Habitación:** recorre solo los diseños presentes en el catálogo compilado. Un paquete con una única habitación ofrece únicamente esa.

**Avanzado:** depuración y cajas de entidades, interacciones y límites. Estos interruptores son de sesión.

**Controles** y **Créditos:** se presentan en el idioma elegido al arrancar (`pnpm ventana es` o `pnpm ventana en`). Los créditos muestran la versión efectiva del paquete y la fecha real de compilación.

## Preferencias y recursos

Se conservan volumen, silencio, música, estéreo, filtro y marco; se añaden efectos digitales, pantalla completa e identificador de habitación. El perfil personal prevalece sobre `configuracion/inicial.json`. Los perfiles antiguos se admiten y los formatos desconocidos permanecen intactos. Partidas y recursos comerciales no se modifican.

El logo existente `lanzador/presentacion/curs-of-enchantia-reborn-logo.png` se copia a la compilación y al paquete y pasa por las mismas comprobaciones de integridad que las demás imágenes. No se generan nuevos fondos. La tipografía usa Georgia del sistema mediante GDI en Windows, sin distribuir el archivo de fuente ni exigir SDL_ttf; las demás plataformas tienen un respaldo SDL. La capa se compone a 900 líneas antes de escalar para conservar los detalles en ventanas pequeñas.

## Comprobaciones sin ventanas ni audio audible

```powershell
$env:SDL_AUDIODRIVER='dummy'
curse-reborn.exe apps/juego/datos_originales/CURSE/DATA --comprobar-menu-port apps/juego/trabajo/menu_port
curse-reborn.exe apps/juego/datos_originales/CURSE/DATA --comprobar-menu-port-integracion apps/juego/trabajo/menu_port/integracion
$env:CURSE_REBORN_PARTIDAS='apps/juego/trabajo/menu_port/ranuras_prueba.sav'
curse-reborn.exe apps/juego/datos_originales/CURSE/DATA --comprobar-menu-port-carga apps/juego/trabajo/menu_port/carga
```

La primera prueba recorre teclado/ratón, persistencia y seis proporciones en ambos idiomas, además de seis páginas internas: 18 capturas. Comprueba que el PC permanece apagado y que el dibujante recupera escala, viewport y presentación lógica. Las otras dos recorren el bucle real: jugar/cargar, BIOS/DOS completos, apertura del menú, DS y muestras musicales detenidos durante 500 ms, volumen, reanudación y menú DOS. Los perfiles se deshabilitan en estas pruebas y las evidencias locales quedan bajo trabajo. La variante de carga abre/cancela las ranuras; no acredita cargar una ranura concreta adicional a las pruebas originales de partidas.

## Resultado del 22/10/2026

Las tres pruebas del menú pasan: 18 capturas y los dos recorridos de entrada. Pasan también los 120 casos de preferencias anteriores, estéreo, efectos SDL, menú original, quinta ficha de información y arranque oculto. Compilación O2 con avisos como errores, formato directo de 225 C++, Ruff (102 Python), ESLint, Prettier, monorepo y seis pruebas del builder correctos.

`pnpm calidad` no completa el agregado en este entorno: pnpm intenta comprobar/instalar dependencias, falla la consulta del registro y aborta sin TTY antes de ejecutar los controles. Se conservan los resultados directos, sin declarar aprobado el agregado. No se ha generado un ZIP nuevo. Evidencia reproducible en `trabajo/menu_port/`, fuera del repositorio.
