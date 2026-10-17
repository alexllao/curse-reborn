"""Contrasta el montaje de fondos y los sprites con sus recorridos originales."""

import argparse
import ctypes
import hashlib
from itertools import product
import json
from pathlib import Path
import struct

from comparar_video import ComparacionVideo, ErrorAnalisis
from generar_titulo import guardar_png


class Imagen(ctypes.Structure):
    _fields_ = [('indices', ctypes.c_ubyte * 64000), ('paleta', ctypes.c_ubyte * 768)]


class ComparacionEscena(ComparacionVideo):
    # Reutiliza el modelo VGA existente y prepara los contratos de las dos API nuevas.
    def __init__(self, raiz, biblioteca, muestra):
        super().__init__(raiz, biblioteca, muestra)
        memoria = ctypes.POINTER(ctypes.c_ubyte)
        self.nativa.preparar_fondo_mapa.argtypes = [
            memoria,
            ctypes.c_uint,
            memoria,
            ctypes.POINTER(Imagen),
            memoria,
            memoria,
        ]
        self.nativa.preparar_fondo_mapa.restype = ctypes.c_int
        self.nativa.dibujar_fotograma.argtypes = [
            memoria,
            ctypes.c_uint,
            ctypes.c_uint,
            memoria,
            ctypes.c_int,
            ctypes.c_int,
            ctypes.c_int,
        ]
        self.nativa.dibujar_fotograma.restype = ctypes.c_int
        self.carpeta = raiz / 'datos_originales/CURSE/DATA'

    # Reinicia los registros VGA y la memoria de planos antes de cada caso.
    def reiniciar_video(self):
        self.planos = bytearray(self.planos_base)
        self.traza = []
        self.control, self.mascara, self.plano_lectura = 0, 15, 0
        self.indice_grafico, self.indice_secuenciador = 0, 0

    # Recupera los píxeles visibles a partir del paso por plano utilizado por el original.
    def imagen_original(self, paso):
        return bytes(
            self.planos[(x & 3) * 65536 + y * paso + x // 4] for y in range(200) for x in range(320)
        )

    # Ejecuta 50D2–5138 sin DOS y compara el fondo, la paleta y todos los planos C++.
    def fondo(self, nombre, seleccion):
        m, r = self.maquina, self.r
        mapa = (self.carpeta / nombre).read_bytes()
        self.reiniciar_video()
        m.mem_write(0x40000, mapa[578:])
        m.mem_write(0x000A, struct.pack('<H', 0xA000))
        m.mem_write(0x0012, struct.pack('<H', 0x3000))
        m.mem_write(0x0016, struct.pack('<H', 0x4000))
        m.mem_write(self.segmento + 0x0AED, struct.pack('<H', 0x9000))
        m.mem_write(self.segmento + 0x3597, b'\0')
        m.mem_write(self.segmento + 0x9000, bytes(seleccion))
        for registro, valor in (
            (r.UC_X86_REG_CS, 0),
            (r.UC_X86_REG_DS, 0x0F31),
            (r.UC_X86_REG_SS, 0x8000),
            (r.UC_X86_REG_SP, 0xFF00),
            (r.UC_X86_REG_BP, 0xF000),
            (r.UC_X86_REG_EFLAGS, 2),
        ):
            m.reg_write(registro, valor)
        m.emu_start(0x50D2, 0x513A, count=20_000_000)
        if m.reg_read(r.UC_X86_REG_IP) != 0x513A or m.reg_read(r.UC_X86_REG_SP) != 0xFF00:
            raise ErrorAnalisis('El montaje original no llegó al final con la pila equilibrada.')
        imagen = Imagen()
        reservados = (self.carpeta / 'SPRITES.PAL').read_bytes()
        imagen.paleta[:] = reservados
        planos = (ctypes.c_ubyte * 262144).from_buffer_copy(self.planos_base)
        tira = (ctypes.c_ubyte * 6400)()
        if not self.nativa.preparar_fondo_mapa(
            (ctypes.c_ubyte * len(mapa)).from_buffer_copy(mapa),
            len(mapa),
            (ctypes.c_ubyte * 10)(*seleccion),
            ctypes.byref(imagen),
            planos,
            tira,
        ):
            raise ErrorAnalisis('El montaje C++ rechazó un fondo válido.')
        if bytes(planos) != self.planos or bytes(imagen.indices) != self.imagen_original(80):
            diferencia = next(
                (i for i, (a, b) in enumerate(zip(bytes(planos), self.planos)) if a != b), None
            )
            raise ErrorAnalisis(
                f'El fondo C++ o los planos difieren del original: {nombre}, byte {diferencia}, '
                f'C++={planos[diferencia] if diferencia is not None else "igual"}, '
                f'original={self.planos[diferencia] if diferencia is not None else "igual"}.'
            )
        if bytes(imagen.paleta) != mapa[:576] + reservados[576:]:
            raise ErrorAnalisis('El montaje no conserva los 64 colores reservados.')
        self.resumen['fondos'] = self.resumen.get('fondos', 0) + 1
        return imagen

    # Ejecuta 6D8C con transparencia, espejo y recorte, y compara sus 64.000 píxeles.
    def sprite(self, banco, indice, x, y, invertido):
        m, r = self.maquina, self.r
        datos = (self.carpeta / banco).read_bytes()
        inicio = int.from_bytes(datos[indice * 2 : indice * 2 + 2], 'little') * 16
        self.reiniciar_video()
        inicial = self.imagen_original(112)
        m.mem_write(0x40000, datos)
        m.mem_write(0x0BF7, b'\0')
        m.mem_write(0x6BFC, struct.pack('<200H', *(i * 112 for i in range(200))))
        for registro, valor in (
            (r.UC_X86_REG_CS, 0),
            (r.UC_X86_REG_DS, 0x4000 + inicio // 16),
            (r.UC_X86_REG_ES, 0xA000),
            (r.UC_X86_REG_SS, 0x8000),
            (r.UC_X86_REG_SP, 0xFF00),
            (r.UC_X86_REG_SI, 0),
            (r.UC_X86_REG_AX, 0x80 if invertido else 0),
            (r.UC_X86_REG_EFLAGS, 2),
        ):
            m.reg_write(registro, valor)
        m.mem_write(0x8FF00, struct.pack('<3H', 0x7000, x & 65535, y & 65535))
        m.emu_start(0x6D8C, 0x7000, count=2_000_000)
        if m.reg_read(r.UC_X86_REG_IP) != 0x7000 or m.reg_read(r.UC_X86_REG_SP) != 0xFF02:
            raise ErrorAnalisis('El dibujo original no terminó con la pila equilibrada.')
        imagen = (ctypes.c_ubyte * 64000).from_buffer_copy(inicial)
        if not self.nativa.dibujar_fotograma(
            (ctypes.c_ubyte * len(datos)).from_buffer_copy(datos),
            len(datos),
            indice,
            imagen,
            x,
            y,
            int(invertido),
        ):
            raise ErrorAnalisis(f'Fotograma válido rechazado: {banco}:{indice}.')
        esperado = self.imagen_original(112)
        if bytes(imagen) != esperado:
            raise ErrorAnalisis(
                f'Sprite diferente: {banco}:{indice}, ({x}, {y}), espejo={invertido}.'
            )
        self.resumen['sprites'] = self.resumen.get('sprites', 0) + 1

    # Agrupa fondos reales y posiciones de sprite; guarda una vista nativa de la mazmorra.
    def ejecutar(self):
        inicial = self.fondo('DUNGEON.MAP', range(10))
        self.fondo('CORRIDOR.MAP', (0, 1, 2, 3, 4, 5, 6, 7, 17, 18))
        self.fondo('DUNGEON.MAP', (255,) * 10)
        if not self.muestra:
            self.fondo('DUNGEON.MAP', tuple(reversed(range(10))))
            self.fondo('DUNGEON.MAP', (0, 255, 1, 255, 2, 255, 3, 255, 4, 255))
        posiciones = ((125, 98), (-7, 30), (300, 180), (13, -9))
        if not self.muestra:
            posiciones += ((-32, 20), (320, 10), (10, 200), (319, 0), (0, 0))
        for banco, indice in (('MANA.BRD', 0), ('STD.BRD', 0), ('STD.BRD', 9), ('STD.BRD', 18)):
            for (x, y), invertido in product(posiciones, (False, True)):
                self.sprite(banco, indice, x, y, invertido)
        datos = (self.carpeta / 'MANA.BRD').read_bytes()
        if not self.nativa.dibujar_fotograma(
            (ctypes.c_ubyte * len(datos)).from_buffer_copy(datos),
            len(datos),
            0,
            inicial.indices,
            125,
            98,
            0,
        ):
            raise ErrorAnalisis('No se pudo componer la vista inicial.')
        salida = self.raiz / 'trabajo/comparacion_escena'
        salida.mkdir(parents=True, exist_ok=True)
        guardar_png(salida / 'mazmorra_cpp.png', inicial.indices, inicial.paleta)
        informe = {
            'sha256_ejecutable': self.cabecera['sha256'],
            'casos_por_grupo': self.resumen,
            'casos_totales': sum(self.resumen.values()),
            'resultado': 'coinciden',
            'muestra': self.muestra,
            'sha256_imagen_inicial': hashlib.sha256(bytes(inicial)).hexdigest(),
            'alcance': 'Montaje de diez tiras, memoria por planos, paleta reservada y sprites. '
            'No verifica aún el bucle de escena, la entrada ni la jugabilidad.',
        }
        (salida / ('muestra.json' if self.muestra else 'resultado.json')).write_text(
            json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8'
        )
        for grupo, cantidad in self.resumen.items():
            print(f'{grupo}: {cantidad} casos coincidentes.')
        print(f'Total: {informe["casos_totales"]} casos. Coinciden fondos y sprites.')
        print(f'Vista local: {salida / "mazmorra_cpp.png"}')


if __name__ == '__main__':
    argumentos = argparse.ArgumentParser(description='Compara los fondos MAP y los sprites.')
    argumentos.add_argument('--biblioteca', type=Path, required=True)
    argumentos.add_argument('--muestra', action='store_true')
    opciones = argumentos.parse_args()
    try:
        ComparacionEscena(
            Path(__file__).resolve().parents[1], opciones.biblioteca, opciones.muestra
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        argumentos.exit(1, f'Error: {error}\n')
