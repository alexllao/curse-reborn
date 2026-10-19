"""Compara las escrituras del bloque x86 original con su traslado a C++."""

import argparse
import ctypes
import hashlib
from itertools import product
import json
from pathlib import Path
import random
import struct
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[3] / 'herramientas'))
from analizar_ejecutable import ErrorAnalisis, leer_mz


def comparar_memoria(maquina, segmento, datos, inicio, fin, registros, funcion, nombre):
    from unicorn.x86_const import (
        UC_X86_REG_CS,
        UC_X86_REG_DS,
        UC_X86_REG_ES,
        UC_X86_REG_IP,
        UC_X86_REG_EFLAGS,
    )

    maquina.mem_write(segmento, bytes(datos))
    for registro, valor in {
        UC_X86_REG_CS: 0,
        UC_X86_REG_DS: 0x0F31,
        UC_X86_REG_ES: 0x0F31,
        UC_X86_REG_EFLAGS: 2,
        **registros,
    }.items():
        maquina.reg_write(registro, valor)
    maquina.emu_start(inicio, fin, timeout=1_000_000, count=10000)
    if maquina.reg_read(UC_X86_REG_IP) != fin:
        raise ErrorAnalisis(f'El original no alcanzó el límite del bloque: {nombre}.')
    esperado = bytes(maquina.mem_read(segmento, 65536))
    obtenido = bytearray(datos)
    memoria_nativa = (ctypes.c_ubyte * 65536).from_buffer(obtenido)
    funcion(memoria_nativa)
    if esperado != obtenido:
        primera = next(i for i, (a, b) in enumerate(zip(esperado, obtenido)) if a != b)
        raise ErrorAnalisis(f'Diferencia en DS:{primera:04X}: {nombre}.')


