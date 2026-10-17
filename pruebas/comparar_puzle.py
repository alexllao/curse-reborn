"""Contrasta acciones de la pared, recogida y apertura de puerta con el ejecutable original."""

import argparse
import ctypes
import json
import struct
from pathlib import Path

from comparar_jugabilidad import ComparacionJugabilidad, ErrorAnalisis
from comparar_objetos import ComparacionObjetos, guardar, palabra


class ComparacionPuzle(ComparacionJugabilidad):
    # Detiene también los saltos a bloques ya traducidos por Unicorn en llamadas anteriores.
    def ejecutar_hasta(self, entrada, final):
        # El límite de emu_start puede quedar dentro de un bloque que ya estaba en su caché.
        def detener(maquina, direccion, tamano, contexto):
            if direccion == final:
                maquina.emu_stop()

        gancho = self.maquina.hook_add(self.unicorn.UC_HOOK_CODE, detener)
        try:
            super().ejecutar_hasta(entrada, final)
        finally:
            self.maquina.hook_del(gancho)

    # Abre el menú real antes de seleccionar sus casillas; no sustituye inventario ni guiones.
    def menu(self, datos, casillas):
        self.nativa.procesar_menu(0, (ctypes.c_ubyte * 65536).from_buffer(datos), 152, 92)
        for casilla in casillas:
            self.seleccionar(datos, casilla)

    # Compara cada paso persistente, incluidas las colisiones y las mutaciones de las listas.
    def esperar(self, datos, semilla, pasos):
        for _ in range(pasos):
            self.paso(datos, semilla)

    # Compara el plan de movimiento con 0F5F después de construir el destino con 89DA.
    def caminar(self, datos, semilla, x, y):
        guardar(datos, 0x2C0C, x)
        guardar(datos, 0x2C0E, y)
        self.bloque('caminos_puzle', 1, 0x89DA, datos, self.geometria)
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
            raise ErrorAnalisis('Planificación del recorrido del puzle distinta.')
        self.esperar(datos, semilla, 96)

    # Recorre los fragmentos reales del cargador que preparan las entradas 0 y 2, sin servicios DOS.
    def entrada(self, datos, salida):
        m, r = self.maquina, self.r
        m.mem_write(self.segmento, bytes(datos))
        m.mem_write(0x000E, struct.pack('<H', 0x6400))
        m.reg_write(r.UC_X86_REG_BX, salida)
        self.ejecutar_hasta(0x4B05, 0x4B14)
        m.reg_write(r.UC_X86_REG_SI, 0x85AC + salida * 14)
        m.reg_write(r.UC_X86_REG_DI, 0x2C2B)
        self.ejecutar_hasta(0x4C43, 0x4D2B)
        m.reg_write(r.UC_X86_REG_BX, salida)
        self.ejecutar_hasta(0x4D2B, 0x4D40)
        self.ejecutar_hasta(0x5145, 0x5182)
        esperado = bytes(m.mem_read(self.segmento, 65536))
        if (
            self.nativa.procesar_menu(
                4, (ctypes.c_ubyte * 65536).from_buffer(datos), salida, 0x6400
            )
            != 0
            or bytes(datos) != esperado
        ):
            diferencias = [f'{i:04X}' for i in range(65536) if datos[i] != esperado[i]][:8]
            raise ErrorAnalisis(f'Entrada {salida}: diferencias {diferencias}.')
        self.resumen['entradas'] = self.resumen.get('entradas', 0) + 1

    # Permite prolongar la partida validada sin inventar un segundo estado inicial para el corredor.
    def despues_puerta(self, datos, semilla, salida):
        pass

    # Cubre las ramas de aceptación y rechazo y después el puzle con sus menús y estado persistente.
    def ejecutar(self):
        preparacion = ComparacionObjetos(self.raiz, self.biblioteca, True)
        datos = preparacion.caso('entrada', 4, bytearray(self.base), 'MANA.BRD')
        guardar(datos, 0x2C4A, 0x5400)
        preparacion.caso('carga', 0, datos)
        guardar(datos, 0x2C29, 0x7400)
        guardar(datos, 0x0DF0, self.cantidad)
        guardar(datos, 0x0DF2, 0)
        self.resumen = {}
        for salida in (0, 2):
            d = bytearray(datos)
            guardar(d, 0x8082, 0 if salida == 0 else 1)
            self.entrada(d, salida)
        for ventana in (0, 24, 28):
            for activa in (0, 1):
                for x in (50, 90, 150):
                    d = bytearray(datos)
                    guardar(d, 0x8082, 1)
                    d[0x3597] = ventana
                    d[0x3594] = activa
                    guardar(d, 0x2C2D, x)
                    self.bloque('rutina_corredor', 5, 0x1791, d, self.geometria)
        for manejador, ids in (
            (0x563A, (5,)),
            (0x2FCA, range(6)),
            (0x3143, range(6)),
            (0x384A, range(6)),
        ):
            for id in ids:
                for lleno in (False, True):
                    d = bytearray(datos)
                    if lleno:
                        for i in range(10):
                            guardar(d, 0x5225 + i * 2, i + 40)
                    guardar(d, 0xB60C, manejador)
                    guardar(d, 0xB620, id + 0x1A)
                    self.seleccionar(d, 0)
        # El despacho combina ambos órdenes, rechaza parejas incorrectas y distingue la fase de la entidad.
        for accion in range(8):
            for previo in (0xFFFF, *range(6)):
                for objeto in range(6):
                    d = bytearray(datos)
                    d[0x2C8F] = 3 if objeto & 1 else 4
                    guardar(d, 0xB60C, 0x3B53)
                    guardar(d, 0xB620, objeto + 0x1A)
                    guardar(d, 0xB634, accion)
                    guardar(d, 0xB638, previo)
                    self.seleccionar(d, 0)
        for fotograma in (0, 3, 4, 255) if self.muestra else range(256):
            d = bytearray(datos)
            d[0x2C8F] = fotograma
            guardar(d, 0xB60C, 0x3B53)
            guardar(d, 0xB620, 0x1C)
            guardar(d, 0xB634, 2)
            guardar(d, 0xB638, 0xFFFF)
            self.seleccionar(d, 0)
        semilla = (ctypes.c_ushort * 4)(*self.semilla_original)
        self.esperar(datos, semilla, 1)
        self.menu(datos, (4, 1))
        self.esperar(datos, semilla, 192)
        self.menu(datos, (1, 0))
        self.esperar(datos, semilla, 8)
        self.menu(datos, (2, 0, 0, 0, 1))
        self.esperar(datos, semilla, 192)
        self.caminar(datos, semilla, 210, 157)
        self.menu(datos, (2, 2))
        self.seleccionar(datos, 1)
        self.esperar(datos, semilla, 192)
        self.menu(datos, (1,))
        self.seleccionar(datos, 0)
        self.esperar(datos, semilla, 192)
        self.caminar(datos, semilla, 55, 157)
        self.menu(datos, (2, 0, 0))
        self.seleccionar(datos, 1)
        self.seleccionar(datos, 1)
        self.esperar(datos, semilla, 192)
        if datos[0x2CB0] != 0x10:
            raise ErrorAnalisis(f'La puerta no se abrió: {datos[0x2CB0]}.')
        salida = self.raiz / 'trabajo' / getattr(self, 'carpeta_resultados', 'comparacion_puzle')
        salida.mkdir(parents=True, exist_ok=True)
        self.componer_resultado(datos, salida)
        self.despues_puerta(datos, semilla, salida)
        ruta = salida / ('muestra.json' if self.muestra else 'resultado.json')
        ruta.write_text(
            json.dumps(
                {
                    'resultado': 'coinciden',
                    'casos_por_grupo': self.resumen,
                    'casos_totales': sum(self.resumen.values()),
                    'puerta_abierta': True,
                    'recorrido_por_menu': True,
                    'muestra': self.muestra,
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
            f'Total: {sum(self.resumen.values())} casos/pasos. Coinciden memoria, guiones y composición.'
        )
        print(f'Informe local: {ruta}')


if __name__ == '__main__':
    argumentos = argparse.ArgumentParser(description='Compara el puzle de la mazmorra.')
    argumentos.add_argument('--biblioteca', type=Path, required=True)
    argumentos.add_argument('--muestra', action='store_true')
    opciones = argumentos.parse_args()
    try:
        ComparacionPuzle(
            Path(__file__).resolve().parents[1], opciones.biblioteca, opciones.muestra
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        argumentos.exit(1, f'Error: {error}\n')
