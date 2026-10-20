# Textos en inglés y castellano

`pnpm ventana` y `pnpm ventana en` arrancan con los textos originales en inglés.
`pnpm ventana es` selecciona castellano. Se admite también `--sin-sonido`.
El futuro selector dentro de la habitación queda aplazado.

## Cadenas localizadas

| Bloque                                | Localización original | Dibujo         |
| ------------------------------------- | --------------------- | -------------- |
| Puntuación y créditos de programación | DS:B758, tabla B74E   | E160/D623/D87A |
| Créditos de gráficos, fondos y sonido | DS:B804               | E160/D623/D87A |
| Diseño y creación de puzles           | DS:B8BA               | E160/D623/D87A |
| Agradecimientos                       | DS:B90B               | E160/D623/D87A |
| Cargar                                | DS:B949               | E997/E89D      |
| Guardar                               | DS:B9CF               | E997/E89D      |
| Editor del nombre                     | DS:BA62               | E46D           |
| Borrar                                | DS:BAE9               | E997/E89D      |
| Ranura vacía                          | DS:BB7B               | E997           |

Son nueve bloques originales identificados mediante sus rutinas de dibujo, más la ficha propia del port. Las rutas, nombres de recursos y errores técnicos de DOS no equivalen a textos mostrados durante la partida. No se traducen letreros ni imágenes, por petición de Àlex. No se afirma que este inventario descarte cualquier otra cadena en zonas todavía pendientes.

SDL conserva el inglés original y dibuja las fichas castellanas con FONT.SPR, añadiendo tildes y Ñ sin modificar el recurso. Respeta las placas originales, la puntuación dinámica y todos los nombres de sus autores. La ficha propia usa «Curse of Enchantia: Reborn», Àlex Llaó, versión y fecha de compilación en ambos idiomas.

Los rótulos de partidas se sustituyen en una copia usada exclusivamente para dibujar. El estado DOS, los archivos originales, las descripciones de las partidas y el teclado permanecen intactos. La selección no se guarda dentro de CURSE.SAV. Avisos de depuración, consola y presentación BIOS son independientes de estos nueve bloques y no se consideran traducción terminada.

## Comprobación

`pnpm comprobar:informacion es` y `pnpm comprobar:informacion en` abren las cinco fichas sin ventanas visibles y capturan el resultado. El caso inicial comprueba ambos idiomas, los límites de las placas y seis variantes de panel, conservando nombres de usuario y teclado. La comparación nativa de las cuatro fichas inglesas sigue contrastando cuarenta casos con el EXE.
