"""Contrasta BOXDET, caminos, colisiones y la liberación del personaje con el bucle original."""

import argparse
import ctypes
import json
from pathlib import Path
import struct

from comparar_menu import ComparacionMenu, ErrorAnalisis
from comparar_objetos import ComparacionObjetos, guardar, palabra


class ComparacionJugabilidad(ComparacionMenu):
    # Conserva bancos separados y añade los contratos del bucle jugable, sin simular sus resultados.
    def __init__(self, raiz, biblioteca, muestra):
        super().__init__(raiz, biblioteca, muestra)
        memoria = ctypes.POINTER(ctypes.c_ubyte)
        palabras = ctypes.POINTER(ctypes.c_ushort)
        self.nativa.procesar_geometria.argtypes = [
            ctypes.c_uint,
            memoria,
            memoria,
            ctypes.c_uint,
            palabras,
            palabras,
        ]
        self.nativa.procesar_geometria.restype = ctypes.c_int
        self.nativa.avanzar_jugabilidad.argtypes = [
            memoria,
            palabras,
            self.tipo_dimensiones,
            self.tipo_modelo,
            memoria,
            ctypes.c_uint,
            ctypes.c_ushort,
        ]
        self.nativa.avanzar_jugabilidad.restype = ctypes.c_int
        archivo = (self.carpeta / 'BOXDET.DAT').read_bytes()
        p = struct.unpack_from('<H', archivo)[0]
        self.cantidad = struct.unpack_from('<H', archivo, p)[0]
        self.geometria = archivo[p + 2 : p + 2 + self.cantidad * 10]
        self.buffer_geometria = (ctypes.c_ubyte * len(self.geometria)).from_buffer_copy(
            self.geometria
        )

    # Dispone el bloque de geometría en su propio segmento antes de llamar al programa original.
    def disponer(self, datos, geometria, semilla):
        m = self.maquina
        self.preparar_bancos_originales()
        m.mem_write(0x0018, struct.pack('<H', 0x7A00))
        if geometria:
            m.mem_write(0x7A000, geometria)
        m.mem_write(self.segmento, bytes(datos))
        m.mem_write(0x78DE, struct.pack('<4H', *semilla))

    # Compara DS completo, generador y retornos de un bloque independiente.
    def bloque(self, grupo, operacion, entrada, datos, geometria, parametros=None):
        semilla = (ctypes.c_ushort * 4)(*self.semilla_original)
        self.disponer(datos, geometria, semilla)
        r = self.r
        parametros = parametros or [0] * 6
        if operacion == 2:
            for registro, valor in zip(
                (r.UC_X86_REG_AX, r.UC_X86_REG_BX, r.UC_X86_REG_CX, r.UC_X86_REG_DX), parametros
            ):
                self.maquina.reg_write(registro, valor)
        if operacion == 3:
            self.ejecutar_hasta(0x0A6E, 0x0BA6)
        elif operacion == 4:
            self.ejecutar_hasta(0x0BA6, 0x0BC1)
        else:
            self.ejecutar_original(entrada)
        esperado = bytes(self.maquina.mem_read(self.segmento, 65536))
        azar = bytes(self.maquina.mem_read(0x78DE, 8))
        retorno = self.maquina.reg_read(r.UC_X86_REG_AX)
        parametro = self.maquina.reg_read(r.UC_X86_REG_BX) & 255
        p = (ctypes.c_ushort * 6)(*parametros)
        buffer = (ctypes.c_ubyte * len(geometria)).from_buffer_copy(geometria)
        if not self.nativa.procesar_geometria(
            operacion,
            (ctypes.c_ubyte * 65536).from_buffer(datos),
            buffer,
            len(geometria),
            semilla,
            p,
        ):
            raise ErrorAnalisis(f'{grupo}: bloque nativo rechazado.')
        if (
            bytes(datos) != esperado
            or bytes(semilla) != azar
            or (operacion == 2 and (p[4] != retorno or (retorno and p[5] != parametro)))
        ):
            diferencias = [
                f'{i:04X}: {datos[i]:02X}/{esperado[i]:02X}'
                for i in range(65536)
                if datos[i] != esperado[i]
            ][:8]
            raise ErrorAnalisis(f'{grupo}: diferencias {diferencias}; retorno {p[4]}/{retorno}.')
        self.resumen[grupo] = self.resumen.get(grupo, 0) + 1

    # Recorre 096D–0BC1 después de aplicar la misma dirección y posición anterior que el original.
    def paso(self, datos, semilla, direccion=0xFFFF):
        self.disponer(datos, self.geometria, semilla)
        guardar(datos, 0x0AE6, palabra(datos, 0x2C2D))
        guardar(datos, 0x0AE8, palabra(datos, 0x2C2F))
        self.maquina.mem_write(self.segmento, bytes(datos))
        if direccion != 0xFFFF and datos[0x3594]:
            self.maquina.mem_write(self.segmento + 0x0DFA, b'\0\0')
            self.maquina.reg_write(self.r.UC_X86_REG_SI, direccion)
            self.ejecutar_original(0x0BF8)
        self.ejecutar_hasta(0x096D, 0x0BC4)
        esperado = bytes(self.maquina.mem_read(self.segmento, 65536))
        azar = bytes(self.maquina.mem_read(0x78DE, 8))
        resultado = self.nativa.avanzar_jugabilidad(
            (ctypes.c_ubyte * 65536).from_buffer(datos),
            semilla,
            self.callback_dimensiones,
            self.callback_modelo,
            self.buffer_geometria,
            len(self.geometria),
            direccion,
        )
        if resultado or bytes(datos) != esperado or bytes(semilla) != azar:
            diferencias = [
                f'{i:04X}: {datos[i]:02X}/{esperado[i]:02X}'
                for i in range(65536)
                if datos[i] != esperado[i]
            ][:8]
            raise ErrorAnalisis(
                f'Paso jugable {self.resumen.get("pasos_jugables", 0) + 1}: {diferencias}, retorno={resultado}.'
            )
        self.resumen['pasos_jugables'] = self.resumen.get('pasos_jugables', 0) + 1

    # Cubre límites reales, parámetros de pared, camino directo y la secuencia que antes movía la puerta.
    def ejecutar(self):
        preparacion = ComparacionObjetos(self.raiz, self.biblioteca, True)
        datos = preparacion.caso('entrada', 4, bytearray(self.base), 'MANA.BRD')
        guardar(datos, 0x2C4A, 0x5400)
        preparacion.caso('carga', 0, datos)
        guardar(datos, 0x2C29, 0x7400)
        guardar(datos, 0x0DF0, self.cantidad)
        guardar(datos, 0x0DF2, 0)
        base = bytes(datos)
        for x in (0, 12, 34, 40, 48, 100, 200, 250, 300, 319):
            for y in (140, 152, 160, 175, 199):
                self.bloque(
                    'rectangulos_reales',
                    2,
                    0x254A,
                    bytearray(base),
                    self.geometria,
                    [x, y, x + 8, y + 3, 0, 0],
                )
        for tipo in (0, 1, 2, 3):
            for id in (0, 1, 2, 3, 0x11, 0x38, 0x46, 0x6B):
                g = struct.pack('<5H', tipo + id * 256, 0, 0, 319, 199)
                d = bytearray(base)
                guardar(d, 0x0DF0, 1)
                self.bloque('condiciones', 2, 0x254A, d, g, [50, 150, 60, 155, 0, 0])
        for x, y in ((125, 157), (40, 160), (260, 170), (319, 199)):
            d = bytearray(base)
            guardar(d, 0x2C0C, x)
            guardar(d, 0x2C0E, y)
            self.bloque('destinos', 1, 0x89DA, d, self.geometria)
        g = bytearray(0x1000)
        struct.pack_into('<8H', g, 0, 0, 150, 100, 199, 101, 150, 319, 199)
        struct.pack_into('<H', g, 0x0DF8, 0x40)
        struct.pack_into('<6H', g, 0x40, 4, 8, 0, 12, 20, 0)
        struct.pack_into('<3H', g, 0x4C, 1, 150, 180)
        struct.pack_into('<3H', g, 0x54, 1, 120, 170)
        for origen, objeto in ((50, 200), (200, 50), (50, 70), (400, 50), (50, 400)):
            d = bytearray(base)
            guardar(d, 0x0DF2, 2)
            guardar(d, 0x0DF6, 0)
            guardar(d, 0x2C2D, origen)
            guardar(d, 0x2C2F, 130)
            guardar(d, 0x2C33, 30)
            guardar(d, 0x2C0C, objeto)
            guardar(d, 0x2C0E, 170)
            self.bloque('rutas_enlazadas', 1, 0x89DA, d, bytes(g))
        self.bloque('historial', 4, 0, bytearray(base), self.geometria)
        for escena in (0, 5):
            for activa in (0, 1):
                d = bytearray(base)
                guardar(d, 0x8082, escena)
                d[0x3594] = activa
                guardar(d, 0x2D01, 0)
                self.bloque('despacho_escena', 5, 0x1791, d, self.geometria)
        for x in (0, 30, 40, 100, 232, 250, 300, 315):
            for y in (80, 140, 155, 180):
                d = bytearray(base)
                d[0x3596] = 1
                d[0x2CB0] = 0
                guardar(d, 0x2C2D, x)
                guardar(d, 0x2C2F, y)
                guardar(d, 0x0AE6, 125)
                guardar(d, 0x0AE8, 98)
                self.bloque('correcciones', 3, 0, d, self.geometria)
        semilla = (ctypes.c_ushort * 4)(*self.semilla_original)
        self.paso(datos, semilla)
        # El recorrido usa los menús reales: llamar al guardia, recoger la llave y combinarla con las esposas.
        buffer = (ctypes.c_ubyte * 65536).from_buffer(datos)
        self.nativa.procesar_menu(0, buffer, 152, 92)
        self.seleccionar(datos, 4)
        self.seleccionar(datos, 1)
        for i in range(192):
            self.paso(datos, semilla)
        self.nativa.procesar_menu(0, buffer, 152, 92)
        self.seleccionar(datos, 1)
        self.seleccionar(datos, 0)
        for i in range(8):
            self.paso(datos, semilla)
        self.nativa.procesar_menu(0, buffer, 152, 92)
        for casilla in (2, 0, 0, 0, 1):
            self.seleccionar(datos, casilla)
        for i in range(80 if self.muestra else 192):
            self.paso(datos, semilla)
        if datos[0x3594] != 1 or datos[0x358C] != 0xFF:
            raise ErrorAnalisis('La liberación no habilitó el personaje original.')
        if palabra(datos, 0x2CF3) != 30 or palabra(datos, 0x2CF5) != 83:
            raise ErrorAnalisis('La puerta se desplazó fuera de su posición original.')
        salida = self.raiz / 'apps/juego/trabajo/comparacion_jugabilidad'
        salida.mkdir(parents=True, exist_ok=True)
        self.componer_resultado(datos, salida)
        (salida / 'liberacion_cpp.png').write_bytes(
            (salida / 'interaccion_32_cpp.png').read_bytes()
        )
        posicion_inicial = (palabra(datos, 0x2C2D), palabra(datos, 0x2C2F))
        hubo_movimiento = False
        for direccion in (4, 8, 2, 1, 0):
            for i in range(8 if self.muestra else 32):
                self.paso(datos, semilla, direccion)
                hubo_movimiento = hubo_movimiento or posicion_inicial != (
                    palabra(datos, 0x2C2D),
                    palabra(datos, 0x2C2F),
                )
        if not hubo_movimiento:
            raise ErrorAnalisis('Las direcciones no movieron al personaje liberado.')
        # Los clics conservan el contrato de ruta y planificación antes de consumir sus pasos.
        for x, y in ((180, 157), (80, 160), (150, 185), (300, 180)):
            guardar(datos, 0x2C0C, x)
            guardar(datos, 0x2C0E, y)
            self.bloque('clics_reales', 1, 0x89DA, datos, self.geometria)
            self.maquina.mem_write(self.segmento, bytes(datos))
            self.maquina.reg_write(self.r.UC_X86_REG_SI, palabra(datos, 0x0DFA))
            self.ejecutar_original(0x0F5F)
            esperado = bytes(self.maquina.mem_read(self.segmento, 65536))
            pasos = ctypes.c_ushort()
            if (
                not self.nativa.planificar_movimiento(
                    (ctypes.c_ubyte * 65536).from_buffer(datos),
                    palabra(datos, 0x0DFA),
                    ctypes.byref(pasos),
                )
                or bytes(datos) != esperado
            ):
                raise ErrorAnalisis('Planificación de clic distinta.')
            for i in range(48 if self.muestra else 80):
                self.paso(datos, semilla)
        salida = self.raiz / 'apps/juego/trabajo/comparacion_jugabilidad'
        salida.mkdir(parents=True, exist_ok=True)
        self.componer_resultado(datos, salida)
        (salida / 'movimiento_cpp.png').write_bytes(
            (salida / 'interaccion_32_cpp.png').read_bytes()
        )
        ruta = salida / ('muestra.json' if self.muestra else 'resultado.json')
        ruta.write_text(
            json.dumps(
                {
                    'resultado': 'coinciden',
                    'casos_por_grupo': self.resumen,
                    'casos_totales': sum(self.resumen.values()),
                    'puerta_inmovil': True,
                    'control_habilitado': True,
                },
                ensure_ascii=False,
                indent=2,
            )
            + '\n',
            encoding='utf-8',
        )
        for grupo, cantidad in self.resumen.items():
            print(f'{grupo}: {cantidad} casos coincidentes.')
        print(
            f'Total: {sum(self.resumen.values())} casos/pasos. Coinciden rutas, colisiones y secuencias.'
        )
        print(f'Informe local: {ruta}')


if __name__ == '__main__':
    argumentos = argparse.ArgumentParser(description='Compara caminos y colisiones del juego.')
    argumentos.add_argument('--biblioteca', type=Path, required=True)
    argumentos.add_argument('--muestra', action='store_true')
    opciones = argumentos.parse_args()
    try:
        ComparacionJugabilidad(
            Path(__file__).resolve().parents[4], opciones.biblioteca, opciones.muestra
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        argumentos.exit(1, f'Error: {error}\n')
