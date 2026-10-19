"""Contrasta el regreso al mago, el paso del guardia y la llegada al valle; POST aún sin SDL."""

import argparse
import ctypes
import json
from pathlib import Path

from comparar_loro import ComparacionLoro, ErrorAnalisis
from comparar_calles import ComparacionCalles
from comparar_objetos import palabra, guardar


class ComparacionRegresoLoro(ComparacionLoro):
    # Conserva los clics originales junto al borde, usando el mismo adaptador de las calles.
    def ruta(self, d, puntos, salida):
        encontrada = None
        for x, y in puntos:
            encontrada = ComparacionCalles.ir(self, d, self.semilla_menu, x, y)
            if encontrada is not None:
                break
        if encontrada != salida:
            raise ErrorAnalisis(f'Regreso del loro: salida {encontrada}, esperada{salida}.')
        self.cambiar(d, self.semilla_menu, salida)
        if self.andar(d, self.semilla_menu, 65535, 128) is not None:
            raise ErrorAnalisis('La entrada siguiente abandona la escena antes de tiempo.')

    # El acceso72 desde MAGE se comprueba como contrato aislado; no inventa su activación argumental.
    def contrato_mago(self, base):
        d = bytearray(base)
        semilla = list(self.semilla_menu)
        self.cambiar(d, self.semilla_menu, 72)
        if self.andar(d, self.semilla_menu, 65535, 128) is not None:
            raise ErrorAnalisis('El contrato de acceso al loro abandona WISE.')
        self.hito_borde(d, 'variante_mago_loro')
        self.auditar(d, 'contrato aislado MAGE72')
        self.semilla_menu[:] = semilla
        self.retomar_recursos(base)

    # Contrasta C794 directamente: inventario, fronteras y agotamiento, sin el planificador de movimiento.
    def variantes_poste(self, base):
        self.nativa.procesar_poste.argtypes = [
            ctypes.POINTER(ctypes.c_ubyte),
            ctypes.POINTER(ctypes.c_ushort),
            self.tipo_dimensiones,
            ctypes.POINTER(ctypes.c_uint),
        ]
        self.nativa.procesar_poste.restype = ctypes.c_int
        numero = 0
        for activo in (0, 1, 3):
            for objetos in ((), (111,), (112,), (111, 112)):
                for global_x in (0, 319, 320, 599, 600, 65535):
                    for camara in (0, 256):
                        for lleno in (False, True):
                            d = bytearray(base)
                            if lleno:
                                for registro in range(0x2C0A + palabra(d, 0x3588) * 33, 0x3510, 33):
                                    d[registro] = 1
                            d[0x2C4C] = activo
                            guardar(d, 0x3591, camara)
                            guardar(d, 0x2C2D, global_x - camara)
                            guardar(d, 0x2C2F, (0, 100, 65535)[numero % 3])
                            for i in range(10):
                                guardar(
                                    d, 0x5225 + i * 2, objetos[i] if i < len(objetos) else 65535
                                )
                            semilla = (ctypes.c_ushort * 4)(
                                *((n + numero * 17) & 65535 for n in self.semilla_menu)
                            )
                            self.disponer(d, self.geometria, semilla)
                            self.ejecutar_original(0xC794)
                            esperado = bytes(self.maquina.mem_read(self.segmento, 65536))
                            azar = bytes(self.maquina.mem_read(0x78DE, 8))
                            sonidos = ctypes.c_uint()
                            correcto = self.nativa.procesar_poste(
                                (ctypes.c_ubyte * 65536).from_buffer(d),
                                semilla,
                                self.callback_dimensiones,
                                ctypes.byref(sonidos),
                            )
                            if (
                                not correcto
                                or bytes(d) != esperado
                                or bytes(semilla) != azar
                                or sonidos.value
                            ):
                                diferencias = [
                                    f'{i:04X}' for i in range(65536) if d[i] != esperado[i]
                                ][:8]
                                raise ErrorAnalisis(
                                    f'C794 variante{numero} distinta: {diferencias}.'
                                )
                            numero += 1
        self.resumen['variantes_poste'] = numero
        self.retomar_recursos(base)

    # Retoma la partida real del loro y valida el segundo pago, el guardia y la entrada52.
    def ejecutar(self):
        origen = self.raiz / 'apps/juego/trabajo/comparacion_loro/loro_regreso'
        if not origen.with_suffix('.ds.bin').exists():
            ComparacionLoro(self.raiz, self.biblioteca, True).ejecutar()
        d = bytearray(origen.with_suffix('.ds.bin').read_bytes())
        self.semilla_menu[:] = json.loads(
            origen.with_suffix('.semilla.json').read_text(encoding='utf-8')
        )
        self.destino = self.raiz / 'apps/juego/trabajo/comparacion_regreso_loro'
        self.destino.mkdir(parents=True, exist_ok=True)
        self.resumen = {}
        self.retomar_recursos(d)
        self.ruta(d, ((160, 175),), 49)
        self.ruta(d, ((420, 185), (470, 185), (479, 180)), 40)
        self.ruta(d, ((220, 180), (235, 120), (225, 70), (170, 50), (170, 5)), 28)
        self.ruta(d, ((115, 170), (110, 130)), 32)
        self.hito_borde(d, 'regreso_mago')
        self.auditar(d, 'mago después del loro')
        self.elegir_objeto(d, (2, 6), 76)
        self.elegir_objeto(d, (), 77)
        if self.andar(d, self.semilla_menu, 65535, 192) is not None:
            raise ErrorAnalisis('El segundo pago produce una salida inesperada.')
        self.hito_borde(d, 'regreso_mago_pagado')
        self.auditar(d, 'mago con nuevas ofertas')
        if not d[0x402E]:
            raise ErrorAnalisis('El pago no conserva la progresión original del mago.')
        if self.andar(d, self.semilla_menu, 65535, 256) is not None:
            raise ErrorAnalisis('La espera posterior abandona MAGE.')
        self.contrato_mago(d)
        self.ruta(d, ((150, 185), (150, 199)), 44)
        self.ruta(d, ((120, 170), (100, 195), (100, 199)), 34)
        self.ruta(d, ((60, 180), (8, 100), (0, 70)), 30)
        for x, y in ((420, 185), (210, 180), (200, 145), (100, 145), (80, 165), (20, 165)):
            if ComparacionCalles.ir(self, d, self.semilla_menu, x, y) is not None:
                raise ErrorAnalisis('El guardia provoca una salida antes de alcanzar el borde.')
        self.hito_borde(d, 'regreso_guardia')
        self.auditar(d, 'paso inferior del guardia')
        if ComparacionCalles.ir(self, d, self.semilla_menu, 0, 165) != 52:
            raise ErrorAnalisis('El paso del guardia no solicita52.')
        self.cambiar(d, self.semilla_menu, 52)
        self.hito_borde(d, 'valle_post_cargado')
        if self.andar(d, self.semilla_menu, 65535, 128) is not None:
            raise ErrorAnalisis('La llegada a POST abandona la escena.')
        self.hito_borde(d, 'valle_post_entrada')
        self.auditar(d, 'llegada a POST')
        self.variantes_poste(d)
        informe = {
            'resultado': 'coinciden',
            'casos_y_pasos': sum(self.resumen.values()),
            'grupos': self.resumen,
            'capturas': self.capturas,
            'estados_auditados': self.estados_auditados,
            'alcance': 'Regreso real al mago, segundo pago, guardia y llegada52 aPOST35; contrato aislado MAGE72 y rutinaC794. No acredita los puzles posteriores ni SDL de POST.',
        }
        (self.destino / 'resultado.json').write_text(
            json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8'
        )
        print(
            f'Regreso del loro: {informe["casos_y_pasos"]} casos/pasos y {self.capturas} composiciones coincidentes.'
        )


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Contrasta la continuación real después del loro.')
    parser.add_argument('--biblioteca', type=Path, required=True)
    parser.add_argument('--muestra', action='store_true')
    opciones = parser.parse_args()
    try:
        ComparacionRegresoLoro(
            Path(__file__).resolve().parents[3], opciones.biblioteca, opciones.muestra
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        parser.exit(1, f'Error: {error}\n')
