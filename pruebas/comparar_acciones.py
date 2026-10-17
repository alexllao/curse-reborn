"""Compara las escrituras del bloque x86 original con su traslado a C++."""

import argparse
import ctypes
import hashlib
import json
from pathlib import Path
import random
import struct
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'herramientas'))
from analizar_ejecutable import ErrorAnalisis, leer_mz


def comparar(ejecutable, biblioteca, salida, mapa):
    try:
        import unicorn
        from unicorn.x86_const import (UC_X86_REG_CS, UC_X86_REG_DS, UC_X86_REG_ES,
                                      UC_X86_REG_IP, UC_X86_REG_EFLAGS)
    except ImportError as error:
        raise ErrorAnalisis('Falta Unicorn. Ejecuta «pnpm preparar».') from error
    cabecera, imagen = leer_mz(ejecutable.read_bytes())
    if cabecera['sha256'] != mapa['sha256']:
        raise ErrorAnalisis('El ejecutable no corresponde a la versión del bloque C++ trasladado.')

    # Base de carga cero: las reubicaciones conservan sus valores relativos.
    maquina = unicorn.Uc(unicorn.UC_ARCH_X86, unicorn.UC_MODE_16)
    maquina.mem_map(0, 2 * 1024 * 1024)
    maquina.mem_write(0, imagen)
    maquina.mem_write(0, struct.pack('<H', 0x0F31))  # CS:0000 preparado por el arranque.
    segmento_datos = 0x0F31 * 16
    datos_originales = imagen[segmento_datos:segmento_datos + 65536].ljust(65536, b'\0')
    tabla_base = datos_originales[0xB686:0xB686 + 46]
    nativa = ctypes.CDLL(str(biblioteca.resolve()))
    nativa.reconstruir_tabla.argtypes = [ctypes.POINTER(ctypes.c_ubyte)]
    nativa.reconstruir_tabla.restype = None
    generador = random.Random(0xC0DE)
    casos = []
    for casilla in range(10):
        indices = list(range(10))
        indices[casilla] = 0xFFFF
        casos.append((f'Marca final en casilla {casilla}', indices, 0x563A, False))
    for indice in range(23):
        casos.append((f'Tabla base, identificador {indice}', [indice] * 10, 0, False))
    casos.extend([
        ('Umbral y comparación con signo', [22, 23, 24, 0x7FFF, 0x8000, 0x8001, 0xFFFE, 0, 1, 2], 0x3143, True),
        ('Respaldo nulo', [23] * 10, 0, False),
        ('Respaldo de 16 bits', [0x7FFF] * 10, 0xFFFF, False),
    ])
    for numero in range(512):
        indices = [generador.randrange(65536) for _ in range(10)]
        if numero % 3 == 0:
            indices[generador.randrange(10)] = 0xFFFF
        casos.append((f'Caso reproducible {numero}', indices, generador.randrange(65536), True))

    for nombre, indices, respaldo, aleatorio in casos:
        datos = bytearray(generador.randbytes(65536) if aleatorio else datos_originales)
        datos[0xB686:0xB686 + 46] = tabla_base
        struct.pack_into('<10H', datos, 0xB620, *indices)
        struct.pack_into('<H', datos, 0xB636, respaldo)
        maquina.mem_write(segmento_datos, bytes(datos))
        for registro, valor in [(UC_X86_REG_CS, 0), (UC_X86_REG_DS, 0x1234),
                                 (UC_X86_REG_ES, 0x5678), (UC_X86_REG_EFLAGS, 2)]:
            maquina.reg_write(registro, valor)
        maquina.emu_start(0xDF15, 0xDF4E, timeout=1_000_000, count=10000)
        if maquina.reg_read(UC_X86_REG_IP) != 0xDF4E:
            raise ErrorAnalisis(f'El original no alcanzó el límite del bloque: {nombre}.')
        esperado = bytes(maquina.mem_read(segmento_datos, 65536))
        obtenido = bytearray(datos)
        memoria_nativa = (ctypes.c_ubyte * 65536).from_buffer(obtenido)
        nativa.reconstruir_tabla(memoria_nativa)
        if esperado != obtenido:
            primera = next(i for i, (a, b) in enumerate(zip(esperado, obtenido)) if a != b)
            raise ErrorAnalisis(f'Diferencia en DS:{primera:04X}: {nombre}.')

    informe = {
        'sha256_ejecutable': cabecera['sha256'], 'bloque_original': '0000:DF15–DF4D',
        'sha256_biblioteca_comparada': hashlib.sha256(biblioteca.read_bytes()).hexdigest(),
        'version_unicorn': unicorn.__version__, 'casos_comparados': len(casos),
        'bytes_comparados_por_caso': 65536, 'resultado': 'coinciden',
        'limites': 'Se comparan escrituras en DS con DF=0; no registros, indicadores, dibujo ni el juego completo.',
    }
    salida.parent.mkdir(parents=True, exist_ok=True)
    salida.write_text(json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
    print(f'Bloque original: {informe["bloque_original"]}')
    print(f'Casos comparados contra instrucciones originales: {len(casos)}')
    print('Resultado: coinciden los 65.536 bytes del segmento de datos en todos los casos.')
    print(f'Informe local: {salida}')


def principal():
    raiz = Path(__file__).resolve().parents[1]
    argumentos = argparse.ArgumentParser(description='Compara la construcción de la tabla de acciones x86 y C++.')
    argumentos.add_argument('--biblioteca', type=Path, required=True)
    opciones = argumentos.parse_args()
    try:
        comparar(raiz / 'datos_originales' / 'CURSE' / 'CURSE.EXE', opciones.biblioteca,
                 raiz / 'trabajo' / 'comparacion_acciones' / 'resultado.json',
                 json.loads((raiz / 'documentacion' / 'mapa_ejecutable.json').read_text(encoding='utf-8')))
    except (OSError, ErrorAnalisis) as error:
        argumentos.exit(1, f'Error: {error}\n')


if __name__ == '__main__':
    principal()
