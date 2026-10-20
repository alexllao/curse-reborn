# Música MT-32 por zona

El port interpreta los cinco recursos originales `CURSE.RS1`–`CURSE.RS5` mediante el alimentador reconstruido en `apps/juego/motor/sonido/musica_rs.cpp`. Munt sustituye al sintetizador físico. El juego no ejecuta instrucciones DOS para reproducirlos.

## Selección y paquetes originales

`4DD5–4E36` consulta DS:80CD, una tabla de 78 referencias, y los descriptores de cuatro bytes desde DS:80B9. Cada descriptor aporta el tempo y el nombre del banco. La presentación utiliza RS5; la mazmorra, el corredor y la caída reutilizan RS1 y conservan el avance al cambiar entre ellos. LAKEBED utiliza RS3. Cubrir las 78 referencias musicales no significa tener implementadas las 78 escenas.

| Banco     | Tempo | Contexto comprobado         |
| --------- | ----- | --------------------------- |
| CURSE.RS1 | 140   | Mazmorra, corredor y caída  |
| CURSE.RS2 | 135   | Referencia de escena 64     |
| CURSE.RS3 | 140   | Lago, escena 3              |
| CURSE.RS4 | 125   | Referencia de escena 15     |
| CURSE.RS5 | 110   | Core, título e introducción |

`7E3B–7EA1` suministra los paquetes al MPU-401: tiempo, mensajes MIDI con estado implícito, marcas y saltos absolutos FF. Se conservan los bytes originales para poder compararlos. La plataforma interpreta sus tiempos con 120 pulsos por negra y un acumulador racional a 48.000 Hz; las marcas F9 no son notas y F8 prolonga la espera.

## Salida de la introducción

`2F18–2F26` llama al fundido gráfico `77E4`. Al cargar una escena con otro banco, `4DF0` solicita parar mediante `7D5D`: DS:9955 señala la petición. La siguiente solicitud de pista envía DS:995E (`00 FC`, fin de pista); la recepción de FC ejecuta DS:995B (`00 05 FF`, parada MPU). La secuencia nueva configura tempo y pista y ordena B8/08. No hay una rampa de volumen musical en esta ruta.

La plataforma sustituye la parada MPU por All Notes Off en los 16 canales antes de cambiar de banco, sin destruir el sintetizador: las envolventes y la reverberación pueden dejar una cola natural. No se reproduce la latencia exacta de la petición de parada del MPU ni se ha medido la transición en un módulo físico. No añadir un fundido musical por intuición.

La carga de imágenes no debe vaciar ni detener el flujo musical. SDL solicita sus muestras desde su callback; los bancos están cargados previamente y el sintetizador no accede al DS de la partida. Los cambios de banco y perfil usan el bloqueo de ese flujo. Los efectos digitales Sound Blaster y PC Speaker siguen atendiendo el DS desde el hilo principal.

## ROM y dependencia local

Las ROM del propietario se conservan en `apps/juego/datos_originales/ROMS/MT32_CONTROL.ROM` y `MT32_PCM.ROM`, reconocidas por Munt. No se descargan, modifican ni distribuyen. Con ROM válidas, MT-32 es la selección inicial. F4 recorre sin música, AdLib OPL2, Sound Blaster 16 OPL3 y MT-32; M silencia la mezcla sin detener sus relojes. AdLib mantiene el tema continuo del EXE durante las escenas; MT-32 es la opción que utiliza bancos diferentes por zona.

