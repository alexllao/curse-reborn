"""Contrasta copias VGA y paleta; obtiene una referencia visual del título original."""

import argparse
import ctypes
import hashlib
from itertools import product
import json
from pathlib import Path
import struct
import zlib

from comparar_avance import Comparacion, ErrorAnalisis


class ComparacionVideo(Comparacion):
    def __init__(self, raiz, biblioteca, muestra):
        super().__init__(raiz, biblioteca, muestra)
        memoria = ctypes.POINTER(ctypes.c_ubyte)
        self.nativa.procesar_video.argtypes = [ctypes.c_uint, memoria, memoria,
            ctypes.POINTER(ctypes.c_ushort), ctypes.POINTER(ctypes.c_uint)]
        self.nativa.procesar_video.restype = ctypes.c_int
        self.traza_nativa = (ctypes.c_uint * 262149)()
        self.planos_base = bytes((i * 13 + i // 65536 * 17) % 256 for i in range(262144))
        self.planos = bytearray(self.planos_base)
        self.control, self.mascara, self.plano_lectura = 0, 15, 0
        self.indice_secuenciador, self.indice_grafico = 0, 0
        self.cerrojos = [0] * 4
        self.traza = []
        u, r = self.unicorn, self.r
        self.maquina.hook_add(u.UC_HOOK_INSN, self.leer_puerto, None, 1, 0, r.UC_X86_INS_IN)
        self.maquina.hook_add(u.UC_HOOK_INSN, self.escribir_puerto, None, 1, 0, r.UC_X86_INS_OUT)
        self.maquina.hook_add(u.UC_HOOK_MEM_READ, self.leer_video, None, 0xA0000, 0xAFFFF)
        self.maquina.hook_add(u.UC_HOOK_MEM_WRITE, self.escribir_video, None, 0xA0000, 0xAFFFF)

    def leer_puerto(self, maquina, puerto, tamano, contexto):
        if puerto != 0x3CF or self.indice_grafico != 5 or tamano != 1:
            raise ErrorAnalisis('Lectura VGA no prevista.')
        self.traza.append((0, puerto, self.control, tamano))
        return self.control

    def escribir_puerto(self, maquina, puerto, tamano, valor, contexto):
        self.traza.append((1, puerto, valor, tamano))
        if tamano == 2:
            self.aplicar_puerto(puerto, valor & 255)
            self.aplicar_puerto(puerto + 1, valor >> 8)
        else: self.aplicar_puerto(puerto, valor)

    def aplicar_puerto(self, puerto, valor):
        if puerto == 0x3C4: self.indice_secuenciador = valor
        elif puerto == 0x3C5 and self.indice_secuenciador == 2: self.mascara = valor & 15
        elif puerto == 0x3CE: self.indice_grafico = valor
        elif puerto == 0x3CF:
            if self.indice_grafico == 4: self.plano_lectura = valor & 3
            elif self.indice_grafico == 5: self.control = valor
            else: raise ErrorAnalisis('Registro gráfico VGA no previsto.')
        elif puerto not in (0x3C8, 0x3C9): raise ErrorAnalisis('Puerto VGA no previsto.')

    def leer_video(self, maquina, acceso, direccion, tamano, valor, contexto):
        if tamano != 1: raise ErrorAnalisis('Lectura VGA de tamaño no previsto.')
        posicion = direccion - 0xA0000
        self.cerrojos = [self.planos[i * 65536 + posicion] for i in range(4)]
        maquina.mem_write(direccion, bytes((self.cerrojos[self.plano_lectura],)))

    def escribir_video(self, maquina, acceso, direccion, tamano, valor, contexto):
        if tamano != 1: raise ErrorAnalisis('Escritura VGA de tamaño no previsto.')
        posicion = direccion - 0xA0000
        for plano in range(4):
            if self.mascara & (1 << plano):
                self.planos[plano * 65536 + posicion] = self.cerrojos[plano] if self.control & 3 == 1 else valor

    def caso_video(self, grupo, operacion, datos, parametros, planos=None):
        r, m = self.r, self.maquina
        ancho, alto, paso, origen, destino, paso_destino, control = parametros
        inicial = bytes(planos if planos is not None else self.planos_base)
        self.planos, self.traza = bytearray(inicial), []
        self.control, self.mascara, self.plano_lectura = control, 15, 0
        self.indice_grafico, self.indice_secuenciador = 0, 0
        m.mem_write(0x40000, bytes(datos))
        ds, es = (0x4000, 0xA000) if operacion == 0 else (0xA000, 0x4000) if operacion == 1 else (0xA000, 0xA000) if operacion == 2 else (0x4000, 0x4567)
        for registro, valor in ((r.UC_X86_REG_CS, 0), (r.UC_X86_REG_DS, ds), (r.UC_X86_REG_ES, es),
            (r.UC_X86_REG_SS, 0x8000), (r.UC_X86_REG_SP, 0xFF00), (r.UC_X86_REG_EFLAGS, 2),
            (r.UC_X86_REG_AX, destino if operacion == 3 else ancho), (r.UC_X86_REG_BX, alto),
            (r.UC_X86_REG_CX, ancho if operacion == 3 else paso * 4),
            (r.UC_X86_REG_DX, paso_destino * 4), (r.UC_X86_REG_SI, origen), (r.UC_X86_REG_DI, destino)):
            m.reg_write(registro, valor)
        m.mem_write(0x8FF00, struct.pack('<H', 0x7000))
        m.emu_start((0x7636, 0x7679, 0x76BC, 0x787E)[operacion], 0x7000, count=3_000_000)
        if m.reg_read(r.UC_X86_REG_IP) != 0x7000 or m.reg_read(r.UC_X86_REG_SP) != 0xFF02:
            raise ErrorAnalisis(f'{grupo}: el original no terminó con la pila equilibrada.')
        if m.reg_read(r.UC_X86_REG_DS) != ds or m.reg_read(r.UC_X86_REG_ES) != es:
            raise ErrorAnalisis(f'{grupo}: segmentos originales modificados.')
        ram = (ctypes.c_ubyte * 65536).from_buffer(datos)
        nativos = (ctypes.c_ubyte * 262144).from_buffer_copy(inicial)
        opciones = (ctypes.c_ushort * 7)(*parametros)
        self.traza_nativa[0] = 0
        if self.nativa.procesar_video(operacion, ram, nativos, opciones, self.traza_nativa) != 1:
            raise ErrorAnalisis(f'{grupo}: entrada preparada rechazada por C++.')
        if bytes(nativos) != self.planos: raise ErrorAnalisis(f'{grupo}: planos distintos.')
        if bytes(m.mem_read(0x40000, 65536)) != datos: raise ErrorAnalisis(f'{grupo}: RAM distinta.')
        traza = [tuple(self.traza_nativa[1 + i * 4:5 + i * 4]) for i in range(self.traza_nativa[0])]
        if traza != self.traza: raise ErrorAnalisis(f'{grupo}: operaciones de puerto distintas.')
        self.resumen[grupo] = self.resumen.get(grupo, 0) + 1
        return bytes(nativos)

    def titulo_original(self):
        archivo = self.raiz / 'datos_originales/CURSE/DATA/TITLE.DAT'
        comprimido = archivo.read_bytes()
        if comprimido[:4] != b'RNC\x01' or int.from_bytes(comprimido[4:8], 'big') != 64000:
            raise ErrorAnalisis('La pantalla de título no tiene la cabecera esperada.')
        m, r = self.maquina, self.r
        m.mem_write(0x40000, comprimido.ljust(65536, b'\0'))
        m.mem_write(0x50000, bytes([0xCD]) * 65536)
        for registro, valor in ((r.UC_X86_REG_CS, 0), (r.UC_X86_REG_DS, 0x4000), (r.UC_X86_REG_ES, 0x5000),
            (r.UC_X86_REG_SS, 0x8000), (r.UC_X86_REG_SP, 0xFF00), (r.UC_X86_REG_EFLAGS, 2)):
            m.reg_write(registro, valor)
        m.mem_write(0x8FF00, struct.pack('<H', 0x7000))
        # Referencia diagnóstica: se ejecuta el descompresor original, aún sin trasladar a C++.
        m.emu_start(0x64E0, 0x7000, count=20_000_000)
        if m.reg_read(r.UC_X86_REG_IP) != 0x7000 or m.reg_read(r.UC_X86_REG_SP) != 0xFF02:
            raise ErrorAnalisis('El descompresor original no terminó.')
        imagen = bytes(m.mem_read(0x50000, 64000))
        datos = bytearray(imagen.ljust(65536, b'\0'))
        planos = self.caso_video('titulo', 0, datos, (320, 200, 112, 0, 0, 112, 0))
        vuelta = bytearray([0xCD] * 65536)
        self.caso_video('titulo', 1, vuelta, (320, 200, 112, 0, 0, 112, 0), planos)
        if vuelta[:64000] != imagen: raise ErrorAnalisis('El título no coincide después de subir y recuperar los planos.')
        paleta = (self.raiz / 'datos_originales/CURSE/DATA/TITLE.PAL').read_bytes()
        if len(paleta) != 768 or max(paleta) > 63: raise ErrorAnalisis('La paleta del título no tiene el formato observado.')
        self.caso_video('titulo', 3, bytearray(paleta.ljust(65536, b'\0')), (256, 1, 0, 0, 0, 0, 0))
        carpeta = self.raiz / 'trabajo/vista_titulo'
        carpeta.mkdir(parents=True, exist_ok=True)
        guardar_png(carpeta / 'titulo.png', vuelta[:64000], paleta)
        (carpeta / 'procedencia.json').write_text(json.dumps({
            'sha256_ejecutable': self.cabecera['sha256'], 'sha256_title_dat': hashlib.sha256(comprimido).hexdigest(),
            'sha256_imagen_indexada': hashlib.sha256(imagen).hexdigest(),
            'descompresion': 'Rutina original 64E0 ejecutada en Unicorn; no constituye un traslado C++.',
            'copias': 'Subida y recuperación C++ comparadas con 7636 y 7679; paleta con 787E.',
            'vista': 'PNG diagnóstico; no ejecuta una partida ni SDL.'}, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')

    def ejecutar(self):
        for operacion in (0, 1):
            for ancho, alto, modalidad, inicio, numero in product(
                (4, 7, 12) if self.muestra else (4, 7, 8, 12, 31, 64, 320),
                (1, 2) if self.muestra else (1, 2, 7),
                range(2 if self.muestra else 3), (0, 123) if self.muestra else (0, 123, 0x4000),
                range(1 if self.muestra else 2)):
                paso = (ancho >> 2, 112, 160)[modalidad]
                self.caso_video('subida' if operacion == 0 else 'recuperacion', operacion, self.datos(numero),
                    (ancho, alto, paso, inicio, inicio, paso, 0xA0))
        for ancho, alto, pasos, posiciones, numero in product(
            (4, 7) if self.muestra else (4, 7, 16, 64),
            (1, 2) if self.muestra else (1, 2, 5), range(2 if self.muestra else 3),
            ((0, 0), (0, 1), (1, 0)) if self.muestra else ((0, 0), (0, 1), (1, 0), (80, 112), (0x2000, 0x3000)),
            range(1 if self.muestra else 2)):
            po, pd = ((ancho >> 2, ancho >> 2), (80, 112), (112, 80))[pasos]
            self.caso_video('copia', 2, self.datos(numero), (ancho, alto, po, *posiciones, pd, 0xA0))
        for cantidad, inicio, posicion, numero in product((1, 16) if self.muestra else (1, 2, 16, 256),
            (0, 255) if self.muestra else (0, 1, 128, 255), (0, 0xFF00), range(1 if self.muestra else 2)):
            self.caso_video('paleta', 3, self.datos(numero), (cantidad, 1, 0, posicion, inicio, 0, 0))
        self.titulo_original()
        informe = {'sha256_ejecutable': self.cabecera['sha256'],
            'sha256_biblioteca_comparada': hashlib.sha256(self.biblioteca.read_bytes()).hexdigest(),
            'version_unicorn': self.unicorn.__version__, 'casos_por_grupo': self.resumen,
            'casos_totales': sum(self.resumen.values()), 'resultado': 'coinciden', 'muestra': self.muestra,
            'alcance': 'RAM completa, cuatro planos completos y operaciones de puerto. '
                       'Título descomprimido por el original y copiado por C++. Sin descompresor C++ ni juego SDL.'}
        salida = self.raiz / 'trabajo/comparacion_video' / ('muestra.json' if self.muestra else 'resultado.json')
        salida.parent.mkdir(parents=True, exist_ok=True)
        salida.write_text(json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
        for grupo, cantidad in self.resumen.items(): print(f'{grupo}: {cantidad} casos coincidentes.')
        print(f'Total: {informe["casos_totales"]} casos. Coinciden memoria de vídeo y operaciones de puerto.')
        print(f'Informe local: {salida}')
        print(f'Vista diagnóstica: {self.raiz / "trabajo/vista_titulo/titulo.png"}')


def guardar_png(ruta, imagen, paleta):
    def bloque(tipo, contenido):
        return struct.pack('>I', len(contenido)) + tipo + contenido + struct.pack('>I', zlib.crc32(tipo + contenido))
    colores = bytes(v * 255 // 63 for v in paleta)
    filas = b''.join(b'\0' + bytes(imagen[y * 320:(y + 1) * 320]) for y in range(200))
    ruta.write_bytes(b'\x89PNG\r\n\x1a\n' + bloque(b'IHDR', struct.pack('>2I5B', 320, 200, 8, 3, 0, 0, 0)) +
                     bloque(b'PLTE', colores) + bloque(b'IDAT', zlib.compress(filas)) + bloque(b'IEND', b''))


if __name__ == '__main__':
    argumentos = argparse.ArgumentParser(description='Compara copias de vídeo y paleta del original.')
    argumentos.add_argument('--biblioteca', type=Path, required=True)
    argumentos.add_argument('--muestra', action='store_true')
    opciones = argumentos.parse_args()
    try:
        ComparacionVideo(Path(__file__).resolve().parents[1], opciones.biblioteca, opciones.muestra).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        argumentos.exit(1, f'Error: {error}\n')
