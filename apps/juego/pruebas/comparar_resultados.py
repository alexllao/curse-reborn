"""Contrasta el tramo integrado B740–B939, incluidas sus llamadas originales."""

import argparse
import ctypes
import hashlib
import json
from pathlib import Path
import random
import struct
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[3] / 'apps' / 'juego' / 'herramientas'))
from analizar_ejecutable import ErrorAnalisis, leer_mz


def comparar(raiz, biblioteca):
    try:
        import unicorn
        from unicorn import x86_const as r
    except ImportError as error:
        raise ErrorAnalisis('Falta Unicorn. Ejecuta «pnpm preparar».') from error
    mapa = json.loads(
        (raiz / 'apps' / 'juego' / 'documentacion' / 'mapa_ejecutable.json').read_text(
            encoding='utf-8'
        )
    )
    cabecera, imagen = leer_mz(
        (raiz / 'apps' / 'juego' / 'datos_originales' / 'CURSE' / 'CURSE.EXE').read_bytes()
    )
    if cabecera['sha256'] != mapa['sha256']:
        raise ErrorAnalisis('El ejecutable no corresponde a la versión trasladada.')
    maquina = unicorn.Uc(unicorn.UC_ARCH_X86, unicorn.UC_MODE_16)
    maquina.mem_map(0, 2 * 1024 * 1024)
    maquina.mem_write(0, imagen)
    segmento = 0x0F31 * 16
    base = imagen[segmento : segmento + 65536].ljust(65536, b'\0')
    nativa = ctypes.CDLL(str(biblioteca.resolve()))
    nativa.preparar_resultados.argtypes = [
        ctypes.POINTER(ctypes.c_ubyte),
        ctypes.POINTER(ctypes.c_ushort),
    ]
    nativa.preparar_resultados.restype = None
    generador = random.Random(0xB740)
    semillas = [
        struct.unpack_from('<4H', imagen, 0x78DE),
        (0, 0, 0, 0),
        (65535, 65535, 65535, 65535),
        (0x1234, 0xABCD, 0x5678, 0x8001),
    ]
    semillas.extend(tuple(generador.randrange(65536) for _ in range(4)) for _ in range(60))
    casos = 0
    for numero, semilla in enumerate(semillas):
        for aleatorio in (False, True):
            datos = bytearray(generador.randbytes(65536) if aleatorio else base)
            lista = [0x100 + i for i in range(10)]
            escenario = numero % 12
            if escenario < 10:
                lista[escenario] = 0x008C
            elif escenario == 11:
                lista[1] = lista[7] = 0x008C
                lista[0] = 0xFFFF  # La búsqueda original sigue tras esta marca.
            struct.pack_into('<10H', datos, 0x5225, *lista)
            maquina.mem_write(segmento, bytes(datos))
            maquina.mem_write(0x78DE, struct.pack('<4H', *semilla))
            estado = (ctypes.c_ushort * 4)(*semilla)
            for paso in range(16):
                # Conserva los cambios de memoria y semilla entre operaciones sucesivas.
                for registro, valor in {
                    r.UC_X86_REG_CS: 0,
                    r.UC_X86_REG_DS: 0x0F31,
                    r.UC_X86_REG_ES: 0x4567,
                    r.UC_X86_REG_SS: 0x8000,
                    r.UC_X86_REG_SP: 0xFF00,
                    r.UC_X86_REG_EFLAGS: 2,
                }.items():
                    maquina.reg_write(registro, valor)
                maquina.emu_start(0xB740, 0xB93A, timeout=1_000_000, count=10000)
                if maquina.reg_read(r.UC_X86_REG_IP) != 0xB93A:
                    raise ErrorAnalisis('El original no alcanzó el salto común de animación.')
                if maquina.reg_read(r.UC_X86_REG_SP) != 0xFF00:
                    raise ErrorAnalisis(
                        'Las llamadas auxiliares originales no conservaron la pila.'
                    )
                memoria = (ctypes.c_ubyte * 65536).from_buffer(datos)
                nativa.preparar_resultados(memoria, estado)
                esperado = bytes(maquina.mem_read(segmento, 65536))
                if esperado != datos:
                    diferencia = next(i for i, (a, b) in enumerate(zip(esperado, datos)) if a != b)
                    raise ErrorAnalisis(
                        f'Diferencia en DS:{diferencia:04X}, semilla {numero}, paso {paso}, memoria aleatoria={aleatorio}.'
                    )
                if tuple(estado) != struct.unpack('<4H', maquina.mem_read(0x78DE, 8)):
                    raise ErrorAnalisis(
                        f'Difiere el estado aleatorio: semilla {numero}, paso {paso}.'
                    )
                casos += 1
    informe = {
        'bloque_original': '0000:B740–B939',
        'auxiliares_originales': ['0000:88F3–891D', '0000:788E–78DD'],
        'sha256_ejecutable': cabecera['sha256'],
        'sha256_biblioteca_comparada': hashlib.sha256(biblioteca.read_bytes()).hexdigest(),
        'version_unicorn': unicorn.__version__,
        'semillas': len(semillas),
        'operaciones_comparadas': casos,
        'operaciones_por_secuencia': 16,
        'resultado': 'coinciden',
        'alcance': 'DS completo, estado aleatorio y equilibrio de la pila original. Se detiene antes del salto común B93A; no valida toda la CPU ni la animación posterior.',
    }
    salida = raiz / 'apps' / 'juego' / 'trabajo' / 'comparacion_resultados' / 'resultado.json'
    salida.parent.mkdir(parents=True, exist_ok=True)
    salida.write_text(json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
    print(f'Bloque integrado original: {informe["bloque_original"]}')
    print(f'Operaciones comparadas: {casos}; semillas: {len(semillas)}; pasos por secuencia: 16.')
    print('Resultado: coinciden el segmento DS completo y el estado del generador.')
    print(f'Informe local: {salida}')


if __name__ == '__main__':
    argumentos = argparse.ArgumentParser(
        description='Compara la preparación integrada de resultados en C++ y x86.'
    )
    argumentos.add_argument('--biblioteca', type=Path, required=True)
    opciones = argumentos.parse_args()
    try:
        comparar(Path(__file__).resolve().parents[3], opciones.biblioteca)
    except (OSError, ErrorAnalisis) as error:
        argumentos.exit(1, f'Error: {error}\n')
