"""Comparación de los tres canales de animación y la retirada de registros."""

import argparse
import ctypes
import hashlib
from itertools import product
import json
from pathlib import Path
import struct

from comparar_avance import Comparacion, ErrorAnalisis, escribir


class ComparacionSecuencias(Comparacion):
    def __init__(self, raiz, biblioteca, muestra):
        super().__init__(raiz, biblioteca, muestra)
        self.nativa.procesar_secuencias.argtypes = [ctypes.c_uint,
            ctypes.POINTER(ctypes.c_ubyte), ctypes.POINTER(ctypes.c_ushort),
            ctypes.POINTER(ctypes.c_ushort), ctypes.c_ushort]
        self.nativa.procesar_secuencias.restype = ctypes.c_int
        self.dimensiones = (ctypes.c_ushort * 768)(
            *(valor for recurso in range(3) for i in range(128)
              for valor in (8 + i % 41 + recurso * 3, 6 + i % 53 + recurso * 5)))
        for recurso in range(3):
            segmento = 0x4000 + recurso * 0x100
            for i in range(128):
                parrafo = 0x500 + i
                self.maquina.mem_write(segmento * 16 + i * 2, struct.pack('<H', parrafo))
                indice = (recurso * 128 + i) * 2
                self.maquina.mem_write((segmento + parrafo) * 16,
                    struct.pack('<2H', *self.dimensiones[indice:indice + 2]))
        for direccion, segmento in ((0x4AEE, 0x4000), (0x0E, 0x4100), (0x10, 0x4200)):
            self.maquina.mem_write(direccion, struct.pack('<H', segmento))
        self.instrucciones_visitadas = set()
        def registrar(emulador, direccion, tamano, contexto):
            if 0x83DE <= direccion <= 0x8821:
                self.instrucciones_visitadas.add(direccion)
        # Mantener instrumentación estable también al alternar RET y paradas de bloque.
        self.maquina.hook_add(self.unicorn.UC_HOOK_CODE, registrar)

    def preparar(self, numero):
        datos = self.datos(numero)
        for i in range(70): datos[0x2C0A + i * 33] = 0
        escribir(datos, 0x3588, 2)
        datos[0x9974] = 3
        return datos

    def registro(self, datos, posicion, canal, programa, contador=1, indice=0):
        datos[posicion] = 1
        datos[posicion + 1] = 5
        for campo, valor in ((2, 150), (4, 100), (6, 32), (8, 48), (0x1F, 0x4000)):
            escribir(datos, posicion + campo, valor)
        for i in range(3): escribir(datos, posicion + 0x10 + i * 5, 0)
        base = posicion + 0x10 + canal * 5
        referencia = 0x9000 + canal * 0x200
        escribir(datos, base, referencia)
        datos[base + 2], datos[base + 3], datos[base + 4] = indice, contador, 2
        datos[referencia:referencia + 256] = bytes([1] * 256)
        datos[referencia:referencia + len(programa)] = programa
        datos[0x9600:0x9603] = bytes((7, 8, 0x7F))

    def contrastar(self, grupo, datos, operacion, posicion=0x2C4C, semilla=None, esperado=1, destino=None):
        r = self.r
        return self.caso(grupo, datos, (0x83EA, 0x87E4, 0x83DE)[operacion],
            lambda m, ax, e: int(self.nativa.procesar_secuencias(operacion, m, e,
                self.dimensiones, posicion) == esperado),
            {r.UC_X86_REG_SI: posicion, r.UC_X86_REG_CX: 70},
            semilla=semilla or tuple(self.azar.randrange(65536) for _ in range(4)),
            destinos=({destino: 1} if destino is not None else
                      {0x87C7: 1} if operacion == 0 else {0x881B: 1} if operacion == 1 else None))

    def ejecutar(self):
        comunes = [bytes(x) for x in ((0,), (1,), (119,), (128,), (255,),
            (5, 0x7F), (5, 0x79), (0x7E, 0, 0x96), (0x7D,), (0x7C,),
            (0x7B, 0), (0x7B, 255), (0x7A, 0, 5), (0x7A, 3, 5), (0x78, 9, 4))]
        imagenes = [bytes(x) for x in ((0xF5,), (0xFE,), (0xFD, 5), (0xFC, 5),
            (0xFA, 5), (0xF9, 5), (0xF8, 255, 5), (0xF8, 254, 5),
            (0xF8, 3, 5), (0xF7, 7, 5), (0xF6, 23))]
        movimientos = [bytes(x) for x in ((0x81,), (0x80, 1), (0x80, 0x80, 255),
            (0x80, 0x7A, 4, 255), (0x80, 0x7E, 0, 0x96))]
        programas = [(canal, p) for canal in range(3)
                     for p in comunes + (imagenes if canal == 0 else movimientos)]
        for (canal, programa), contador, numero in product(programas, (0, 1, 2, 255),
                                                          range(1 if self.muestra else 8)):
            datos = self.preparar(numero)
            indice = 1 if programa in (bytes((5, 0x7F)), bytes((5, 0x79))) else 0
            self.registro(datos, 0x2C4C, canal, programa, contador, indice)
            self.contrastar('canales', datos, 0)
        for numero in range(12 if self.muestra else 128):
            datos = self.preparar(numero)
            for canal, posicion in enumerate((0x2C0A, 0x2C4C, 0x2C6D)):
                self.registro(datos, posicion, canal, bytes((0x78, 13, 8)), 1)
                for otro in range(canal + 1, 3):
                    escribir(datos, posicion + 0x10 + otro * 5, 0x9600)
                    datos[posicion + 0x13 + otro * 5] = 1
            self.contrastar('espera_compartida', datos, 2)
        for primero, numero in product((bytes((0x7C,)), bytes((0x7D,)), bytes((0x7B, 3)),
                                        bytes((0xFE,)), bytes((0xFD, 4)), bytes((0x78, 4, 8))),
                                       range(2 if self.muestra else 16)):
            datos = self.preparar(numero)
            self.registro(datos, 0x2C4C, 0, primero)
            for canal, valor in ((1, 2), (2, 253)):
                base = 0x2C4C + 0x10 + canal * 5
                referencia = 0x9000 + canal * 0x200
                escribir(datos, base, referencia)
                datos[base + 2:base + 5] = bytes((0, 1, 2))
                datos[referencia] = valor
            self.contrastar('canales_combinados', datos, 0)
        for canal, indice, estado in product(range(3), (0, 255), (0, 1, 2, 3)):
            datos = self.preparar(estado)
            self.registro(datos, 0x2C4C, canal, bytes((1,)), 1, indice)
            datos[0x2C4C] = estado
            escribir(datos, 0x2C4E, 65535)
            escribir(datos, 0x2C50, 32767)
            self.contrastar('fronteras', datos, 0)
        for y, alto in ((299, 32768), (300, 32768), (300, 1), (32767, 65535)):
            datos = self.preparar(0)
            self.registro(datos, 0x2C4C, 2, bytes((0,)))
            escribir(datos, 0x2C50, y)
            escribir(datos, 0x2C54, alto)
            self.contrastar('fronteras', datos, 0)
        for numero in range(4 if self.muestra else 32):
            for programa, esperado, destino in ((bytes((0xFD, 0xFB, 7)), 3, 0x84E9),
                                                (bytes((0xF8, 4, 5)), 4, 0x8574)):
                datos = self.preparar(numero)
                self.registro(datos, 0x2C4C, 0, programa)
                self.contrastar('operaciones_pendientes', datos, 0, esperado=esperado, destino=destino)
        for numero, canal in product(range(4 if self.muestra else 64), range(3)):
            datos = self.preparar(numero)
            self.registro(datos, 0x2C4C, canal, bytes((0x7A, 2, 1, 255, 0x7B, 3, 0x7F)))
            semilla = self.semilla_original
            for paso in range(8 if self.muestra else 32):
                semilla = self.contrastar('secuencias_persistentes', datos, 2, semilla=semilla)
        for x, y, numero in product((-97, -96, -95, 383, 384, 385),
                                    (-99, -98, -97, 249, 250, 251),
                                    range(1 if self.muestra else 8)):
            datos = self.preparar(numero)
            self.registro(datos, 0x2C4C, 0, bytes((1,)), 2)
            escribir(datos, 0x2C4E, x)
            escribir(datos, 0x2C50, y)
            self.contrastar('limites', datos, 1)
        for numero in range(6 if self.muestra else 64):
            datos = self.preparar(numero)
            for i in range(70):
                canal, programa = programas[(i + numero) % len(programas)]
                indice = 1 if programa in (bytes((5, 0x7F)), bytes((5, 0x79))) else 0
                # Cada canal comparte programa; el último preparado queda presente para todos.
                self.registro(datos, 0x2C0A + i * 33, canal, programa, 1 + i % 3, indice)
            # Una secuencia común segura por canal evita sobrescribir operandos entre registros.
            for canal in range(3):
                referencia = 0x9000 + canal * 0x200
                datos[referencia:referencia + 256] = bytes([1] * 256)
                datos[referencia:referencia + 7] = bytes((0x7A, 2, 1, 255, 0x7B, 3, 0x7F))
            escribir(datos, 0x3588, (0, 1, 2, 35, 69, 2)[numero % 6])
            self.contrastar('recorrido_completo', datos, 2)
        informe = {'sha256_ejecutable': self.cabecera['sha256'],
            'sha256_biblioteca_comparada': hashlib.sha256(self.biblioteca.read_bytes()).hexdigest(),
            'version_unicorn': self.unicorn.__version__, 'casos_por_grupo': self.resumen,
            'casos_totales': sum(self.resumen.values()), 'resultado': 'coinciden', 'muestra': self.muestra,
            'instrucciones_visitadas': [f'{p:04X}' for p in sorted(self.instrucciones_visitadas)],
            'alcance': 'DS completo y generador; canales, retirada y recorrido con pila equilibrada. '
                'Recursos sintéticos. Excluye llamada de sonido FB y carga DOS de modelos no residentes.'}
        salida = self.raiz / 'trabajo/comparacion_secuencias' / ('muestra.json' if self.muestra else 'resultado.json')
        salida.parent.mkdir(parents=True, exist_ok=True)
        salida.write_text(json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
        for grupo, cantidad in self.resumen.items(): print(f'{grupo}: {cantidad} casos coincidentes.')
        print(f'Total: {informe["casos_totales"]} casos. Coinciden los datos y el estado aleatorio.')
        print(f'Informe local: {salida}')


if __name__ == '__main__':
    argumentos = argparse.ArgumentParser(description='Compara el avance de secuencias de animación.')
    argumentos.add_argument('--biblioteca', type=Path, required=True)
    argumentos.add_argument('--muestra', action='store_true')
    opciones = argumentos.parse_args()
    try:
        ComparacionSecuencias(Path(__file__).resolve().parents[1], opciones.biblioteca, opciones.muestra).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        argumentos.exit(1, f'Error: {error}\n')
