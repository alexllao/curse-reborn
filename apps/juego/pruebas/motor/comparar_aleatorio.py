"""Comprueba secuencias y estado contra la rutina x86 completa de 788E."""

import argparse
import ctypes
import hashlib
import json
from pathlib import Path
import random
import struct
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[4] / 'apps' / 'juego' / 'herramientas'))
from analizar_ejecutable import ErrorAnalisis, leer_mz


def comparar(raiz, biblioteca):
    try:
        import unicorn
        from unicorn import x86_const as registros
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
    nativa = ctypes.CDLL(str(biblioteca.resolve()))
    nativa.generar_valor.argtypes = [
        ctypes.POINTER(ctypes.c_ushort),
        ctypes.c_ushort,
        ctypes.POINTER(ctypes.c_ushort),
    ]
    nativa.generar_valor.restype = ctypes.c_int
    generador = random.Random(0x788E)
    semillas = [
        struct.unpack_from('<4H', imagen, 0x78DE),
        (0, 0, 0, 0),
        (65535, 65535, 65535, 65535),
        (0x1234, 0xABCD, 0x5678, 0x8001),
    ]
    for palabra in range(4):
        for bit in range(16):
            semilla = [0, 0, 0, 0]
            semilla[palabra] = 1 << bit
            semillas.append(tuple(semilla))
    semillas.extend(tuple(generador.randrange(65536) for _ in range(4)) for _ in range(16))
    limites = [1, 2, 6, 100, 256, 32768, 65535]
    llamadas = 0
    for numero, semilla in enumerate(semillas):
        for limite in limites:
            maquina.mem_write(0x78DE, struct.pack('<4H', *semilla))
            estado = (ctypes.c_ushort * 4)(*semilla)
            for paso in range(32):
                valores = {
                    registros.UC_X86_REG_CS: 0,
                    registros.UC_X86_REG_SS: 0x8000,
                    registros.UC_X86_REG_SP: 0xFF00,
                    registros.UC_X86_REG_AX: limite,
                    registros.UC_X86_REG_BX: 0x1234,
                    registros.UC_X86_REG_CX: 0x5678,
                    registros.UC_X86_REG_DX: 0x9ABC,
                    registros.UC_X86_REG_EFLAGS: 2 | (paso & 1),
                }
                for registro, valor in valores.items():
                    maquina.reg_write(registro, valor)
                maquina.mem_write(0x80000 + 0xFF00, struct.pack('<H', 0x7000))
                maquina.emu_start(0x788E, 0x7000, timeout=1_000_000, count=1000)
                if maquina.reg_read(registros.UC_X86_REG_IP) != 0x7000:
                    raise ErrorAnalisis('El original no retornó al punto de prueba.')
                resultado = ctypes.c_ushort(0xBEEF)
                aceptado = nativa.generar_valor(estado, limite, ctypes.byref(resultado))
                esperado = maquina.reg_read(registros.UC_X86_REG_AX)
                estado_original = struct.unpack('<4H', maquina.mem_read(0x78DE, 8))
                if aceptado != 1 or resultado.value != esperado or tuple(estado) != estado_original:
                    raise ErrorAnalisis(
                        f'Diferencia: semilla {numero}, límite {limite}, paso {paso}.'
                    )
                if not 0 <= resultado.value < limite:
                    raise ErrorAnalisis('Resultado fuera del límite solicitado.')
                for registro in (
                    registros.UC_X86_REG_BX,
                    registros.UC_X86_REG_CX,
                    registros.UC_X86_REG_DX,
                ):
                    if maquina.reg_read(registro) != valores[registro]:
                        raise ErrorAnalisis(
                            'El original no conservó un registro del contrato observado.'
                        )
                if maquina.reg_read(registros.UC_X86_REG_SP) != 0xFF02:
                    raise ErrorAnalisis('La rutina original no dejó la pila como se esperaba.')
                llamadas += 1

    # Contrato añadido por C++: no se ejecuta DIV/0 en el original.
    estado = (ctypes.c_ushort * 4)(1, 2, 3, 4)
    resultado = ctypes.c_ushort(0xBEEF)
    if (
        nativa.generar_valor(estado, 0, ctypes.byref(resultado)) != 0
        or tuple(estado) != (1, 2, 3, 4)
        or resultado.value != 0xBEEF
    ):
        raise ErrorAnalisis('El límite cero no se rechazó sin modificar los datos.')
    informe = {
        'bloque_original': '0000:788E–78DD',
        'sha256_ejecutable': cabecera['sha256'],
        'sha256_biblioteca_comparada': hashlib.sha256(biblioteca.read_bytes()).hexdigest(),
        'version_unicorn': unicorn.__version__,
        'semillas': len(semillas),
        'limites': limites,
        'pasos_por_secuencia': 32,
        'llamadas_comparadas': llamadas,
        'resultado': 'coinciden',
        'limite_cero': 'Contrato C++ comprobado; no se compara con la excepción DIV/0 del original.',
        'alcance': 'Salida AX, cuatro palabras de estado, retorno, pila y conservación de BX/CX/DX del original. No todos los indicadores ni el juego completo.',
    }
    salida = raiz / 'apps' / 'juego' / 'trabajo' / 'comparacion_aleatorio' / 'resultado.json'
    salida.parent.mkdir(parents=True, exist_ok=True)
    salida.write_text(json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
    print(f'Bloque original: {informe["bloque_original"]}')
    print(f'Llamadas comparadas: {llamadas}; semillas: {len(semillas)}; pasos por secuencia: 32.')
    print('Resultado: coinciden las secuencias y las cuatro palabras del estado.')
    print(f'Informe local: {salida}')


if __name__ == '__main__':
    argumentos = argparse.ArgumentParser(
        description='Compara el generador C++ con la rutina x86 original.'
    )
    argumentos.add_argument('--biblioteca', type=Path, required=True)
    opciones = argumentos.parse_args()
    try:
        comparar(Path(__file__).resolve().parents[4], opciones.biblioteca)
    except (OSError, ErrorAnalisis) as error:
        argumentos.exit(1, f'Error: {error}\n')
