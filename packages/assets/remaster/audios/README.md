# Música remasterizada

Cinco WAV aportados por Àlex, versionados como maestros. `titulos.json` asocia RS1–RS5 con el archivo, título y SHA-256. El juego conserva la selección de bancos de las tablas originales; cambiar de sala dentro del mismo banco no reinicia la canción.

La configuración inicial está en `apps/juego/configuracion/inicial.json`. F4 cambia de modo musical y muestra el título; F8 conserva su función de depuración. Las preferencias personales de volumen, silencio, marco y filtro tienen prioridad sobre los valores iniciales.

El builder copia los WAV en desarrollo y convierte copias a MP3 VBR de máxima calidad al empaquetar. El catálogo entregado apunta a MP3. El manifiesto de entrega registra las huellas SHA-256 tanto del maestro como del archivo convertido, igual que hace con las imágenes optimizadas. Un maestro cambiado sin actualizar su huella detiene la preparación.

Estas versiones repiten la canción completa con bordes de 20 ms para evitar clics. No se reutilizan los puntos temporales de los bucles de la música DOS: aún requieren revisión musical de estas nuevas interpretaciones.
