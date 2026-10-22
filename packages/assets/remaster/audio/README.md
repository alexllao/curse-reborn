# Canciones reinterpretadas en Suno

Coloca en esta carpeta las cinco mezclas completas descargadas en WAV:

| Referencia original | Nombre del archivo |
| ------------------- | ------------------ |
| CURSE.RS1           | `rs1.wav`          |
| CURSE.RS2           | `rs2.wav`          |
| CURSE.RS3           | `rs3.wav`          |
| CURSE.RS4           | `rs4.wav`          |
| CURSE.RS5           | `rs5.wav`          |

La correspondencia depende del audio original utilizado como guía, no del título generado por Suno. Escribe el título de cada canción en el campo `titulo` de [titulos.json](titulos.json); conserva los identificadores y nombres de archivo para poder reconocerlas.

Los audios y las subcarpetas de trabajo quedan fuera de Git. Este README y la plantilla de títulos sí se publican para que la carpeta aparezca después de `git pull`. Copiar un WAV aquí no lo activa en el juego: primero se comprobarán la melodía, el volumen, la entrada y el cuerpo repetible, incluidas las notas sostenidas y las colas de reverberación.

Si también descargaste stems, puedes crear una subcarpeta local `stems/rs1/`, `stems/rs2/`, etc. No se necesita MIDI para esta vía de reinterpretación desde el audio.
