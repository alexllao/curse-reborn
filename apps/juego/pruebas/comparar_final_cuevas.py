"""Contrasta agujeros, barro, moneda, pozo, casco y cubo desde la partida persistente."""

import argparse
import ctypes
import json
from pathlib import Path

from comparar_catapulta import ComparacionCatapulta, ErrorAnalisis
from comparar_objetos import guardar, palabra


class ComparacionFinalCuevas(ComparacionCatapulta):
    # Aporta el servicio de fragmentos con el mismo banco y estado aleatorio que el adaptador SDL.
    def __init__(self, raiz, biblioteca, muestra):
        super().__init__(raiz, biblioteca, muestra)
        self.carpeta_resultados = 'comparacion_final_cuevas'
        memoria = ctypes.POINTER(ctypes.c_ubyte)
        self.nativa.seleccionar_menu_recursos.argtypes = [
            memoria,
            ctypes.c_ushort,
            memoria,
            ctypes.c_uint,
            ctypes.POINTER(ctypes.c_ushort),
        ]
        self.nativa.seleccionar_menu_recursos.restype = ctypes.c_int
        self.semilla_menu = (ctypes.c_ushort * 4)(*self.semilla_original)
        self.recursos_menu = False

    # Conserva una referencia reanudable y descarta traducciones del emulador entre bancos y variantes.
    def captura(self, datos, carpeta, nombre):
        super().captura(datos, carpeta, nombre)
        if self.recursos_menu:
            (carpeta / f'{nombre}.semilla.json').write_text(
                json.dumps(list(self.semilla_menu)), encoding='utf-8'
            )
        self.maquina.ctl_flush_tb()

    # Compara el manejador completo, incluidos los fragmentos y todas las escrituras en DS y azar.
    def seleccionar(self, datos, casilla):
        if not self.recursos_menu:
            return super().seleccionar(datos, casilla)
        self.preparar_bancos_originales()
        m, r = self.maquina, self.r
        manejador = palabra(datos, 0xB60C + casilla * 2)
        m.mem_write(self.segmento, bytes(datos))
        m.mem_write(0x78DE, bytes(self.semilla_menu))
        m.reg_write(r.UC_X86_REG_BX, casilla)
        self.ejecutar_original(manejador)
        esperado = bytes(m.mem_read(self.segmento, 65536))
        azar = bytes(m.mem_read(0x78DE, 8))
        resultado = self.nativa.seleccionar_menu_recursos(
            (ctypes.c_ubyte * 65536).from_buffer(datos),
            casilla,
            (ctypes.c_ubyte * len(self.escena)).from_buffer_copy(self.escena),
            len(self.escena),
            self.semilla_menu,
        )
        if resultado not in (0, 1) or bytes(datos) != esperado or bytes(self.semilla_menu) != azar:
            diferencias = [
                f'{i:04X}:{datos[i]:02X}/{esperado[i]:02X}'
                for i in range(65536)
                if datos[i] != esperado[i]
            ][:8]
            raise ErrorAnalisis(
                f'Menú {manejador:04X} distinto: {diferencias}, retorno={resultado}.'
            )
        if (resultado == 1) != bool(m.reg_read(r.UC_X86_REG_EFLAGS) & 1):
            raise ErrorAnalisis('El retorno del menú final no coincide con el acarreo original.')
        self.resumen['selecciones'] = self.resumen.get('selecciones', 0) + 1

    # Examina cada agujero desde una aproximación que respeta los salientes originales del fondo.
    def recoger_ramitas(self, datos, semilla, carpeta):
        for id in (65, 66, 67, 68):
            if id == 68:
                self.caminar_hasta(datos, semilla, 245, 185)
            p = 0x2CF1 + (id - 65) * 33
            self.caminar_hasta(
                datos,
                semilla,
                palabra(datos, p + 2) + (4 if id == 68 else 19),
                palabra(datos, p + 4) + 42,
            )
            self.elegir_objeto(datos, (3,), id)
            self.andar(datos, semilla, 0xFFFF, 192)
            self.captura(datos, carpeta, f'final_agujero_{id}')
        self.elegir_objeto(datos, (1,), 64)
        self.andar(datos, semilla, 0xFFFF, 192)
        self.captura(datos, carpeta, 'final_ramitas')

    # Contrasta todos los verbos y parejas del tramo, también antes y después del puzle.
    def opciones_finales(self, base, objetos, recogibles):
        for manejador in (0x384A, 0x2FCA, 0x3143):
            for objeto in objetos:
                d = bytearray(base)
                guardar(d, 0xB60C, manejador)
                guardar(d, 0xB620, objeto + 0x1A)
                self.seleccionar(d, 0)
        for accion in range(8):
            for previo in (0xFFFF, *objetos):
                for objeto in objetos:
                    d = bytearray(base)
                    guardar(d, 0xB60C, 0x3B53)
                    guardar(d, 0xB620, objeto + 0x1A)
                    guardar(d, 0xB634, accion)
                    guardar(d, 0xB638, previo)
                    self.seleccionar(d, 0)

        for objeto in recogibles:
            for ocupadas in (0, 9, 10):
                for puntos in (0, 65535):
                    d = bytearray(base)
                    for i in range(10):
                        guardar(d, 0x5225 + i * 2, 3 if i < ocupadas else 0xFFFF)
                    guardar(d, 0x35C7, puntos)
                    guardar(d, 0xB60C, 0x563A)
                    guardar(d, 0xB620, objeto + 0x1A)
                    self.seleccionar(d, 0)

        self.maquina.ctl_flush_tb()

    # Las seis recetas conservan control, modelo, capacidad y desbordamiento de puntos.
    def opciones_recetas(self, base):
        for actual, previo in (
            (37, 64),
            (64, 37),
            (37, 49),
            (49, 37),
            (64, 49),
            (49, 64),
            (49, 71),
            (71, 49),
            (64, 72),
            (72, 64),
            (37, 73),
            (73, 37),
        ):
            for modelo in (14, 254, 255):
                for libre in (0, 1):
                    for lleno in (False, True):
                        d = bytearray(base)
                        d[0x358C] = modelo
                        d[0x3594] = libre
                        for i in range(10):
                            guardar(d, 0x5225 + i * 2, 3 if lleno else 0xFFFF)
                        guardar(d, 0x35C7, 65535)
                        guardar(d, 0xB60C, 0x3B53)
                        guardar(d, 0xB620, actual + 0x1A)
                        guardar(d, 0xB634, 7)
                        guardar(d, 0xB638, previo)
                        self.seleccionar(d, 0)

        self.maquina.ctl_flush_tb()

    # Continúa por las salidas normales sin insertar objetos ni resolver condiciones mediante banderas.
    def despues_catapulta(self, datos, semilla, carpeta):
        self.recursos_menu = True
        self.semilla_menu = semilla
        self.cruzar_cueva(datos, semilla, 160, 199, 22)
        self.recoger_piedra(datos, semilla, 47)
        self.captura(datos, carpeta, 'final_moneda')
        self.caminar_hasta(datos, semilla, 548, 130)
        self.cruzar_cueva(datos, semilla, 548, 80, 16)
        self.opciones_finales(datos, (3, 64, 65, 66, 67, 68, 69), (64,))
        self.recoger_ramitas(datos, semilla, carpeta)
        self.opciones_finales(datos, (3, 64, 65, 66, 67, 68, 69), (64,))
        self.cruzar_cueva(datos, semilla, 160, 199, 23)
        self.caminar_hasta(datos, semilla, 348, 130)
        self.cruzar_cueva(datos, semilla, 348, 80, 14)
        self.cruzar_cueva(datos, semilla, 160, 199, 10)
        self.caminar_hasta(datos, semilla, 860, 179)
        self.elegir_objeto(datos, (2, 7), 63)
        self.elegir_objeto(datos, (), 36)
        self.andar(datos, semilla, 0xFFFF, 256)
        self.captura(datos, carpeta, 'final_trampa')
        self.recoger_piedra(datos, semilla, 37)
        self.captura(datos, carpeta, 'final_barro')
        self.cruzar_cueva(datos, semilla, 598, 90, 8)
        self.caminar_hasta(datos, semilla, 348, 130)
        self.cruzar_cueva(datos, semilla, 348, 80, 6)
        self.elegir_objeto(datos, (2, 5), 47)
        self.andar(datos, semilla, 0xFFFF, 192)
        self.captura(datos, carpeta, 'final_ofertas_pozo')
        self.opciones_finales(datos, (3, 47, 51, 52, 53), (51, 52, 53))
        self.elegir_objeto(datos, (1,), 53)
        self.andar(datos, semilla, 0xFFFF, 192)
        self.captura(datos, carpeta, 'final_casco_suelo')
        self.recoger_piedra(datos, semilla, 53)
        self.captura(datos, carpeta, 'final_casco')
        self.cruzar_cueva(datos, semilla, 150, 199, 18)
        self.cruzar_cueva(datos, semilla, 160, 199, 5)
        self.cruzar_cueva(datos, semilla, 148, 90, 7)
        self.caminar_hasta(datos, semilla, 820, 160)
        self.elegir_objeto(datos, (2, 4), 53)
        self.andar(datos, semilla, 0xFFFF, 256)
        self.captura(datos, carpeta, 'final_desprendimiento')
        self.caminar_hasta(datos, semilla, 1048, 130)
        self.cruzar_cueva(datos, semilla, 1048, 80, 13)
        self.captura(datos, carpeta, 'final_cubo')
        self.opciones_finales(datos, (3, 37, 49, 60, 64, 72, 74), ())
        self.opciones_recetas(datos)
        self.elegir_objeto(datos, (6,), 60)
        self.andar(datos, semilla, 0xFFFF, 192)
        self.captura(datos, carpeta, 'final_dentro_cubo')
        self.elegir_objeto(datos, (2, 7), 37)
        self.seleccionar(datos, 0)
        self.elegir_objeto(datos, (), 49)
        self.andar(datos, semilla, 0xFFFF, 96)
        self.elegir_objeto(datos, (2, 7), 72)
        self.seleccionar(datos, 0)
        self.elegir_objeto(datos, (), 64)
        self.andar(datos, semilla, 0xFFFF, 96)
        self.captura(datos, carpeta, 'final_mascara')
        self.elegir_objeto(datos, (2, 4), 74)
        for _ in range(512):
            salida = self.andar(datos, semilla, 0xFFFF, 1)
            if salida is not None:
                break
        else:
            raise ErrorAnalisis('La máscara no permite salir por el cubo.')
        if salida != 24:
            raise ErrorAnalisis(f'El cubo solicita una salida distinta: {salida}.')
        self.cambiar(datos, semilla, salida)
        self.captura(datos, carpeta, 'final_salida_cuevas')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Compara el final de las cuevas con DOS.')
    parser.add_argument('--biblioteca', type=Path, required=True)
    parser.add_argument('--muestra', action='store_true')
    opciones = parser.parse_args()
    try:
        ComparacionFinalCuevas(
            Path(__file__).resolve().parents[3], opciones.biblioteca, opciones.muestra
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        parser.exit(1, f'Error: {error}\n')
