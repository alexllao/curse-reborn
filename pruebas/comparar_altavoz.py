"""Compara memoria y operaciones de puerto del reproductor del altavoz del PC."""

import argparse
import ctypes
import hashlib
from itertools import product
import json
from pathlib import Path
import struct

from comparar_avance import Comparacion, ErrorAnalisis, escribir


class ComparacionAltavoz(Comparacion):
    def __init__(self, raiz, biblioteca, muestra):
        super().__init__(raiz, biblioteca, muestra)
        self.nativa.procesar_altavoz.argtypes = [
            ctypes.c_uint,
            ctypes.POINTER(ctypes.c_ubyte),
            ctypes.c_ushort,
            ctypes.POINTER(ctypes.c_ushort),
            ctypes.POINTER(ctypes.c_ushort),
        ]
        self.nativa.procesar_altavoz.restype = ctypes.c_int
        self.traza = []
        self.control = 0
        self.visitadas = set()

        def leer(maquina, puerto, tamano, contexto):
            if puerto != 0x61 or tamano != 1:
                raise ErrorAnalisis('Lectura de puerto inesperada.')
            self.traza.append((0, puerto, self.control))
            return self.control

        def escribir_puerto(maquina, puerto, tamano, valor, contexto):
            if puerto not in (0x61, 0x42, 0x43) or tamano != 1:
                raise ErrorAnalisis('Escritura de puerto inesperada.')
            self.traza.append((1, puerto, valor))
            if puerto == 0x61:
                self.control = valor

        def registrar(maquina, direccion, tamano, contexto):
            if 0x7A46 <= direccion <= 0x7B0C:
                self.visitadas.add(direccion)

        self.maquina.hook_add(self.unicorn.UC_HOOK_INSN, leer, None, 1, 0, self.r.UC_X86_INS_IN)
        self.maquina.hook_add(
            self.unicorn.UC_HOOK_INSN, escribir_puerto, None, 1, 0, self.r.UC_X86_INS_OUT
        )
        self.maquina.hook_add(self.unicorn.UC_HOOK_CODE, registrar)

    def contrastar(self, grupo, datos, operacion, posicion=0x9000, control=0, prioridad=0):
        self.traza, self.control = [], control
        salida = (ctypes.c_ushort * 32)()
        salida[31] = control
        estado = ctypes.c_ushort(prioridad)
        self.maquina.mem_write(0x5306, bytes((prioridad,)))
        self.caso(
            grupo,
            datos,
            (0x7A46, 0x7A61, 0x7AC6, 0x5288, 0x83EA)[operacion],
            lambda m, ax, e: self.nativa.procesar_altavoz(
                operacion, m, posicion, ctypes.byref(estado), salida
            ),
            {
                self.r.UC_X86_REG_SI: 0x2C4C if operacion >= 3 else posicion,
                self.r.UC_X86_REG_AX: posicion if operacion == 3 else 0x5555,
            },
            destinos={0x87C7: 1} if operacion == 4 else None,
            conservar_si=operacion < 4,
        )
        traza_nativa = [tuple(salida[1 + i * 3 : 4 + i * 3]) for i in range(salida[0])]
        if traza_nativa != self.traza or salida[31] != self.control:
            raise ErrorAnalisis(
                f'{grupo}: operaciones de puerto distintas: {traza_nativa} frente a {self.traza}.'
            )
        if estado.value != self.maquina.mem_read(0x5306, 1)[0]:
            raise ErrorAnalisis(f'{grupo}: prioridad de sonido distinta.')

    def preparar_sonido(self, numero, modalidad, activo):
        datos = self.datos(numero)
        datos[0x0AD4], datos[0x8082], datos[0x9048 + 11] = modalidad, 11, 1
        escribir(datos, 0x8E43, 0xA000)
        for tabla, cantidad in ((0x8E14, 15), (0xA000, 240)):
            for i in range(cantidad):
                struct.pack_into('<BH', datos, tabla + i * 3, 10, 0x9600)
        for campo, valor in ((2, 100 if numero == 0 else 320), (4, 80), (6, 32), (8, 48)):
            escribir(datos, 0x2C4C + campo, valor)
        datos[0x9924] = activo
        datos[0x2C4C] = 1
        for campo in (0x10, 0x15, 0x1A):
            escribir(datos, 0x2C4C + campo, 0)
        escribir(datos, 0x2C5C, 0x9000)
        datos[0x2C5E:0x2C61] = bytes((0, 1, 2))
        return datos

    def ejecutar(self):
        for numero, posicion, variante in product(
            range(2), (0, 1, 0x9000, 0xFFFE, 0xFFFF), range(1 if self.muestra else 8)
        ):
            self.contrastar('inicio', self.datos(numero), 0, posicion, variante)
        controles = (0, 1, 2, 3, 0x80, 0xFC, 0xFE, 0xFF) if self.muestra else range(256)
        for numero, control in product(range(2), controles):
            self.contrastar('parada', self.datos(numero), 1, control=control)
        for numero, posicion, contador, duracion, divisor, control in product(
            range(2),
            (0, 0x9000) if self.muestra else (0, 0x9000, 0xFFFE),
            (0, 1, 2) if self.muestra else (0, 1, 2, 0xFFFF),
            (0, 1, 2) if self.muestra else (0, 1, 2, 0xFFFF),
            (0, 0x1234) if self.muestra else (0, 1, 0xFFFF, 0x1234),
            (0, 0xFF) if self.muestra else (0, 0x80, 0xFF),
        ):
            datos = self.datos(numero)
            escribir(datos, 0x9920, posicion)
            escribir(datos, 0x9922, contador)
            escribir(datos, posicion, duracion)
            escribir(datos, posicion + 2, divisor)
            self.contrastar('temporizador', datos, 2, control=control)
        for numero in range(4 if self.muestra else 64):
            datos = self.datos(numero)
            struct.pack_into('<7H', datos, 0x9000, 1, 0x1234, 2, 0xFFFF, 3, 0, 0)
            self.contrastar('inicio_persistente', datos, 0)
            for paso in range(16):
                # Una parada explícita intercalada conserva el indicador de actividad.
                self.contrastar(
                    'secuencia_persistente',
                    datos,
                    1 if paso == 3 and numero % 2 else 2,
                    control=(numero * 17 + paso) % 256,
                )
        for operacion, modalidad, identificador, prioridad, activo, numero in product(
            (3, 4),
            range(3),
            (0, 15, 255) if self.muestra else (0, 14, 15, 254, 255),
            (9, 10) if self.muestra else (0, 9, 10, 255),
            range(2),
            range(1 if self.muestra else 2),
        ):
            datos = self.preparar_sonido(numero, modalidad, activo)
            datos[0x9000:0x9003] = bytes((0xFB, identificador, 0x7C))
            self.contrastar(
                'seleccion_integrada' if operacion == 3 else 'animacion_integrada',
                datos,
                operacion,
                identificador,
                prioridad=prioridad,
            )
        informe = {
            'sha256_ejecutable': self.cabecera['sha256'],
            'sha256_biblioteca_comparada': hashlib.sha256(self.biblioteca.read_bytes()).hexdigest(),
            'version_unicorn': self.unicorn.__version__,
            'casos_por_grupo': self.resumen,
            'casos_totales': sum(self.resumen.values()),
            'resultado': 'coinciden',
            'muestra': self.muestra,
            'instrucciones_visitadas': [f'{p:04X}' for p in sorted(self.visitadas)],
            'alcance': 'DS completo, prioridad, pila y SI; lecturas y escrituras de puertos 61h, 42h y 43h. '
            'Arranque y parada, temporizador y animación con selector real. Sin síntesis PCM, SDL ni audio físico.',
        }
        salida = (
            self.raiz
            / 'trabajo/comparacion_altavoz'
            / ('muestra.json' if self.muestra else 'resultado.json')
        )
        salida.parent.mkdir(parents=True, exist_ok=True)
        salida.write_text(
            json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8'
        )
        for grupo, cantidad in self.resumen.items():
            print(f'{grupo}: {cantidad} casos coincidentes.')
        print(
            f'Total: {informe["casos_totales"]} casos. Coinciden memoria y operaciones de puerto.'
        )
        print(f'Informe local: {salida}')


if __name__ == '__main__':
    argumentos = argparse.ArgumentParser(description='Compara el reproductor del altavoz del PC.')
    argumentos.add_argument('--biblioteca', type=Path, required=True)
    argumentos.add_argument('--muestra', action='store_true')
    opciones = argumentos.parse_args()
    try:
        ComparacionAltavoz(
            Path(__file__).resolve().parents[1], opciones.biblioteca, opciones.muestra
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        argumentos.exit(1, f'Error: {error}\n')