def comparar(ejecutable, biblioteca, salida, mapa):
    try:
        import unicorn
        from unicorn.x86_const import (
            UC_X86_REG_CS,
            UC_X86_REG_DS,
            UC_X86_REG_ES,
            UC_X86_REG_AX,
            UC_X86_REG_DX,
            UC_X86_REG_EFLAGS,
        )
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
    datos_originales = imagen[segmento_datos : segmento_datos + 65536].ljust(65536, b'\0')
    tabla_base = datos_originales[0xB686 : 0xB686 + 46]
    nativa = ctypes.CDLL(str(biblioteca.resolve()))
    nativa.reconstruir_tabla.argtypes = [ctypes.POINTER(ctypes.c_ubyte)]
    nativa.reconstruir_tabla.restype = None
    nativa.seleccionar_coincidencias.argtypes = [
        ctypes.POINTER(ctypes.c_ubyte),
        ctypes.c_ubyte,
        ctypes.c_ubyte,
        ctypes.c_ubyte,
    ]
    nativa.seleccionar_coincidencias.restype = ctypes.c_int
    nativa.interpretar_retorno.argtypes = [ctypes.c_ushort, ctypes.c_uint]
    nativa.interpretar_retorno.restype = ctypes.c_uint
    generador = random.Random(0xC0DE)
    casos = []
    for casilla in range(10):
        indices = list(range(10))
        indices[casilla] = 0xFFFF
        casos.append((f'Marca final en casilla {casilla}', indices, 0x563A, False))
    for indice in range(23):
        casos.append((f'Tabla base, identificador {indice}', [indice] * 10, 0, False))
    casos.extend(
        [
            (
                'Umbral y comparación con signo',
                [22, 23, 24, 0x7FFF, 0x8000, 0x8001, 0xFFFE, 0, 1, 2],
                0x3143,
                True,
            ),
            ('Respaldo nulo', [23] * 10, 0, False),
            ('Respaldo de 16 bits', [0x7FFF] * 10, 0xFFFF, False),
        ]
    )
    for numero in range(512):
        indices = [generador.randrange(65536) for _ in range(10)]
        if numero % 3 == 0:
            indices[generador.randrange(10)] = 0xFFFF
        casos.append((f'Caso reproducible {numero}', indices, generador.randrange(65536), True))

    for nombre, indices, respaldo, aleatorio in casos:
        datos = bytearray(generador.randbytes(65536) if aleatorio else datos_originales)
        datos[0xB686 : 0xB686 + 46] = tabla_base
        struct.pack_into('<10H', datos, 0xB620, *indices)
        struct.pack_into('<H', datos, 0xB636, respaldo)
        comparar_memoria(
            maquina,
            segmento_datos,
            datos,
            0xDF15,
            0xDF4E,
            {UC_X86_REG_DS: 0x1234, UC_X86_REG_ES: 0x5678},
            nativa.reconstruir_tabla,
            nombre,
        )

    casos_coincidencias = 0
    for primero, segundo, tercero in product(range(6), repeat=3):
        for aleatorio in (False, True):
            datos = bytearray(generador.randbytes(65536) if aleatorio else datos_originales)

            def trasladar(memoria):
                if nativa.seleccionar_coincidencias(memoria, primero, segundo, tercero) != 1:
                    raise ErrorAnalisis('El bloque C++ rechazó una combinación válida.')

            comparar_memoria(
                maquina,
                segmento_datos,
                datos,
                0xB7D1,
                0xB8A3,
                {UC_X86_REG_AX: tercero, UC_X86_REG_DX: primero | (segundo << 8)},
                trasladar,
                f'Coincidencias {primero}, {segundo}, {tercero}',
            )
            casos_coincidencias += 1

    destinos_retorno = {0xDE2F: 0, 0xDE5B: 1, 0xDE5E: 2}
    observados = []

    def detener_en_decision(emulador, posicion, tamano, contexto):
        if posicion in destinos_retorno:
            observados.append(posicion)
            emulador.emu_stop()

    gancho = maquina.hook_add(unicorn.UC_HOOK_CODE, detener_en_decision)
    casos_retorno = 0
    valores_ax = [0, 1, 0x7FFF, 0x8000, 0x8001, 0xFFFF] + [
        generador.randrange(65536) for _ in range(64)
    ]
    try:
        for ax, acarreo in product(valores_ax, (0, 1)):
            observados.clear()
            maquina.reg_write(UC_X86_REG_CS, 0)
            maquina.reg_write(UC_X86_REG_AX, ax)
            maquina.reg_write(UC_X86_REG_EFLAGS, 2 | acarreo)
            maquina.emu_start(0xDE2D, 0xDE80, timeout=1_000_000, count=100)
            if len(observados) != 1:
                raise ErrorAnalisis('No se pudo observar la decisión del llamador de acciones.')
            if nativa.interpretar_retorno(ax, acarreo) != destinos_retorno[observados[0]]:
                raise ErrorAnalisis(f'Diferencia al interpretar AX={ax:04X}, acarreo={acarreo}.')
            casos_retorno += 1
    finally:
        maquina.hook_del(gancho)

    informe = {
        'sha256_ejecutable': cabecera['sha256'],
        'sha256_biblioteca_comparada': hashlib.sha256(biblioteca.read_bytes()).hexdigest(),
        'version_unicorn': unicorn.__version__,
        'casos_comparados': len(casos) + casos_coincidencias + casos_retorno,
        'bloques': [
            {
                'original': '0000:DF15–DF4D',
                'casos': len(casos),
                'comprobacion': 'Segmento DS completo, DF=0.',
            },
            {
                'original': '0000:B7D1–B8A2',
                'casos': casos_coincidencias,
                'comprobacion': 'Segmento DS completo; 216 combinaciones con dos estados de memoria.',
            },
            {
                'original': '0000:DE2D–DE59',
                'casos': casos_retorno,
                'comprobacion': 'Destino de continuación según AX y acarreo.',
            },
        ],
        'resultado': 'coinciden',
        'limites': 'No reproduce todos los registros e indicadores, el dibujo, la generación aleatoria ni el juego completo.',
    }
    salida.parent.mkdir(parents=True, exist_ok=True)
    salida.write_text(json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
    for bloque in informe['bloques']:
        print(f'Bloque original: {bloque["original"]}; casos: {bloque["casos"]}')
    print(f'Total de casos comparados: {informe["casos_comparados"]}')
    print('Resultado: coinciden las escrituras en DS y las decisiones comprobadas.')
    print(f'Informe local: {salida}')


def principal():
    raiz = Path(__file__).resolve().parents[3]
    argumentos = argparse.ArgumentParser(
        description='Compara la construcción de la tabla de acciones x86 y C++.'
    )
    argumentos.add_argument('--biblioteca', type=Path, required=True)
    opciones = argumentos.parse_args()
    try:
        comparar(
            raiz / 'datos_originales' / 'CURSE' / 'CURSE.EXE',
            opciones.biblioteca,
            raiz / 'apps' / 'juego' / 'trabajo' / 'comparacion_acciones' / 'resultado.json',
            json.loads(
                (raiz / 'documentacion' / 'mapa_ejecutable.json').read_text(encoding='utf-8')
            ),
        )
    except (OSError, ErrorAnalisis) as error:
        argumentos.exit(1, f'Error: {error}\n')


if __name__ == '__main__':
    principal()
