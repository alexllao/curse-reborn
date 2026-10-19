"""Contrasta RNC de método uno con el descompresor original, sin incorporar código ajeno."""

import argparse
import ctypes
import hashlib
from itertools import product
import json
from pathlib import Path
import struct

from comparar_avance import Comparacion, ErrorAnalisis


class EscritorBits:
    def __init__(self):
        self.datos = bytearray()
        self.bit = 16
        self.posicion = 0

    def poner(self, valor, cantidad):
        for i in range(cantidad):
            if self.bit == 16:
                self.posicion = len(self.datos)
                self.datos.extend(b'\0\0')
                self.bit = 0
            self.datos[self.posicion + self.bit // 8] |= ((valor >> i) & 1) << (self.bit % 8)
            self.bit += 1

    def tabla(self, longitudes):
        self.poner(len(longitudes), 5)
        for longitud in longitudes:
            self.poner(longitud, 4)

    def valor_unico(self, valor):
        self.poner(0, 1)
        simbolo = simbolo_valor(valor)
        if simbolo >= 2:
            self.poner(valor - (1 << (simbolo - 1)), simbolo - 1)

    def empaquetar(self, longitud, bloques=1):
        cabecera = b'RNC\x01' + struct.pack('>2I2H2B', longitud, len(self.datos), 0, 0, 0, bloques)
        return cabecera + self.datos


def simbolo_valor(valor):
    return valor if valor < 2 else valor.bit_length()


def literales(cantidad):
    w = EscritorBits()
    w.poner(0, 2)
    simbolo = simbolo_valor(cantidad)
    w.tabla([0] * simbolo + [1])
    w.tabla([])
    w.tabla([])
    w.poner(1, 16)
    w.valor_unico(cantidad)
    esperado = bytes((i * 29 + 7) % 256 for i in range(cantidad))
    w.datos.extend(esperado)
    return w.empaquetar(cantidad), esperado


def repeticion(cantidad):
    w = EscritorBits()
    w.poner(0, 2)
    w.tabla([1, 1])
    w.tabla([1])
    w.tabla([0] * simbolo_valor(cantidad - 2) + [1])
    w.poner(2, 16)
    w.poner(1, 1)
    w.datos.extend(b'A')
    w.poner(0, 1)
    w.valor_unico(cantidad - 2)
    w.poner(0, 1)
    return w.empaquetar(cantidad + 1), b'A' * (cantidad + 1)


def reutilizar_tablas(bloques):
    w = EscritorBits()
    w.poner(0, 2)
    for bloque in range(bloques):
        w.tabla([1, 1] if bloque == 0 else [])
        w.tabla([])
        w.tabla([])
        w.poner(1, 16)
        w.poner(1, 1)
        w.datos.append(65 + bloque)
    return w.empaquetar(bloques, bloques), bytes(range(65, 65 + bloques))


class ComparacionRnc(Comparacion):
    metodo = 1
    entrada_original = 0x64E0

    def __init__(self, raiz, biblioteca, muestra):
        super().__init__(raiz, biblioteca, muestra)
        memoria = ctypes.POINTER(ctypes.c_ubyte)
        self.funcion = getattr(self.nativa, f'descomprimir_rnc{self.metodo}')
        self.funcion.argtypes = [
            memoria,
            ctypes.c_uint,
            memoria,
            ctypes.c_uint,
            ctypes.POINTER(ctypes.c_uint),
        ]
        self.funcion.restype = ctypes.c_uint
        self.archivos = {}

    def nativo(self, paquete, salida, capacidad):
        origen = (ctypes.c_ubyte * len(paquete)).from_buffer_copy(paquete)
        destino = (ctypes.c_ubyte * 65536).from_buffer(salida)
        escritos = ctypes.c_uint()
        estado = self.funcion(origen, len(paquete), destino, capacidad, ctypes.byref(escritos))
        return estado, escritos.value

    def contrastar(self, grupo, paquete, patron, mismo_buffer, capacidad, esperado=None, estado=0):
        m, r = self.maquina, self.r
        inicial = bytes([patron] * 65536)
        m.mem_write(0x40000, paquete.ljust(65536, bytes((patron,))))
        salida_original = 0x40000 if mismo_buffer else 0x50000
        if not mismo_buffer:
            m.mem_write(salida_original, inicial)
        contexto = {
            r.UC_X86_REG_CS: 0,
            r.UC_X86_REG_DS: 0x4000,
            r.UC_X86_REG_ES: 0x4000 if mismo_buffer else 0x5000,
            r.UC_X86_REG_SS: 0x8000,
            r.UC_X86_REG_SP: 0xFF00,
            r.UC_X86_REG_EFLAGS: 2,
            r.UC_X86_REG_AX: 0x1234,
            r.UC_X86_REG_BX: 0x5678,
            r.UC_X86_REG_CX: 0x3456,
            r.UC_X86_REG_DX: 0x789A,
            r.UC_X86_REG_SI: 0x1357,
            r.UC_X86_REG_DI: 0x2468,
            r.UC_X86_REG_BP: 0x5678,
        }
        for registro, valor in contexto.items():
            m.reg_write(registro, valor)
        m.mem_write(0x8FF00, struct.pack('<H', 0x7000))
        m.emu_start(self.entrada_original, 0x7000, count=20_000_000)
        if m.reg_read(r.UC_X86_REG_IP) != 0x7000 or m.reg_read(r.UC_X86_REG_SP) != 0xFF02:
            raise ErrorAnalisis(f'{grupo}: el original no retornó.')
        for registro, valor in contexto.items():
            if (
                registro not in (r.UC_X86_REG_SP, r.UC_X86_REG_EFLAGS)
                and m.reg_read(registro) != valor
            ):
                raise ErrorAnalisis(f'{grupo}: registro original no conservado.')
        salida = bytearray(inicial)
        resultado, escritos = self.nativo(paquete, salida, capacidad)
        if resultado != estado:
            raise ErrorAnalisis(
                f'{grupo}: estado nativo inesperado: {resultado}, escritos {escritos}.'
            )
        cantidad = int.from_bytes(paquete[4:8], 'big') if estado == 0 else 0
        original = bytes(m.mem_read(salida_original, cantidad))
        if (
            escritos != cantidad
            or salida[:cantidad] != original
            or salida[cantidad:] != inicial[cantidad:]
        ):
            raise ErrorAnalisis(f'{grupo}: salida o cola del búfer diferente.')
        if esperado is not None and original != esperado:
            raise ErrorAnalisis(
                f'{grupo}: el programa sintético no produjo la referencia prevista.'
            )
        if not mismo_buffer and bytes(m.mem_read(0x50000, 65536)) != salida:
            raise ErrorAnalisis(f'{grupo}: diferencia fuera de la salida en el original.')
        self.resumen[grupo] = self.resumen.get(grupo, 0) + 1
        return bytes(salida[:cantidad])

    def ejecutar(self):
        for nombre, patron, compartido, exacta in product(
            ('CORE.DAT', 'MENU.DAT', 'TITLE.DAT'),
            (0xCD,) if self.muestra else (0, 0xCD),
            (False, True),
            (False,) if self.muestra else (False, True),
        ):
            paquete = (self.raiz / 'apps/juego/datos_originales/CURSE/DATA' / nombre).read_bytes()
            capacidad = int.from_bytes(paquete[4:8], 'big') if exacta else 65536
            salida = self.contrastar('archivos_reales', paquete, patron, compartido, capacidad)
            self.archivos[nombre] = {
                'sha256_comprimido': hashlib.sha256(paquete).hexdigest(),
                'sha256_descomprimido': hashlib.sha256(salida).hexdigest(),
                'longitud': len(salida),
            }
        preparados = [
            literales(n)
            for n in (
                (0, 1, 3, 256, 4095)
                if self.muestra
                else (0, 1, 2, 3, 7, 8, 15, 16, 255, 256, 1023, 4095, 8191, 16383, 32767)
            )
        ]
        preparados += [
            repeticion(n)
            for n in ((2, 17, 1024) if self.muestra else (2, 3, 4, 7, 16, 255, 1024, 8192, 32760))
        ]
        preparados += [reutilizar_tablas(n) for n in ((2,) if self.muestra else (2, 3))]
        for (paquete, esperado), patron, compartido in product(
            preparados, (0xCD,) if self.muestra else (0, 0xCD), (False, True)
        ):
            self.contrastar('programas_sinteticos', paquete, patron, compartido, 65536, esperado)
        for paquete, estado in (
            (b'ABCD', 1),
            (b'RNC\x02' + bytes(14), 2),
            (b'RNXX' + bytes(14), 1),
        ):
            for patron in (0, 0xCD):
                self.contrastar('sin_metodo_uno', paquete, patron, False, 65536, estado=estado)
        valido, esperado = literales(16)
        variantes = [(valido[:n], 65536) for n in (4, 8, 17, len(valido) - 1)]
        variantes += [
            (valido, 15),
            (valido[:17] + b'\0' + valido[18:], 65536),
            (valido[:8] + b'\xff\xff\xff\xff' + valido[12:], 65536),
            (valido[:4] + b'\xff\xff\xff\xff' + valido[8:], 65536),
        ]
        for paquete, capacidad in variantes:
            destino = bytearray([0xCD] * 65536)
            estado, escritos = self.nativo(paquete, destino, capacidad)
            if estado != 3 or escritos != 0 or destino != bytes([0xCD] * 65536):
                raise ErrorAnalisis('Una cabecera inválida no se rechazó antes de escribir.')
            self.resumen['limites_api'] = self.resumen.get('limites_api', 0) + 1
        informe = {
            'sha256_ejecutable': self.cabecera['sha256'],
            'sha256_biblioteca_comparada': hashlib.sha256(self.biblioteca.read_bytes()).hexdigest(),
            'version_unicorn': self.unicorn.__version__,
            'casos_por_grupo': self.resumen,
            'casos_totales': sum(self.resumen.values()),
            'archivos': self.archivos,
            'resultado': 'coinciden',
            'muestra': self.muestra,
            'alcance': 'Salida completa, cola sin modificar y registros/pila originales; entrada original separada o compartida. '
            'Búferes separados en C++. Método uno, tablas de hasta 16 símbolos; no verifica CRC ni método dos.',
        }
        salida = (
            self.raiz
            / 'apps/juego/trabajo/comparacion_rnc'
            / ('muestra.json' if self.muestra else 'resultado.json')
        )
        salida.parent.mkdir(parents=True, exist_ok=True)
        salida.write_text(
            json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8'
        )
        for grupo, cantidad in self.resumen.items():
            print(f'{grupo}: {cantidad} casos coincidentes.')
        print(
            f'Total: {informe["casos_totales"]} casos. Coinciden las salidas y los contratos comprobados.'
        )
        print(f'Informe local: {salida}')


if __name__ == '__main__':
    argumentos = argparse.ArgumentParser(description='Compara el descompresor RNC de método uno.')
    argumentos.add_argument('--biblioteca', type=Path, required=True)
    argumentos.add_argument('--muestra', action='store_true')
    opciones = argumentos.parse_args()
    try:
        ComparacionRnc(
            Path(__file__).resolve().parents[3], opciones.biblioteca, opciones.muestra
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        argumentos.exit(1, f'Error: {error}\n')