[Munt](https://github.com/munt/munt) se fija a la revisión de `apps/juego/herramientas/dependencias_mt32.json`, con huella SHA-256 del archivo fuente. El preparador Windows descarga ese código oficial y compila una biblioteca compartida, en `apps/juego/trabajo/dependencias/mt32emu`. Usa CMake 3.31.6 y Ninja 1.11.1.4 en el entorno local. La revisión corresponde a la etiqueta Munt 2.8.2; su biblioteca declara versión 2.8.3. No se incorpora al historial código comercial ni las ROM.

La biblioteca mantiene LGPL 2.1 o posterior. Se carga dinámicamente como `mt32emu.dll` junto al ejecutable y se puede sustituir por otra compilación compatible del API C. Se copian los textos LGPL y GPL junto al ejecutable. La fuente exacta, su licencia y las instrucciones de compilación se obtienen del repositorio oficial y del preparador versionado; no se modifica el código de Munt. Esto no asigna licencia al resto del proyecto. La compilación manual CMake requiere sus cabeceras y una biblioteca compartida; macOS y WebAssembly siguen pendientes de validar.

El aviso de percusión no asignada procede del mapa rítmico del sintetizador: Munt lo emite si el timbre está marcado como desactivado o excede los timbres disponibles en el modelo. No detiene la pista. La inicialización MPU analizada envía FF/33 y órdenes de transporte; no se ha encontrado una carga SysEx de timbres en esa ruta. No sustituir notas por otras para ocultarlo. La verificación no garantiza equivalencia acústica con cada módulo o modelo Roland.

## Comprobaciones

- `pnpm comparar:musica-rs --muestra`: 2.565 paquetes ejecutados contra la rutina original con Unicorn, con coincidencia de DS completo, pila, estado MIDI y bytes de puerto; también comprueba las 78 referencias contra las tablas del EXE y los cinco enlaces circulares. La selección de escena se contrasta con sus datos; no ejecuta todo el cargador DOS.
- `pnpm comparar:musica-rs`: 13.130 paquetes coincidentes al recorrer cada banco hasta su primer enlace circular, con límite de 20.000 paquetes por banco. Comparación amplia ejecutada.
- `pnpm comprobar:mt32 --muestra`: añade síntesis de 32 segundos de cada banco con las ROM locales, exige señal y guarda `apps/juego/trabajo/comparacion_musica_rs/mazmorra_mt32.wav`. Comprueba también la permanencia del banco en escenas 1 y 2.
- La comprobación SDL de AdLib exige señal efectiva después de conectar la mazmorra y ejercita los perfiles, el silencio y la desconexión de escena; comprueba que el callback sigue produciendo muestras durante una espera de 150 ms del hilo principal. El dispositivo de pruebas es `dummy`: falta escuchar la continuidad en el equipo del propietario.

**Volumen:** `+` y `−` ajustan el nivel general en pasos del 10 %, entre 0 y 400 %. También funcionan `=` y las teclas del teclado numérico. La consola muestra el porcentaje al arrancar y al cambiarlo. `M` silencia y recupera el ajuste elegido; cambiar de perfil no lo reinicia.

La salida MT-32 conserva los dos canales de Munt a 48.000 Hz. Cada canal se amplifica por dos y se limita a ±28.000 para evitar desbordamientos; su nivel medio equivale a la mezcla mono previa, elevada 12 dB respecto a la primera integración. AdLib y PC Speaker conservan su ganancia. El dispositivo común es estéreo; SDL convierte los efectos mono y AdLib se duplica en ambos canales. La comprobación informa del pico y de las muestras limitadas de cada banco.

## Investigación de instrumentos y referencia de síntesis

Àlex confirma que el original del paquete DOSBox suena bien y que el port le resulta extraño. Las ROM locales se han identificado como `ctrl_mt32_1_07` y `pcm_mt32`, con SHA-1 de control `b083518fffb7f66b03c23b7eb4f868e62dc5a987`. El mapa rítmico de esa ROM, en 73FEh, contiene 127 (desactivado) para la nota 86; el banco original RS1 sí solicita esa nota con velocidad 65. Esto explica el aviso, pero no permite atribuirle toda la diferencia percibida.

Se han corregido dos diferencias de salida: la reducción de MT-32 a mono y el uso implícito de `COARSE`. La configuración aportada contiene `mt32.analog = 2`; la plataforma ahora fija explícitamente `ACCURATE`, mantiene DAC NICE, 32 parciales y las rampas de amplitud predeterminadas. Sigue generando a 48.000 Hz; el paquete DOSBox estaba configurado a 44.100 Hz. Ya se han capturado MIDI y WAV del DOSBox original en una copia local, con efectos desactivados para aislar la música; el propietario confirma que allí suena bien.

`pnpm comprobar:mt32 --muestra` también ejecuta `apps/juego/pruebas/audio/comparar_sintesis_mt32.py`: obtiene los paquetes de puerto de la rutina original con Unicorn, restituye MIDI con un decodificador separado y coloca sus eventos en tiempos absolutos del MPU. Alimenta las mismas ROM y biblioteca Munt, y compara los 32 segundos de mazmorra con la salida C++. Guarda `referencia_ensamblador_mt32.wav` e informe `sintesis.json`. La referencia comparte sintetizador y conversión de volumen, no emula un MPU completo ni acredita equivalencia con hardware o con el mezclador del DOSBox. No presentar este resultado como sustituto de la comprobación auditiva del propietario.

Resultado local tras conservar estéreo y fijar ACCURATE: **3.072.000 muestras idénticas** en 32 segundos de mazmorra; **1.454.256 de los 1.536.000 pares** tienen distinto valor izquierdo/derecho. No hubo muestras limitadas en los 32 segundos comprobados de ninguno de los cinco bancos. La mezcla SDL, los cambios de perfil y la continuidad durante carga también pasan su comprobación.

## Cola MIDI y contraste con el DOSBox original

Se ha corregido el uso de `mt32emu_play_msg_now`: el propio API advierte que sus órdenes pueden no tener efecto mientras se aborta una voz y que Note On seguido inmediatamente de Note Off queda siempre mudo. La plataforma ahora utiliza `mt32emu_play_msg`, conserva la cola y sus retrasos MIDI predeterminados y comprueba el código de retorno de cada envío. No ocultar un desbordamiento de cola ni sustituir notas para silenciar avisos.

La referencia PCM ahora reproduce ocho segundos de presentación antes de pasar a la mazmorra, con voces todavía activas. El oráculo y la plataforma deben coincidir también en esa transición persistente. La comparación PCM sigue compartiendo sintetizador y ROM; no sustituye la referencia externa.

El DOSBox AbandonSocios del propietario se ha ejecutado con los originales copiados a `apps/juego/trabajo/referencia_dosbox/CURSE`, conservando las ROM, los 3.000 ciclos, la síntesis y la frecuencia de 44.100 Hz del paquete. Solo la copia de CURSE.CFG desactiva efectos (campo nueve = dos) para aislar la música. La captura se ha realizado con sus funciones internas de MIDI y WAV; no se modifica el paquete del escritorio. Los archivos derivados permanecen en `apps/juego/trabajo/`.

`pnpm comparar:dosbox-midi` contrasta `apps/juego/trabajo/referencia_dosbox/capturas/curse_000.mid` con el alimentador original y el traslado. También admite `--captura ruta.mid`. La captura local contiene **231 mensajes de mazmorra coincidentes**, incluidos programas, notas y velocidades, con **desfase máximo de 2 ms** respecto al reloj reconstruido; tolerancia 3 ms por la resolución de captura y el procesamiento del emulador. El resultado se guarda en `apps/juego/trabajo/comparacion_musica_rs/dosbox_midi.json`. Esta prueba externa verifica el flujo MIDI, no los timbres.

Como diagnóstico local, la correlación de las envolventes de amplitud a 100 Hz contra el WAV real pasa aproximadamente de 0,864 con envío inmediato a 0,924 con cola, tras alinear la pista. Es una medida de envolvente, no un porcentaje de fidelidad acústica. Se mantiene pendiente la confirmación auditiva de Àlex sobre el resultado corregido. El aviso de la nota 86 puede seguir apareciendo: también está presente en el flujo MIDI original y su timbre está desactivado en la ROM 1.07.
