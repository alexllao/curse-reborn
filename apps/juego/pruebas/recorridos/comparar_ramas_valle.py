"""Contrasta primeras ramas del valle con el EXE; aún fuera del catálogo SDL."""

import argparse
import ctypes
import json
from pathlib import Path

from comparar_regreso_loro import ComparacionRegresoLoro, ErrorAnalisis
from comparar_objetos import guardar, palabra


class ComparacionRamasValle(ComparacionRegresoLoro):
    # Exige pasos originales sin salidas imprevistas, conservando DS completo y azar.
    def esperar(self, d, pasos, direccion=65535):
        if self.andar(d, self.semilla_menu, direccion, pasos) is not None:
            raise ErrorAnalisis('La espera del valle solicita una salida inesperada.')

    # Contrasta los umbrales C714/C737 directamente, sin confundir variantes con partida real.
    def variantes_rutinas(self, base):
        rutina = self.nativa.procesar_ramas_valle
        rutina.argtypes = [ctypes.POINTER(ctypes.c_ubyte)]
        rutina.restype = ctypes.c_int
        numero = 0
        for escena in (33, 34):
            for activo in (0, 1, 0xB19C):
                for x in (0, 64, 65, 200, 201, 65535):
                    for y in (109, 110, 65535):
                        for fotograma in (18, 19, 20):
                            d = bytearray(base)
                            guardar(d, 0x8082, escena)
                            guardar(d, 0x2C5C, activo)
                            guardar(d, 0x2C2D, x)
                            guardar(d, 0x2C2F, y)
                            d[0x2C4D] = fotograma
                            guardar(d, 0x2C4E, x)
                            guardar(d, 0x2C50, y)
                            guardar(d, 0x2C52, 20)
                            guardar(d, 0x2C54, 30)
                            guardar(d, 0x3591, 65530 if numero % 2 else 256)
                            self.disponer(d, self.geometria, self.semilla_menu)
                            self.ejecutar_original(0xC714 if escena == 33 else 0xC737)
                            esperado = bytes(self.maquina.mem_read(self.segmento, 65536))
                            if (
                                not rutina((ctypes.c_ubyte * 65536).from_buffer(d))
                                or bytes(d) != esperado
                            ):
                                raise ErrorAnalisis(
                                    f'Rutina del valle distinta en variante{numero}.'
                                )
                            numero += 1
        self.resumen['rutinas_aisladas'] = numero
        self.retomar_recursos(base)

    # Revisa capacidad, iconos de puntos, modelo y protección de las recogidas108/111/112.
    def variantes_objetos(self, base, ids):
        semilla = list(self.semilla_menu)
        for id in ids:
            for ocupadas in (0, 9, 10):
                for modelo in (20, 255):
                    for icono in (0, 0x1F, 0x22):
                        for puntos in (0, 65535):
                            for activo in (0, 1, 3):
                                cambios = [
                                    (0x358C, modelo, True),
                                    (0x524E + id * 2, icono, True),
                                    (0x35C7, puntos, False),
                                    (0x2C4C, activo, True),
                                ]
                                cambios += [
                                    (0x5225 + i * 2, 3 if i < ocupadas else 65535, False)
                                    for i in range(10)
                                ]
                                self.variante(base, f'recoger_{id}', 0x563A, id, cambios=cambios)
        self.semilla_menu[:] = semilla
        self.retomar_recursos(base)

    # Carga dos accesos alternativos sobre copias; no afirma haber recorrido sus condiciones.
    def contratos_carga(self, base, salida):
        semilla = list(self.semilla_menu)
        d = bytearray(base)
        self.cambiar(d, self.semilla_menu, salida)
        self.esperar(d, 128)
        self.hito_borde(d, f'contrato_carga_{salida}')
        self.semilla_menu[:] = semilla
        self.retomar_recursos(base)

    # Recorre POST→REMOTE→CELESTE→POST con entradas reales y recoge111 en su posición original.
    def ejecutar(self):
        origen = self.raiz / 'apps/juego/trabajo/comparacion_regreso_loro/valle_post_entrada'
        if not origen.with_suffix('.ds.bin').exists():
            ComparacionRegresoLoro(self.raiz, self.biblioteca, True).ejecutar()
        d = bytearray(origen.with_suffix('.ds.bin').read_bytes())
        self.semilla_menu[:] = json.loads(
            origen.with_suffix('.semilla.json').read_text(encoding='utf-8')
        )
        self.destino = self.raiz / 'apps/juego/trabajo/comparacion_ramas_valle'
        self.destino.mkdir(parents=True, exist_ok=True)
        self.resumen = {}
        self.variantes = {}
        self.retomar_recursos(d)
        self.contratos_carga(d, 71)
        self.clic(d, palabra(d, 0x2C2D) + 16, 170)
        self.esperar(d, 192)
        self.esperar(d, 24, 8)
        self.esperar(d, 1, 0)
        self.clic(d, palabra(d, 0x2C2D) + 16, 145)
        self.esperar(d, 128)
        self.ruta(d, ((250, 145), (230, 170), (120, 185), (40, 199)), 60)
        self.hito_borde(d, 'remote_entrada')
        self.auditar(d, 'llegada REMOTE')
        self.variantes_rutinas(d)
        self.variantes_objetos(d, (108,))
        self.clic(d, 180, 175)
        self.esperar(d, 192)
        self.hito_borde(d, 'remote_reaccion')
        self.clic(d, 110, 165)
        self.esperar(d, 128)
        self.auditar(d, 'REMOTE ante objeto108 protegido')
        self.elegir_objeto(d, (1,), 108)
        self.esperar(d, 128)
        if 108 in [palabra(d, 0x5225 + i * 2) for i in range(10)]:
            raise ErrorAnalisis('El objeto108 protegido acaba en el inventario.')
        self.hito_borde(d, 'remote_rechazo')
        self.ruta(d, ((170, 122),), 56)
        self.hito_borde(d, 'celeste_entrada')
        self.auditar(d, 'llegada CELESTE')
        self.contratos_carga(d, 55)
        self.ruta(d, ((100, 198),), 61)
        self.hito_borde(d, 'post_regreso_rama')
        self.auditar(d, 'POST al regresar de CELESTE')
        self.variantes_objetos(d, (111, 112))
        self.elegir_objeto(d, (1,), 111)
        self.esperar(d, 128)
        if 111 not in [palabra(d, 0x5225 + i * 2) for i in range(10)] or d[0x2C4C]:
            raise ErrorAnalisis('La recogida real111 no termina como en el original.')
        self.hito_borde(d, 'post_objeto_111')
        self.auditar(d, 'POST después de recoger111')
        informe = {
            'resultado': 'coinciden',
            'casos_y_pasos': sum(self.resumen.values()),
            'grupos': self.resumen,
            'variantes_aisladas': self.variantes,
            'capturas': self.capturas,
            'estados_auditados': self.estados_auditados,
            'alcance': 'Rutas reales60/56/61, reacción REMOTE y recogida111. Cargas55/71 y recogidas108/112 comprobadas aisladamente. Grabación, demás salidas, puzles posteriores y SDL pendientes.',
        }
        (self.destino / 'resultado.json').write_text(
            json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8'
        )
        print(
            f'Ramas del valle: {informe["casos_y_pasos"]} casos/pasos y {self.capturas} composiciones coincidentes.'
        )


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Contrasta las primeras ramas nativas del valle.')
    parser.add_argument('--biblioteca', type=Path, required=True)
    parser.add_argument('--muestra', action='store_true')
    opciones = parser.parse_args()
    try:
        ComparacionRamasValle(
            Path(__file__).resolve().parents[4], opciones.biblioteca, opciones.muestra
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        parser.exit(1, f'Error: {error}\n')
