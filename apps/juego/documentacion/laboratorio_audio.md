# Pruebas de sonido

Acceso desde **Opciones → Sonido → Música y efectos → Pruebas de sonido**, antes de encender el PC o durante una sesión pausada. Incluye reproductor musical y prueba de estéreo, con teclado y ratón. No añade botones sobre la partida.

## Reproductor musical

Elegir versión y pista, después **Reproducir desde el inicio**. **Parar**, cambiar selección o salir de la página detiene la escucha. Las opciones disponibles dependen de los recursos que ya admite la entrega:

- **AdLib OPL2** y **Sound Blaster OPL3:** los dos temas FM originales, 0 y 1. No son cinco versiones FM de los bancos RS.
- **Roland MT-32:** CURSE.RS1–RS5, usando las ROM locales del usuario. Se omite si no se pueden preparar.
- **Remaster:** las cinco canciones propias del catálogo, con sus títulos. Conserva lectura WAV en desarrollo y MP3 en los paquetes que lo utilicen.

Los bucles corresponden al reproductor original de cada formato. La preescucha puede reiniciar el banco remaster incluso si se elige el mismo, sin cambiar el comportamiento normal de los cambios de escena. Se muestra el estado y los volúmenes efectivos seleccionados. **M** funciona también dentro del laboratorio.

## Estéreo posicional

El selector ofrece las dos voces originales de STD.SFX (Help/Hi), denominadas **Voz 1** y **Voz 2** para no asignar una palabra a un identificador sin comprobarla auditivamente. No son voces regeneradas.

Cinco botones dispuestos de izquierda a derecha reproducen la misma voz en **0 %, 25 %, 50 %, 75 % y 100 %** de la pantalla lógica. Sus centros son x=0/80/160/240/320; el emisor conserva su geometría y pasa por el selector original y el mismo procesador de estéreo que usa la partida. Se ven las ganancias L/R de la última posición elegida.

- Clic en un punto o teclas **1–5**: reproducir ahí.
- Flechas izquierda/derecha entre puntos e Intro: elegir y reproducir.
- Arriba/abajo: pasar entre el grupo de posiciones y los controles.
- Tab/Mayús + Tab: recorrer todos los controles.
- **Estéreo posicional desactivado:** comparar ambas orejas al mismo nivel desde cualquiera de los puntos.
- **Parar** o volver: cortar la prueba. La voz también termina automáticamente al agotarse su cola real.

La opción de estéreo de esta prueba es temporal y no cambia la preferencia de la partida. Las voces son digitales aunque el usuario haya elegido PC Speaker para jugar, porque son las muestras de voz originales que se están comparando.

## Aislamiento

El laboratorio posee su propio Audio, bancos, DS de prueba, dispositivo lógico SDL y sintetizadores. Solo copia los porcentajes generales, musicales, de efectos y silencio. No recibe punteros a la partida. Durante la escucha permanecen pausados el juego, sus fundidos, el tiempo efectivo y su música. Al volver se cierran los recursos de prueba y se recupera el punto musical anterior.

Las selecciones de tema, versión, voz y posición no se guardan como preferencias del juego. Cambiar M sí conserva su comportamiento general. No se modifica ni se empaqueta ningún audio adicional.

## Verificación reproducible

```powershell
$env:SDL_AUDIODRIVER='dummy'
curse-reborn.exe apps/juego/datos_originales/CURSE/DATA --comprobar-laboratorio-audio apps/juego/trabajo/laboratorio_audio
curse-reborn.exe apps/juego/datos_originales/CURSE/DATA --comprobar-menu-port apps/juego/trabajo/laboratorio_audio
$env:CURSE_REBORN_PARTIDAS='apps/juego/trabajo/laboratorio_audio/ranuras.sav'
curse-reborn.exe apps/juego/datos_originales/CURSE/DATA --comprobar-menu-port-integracion apps/juego/trabajo/laboratorio_audio/integracion
```

La prueba de audio renderiza dos segundos por tema/versión disponible con el dispositivo pausado, exige señal y comprueba el banco/reinicio. Con todos los recursos son **14 combinaciones**: 2×OPL2, 2×OPL3, 5×MT-32 y 5×remaster. Los recursos ausentes se omiten y constan en la máscara de disponibilidad del informe; no se acreditan como escuchados.

Comprueba además ambas voces en cinco posiciones, idénticos PCM/duración en las repeticiones, energía del canal correspondiente y mono exacto al desactivar estéreo. El fin de cola se prueba con dispositivo dummy. No abre ventanas ni produce audio audible; escribe `resultado.json` con cifras, sin publicar los datos comerciales.

Las pruebas del menú incluyen navegación, los cinco puntos con ratón y teclado y **22 capturas**: seis proporciones en dos idiomas y diez páginas internas. El recorrido integrado reproduce música y las cinco posiciones desde la interfaz real mientras contrasta que DS y el contador musical de la partida siguen inmóviles, y comprueba la reanudación posterior.

## Resultado de la tanda

Compilación directa O2 con avisos como errores correcta. Pasan las 14 combinaciones con señal, diez pruebas de voces/posiciones con comparación mono, fin de cola, 22 capturas y navegación, y el recorrido real de aislamiento/reanudación. Pasan también regresiones de cinco pistas remaster/bucles y 25 balances independientes. Controles directos de formato (227 C++), Ruff (102 Python), ESLint, Prettier, monorepo y seis pruebas del builder correctos. El agregado `pnpm calidad` sigue bloqueado antes de los controles por la comprobación de dependencias de pnpm en este entorno; no se declara aprobado. No se ha generado ZIP. Evidencia ignorada en `trabajo/laboratorio_audio/final/`.
