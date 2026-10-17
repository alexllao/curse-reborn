"""Genera el título con el núcleo C++: no carga CURSE.EXE ni ejecuta Unicorn."""

import argparse
import ctypes
import hashlib
import json
from pathlib import Path
import struct
import zlib


def guardar_png(ruta, imagen, paleta):
    def bloque(tipo, contenido):
        return struct.pack('>I', len(contenido)) + tipo + contenido + struct.pack('>I', zlib.crc32(tipo + contenido))
    colores = bytes(v * 255 // 63 for v in paleta)
    filas = b''.join(b'\0' + bytes(imagen[y * 320:(y + 1) * 320]) for y in range(200))
    ruta.write_bytes(b'\x89PNG\r\n\x1a\n' + bloque(b'IHDR', struct.pack('>2I5B', 320, 200, 8, 3, 0, 0, 0)) +
                     bloque(b'PLTE', colores) + bloque(b'IDAT', zlib.compress(filas)) + bloque(b'IEND', b''))


def generar(raiz, biblioteca, salida):
    archivo = raiz / 'datos_originales/CURSE/DATA/TITLE.DAT'
    comprimido = archivo.read_bytes()
    paleta = (archivo.parent / 'TITLE.PAL').read_bytes()
    if len(paleta) != 768 or max(paleta) > 63: raise ValueError('Paleta de título no válida.')
    n = ctypes.CDLL(str(biblioteca.resolve()))
    memoria = ctypes.POINTER(ctypes.c_ubyte)
    n.descomprimir_rnc1.argtypes = [memoria, ctypes.c_uint, memoria, ctypes.c_uint, ctypes.POINTER(ctypes.c_uint)]
    n.descomprimir_rnc1.restype = ctypes.c_uint
    n.procesar_video.argtypes = [ctypes.c_uint, memoria, memoria, ctypes.POINTER(ctypes.c_ushort), ctypes.POINTER(ctypes.c_uint)]
    n.procesar_video.restype = ctypes.c_int
    origen = (ctypes.c_ubyte * len(comprimido)).from_buffer_copy(comprimido)
    imagen, planos, recuperada = (ctypes.c_ubyte * 65536)(), (ctypes.c_ubyte * 262144)(), (ctypes.c_ubyte * 65536)()
    escritos = ctypes.c_uint()
    estado = n.descomprimir_rnc1(origen, len(comprimido), imagen, 65536, ctypes.byref(escritos))
    if estado != 0 or escritos.value != 64000: raise ValueError('El descompresor C++ no obtuvo el título esperado.')
    traza = (ctypes.c_uint * 262149)()
    parametros = (ctypes.c_ushort * 7)(320, 200, 112, 0, 0, 112, 0)
    if n.procesar_video(0, imagen, planos, parametros, traza) != 1: raise ValueError('Falló la subida C++ por planos.')
    traza[0] = 0
    if n.procesar_video(1, recuperada, planos, parametros, traza) != 1: raise ValueError('Falló la recuperación C++ por planos.')
    if bytes(recuperada[:64000]) != bytes(imagen[:64000]): raise ValueError('Los índices cambiaron al pasar por los planos.')
    rgb = (ctypes.c_ubyte * 65536).from_buffer_copy(paleta.ljust(65536, b'\0'))
    traza[0] = 0
    parametros[0] = 256
    if n.procesar_video(3, rgb, planos, parametros, traza) != 1: raise ValueError('Falló el envío C++ de paleta.')
    enviada = bytes(traza[3 + i * 4] for i in range(1, traza[0]))
    if enviada != paleta: raise ValueError('La paleta enviada por C++ difiere del recurso.')
    salida.parent.mkdir(parents=True, exist_ok=True)
    guardar_png(salida, recuperada[:64000], enviada)
    salida.with_suffix('.json').write_text(json.dumps({
        'sha256_title_dat': hashlib.sha256(comprimido).hexdigest(),
        'sha256_imagen_indexada': hashlib.sha256(bytes(imagen[:64000])).hexdigest(),
        'descompresion': 'Núcleo C++ reconstruido desde 64E0–67AC; sin ejecutar CURSE.EXE ni Unicorn.',
        'copias': 'Núcleo C++ trasladado de 7636, 7679 y 787E.',
        'vista': 'Imagen fija de diagnóstico; sin ventana SDL ni partida.'}, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
    print('Título generado con descompresión, copias y paleta en C++.')
    print(f'Imagen local: {salida}')


if __name__ == '__main__':
    argumentos = argparse.ArgumentParser(description='Genera el título con el núcleo C++, sin ejecutar el EXE.')
    argumentos.add_argument('--biblioteca', type=Path, required=True)
    argumentos.add_argument('--salida', type=Path)
    opciones = argumentos.parse_args()
    raiz = Path(__file__).resolve().parents[1]
    try:
        generar(raiz, opciones.biblioteca, opciones.salida or raiz / 'trabajo/vista_titulo/titulo_cpp.png')
    except (OSError, ValueError) as error:
        argumentos.exit(1, f'Error: {error}\n')
