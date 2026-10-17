"""Comparación conjunta de listas, movimiento y acciones completas reconstruidas."""

import argparse
import ctypes
import hashlib
from itertools import product
import json
from pathlib import Path
import random
import struct
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'herramientas'))
from analizar_ejecutable import ErrorAnalisis, leer_mz


class Comparacion:
    def __init__(self, raiz, biblioteca, muestra=False):
        try:
            import unicorn
            from unicorn import x86_const
        except ImportError as error:
            raise ErrorAnalisis('Falta Unicorn. Ejecuta «pnpm preparar».') from error
        self.unicorn = unicorn
        self.r = x86_const
        self.raiz = raiz
        self.muestra = muestra
        self.biblioteca = biblioteca
        self.cabecera, imagen = leer_mz(
            (raiz / 'datos_originales' / 'CURSE' / 'CURSE.EXE').read_bytes()
        )
        mapa = json.loads(
            (raiz / 'documentacion' / 'mapa_ejecutable.json').read_text(encoding='utf-8')
        )
        if self.cabecera['sha256'] != mapa['sha256']:
            raise ErrorAnalisis('El ejecutable no corresponde a la copia trasladada.')
        self.maquina = unicorn.Uc(unicorn.UC_ARCH_X86, unicorn.UC_MODE_16)
        self.maquina.mem_map(0, 2 * 1024 * 1024)
        self.maquina.mem_write(0, imagen)
        self.segmento = 0x0F31 * 16
        self.maquina.mem_write(0, struct.pack('<H', 0x0F31))
        self.base = imagen[self.segmento : self.segmento + 65536].ljust(65536, b'\0')
        self.semilla_original = struct.unpack_from('<4H', imagen, 0x78DE)
        self.azar = random.Random(0x886B)
        self.alternativa = self.azar.randbytes(65536)
        self.nativa = ctypes.CDLL(str(biblioteca.resolve()))
        memoria = ctypes.POINTER(ctypes.c_ubyte)
        palabras = ctypes.POINTER(ctypes.c_ushort)
        for nombre in ('reiniciar_accion', 'rechazar_accion'):
            funcion = getattr(self.nativa, nombre)
            funcion.argtypes, funcion.restype = [memoria], None
        self.nativa.gestionar_lista.argtypes = [ctypes.c_uint, memoria, ctypes.c_ushort]
        self.nativa.gestionar_lista.restype = ctypes.c_int
        self.nativa.seleccionar_direccion.argtypes = [memoria, ctypes.c_ushort]
        self.nativa.seleccionar_direccion.restype = ctypes.c_int
        self.nativa.planificar_movimiento.argtypes = [memoria, ctypes.c_ushort, palabras]
        self.nativa.planificar_movimiento.restype = ctypes.c_int
        self.nativa.iniciar_accion.argtypes = [memoria, palabras]
        self.nativa.iniciar_accion.restype = ctypes.c_int
        self.nativa.ejecutar_accion_resultados.argtypes = [memoria, palabras, palabras]
        self.nativa.ejecutar_accion_resultados.restype = ctypes.c_int
        self.nativa.avanzar_movimiento.argtypes = [memoria]
        self.nativa.avanzar_movimiento.restype = ctypes.c_int
        self.resumen = {}

    def datos(self, numero):
        return bytearray(self.base if numero % 2 == 0 else self.alternativa)

    def geometria(self, datos, numero, nodo=0x6000):
        modo = (numero // 25) % 3 if numero < 75 else numero % 3
        datos[0x358C] = (0, 0x14, 0xFE)[modo]
        escribir(datos, 0x8082, 0x24 if modo == 2 else 0)
        datos[0x2C2C] = (0, 8, 9, 17, 18, 26, 127, 128, 136, 137, 255)[numero % 11]
        x, y = self.azar.randrange(320), self.azar.randrange(200)
        escribir(datos, 0x2C2D, x)
        escribir(datos, 0x2C2F, y)
        escribir(datos, 0x2C31, 32)
        escribir(datos, 0x2C33, 48)
        escribir(datos, 0x359A, numero % 17)
        if numero < 75:
            dx, dy = list(product((-32, -1, 0, 1, 32), repeat=2))[numero % 25]
        else:
            dx, dy = self.azar.randrange(-600, 601), self.azar.randrange(-600, 601)
        escribir(datos, nodo + 2, x + 15 + dx)
        escribir(datos, nodo + 4, y + 47 + dy)

    def caso(
        self,
        grupo,
        datos,
        entrada,
        funcion,
        registros=None,
        semilla=None,
        verificar_ax=False,
        verificar_cf=None,
        lista=False,
        conservar_si=False,
        destinos=None,
    ):
        r = self.r
        contexto = {
            r.UC_X86_REG_CS: 0,
            r.UC_X86_REG_DS: 0x0F31,
            r.UC_X86_REG_ES: 0x4567,
            r.UC_X86_REG_SS: 0x8000,
            r.UC_X86_REG_SP: 0xFF00,
            r.UC_X86_REG_AX: 0x5555,
            r.UC_X86_REG_BX: 0x1234,
            r.UC_X86_REG_CX: 0x5678,
            r.UC_X86_REG_DX: 0x1357,
            r.UC_X86_REG_SI: 0x6000,
            r.UC_X86_REG_EFLAGS: 2,
            **(registros or {}),
        }
        self.maquina.mem_write(self.segmento, bytes(datos))
        if semilla is not None:
            self.maquina.mem_write(0x78DE, struct.pack('<4H', *semilla))
        for registro, valor in contexto.items():
            self.maquina.reg_write(registro, valor)
        self.maquina.mem_write(0x8FF00, struct.pack('<H', 0x7000))
        observados = []

        def detener(emulador, posicion, tamano, contexto):
            if posicion in destinos:
                observados.append(posicion)
                emulador.emu_stop()

        gancho = self.maquina.hook_add(self.unicorn.UC_HOOK_CODE, detener) if destinos else None
        try:
            self.maquina.emu_start(entrada, 0x7000, timeout=1_000_000, count=20000)
        finally:
            if gancho is not None:
                self.maquina.hook_del(gancho)
        llegada = observados[0] if len(observados) == 1 else None
        if (
            (destinos and llegada is None)
            or (not destinos and self.maquina.reg_read(r.UC_X86_REG_IP) != 0x7000)
            or self.maquina.reg_read(r.UC_X86_REG_SP) != (0xFF00 if destinos else 0xFF02)
        ):
            raise ErrorAnalisis(f'{grupo}: el original no retornó con la pila equilibrada.')
        esperado_ax = self.maquina.reg_read(r.UC_X86_REG_AX)
        esperado_cf = self.maquina.reg_read(r.UC_X86_REG_EFLAGS) & 1
        memoria = (ctypes.c_ubyte * 65536).from_buffer(datos)
        salida_ax = ctypes.c_ushort(0xBEEF)
        estado = (ctypes.c_ushort * 4)(*(semilla or (0, 0, 0, 0)))
        resultado = funcion(memoria, salida_ax, estado)
        numero = self.resumen.get(grupo, 0)
        esperado = bytes(self.maquina.mem_read(self.segmento, 65536))
        if esperado != datos:
            offset = next(i for i, (a, b) in enumerate(zip(esperado, datos)) if a != b)
            raise ErrorAnalisis(f'{grupo}, caso {numero}: diferencia en DS:{offset:04X}.')
        if destinos:
            if resultado != destinos[llegada]:
                raise ErrorAnalisis(f'{grupo}, caso {numero}: continuación del bloque diferente.')
        elif lista:
            if bool(resultado) != (esperado_cf == 0):
                raise ErrorAnalisis(f'{grupo}, caso {numero}: resultado/acarreos diferentes.')
            if esperado_ax != contexto[r.UC_X86_REG_AX]:
                raise ErrorAnalisis('La operación original de lista modificó AX.')
        elif resultado == 0:
            raise ErrorAnalisis(
                f'{grupo}, caso {numero}: el C++ rechazó una entrada del dominio preparado.'
            )
        if verificar_ax and salida_ax.value != esperado_ax:
            raise ErrorAnalisis(f'{grupo}, caso {numero}: AX diferente.')
        if verificar_cf is not None and esperado_cf != verificar_cf:
            raise ErrorAnalisis(f'{grupo}, caso {numero}: acarreo original inesperado.')
        if conservar_si and self.maquina.reg_read(r.UC_X86_REG_SI) != contexto[r.UC_X86_REG_SI]:
            raise ErrorAnalisis(f'{grupo}: el original no conservó SI.')
        if semilla is not None:
            original = struct.unpack('<4H', self.maquina.mem_read(0x78DE, 8))
            if tuple(estado) != original:
                raise ErrorAnalisis(f'{grupo}, caso {numero}: estado aleatorio diferente.')
        self.resumen[grupo] = numero + 1
        return tuple(estado)

    def ejecutar(self):
        r, n = self.r, self.nativa
        listas = [[0xFFFF] * 10, list(range(10)), [0xFFFF, 42, 42] + [0xFFFF] * 7, [0x8C] * 10]
        for posicion in range(10):
            valores = [0x100 + i for i in range(10)]
            valores[posicion] = 0x8C
            listas.append(valores)
        if self.muestra:
            listas = listas[:4]
        for operacion, entrada in enumerate((0x88BC, 0x88D9, 0x88F3)):
            for lista, valor in product(listas, (0, 1, 0xFFFF, 0x8C, 42)):
                datos = self.datos(operacion)
                struct.pack_into('<10H', datos, 0x5225, *lista)
                self.caso(
                    'listas',
                    datos,
                    entrada,
                    lambda m, ax, e: n.gestionar_lista(operacion, m, valor),
                    {r.UC_X86_REG_AX: valor},
                    lista=True,
                )

        imagenes = (
            (0, 9, 18, 128, 255)
            if self.muestra
            else (0, 1, 8, 9, 17, 18, 26, 27, 127, 128, 136, 137, 255)
        )
        anteriores = (
            (0, 0xA97)
            if self.muestra
            else (0, 0xA97, 0xA7E, 0xA8E, 0xA82, 0xAA9, 0xA86, 0xAA0, 0xA8A, 0x1234)
        )
        indices = (0, 1, 2, 4, 8, 9, 10, 15) if self.muestra else range(16)
        for indice, imagen, anterior, modo in product(indices, imagenes, anteriores, range(3)):
            datos = self.datos(indice)
            datos[0x358C], datos[0x2C2C] = (0, 0x14, 0xFE)[modo], imagen
            escribir(datos, 0x8082, 0x24 if modo == 2 else 0)
            escribir(datos, 0x2C3B, anterior)
            escribir(datos, 0x2C40, (0, 0xAC2, 0xAC4)[anterior % 3])
            escribir(datos, 0x2C45, (0xAC6, 0xAC9, 0xACC, 0x7777)[anterior % 4])
            self.caso(
                'direcciones',
                datos,
                0x0BF8,
                lambda m, ax, e: n.seleccionar_direccion(m, indice),
                {r.UC_X86_REG_SI: indice},
            )

        for numero in range(75 if self.muestra else 2048):
            datos = self.datos(numero)
            nodo = 0xFFFC if numero % 31 == 0 else 0x6000
            self.geometria(datos, numero, nodo)
            self.caso(
                'planificacion',
                datos,
                0x0F5F,
                lambda m, ax, e: n.planificar_movimiento(m, nodo, ctypes.byref(ax)),
                {r.UC_X86_REG_SI: nodo},
                verificar_ax=True,
                verificar_cf=0,
            )

        for numero in range(16 if self.muestra else 1024):
            datos = self.datos(numero)
            escribir(
                datos,
                0x359A,
                (0, 1, 2, 0xFFFF)[numero] if numero < 4 else self.azar.randrange(65536),
            )
            for posicion in (0x359E, 0x35A6, 0x2C2D, 0x35A8, 0x35A2, 0x35AA, 0x2C2F, 0x35AC):
                escribir(
                    datos,
                    posicion,
                    self.azar.choice((0, 1, 0xFFFF, 0x7FFF, 0x8000, self.azar.randrange(65536))),
                )
            self.caso(
                'pasos',
                datos,
                0x096D,
                lambda m, ax, e: n.avanzar_movimiento(m),
                destinos={0x099A: 0, 0x0A59: 1},
            )

        for numero in range(6 if self.muestra else 128):
            for grupo, entrada in (
                ('reinicio', 0x8822),
                ('rechazo', 0x88A0),
                ('inicio_accion', 0x886B),
            ):
                datos = self.datos(numero)
                self.geometria(datos, numero, 0x1A36)
                escribir(datos, 0x0AD7, 0 if numero % 2 == 0 else 0xAB1E)
                if grupo == 'reinicio':
                    # Adapta la firma común del comparador al reinicio de la acción.
                    def funcion(m, ax, e):
                        return n.reiniciar_accion(m)
                elif grupo == 'rechazo':
                    # Adapta la firma común del comparador al rechazo de la acción.
                    def funcion(m, ax, e):
                        return n.rechazar_accion(m)
                else:
                    # Entrega AX por referencia al inicio reconstruido de la acción.
                    def funcion(m, ax, e):
                        return n.iniciar_accion(m, ctypes.byref(ax))

                self.caso(
                    grupo,
                    datos,
                    entrada,
                    funcion,
                    verificar_ax=grupo == 'inicio_accion',
                    verificar_cf=1 if grupo != 'reinicio' else None,
                    conservar_si=True,
                )

        for numero in range(3 if self.muestra else 64):
            semilla = (
                self.semilla_original
                if numero == 0
                else tuple(self.azar.randrange(65536) for _ in range(4))
            )
            for patron in range(2):
                datos = self.datos(patron)
                self.geometria(datos, numero)
                # Retirada, duplicados y marca intermedia en una secuencia persistente.
                struct.pack_into('<10H', datos, 0x5225, 0xFFFF, 0x8C, 0x100, 0x8C, 1, 2, 3, 4, 5, 6)
                estado = semilla
                for paso in range(2 if self.muestra else 8):
                    estado = self.caso(
                        'accion_completa',
                        datos,
                        0xB740,
                        lambda m, ax, e: n.ejecutar_accion_resultados(m, e, ctypes.byref(ax)),
                        semilla=estado,
                        verificar_ax=True,
                        verificar_cf=1,
                        conservar_si=True,
                    )

        informe = {
            'sha256_ejecutable': self.cabecera['sha256'],
            'sha256_biblioteca_comparada': hashlib.sha256(self.biblioteca.read_bytes()).hexdigest(),
            'version_unicorn': self.unicorn.__version__,
            'casos_por_grupo': self.resumen,
            'casos_totales': sum(self.resumen.values()),
            'resultado': 'coinciden',
            'muestra': self.muestra,
            'alcance': 'DS completo, AX y acarreo en los contratos declarados, estado aleatorio de la acción completa y equilibrio de pila original. Sin dibujo, partida completa ni divisiones inválidas.',
        }
        salida = (
            self.raiz
            / 'trabajo'
            / 'comparacion_avance'
            / ('muestra.json' if self.muestra else 'resultado.json')
        )
        salida.parent.mkdir(parents=True, exist_ok=True)
        salida.write_text(
            json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8'
        )
        for grupo, cantidad in self.resumen.items():
            print(f'{grupo}: {cantidad} casos coincidentes.')
        print(
            f'Total: {informe["casos_totales"]} casos. Coinciden las escrituras y los contratos comprobados.'
        )
        print(f'Informe local: {salida}')


def escribir(datos, posicion, valor):
    posicion &= 0xFFFF
    valor &= 0xFFFF
    datos[posicion] = valor & 0xFF
    datos[(posicion + 1) & 0xFFFF] = valor >> 8


if __name__ == '__main__':
    argumentos = argparse.ArgumentParser(
        description='Compara el avance conjunto de acciones y movimiento.'
    )
    argumentos.add_argument('--biblioteca', type=Path, required=True)
    argumentos.add_argument(
        '--muestra',
        action='store_true',
        help='Ejecuta solo una muestra corta; no sustituye la comparación completa.',
    )
    opciones = argumentos.parse_args()
    try:
        Comparacion(
            Path(__file__).resolve().parents[1], opciones.biblioteca, opciones.muestra
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        argumentos.exit(1, f'Error: {error}\n')
