# Música MT-32 por zona

El port interpreta los cinco recursos originales `CURSE.RS1`–`CURSE.RS5` mediante el alimentador reconstruido en `motor/sonido/musica_rs.cpp`. Munt sustituye al sintetizador físico. El juego no ejecuta instrucciones DOS para reproducirlos.

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

La carga de imágenes no debe vaciar ni detener el flujo musical. SDL solicita sus muestras desde su callback; los bancos están cargados previamente y el sintetizador no accede al DS de la partida. Los cambios de banco y perfil usan el bloqueo de ese flujo. Los efectos PC Speaker siguen atendiendo el DS desde el hilo principal.

## ROM y dependencia local

Las ROM del propietario se conservan en `datos_originales/ROMS/MT32_CONTROL.ROM` y `MT32_PCM.ROM`, reconocidas por Munt. No se descargan, modifican ni distribuyen. Con ROM válidas, MT-32 es la selección inicial. F4 recorre PC Speaker, AdLib y MT-32; M silencia la mezcla sin detener sus relojes. AdLib mantiene el tema continuo del EXE durante las escenas; MT-32 es la opción que utiliza bancos diferentes por zona.

[Munt](https://github.com/munt/munt) se fija a la revisión de `herramientas/dependencias_mt32.json`, con huella SHA-256 del archivo fuente. El preparador Windows descarga ese código oficial y compila una biblioteca compartida, en `trabajo/dependencias/mt32emu`. Usa CMake 3.31.6 y Ninja 1.11.1.4 en el entorno local. La revisión corresponde a la etiqueta Munt 2.8.2; su biblioteca declara versión 2.8.3. No se incorpora al historial código comercial ni las ROM.

La biblioteca mantiene LGPL 2.1 o posterior. Se carga dinámicamente como `mt32emu.dll` junto al ejecutable y se puede sustituir por otra compilación compatible del API C. Se copian los textos LGPL y GPL junto al ejecutable. La fuente exacta, su licencia y las instrucciones de compilación se obtienen del repositorio oficial y del preparador versionado; no se modifica el código de Munt. Esto no asigna licencia al resto del proyecto. La compilación manual CMake requiere sus cabeceras y una biblioteca compartida; macOS y WebAssembly siguen pendientes de validar.

El aviso de percusión no asignada procede del mapa rítmico del sintetizador: Munt lo emite si el timbre está marcado como desactivado o excede los timbres disponibles en el modelo. No detiene la pista. La inicialización MPU analizada envía FF/33 y órdenes de transporte; no se ha encontrado una carga SysEx de timbres en esa ruta. No sustituir notas por otras para ocultarlo. La verificación no garantiza equivalencia acústica con cada módulo o modelo Roland.

## Comprobaciones

- `pnpm comparar:musica-rs --muestra`: 2.565 paquetes ejecutados contra la rutina original con Unicorn, con coincidencia de DS completo, pila, estado MIDI y bytes de puerto; también comprueba las 78 referencias contra las tablas del EXE y los cinco enlaces circulares. La selección de escena se contrasta con sus datos; no ejecuta todo el cargador DOS.
- `pnpm comparar:musica-rs`: 13.130 paquetes coincidentes al recorrer cada banco hasta su primer enlace circular, con límite de 20.000 paquetes por banco. Comparación amplia ejecutada.
- `pnpm comprobar:mt32 --muestra`: añade síntesis de ocho segundos de cada banco con las ROM locales, exige señal y guarda `trabajo/comparacion_musica_rs/mazmorra_mt32.wav`. Comprueba también la permanencia del banco en escenas 1 y 2.
- La comprobación SDL de AdLib exige señal efectiva después de conectar la mazmorra y ejercita los perfiles, el silencio y la desconexión de escena; comprueba que el callback sigue produciendo muestras durante una espera de 150 ms del hilo principal. El dispositivo de pruebas es `dummy`: falta escuchar la continuidad en el equipo del propietario.

**Volumen:** `+` y `−` ajustan el nivel general en pasos del 10 %, entre 0 y 100 %. También funcionan `=` y las teclas del teclado numérico. La consola muestra el porcentaje al arrancar y al cambiarlo. `M` silencia y recupera el ajuste elegido; cambiar de perfil no lo reinicia.

La mezcla mono MT-32 eleva su nivel 12 dB respecto a la primera integración, sin modificar notas, temporización ni la ganancia de AdLib o del PC Speaker. Se limita a ±28.000 antes de convertir a PCM de 16 bits para evitar desbordamientos y dejar margen para los efectos. La comprobación informa del pico y de las muestras limitadas de cada banco.
