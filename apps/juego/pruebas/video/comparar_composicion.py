"""Contrasta la composición y el avance de la mazmorra con sus entidades y bancos reales."""

import argparse
import ctypes
import json
from pathlib import Path
import struct

from comparar_escena import ComparacionEscena, ErrorAnalisis
from comparar_objetos import ComparacionObjetos, guardar
from generar_titulo import guardar_png


class ComparacionComposicion(ComparacionEscena):
    # Prepara los bancos reales y los puentes de composición y avance sin duplicar el modelo VGA.
    def __init__(self, raiz, biblioteca, muestra):
        super().__init__(raiz, biblioteca, muestra)
        memoria = ctypes.POINTER(ctypes.c_ubyte)
        palabras = ctypes.POINTER(ctypes.c_ushort)
        self.nativa.componer_objetos.argtypes = [
            memoria,
            memoria,
            memoria,
            ctypes.c_uint,
            memoria,
            ctypes.c_uint,
        ]
        self.nativa.componer_objetos.restype = ctypes.c_int
        self.nativa.avanzar_objetos.argtypes = [
            memoria,
            palabras,
            memoria,
            ctypes.c_uint,
            memoria,
            ctypes.c_uint,
        ]
        self.nativa.avanzar_objetos.restype = ctypes.c_int
        self.escena = (self.carpeta / 'DUNGEON.SPR').read_bytes()
        self.modelo = (self.carpeta / 'MANA.BRD').read_bytes()
        self.banco_escena = (ctypes.c_ubyte * len(self.escena)).from_buffer_copy(self.escena)
        self.banco_modelo = (ctypes.c_ubyte * len(self.modelo)).from_buffer_copy(self.modelo)
        self.maquina.mem_write(0x0008, struct.pack('<H', 0xA000))
        self.maquina.mem_write(0x4AEE, struct.pack('<H', 0x4000))
        self.maquina.mem_write(0x000E, struct.pack('<H', 0x5000))
        self.maquina.mem_write(0x0010, struct.pack('<H', 0x5000))

    # Ejecuta un retorno normal con segmentos preparados, sin temporizador ni acceso a DOS.
    def ejecutar_original(self, entrada):
        m, r = self.maquina, self.r
        for registro, valor in (
            (r.UC_X86_REG_CS, 0),
            (r.UC_X86_REG_DS, 0x0F31),
            (r.UC_X86_REG_ES, 0xA000),
            (r.UC_X86_REG_SS, 0x8000),
            (r.UC_X86_REG_SP, 0xFF00),
            (r.UC_X86_REG_EFLAGS, 2),
        ):
            m.reg_write(registro, valor)
        m.mem_write(0x8FF00, struct.pack('<H', 0x7000))
        m.emu_start(entrada, 0x7000, count=3_000_000)
        if m.reg_read(r.UC_X86_REG_IP) != 0x7000 or m.reg_read(r.UC_X86_REG_SP) != 0xFF02:
            raise ErrorAnalisis(f'El recorrido original {entrada:04X} no terminó.')

    # Compara EBBD completo, incluidas tabla de orden y superposiciones, sobre el mismo fondo.
    def componer(self, datos, fondo):
        m = self.maquina
        self.reiniciar_video()
        for y in range(200):
            for x in range(320):
                self.planos[(x & 3) * 65536 + y * 112 + x // 4] = fondo.indices[y * 320 + x]
        m.mem_write(self.segmento, bytes(datos))
        m.mem_write(0x40000, self.escena)
        m.mem_write(0x50000, self.modelo)
        m.mem_write(0x0BF7, b'\0')
        m.mem_write(0x6BFC, struct.pack('<200H', *(i * 112 for i in range(200))))
        self.ejecutar_original(0xEBBD)
        imagen = (ctypes.c_ubyte * 64016)(*bytes(fondo.indices), *([0xCD] * 16))
        if not self.nativa.componer_objetos(
            (ctypes.c_ubyte * 65536).from_buffer(datos),
            imagen,
            self.banco_escena,
            len(self.escena),
            self.banco_modelo,
            len(self.modelo),
        ):
            raise ErrorAnalisis('La composición C++ rechazó los bancos y el estado reales.')
        if bytes(imagen[:64000]) != self.imagen_original(112) or bytes(imagen[64000:]) != bytes(
            [0xCD] * 16
        ):
            raise ErrorAnalisis('La composición difiere o escribió fuera de la imagen.')
        if datos != bytes(m.mem_read(self.segmento, 65536)):
            raise ErrorAnalisis('La composición no conserva la tabla de orden original.')
        self.resumen['composiciones'] = self.resumen.get('composiciones', 0) + 1
        return bytes(imagen[:64000])

    # Compara el paso integrado de guiones y comportamientos, manteniendo el estado entre llamadas.
    def avanzar(self, datos, semilla):
        m = self.maquina
        m.mem_write(self.segmento, bytes(datos))
        m.mem_write(0x78DE, struct.pack('<4H', *semilla))
        self.ejecutar_original(0x83DE)
        self.ejecutar_original(0x10EC)
        esperado = bytes(m.mem_read(self.segmento, 65536))
        azar = struct.unpack('<4H', m.mem_read(0x78DE, 8))
        if not self.nativa.avanzar_objetos(
            (ctypes.c_ubyte * 65536).from_buffer(datos),
            semilla,
            self.banco_escena,
            len(self.escena),
            self.banco_modelo,
            len(self.modelo),
        ):
            raise ErrorAnalisis('El paso C++ quedó pendiente de una operación no integrada.')
        if datos != esperado or tuple(semilla) != azar:
            posicion = next((i for i, (a, b) in enumerate(zip(datos, esperado)) if a != b), None)
            raise ErrorAnalisis(f'Paso integrado diferente en DS:{posicion!s}.')
        self.resumen['pasos_persistentes'] = self.resumen.get('pasos_persistentes', 0) + 1

    # Comprueba estados reales consecutivos y variantes de solapamiento sin resolver ningún puzle.
    def ejecutar(self):
        preparacion = ComparacionObjetos(self.raiz, self.biblioteca, True)
        datos = preparacion.caso('entrada', 4, bytearray(self.base), 'MANA.BRD')
        guardar(datos, 0x2C4A, 0x5000)
        preparacion.caso('carga', 0, datos)
        fondo = self.fondo('DUNGEON.MAP', range(10))
        # La comparación del fondo modifica CS:000A/12/16; la presentación usa CS:0008 y los bancos indicados.
        inicial = self.componer(datos, fondo)
        salida = self.raiz / 'apps/juego/trabajo/comparacion_objetos'
        salida.mkdir(parents=True, exist_ok=True)
        guardar_png(salida / 'mazmorra_objetos_cpp.png', inicial, fondo.paleta)
        semilla = (ctypes.c_ushort * 4)(*self.semilla_original)
        for paso in range(1, (32 if self.muestra else 256) + 1):
            self.avanzar(datos, semilla)
            imagen = self.componer(datos, fondo)
            if paso == 32:
                guardar_png(salida / 'mazmorra_animada_cpp.png', imagen, fondo.paleta)
        informe = {
            'sha256_ejecutable': self.cabecera['sha256'],
            'casos_por_grupo': self.resumen,
            'casos_totales': sum(self.resumen.values()),
            'resultado': 'coinciden',
            'muestra': self.muestra,
            'alcance': 'Estado completo y generador en pasos consecutivos, más 64.000 píxeles por composición '
            'con los bancos reales. Sin entrada de usuario, acciones de menú ni puzles.',
        }
        (
            salida / ('composicion_muestra.json' if self.muestra else 'composicion_resultado.json')
        ).write_text(json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
        for grupo, cantidad in self.resumen.items():
            print(f'{grupo}: {cantidad} casos coincidentes.')
        print(f'Total: {informe["casos_totales"]} casos. Coinciden guiones, estado y composición.')
        print(f'Vista local: {salida / "mazmorra_objetos_cpp.png"}')


if __name__ == '__main__':
    argumentos = argparse.ArgumentParser(
        description='Compara composición y avance de los objetos reales.'
    )
    argumentos.add_argument('--biblioteca', type=Path, required=True)
    argumentos.add_argument('--muestra', action='store_true')
    opciones = argumentos.parse_args()
    try:
        ComparacionComposicion(
            Path(__file__).resolve().parents[4], opciones.biblioteca, opciones.muestra
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        argumentos.exit(1, f'Error: {error}\n')
