# Núcleo YM3812 externo

ymfm, de Aaron Giles, obtenido de https://github.com/aaronsgiles/ymfm en la revisión `81aec25ccbb98f4873a255f7551ac4dadac59b4a`. Se utiliza su YM3812 para distinguir la síntesis OPL2 de Nuked OPL3.

Las fuentes seleccionadas se conservan sin modificar. OPL requiere también sus unidades PCM y ADPCM por las clases compartidas de la dependencia, aunque el juego solo utiliza YM3812. Las huellas están en `herramientas/dependencias_opl2.json`.

La licencia BSD de tres cláusulas está en `LICENSE`; se copia como `LICENCIA_YMFM.txt` junto al ejecutable. Se mantienen sus atribuciones. Esta dependencia no asigna una licencia al resto del proyecto.

Los adaptadores están en `plataformas/sdl/audio/opl2.cpp` y `ymfm_*.cpp`. Los parámetros opcionales no usados de la fuente externa se excluyen de avisos; el código propio conserva sus revisiones. La comparación de bloques y reloj no acredita identidad acústica con una tarjeta física.
