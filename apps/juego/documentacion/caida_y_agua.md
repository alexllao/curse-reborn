# Caída y entrada al agua

La escena dos usa `DROP.MAP` y `DROP.SPR`. Se entra por la salida uno del corredor y el descriptor DS:85BA. Sus tres canales originales en DS:81B8/81E0/8203 avanzan la caída, cambian el modelo mediante F8 y permiten detectar la salida tres hacia la escena tres. No se añaden acciones al guion de la caída.

La escena tres carga `LAKEBED.MAP`, `LAKEBED.SPR` y `PLUNGE.BRD`; su descriptor es DS:85D6 y los canales iniciales son DS:8226/8240/8250. La ventana inicial es 40, con 50 tiras de fondo. Las primeras burbujas y entidades del lago se crean con la rutina específica BEFD–C092, respetando sus consumos de azar y sus ramas de contador.

## Contratos reconstruidos

- `preparar_transicion_escena`: fragmentos 4B05–4B0E, 4C43–4D40 y 5145–5181. Admite las salidas 0–3 y sus escenas de origen concretas. STD conserva 32×48; el modelo cinco usa dimensiones de su banco y canales de intervalo dos. La plataforma lee PLUNGE antes de disponer su referencia.
- `actualizar_rutina_escena(datos, azar, recursos)`: mantiene el despacho sin manejador de la escena dos e integra BEFD–C092 para el lago. Incluye límites dinámicos, burbujas, contador de efecto sin dispositivo en modalidad cero, reposición de dos entidades y activación de otra entidad según ventana y bandera.
- `Mazmorra::iniciar_caida`: dispone una sesión nueva de desarrollo con la entrada original a DROP. Esta función no sirve para saltarse un puzle conservando una partida.

Los bancos de referencia se separan más para admitir los **75.424 bytes de LAKEBED.SPR**: escena 40000h, modelo 54000h, STD 64000h, ratón 74000h, menú 76000h y geometría 7A000h. Las dos rutinas completas de cámara usan además una reserva MAP B0000h separada, porque CORRIDOR.MAP también supera 64 KiB. Antes de cada paso se verifica que los recursos de la comparación no se solapen ni se hayan corrompido.

## Verificación y uso

`pnpm comparar:caida` ha verificado **196 casos/pasos**, con 58 pasos hasta solicitar la salida y 128 pasos de entrada sumergida. La muestra conserva la caída completa y los primeros 24 pasos del agua: **92 casos/pasos**. Compara DS completo, los cuatro valores aleatorios, modelos y tres composiciones originales.

`pnpm comprobar:caida` ha verificado tres capturas SDL RGB de 64.000 píxeles: DROP tras 16 y 32 pasos y LAKEBED tras 24 pasos de entrada. Esta prueba recorre la vista con F6, mantiene la salida automática y acelera exclusivamente las esperas de presentación para la captura.

En uso normal, F6 desde la mazmorra abre el corredor; otro F6 abre la caída y el siguiente, el lago. F5 retrocede desde la escena visible. Cada salto prepara una sesión nueva. Las vistas conservan el ajuste proporcional y vecino más próximo. La entrada al agua y sus entidades autónomas están integradas; las demás acciones y el puzle del lago todavía no están completos.

[Guía en texto](recorridos/02_caida.txt). La prioridad posterior solicitada es el audio compartido: PC Speaker primero, después AdLib y MT-32. No se ha añadido audio en este bloque.
