# Estéreo posicional de efectos

Activado por defecto con `estereo_posicional: true` en `apps/juego/configuracion/inicial.json`. La tecla **P** alterna la opción, muestra un aviso y guarda la elección en el perfil del usuario. No intercepta la escritura de nombres en el panel de partidas. Los perfiles antiguos conservan el valor inicial; la preferencia personal tiene prioridad sobre la configuración distribuida.

Los efectos digitales y PC Speaker se generan con sus muestras, duraciones, prioridades y reloj originales. Después se convierten de mono a estéreo de 48 kHz según la entidad que realmente ha iniciado el sonido. Las voces de Brad siguen a Brad; las demás entidades conservan su propio origen. Una petición rechazada por prioridad no cambia el origen del efecto activo. La música mantiene su estéreo actual.

Se usa el centro horizontal del emisor en la superficie lógica de 320 píxeles. Las coordenadas de las entidades ya incluyen el desplazamiento de cámara: no se resta otra vez. El marco, las bandas y el tamaño de ventana no alteran el balance. Un origen sin registro de entidad válido se reproduce centrado.

En el centro ambos canales conservan la amplitud mono. Hacia cada extremo se atenúa progresivamente el canal opuesto hasta el 35 %, sin amplificar el otro ni añadir saturación. La energía conjunta disminuye moderadamente hacia los extremos. Una rampa de hasta cinco milisegundos suaviza movimientos y cambios de opción. Al desactivar, el balance vuelve al centro y conserva allí el mono original.

## Consola

Al arrancar y pulsar P:

```text
[partida] Estéreo posicional: activado (P para cambiar).
```

Cada efecto que realmente comienza registra su identificador, entidad, posición y ganancia inicial por canal. El balance sigue después a la entidad sin llenar la consola en cada fotograma:

```text
[partida] SFX 0: emisor=2C2B (Brad); x=16.0; estéreo=posicional; L=100% R=42%.
```

## Comprobaciones reproducibles

Con el ejecutable SDL compilado:

```text
curse-reborn.exe apps/juego/datos_originales/CURSE/DATA --comprobar-estereo apps/juego/trabajo/estereo_posicional/resultado.json
curse-reborn.exe apps/juego/datos_originales/CURSE/DATA --comprobar-preferencias apps/juego/trabajo/estereo_posicional/preferencias
```

La primera prueba usa PCM real de STD.SFX sin abrir ventanas ni dispositivos de audio. Comprueba cinco posiciones, prioridad rechazada, canal dominante, ausencia de amplificación, mono exacto desactivado, origen desconocido centrado, rampa independiente del tamaño de bloque, cámara y segmento de juego intacto. Escribe un informe y una muestra WAV local izquierda/centro/derecha; ninguno se versiona. La segunda comprueba perfiles nuevos, antiguos y persistencia de ambas opciones. Para la regresión del servicio SDL se usa `--comprobar-digital` con `SDL_AUDIODRIVER=dummy`, sin sonido audible.

## Resultado del 22/10/2026

Compilación SDL directa con O2 y avisos como errores correcta. Pasan la prueba posicional, los 120 casos de preferencias y la regresión del servicio de efectos SDL en cuatro escenas, con dispositivo dummy. Comparación selectiva contra el EXE original: 140 programaciones DSP/DMA, 31 efectos y 465.270 muestras coincidentes; prioridades y particiones conservadas. No equivale a escuchar cada efecto en todas las salas.

Formato directo de 221 archivos C++, Ruff (102 Python), ESLint, monorepo y las seis pruebas del builder correctos. `pnpm calidad` pasa Prettier y se detiene por `spawnSync clang-format EPERM` al iniciar la herramienta desde Node; los controles directos pasan. No se ha generado un ZIP nuevo como parte de esta comprobación.
