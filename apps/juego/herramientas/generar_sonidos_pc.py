"""Genera recursos de presentación desde la muestra CC0 indicada en ORIGEN.txt."""

import argparse
import math
import random
import struct
import subprocess
import wave
from pathlib import Path


def generar(fuente: Path, destino: Path):
    frecuencia = 24000
    resultado = subprocess.run(
        [
            'ffmpeg',
            '-hide_banner',
            '-loglevel',
            'error',
            '-y',
            '-i',
            str(fuente),
            '-t',
            '7',
            '-ac',
            '1',
            '-ar',
            str(frecuencia),
            '-af',
            'highpass=f=80,lowpass=f=7000,loudnorm=I=-25:TP=-8:LRA=8,'
            'afade=t=in:d=0.05,afade=t=out:st=6.55:d=0.4',
            '-c:a',
            'pcm_s16le',
            '-f',
            's16le',
            'pipe:1',
        ],
        check=True,
        capture_output=True,
    )
    arranque = [valor[0] / 32768 for valor in struct.iter_unpack('<h', resultado.stdout)]
    if len(arranque) != frecuencia * 7:
        raise ValueError('La muestra debe contener al menos siete segundos de sonido.')
    azar = random.Random(386)
    # Clic mecánico propio: impulso amortiguado y rebote, sin sonido de sistema operativo.
    boton = [
        (azar.uniform(-1, 1) * 0.28 + math.sin(i * 0.57) * 0.16) * math.exp(-i / 180)
        for i in range(1920)
    ]
    for i, valor in enumerate(boton):
        arranque[i] += valor
    # La BIOS cuenta 4096 KB entre 400 y 2200 ms: un pulso cada bloque de 128 KB.
    for bloque in range(32):
        inicio = round((0.4 + bloque * 1.8 / 32) * frecuencia)
        for i in range(240):
            envolvente = math.sin(math.pi * i / 240) ** 2
            arranque[inicio + i] += (
                0.09 * envolvente * math.sin(2 * math.pi * 1450 * i / frecuencia)
            )
    destino.mkdir(parents=True, exist_ok=True)
    for nombre, muestras in [('arranque.wav', arranque), ('boton.wav', boton)]:
        with wave.open(str(destino / nombre), 'wb') as audio:
            audio.setparams((1, 2, frecuencia, 0, 'NONE', 'not compressed'))
            audio.writeframes(
                b''.join(
                    struct.pack('<h', round(max(-0.98, min(0.98, muestra)) * 32767))
                    for muestra in muestras
                )
            )


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('fuente', type=Path)
    parser.add_argument('destino', type=Path)
    argumentos = parser.parse_args()
    generar(argumentos.fuente, argumentos.destino)
