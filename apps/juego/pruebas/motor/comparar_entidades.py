"""Contrasta creación y comportamientos de entidades con instrucciones originales."""

import argparse
import ctypes
import hashlib
from itertools import product
import json
from pathlib import Path
import struct

from comparar_avance import Comparacion, ErrorAnalisis, escribir


TIPOS = (
    0x400,
    0x401,
    0x406,
    0x407,
    0x408,
    0x409,
    0x40A,
    0x40B,
    0x40C,
    0x40D,
    0x40F,
    0x410,
    0x411,
    0x412,
    0x3FF,
    0x402,
    0x40E,
    0x413,
    0xFFFF,
)


class ComparacionEntidades(Comparacion):
    def __init__(self, raiz, biblioteca, muestra):
        super().__init__(raiz, biblioteca, muestra)
        memoria = ctypes.POINTER(ctypes.c_ubyte)
        palabras = ctypes.POINTER(ctypes.c_ushort)
        self.nativa.procesar_entidades.argtypes = [
            ctypes.c_uint,
            memoria,
            palabras,
            palabras,
            ctypes.c_ushort,
            palabras,
        ]
        self.nativa.procesar_entidades.restype = ctypes.c_int
        self.dimensiones = (ctypes.c_ushort * 256)(
            *(valor for i in range(128) for valor in (8 + i % 41, 6 + i % 53))
        )
        self.maquina.mem_write(0x4AEE, struct.pack('<H', 0x4000))
        for i in range(128):
            desplazamiento = 0x100 + i
            self.maquina.mem_write(0x40000 + i * 2, struct.pack('<H', desplazamiento))
            self.maquina.mem_write(
                0x40000 + desplazamiento * 16,
                struct.pack('<2H', self.dimensiones[i * 2], self.dimensiones[i * 2 + 1]),
            )

    def preparar(self, numero):
        datos = self.datos(numero)
        for i in range(70):
            datos[0x2C0A + i * 33] = 0
        escribir(datos, 0x3588, 2)
        datos[0x0AD6] = 1
        datos[0x3594] = 1
        datos[0x358C] = (0, 2, 8, 0x14, 0xFE, 0xFF)[numero % 6]
        datos[0x2C2C] = (0, 1, 2, 8, 9, 17, 18, 127, 128, 255)[numero % 10]
        escribir(datos, 0x2C2D, 200)
        escribir(datos, 0x2C2F, 100)
        escribir(datos, 0x2C31, 32)
        escribir(datos, 0x2C33, 48)
        escribir(datos, 0x2C3B, (0, 0x11F0, 0x104A, 0x10F8)[numero % 4])
        escribir(datos, 0x359A, 5)
        escribir(datos, 0x3591, numero % 37)
        escribir(datos, 0x8082, 8 if numero % 2 == 0 else 0)
        escribir(datos, 0x99B8, numero % 13)
        # Definición de 18 bytes: tipo inerte y tres secuencias con terminadores.
        struct.pack_into(
            '<9H', datos, 0x9000, 0, 0, 0xFFFF, 0x9100, 0x0203, 0x9110, 0x0104, 0x9120, 0x0205
        )
        for posicion in (0x9100, 0x9110, 0x9120):
            datos[posicion : posicion + 7] = bytes((1, 2, 3, 4, 5, 6, 0x7F))
        for descriptor in (0x0FED, 0x132E, 0x135E, 0x9400):
            struct.pack_into(
                '<BBB3H',
                datos,
                descriptor,
                (0, 2, 4, 6)[numero % 4],
                1,
                numero % 256,
                120,
                90,
                0x9000,
            )
        for modelo in (0, 2, 8, 0x14, 0xFE, 0xFF):
            indice = ((modelo + 1) & 0xFF) * 4
            escribir(datos, 0x0E56 + indice, 0x9200)
            escribir(datos, 0x0E58 + indice, 0x9300)
        datos[0x9200:0x9300] = bytes((i * 13) % 256 for i in range(256))
        datos[0x9300:0x9400] = bytes((i * 17 + 128) % 256 for i in range(256))
        return datos

    def entidad(self, datos, tipo, numero, posicion):
        # Variantes dirigidas alrededor de los límites observados en las ramas.
        datos[posicion] = 1
        datos[posicion + 1] = 0x3C if numero % 3 else 0x3B
        delta = (
            -145,
            -124,
            -96,
            -51,
            -28,
            -24,
            -16,
            -1,
            0,
            1,
            16,
            20,
            24,
            28,
            50,
            96,
            124,
            144,
            145,
            200,
        )[numero % 20]
        escribir(datos, posicion + 2, 200 + delta)
        escribir(
            datos,
            posicion + 4,
            (69, 70, 79, 80, 100, 125, 126, 138, 144, 145, 199, 200, 201, -1)[numero % 14],
        )
        escribir(datos, posicion + 6, 32)
        escribir(datos, posicion + 8, 48)
        escribir(datos, posicion + 14, tipo)
        for desplazamiento in (16, 21, 26):
            escribir(datos, posicion + desplazamiento, 0 if numero % 2 == 0 else 0x9100)
            datos[posicion + desplazamiento + 2] = (0, 3, 5, 13, 255)[numero % 5]
            datos[posicion + desplazamiento + 3] = 1
            datos[posicion + desplazamiento + 4] = 2
        if numero % 7 == 0:
            datos[0x3594] = 0
        if tipo == 0x400:
            escribir(datos, posicion - 27, 32)
            escribir(datos, posicion - 31, 200 + delta)
        elif tipo == 0x401 and numero % 5 == 0:
            indice = ((datos[0x358C] + 1) & 255) * 4
            escribir(datos, 0x0E56 + indice, 0)
        elif tipo == 0x407:
            escribir(datos, posicion + 2, 200 + (-25, -24, -16, -15, 0, 15, 16, 23, 24)[numero % 9])
            datos[posicion + 18] = 5
        elif tipo == 0x408:
            escribir(datos, posicion + 2, (-97, -96, -95, 320)[numero % 4])
        elif tipo == 0x40B:
            if numero % 5 == 0:
                escribir(datos, posicion + 16, 0x1111)
            if numero % 7 == 0:
                escribir(datos, posicion + 26, 0x110F)
        elif tipo == 0x40C:
            escribir(datos, posicion + 2, (40, 48, 68, 76, 100, 200, 319, 320)[numero % 8])
            if numero % 5 == 0:
                escribir(datos, posicion + 16, 0x11C6)
        elif tipo == 0x40F:
            datos[0x2C4C] = 2 if numero % 3 == 0 else 0
        elif tipo in (0x411, 0x412):
            escribir(datos, posicion + 2, 200 + (-33, -21, -20, 0, 20, 21, 32)[numero % 7])
            escribir(datos, posicion + 4, 100 + (-11, -10, 0, 10, 11)[numero % 5])

    def contrastar(self, grupo, datos, operacion, posicion=0):
        r = self.r
        semilla = tuple(self.azar.randrange(65536) for _ in range(4))
        salida = (ctypes.c_ushort * 2)(0xFFFF, 0xFFFF)
        entradas = (0x8B6F, 0x1119, 0x10EC, 0xC4DC)
        destinos = {0x1786: 1} if operacion == 1 else {0x1791: 1} if operacion == 2 else None
        self.caso(
            grupo,
            datos,
            entradas[operacion],
            lambda m, ax, e: self.nativa.procesar_entidades(
                operacion, m, e, self.dimensiones, posicion, salida
            ),
            {
                r.UC_X86_REG_ES: 0x0F31,
                r.UC_X86_REG_SI: posicion,
                r.UC_X86_REG_BX: posicion,
                r.UC_X86_REG_CX: 68,
            },
            semilla=semilla,
            destinos=destinos,
        )
        if self.maquina.reg_read(r.UC_X86_REG_DS) != 0x0F31:
            raise ErrorAnalisis(f'{grupo}: el original no restauró DS.')
        if operacion == 0:
            di = self.maquina.reg_read(r.UC_X86_REG_DI)
            cf = self.maquina.reg_read(r.UC_X86_REG_EFLAGS) & 1
            if (salida[0] and salida[1] != di) or (not salida[0] and cf != 1):
                raise ErrorAnalisis(f'{grupo}: destino de creación o rechazo diferente.')

    def ejecutar(self):
        for flags, ocupacion, numero in product(
            (0, 2, 4, 6, 0x80, 0x82, 0x86), range(5), range(2 if self.muestra else 8)
        ):
            datos = self.preparar(numero)
            datos[0x9400] = flags
            datos[0x0AD6] = numero % 2
            for i in range(2, 70):
                datos[0x2C0A + i * 33] = 1 if ocupacion == 4 or i < 2 + ocupacion else 0
            self.contrastar('creacion', datos, 0, 0x9400)
        for tipo, numero in product(TIPOS, range(12 if self.muestra else 128)):
            datos = self.preparar(numero)
            posicion = 0x2C4C + 33 * 10
            self.entidad(datos, tipo, numero, posicion)
            self.contrastar(f'tipo_{tipo:04x}', datos, 1, posicion)
        # Fronteras independientes: evitan correlacionar siempre modelo, imagen y fase.
        posicion = 0x2C4C + 33 * 10
        for imagen in (0, 8, 9, 17, 18, 127, 128, 255):
            datos = self.preparar(0)
            self.entidad(datos, 0x410, 0, posicion)
            datos[0x2C2C] = imagen
            self.contrastar('fronteras', datos, 1, posicion)
        for modelo, imagen in product((0xFF, 2, 8), (0, 8, 9, 127, 128, 137, 254, 255)):
            datos = self.preparar(2)
            self.entidad(datos, 0x401, 2, posicion)
            datos[0x358C], datos[0x2C2C] = modelo, imagen
            self.contrastar('fronteras', datos, 1, posicion)
        for numero in range(16):
            datos = self.preparar(numero)
            self.entidad(datos, 0x406, numero, posicion)
            escribir(datos, 0x8082, 8)
            escribir(datos, posicion + 4, 145)
            self.contrastar('fronteras', datos, 1, posicion)
        for imagen in (0, 1, 2, 255):
            datos = self.preparar(2)
            self.entidad(datos, 0x40C, 2, posicion)
            escribir(datos, posicion + 2, 60)
            datos[0x358C], datos[0x2C2C] = 8, imagen
            self.contrastar('fronteras', datos, 1, posicion)
        for estado in (0, 2, 3, 255):
            datos = self.preparar(2)
            self.entidad(datos, 0x40D, 3, posicion)
            datos[posicion] = estado
            self.contrastar('fronteras', datos, 1, posicion)
        for y in (68, 69, 70, 100):
            datos = self.preparar(2)
            self.entidad(datos, 0x411, 2, posicion)
            escribir(datos, posicion + 2, 200)
            escribir(datos, 0x2C2F, y)
            self.contrastar('fronteras', datos, 1, posicion)
        for numero in range(8 if self.muestra else 128):
            self.contrastar('transicion', self.preparar(numero), 3)
        for numero in range(8 if self.muestra else 128):
            datos = self.preparar(numero)
            for i, tipo in enumerate(TIPOS):
                self.entidad(datos, tipo, numero + i, 0x2C4C + 33 * (i * 3))
            self.contrastar('recorrido', datos, 2)
        informe = {
            'sha256_ejecutable': self.cabecera['sha256'],
            'sha256_biblioteca_comparada': hashlib.sha256(self.biblioteca.read_bytes()).hexdigest(),
            'version_unicorn': self.unicorn.__version__,
            'casos_por_grupo': self.resumen,
            'casos_totales': sum(self.resumen.values()),
            'resultado': 'coinciden',
            'muestra': self.muestra,
            'alcance': 'DS completo, cuatro palabras del generador, pila y retorno de creación. '
            'Cabeceras de recursos sintéticas; recorrido detenido antes del despacho por escena. '
            'No comprueba dibujo, carga real de recursos, divisiones inválidas ni partida completa.',
        }
        salida = (
            self.raiz
            / 'apps'
            / 'juego'
            / 'trabajo'
            / 'comparacion_entidades'
            / ('muestra.json' if self.muestra else 'resultado.json')
        )
        salida.parent.mkdir(parents=True, exist_ok=True)
        salida.write_text(
            json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8'
        )
        for grupo, cantidad in self.resumen.items():
            print(f'{grupo}: {cantidad} casos coincidentes.')
        print(
            f'Total: {informe["casos_totales"]} casos. Coinciden los datos y el estado aleatorio.'
        )
        print(f'Informe local: {salida}')


if __name__ == '__main__':
    argumentos = argparse.ArgumentParser(
        description='Compara la creación y actualización de entidades.'
    )
    argumentos.add_argument('--biblioteca', type=Path, required=True)
    argumentos.add_argument(
        '--muestra', action='store_true', help='Ejecuta una muestra corta de cada grupo.'
    )
    opciones = argumentos.parse_args()
    try:
        ComparacionEntidades(
            Path(__file__).resolve().parents[4], opciones.biblioteca, opciones.muestra
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        argumentos.exit(1, f'Error: {error}\n')
