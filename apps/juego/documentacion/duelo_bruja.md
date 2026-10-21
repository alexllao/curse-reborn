# Duelo con la bruja

Escena72, FINAL.MAP/SPR y FINAL.SFX, entrada124 desde la biblioteca. Rutina20CD–2549: fantasma, carga de fuego, bola y derrota. La fase ocupa los bitsC0 de35CA, independientes de los bits inferiores usados en capítulos anteriores. Conserva los límites iniciales de movimiento, modelos, canales, fracasos y partículas con azar.

- Atacar con aspiradora171:4925–4952, solo fase0. La defensa frente al fantasma activa el modelo27 y la rutina cambia de fase al alcanzar su umbral120 con signo.
- Empujar el extintor164 desde el inventario:4953–49D9, solo fase1. XORC0 cambia a fase2 y prepara el humo y los canales auxiliares originales.
- Atacar con ventilador124:49DA–4A1C, solo fase2. El reflejo se decide al llegar el proyectil a su fase4 y comprobar la entidad del ventilador.
- Combinar175 con177, en ese orden:418A/4A1D–4A76. No es el verbo Adjuntar0: el original usa Combinar7. No se inventa otra comprobación de fase dentro del manejador.

Cada defensa válida y el anillo suman cinco puntos sin consumir los útiles. No se permite que una defensa de otra fase sustituya a la correcta. La rutina conserva las animaciones de fracaso y los campos de canal que el EXE no escribe.

`pnpm comparar:bruja --muestra` retoma la solicitud124 real, con los objetos obtenidos durante la aventura. Contrasta1.611 casos/pasos y siete composiciones, además de480 variantes directas de la rutina. Sus856 selecciones incluyen48 variantes de defensas y32 del orden/verbo del anillo. Audita cuatro estados de menú.

El recorrido alcanza125 tras367 pasos jugables, incluyendo cuatro cambios de cámara. Los puntos de control son entrada, fantasma, aspiradora, extintor, ventilador y anillo32/64. El comparador conserva la excepción acotada preexistente de F6: al solicitar125, una cabecera transitoria fuera de su banco se normaliza a ancho/alto cero en cuatro bytes. Esa imagen no se dibuja; el resto del estado y el azar se contrastan. No se presenta como una lectura fiel de RAM ajena al recurso DOS.

`pnpm comprobar:bruja` compara los siete fotogramas SDL y la restauración en memoria y148 cambios F5/F6 en las74 vistas. La instantánea bruja_solicitud125 conserva el final del duelo; el destino73 está contrastado por separado en [Desenlace](desenlace.md).

Revisión ampliada21/10: referencias del recorrido real regeneradas con contexto; consecuencias, respuestas negativas, cierresCLC y regresos documentados en la sección «Consecuencias del cementerio al desenlace» de `auditoria_pantallas.md`. El desenlace sin acciones se comprueba mediante reanudación explícita, no con una lista vacía. No cambia el motor ni los recursos.
