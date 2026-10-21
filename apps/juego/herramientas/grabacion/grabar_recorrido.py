"""Graba el recorrido real en1080p, con mezcla del juego y sin capturar el escritorio."""

import argparse
from datetime import datetime
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess


# Ejecuta herramientas con argumentos separados, sin shell ni ventanas de consola auxiliares.
def ejecutar(argumentos, **opciones):
    return subprocess.run(
        argumentos, check=True, creationflags=subprocess.CREATE_NO_WINDOW, **opciones
    )


# Usa una tuberia local efimera para no volcar cientos de gigabytes de fotogramas sin comprimir.
def grabar(raiz, programa, destino, segundos):
    if os.name != 'nt':
        raise RuntimeError('La tuberia de grabacion esta preparada para Windows.')
    ffmpeg = shutil.which('ffmpeg')
    ffprobe = shutil.which('ffprobe')
    if not ffmpeg or not ffprobe:
        raise RuntimeError('Se necesitan FFmpeg y ffprobe instalados y disponibles en PATH.')
    destino.mkdir(parents=True, exist_ok=False)
    prefix = destino / 'partida'
    env = dict(
        os.environ,
        SDL_RENDER_DRIVER='software',
        CURSE_REBORN_GRABAR_RUTA=str(prefix),
        CURSE_REBORN_PARTIDAS=str(destino / 'CURSE.SAV'),
        CURSE_REBORN_TIEMPO=str(destino / 'tiempo.json'),
        CURSE_REBORN_PREFERENCIAS=str(destino / 'preferencias.json'),
    )
    env.pop('CURSE_REBORN_COMPROBAR_RECARGAS', None)
    env.pop('SDL_AUDIODRIVER', None)
    env['CURSE_REBORN_GRABAR_SEGUNDOS'] = str(segundos)
    ruta_video = destino / 'video_sin_audio.mp4'
    ruta_final = destino / ('muestra.mp4' if segundos else 'partida_completa.mp4')
    juego = codificador = None
    try:
        with (
            (destino / 'juego.log').open('wb') as log,
            (destino / 'ffmpeg.log').open('wb') as log_ffmpeg,
        ):
            codificador = subprocess.Popen(
                [
                    ffmpeg,
                    '-nostdin',
                    '-f',
                    'rawvideo',
                    '-pixel_format',
                    'rgba',
                    '-video_size',
                    '1920x1080',
                    '-framerate',
                    '30',
                    '-i',
                    'pipe:0',
                    '-an',
                    '-c:v',
                    'libx264',
                    '-preset',
                    'veryfast',
                    '-crf',
                    '18',
                    '-pix_fmt',
                    'yuv420p',
                    '-movflags',
                    '+faststart',
                    '-progress',
                    str(destino / 'progreso.txt'),
                    str(ruta_video),
                ],
                stdin=subprocess.PIPE,
                stdout=log_ffmpeg,
                stderr=log_ffmpeg,
                creationflags=subprocess.CREATE_NO_WINDOW,
            )
            juego = subprocess.Popen(
                [
                    str(programa),
                    str(raiz / 'apps/juego/datos_originales/CURSE/DATA'),
                    '--comprobar-final-poblacion-borde_valle_bruja_desenlace_final',
                    str(destino / 'final.bmp'),
                ],
                env=env,
                stdout=codificador.stdin,
                stderr=log,
                creationflags=subprocess.CREATE_NO_WINDOW,
            )
            codificador.stdin.close()
            estado = juego.wait(timeout=14400)
            if codificador.wait(timeout=120) != 0:
                raise RuntimeError(
                    f'FFmpeg no termina correctamente; consultar {destino / "ffmpeg.log"}.'
                )
        informe = json.loads(prefix.with_suffix('.json').read_text(encoding='utf-8'))
        if informe.get('ticks_por_paso') != 5 or informe.get('frecuencia_logica_hz') != 14:
            raise RuntimeError(
                'El ejecutable no acredita la cadencia de cinco ticks por paso; recompilar SDL.'
            )
        if segundos:
            if informe['completo'] or informe['duracion_ms'] < segundos * 1000 - 100:
                raise RuntimeError('La muestra se ha interrumpido antes del plazo pedido.')
        elif estado != 0 or not informe['completo'] or not (destino / 'final.bmp').is_file():
            raise RuntimeError(
                'El recorrido no ha llegado a THE END; se conserva la grabacion parcial.'
            )
        with (destino / 'mezcla.log').open('wb') as log_mezcla:
            ejecutar(
                [
                    ffmpeg,
                    '-nostdin',
                    '-i',
                    str(ruta_video),
                    '-i',
                    str(prefix.with_suffix('.wav')),
                    '-map',
                    '0:v:0',
                    '-map',
                    '1:a:0',
                    '-c:v',
                    'copy',
                    '-c:a',
                    'aac',
                    '-b:a',
                    '192k',
                    '-af',
                    'apad',
                    '-t',
                    str(informe['fotogramas'] / 30),
                    '-movflags',
                    '+faststart',
                    str(ruta_final),
                ],
                stdout=subprocess.DEVNULL,
                stderr=log_mezcla,
            )
        resultado = ejecutar(
            [
                ffprobe,
                '-v',
                'error',
                '-show_streams',
                '-show_format',
                '-of',
                'json',
                str(ruta_final),
            ],
            capture_output=True,
            text=True,
        )
        metadatos = json.loads(resultado.stdout)
        video = next(s for s in metadatos['streams'] if s['codec_type'] == 'video')
        audio = next(s for s in metadatos['streams'] if s['codec_type'] == 'audio')
        if (
            video['width'] != 1920
            or video['height'] != 1080
            or not audio['channels']
            or int(video['nb_frames']) != informe['fotogramas']
        ):
            raise RuntimeError('Formato de grabacion inesperado.')
        with ruta_final.open('rb') as archivo_final:
            huella = hashlib.file_digest(archivo_final, 'sha256').hexdigest()
        informe.update(
            archivo=str(ruta_final),
            bytes=ruta_final.stat().st_size,
            sha256=huella,
            duracion_video=float(metadatos['format']['duration']),
            sonido='mezcla interna grabada; salida del juego silenciada; no captura audio del escritorio',
            muestra=bool(segundos),
        )
        (destino / 'resultado.json').write_text(
            json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8'
        )
        print(json.dumps(informe, ensure_ascii=False), flush=True)
    finally:
        for proceso in (juego, codificador):
            if proceso is not None and proceso.poll() is None:
                proceso.terminate()
                proceso.wait(timeout=15)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--programa', type=Path)
    parser.add_argument('--destino', type=Path)
    parser.add_argument('--muestra', type=int, default=0, metavar='SEGUNDOS')
    args = parser.parse_args()
    raiz = Path(__file__).resolve().parents[4]
    if args.muestra < 0 or args.muestra > 3600:
        parser.error('--muestra admite0-3600 segundos.')
    destino = args.destino or raiz / 'apps/juego/trabajo/grabaciones' / datetime.now().strftime(
        '%Y%m%d-%H%M%S'
    )
    try:
        grabar(
            raiz,
            (args.programa or raiz / 'apps/juego/trabajo/aplicacion/curse-reborn.exe').resolve(),
            destino.resolve(),
            args.muestra,
        )
    except (OSError, RuntimeError, subprocess.SubprocessError) as error:
        parser.exit(1, f'Error: {error}\n')
