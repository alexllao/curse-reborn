"""Contrasta recogidas140/141 y la tirada real de dados con su apertura de puertas."""

import argparse
import ctypes
import json
from pathlib import Path

from comparar_entrada_palacio import ComparacionEntradaPalacio, ErrorAnalisis
from comparar_objetos import guardar, palabra
from comparar_calles import ComparacionCalles


class ComparacionDadosPalacio(ComparacionEntradaPalacio):
    # CFA9 conserva exactamente el rango firmado30..89 y solo detiene el canal al recortar.
    def variantes_dados(self, base):
        funcion = self.nativa.procesar_rutina_nieve
        funcion.argtypes = [ctypes.POINTER(ctypes.c_ubyte)]
        funcion.restype = ctypes.c_int
        for y in (0, 29, 30, 31, 88, 89, 90, 32767, 32768, 65535):
            d = bytearray(base)
            guardar(d, 0x2D16, y)
            guardar(d, 0x2D2C, 0xCAFE)
            self.disponer(d, self.geometria, self.semilla_menu)
            self.ejecutar_original(0xCFA9)
            esperado = bytes(self.maquina.mem_read(self.segmento, 65536))
            if not funcion((ctypes.c_ubyte * 65536).from_buffer(d)) or bytes(d) != esperado:
                raise ErrorAnalisis(f'CFA9 difiere para{y}.')
        self.resumen['rutina_dados'] = 10
        semilla = list(self.semilla_menu)
        self.variantes = {}
        for id in (140, 141):
            for ocupadas in (0, 9, 10):
                for modelo in (20, 255):
                    for icono in (0, 31, 34):
                        for puntos in (0, 65535):
                            cambios = [
                                (0x358C, modelo, True),
                                (0x524E + id * 2, icono, True),
                                (0x35C7, puntos, False),
                            ]
                            cambios += [
                                (0x5225 + i * 2, 76 if i < ocupadas else 65535, False)
                                for i in range(10)
                            ]
                            self.variante(base, f'recoger{id}', 0x563A, id, cambios=cambios)
        for n in range(24):
            self.semilla_menu[:] = [n + 1, n * 17 + 3, n * 31 + 5, n * 53 + 7]
            self.contrato(base, 140, 65535, 5)
        self.semilla_menu[:] = semilla
        self.retomar_recursos(base)

    # Volver a coger140 cierra la puerta y otra tirada la abre con el avance real del mismo azar.
    def rama_segunda_tirada(self, base):
        d = bytearray(base)
        semilla = list(self.semilla_menu)
        self.retomar_recursos(d)
        self.ruta_sin_salida(d, 259, 165)
        self.elegir_objeto(d, (1,), 140)
        self.esperar(d, 192)
        if palabra(d, 0x2D16) != 89:
            raise ErrorAnalisis('Recoger de nuevo los dados no cierra la puerta.')
        self.hito_borde(d, 'dados_retirada')
        self.elegir_objeto(d, (2, 5), 140)
        self.esperar(d, 512)
        if palabra(d, 0x2D16) != 30 or list(self.semilla_menu) == semilla:
            raise ErrorAnalisis('La segunda tirada no abre con azar persistente.')
        self.hito_borde(d, 'dados_segunda_tirada')
        self.semilla_menu[:] = semilla
        self.retomar_recursos(base)

    # Conserva la misma partida de nieve, lago y carámbanos hasta los dados y la solicitud84.
    def ejecutar(self):
        origen = self.raiz / 'apps/juego/trabajo/comparacion_entrada_palacio/palacio_solicitud83'
        d = bytearray(origen.with_suffix('.ds.bin').read_bytes())
        self.semilla_menu[:] = json.loads(
            origen.with_suffix('.semilla.json').read_text(encoding='utf-8')
        )
        self.destino = self.raiz / 'apps/juego/trabajo/comparacion_dados_palacio'
        self.destino.mkdir(parents=True, exist_ok=True)
        self.resumen = {}
        self.retomar_recursos(d)
        self.cambiar(d, self.semilla_menu, 83)
        self.hito_borde(d, 'dados_entrada')
        self.esperar(d, 256)
        self.hito_borde(d, 'dados_control')
        self.auditar(d, 'entrada de la sala de dados')
        self.variantes_dados(d)
        self.ruta_sin_salida(d, 48, 146)
        self.hito_borde(d, 'dados_ante_escoba')
        self.auditar(d, 'ante escoba141')
        self.elegir_objeto(d, (1,), 141)
        self.esperar(d, 192)
        self.exigir_objeto(d, 141)
        self.hito_borde(d, 'dados_escoba')
        self.auditar(d, 'escoba en inventario')
        self.ruta_sin_salida(d, 259, 165)
        self.hito_borde(d, 'dados_ante_mesa')
        self.auditar(d, 'ante dados140')
        self.elegir_objeto(d, (1,), 140)
        self.esperar(d, 192)
        self.exigir_objeto(d, 140)
        if palabra(d, 0x2D16) != 89:
            raise ErrorAnalisis('Recoger140 no cierra la puerta original.')
        self.hito_borde(d, 'dados_recogidos')
        self.auditar(d, 'dados en inventario')
        self.elegir_objeto(d, (2, 5), 140)
        self.esperar(d, 512)
        self.exigir_objeto(d, 140, False)
        if palabra(d, 0x2D16) != 30:
            raise ErrorAnalisis('La tirada no abre la puerta84 de la sala de dados.')
        self.hito_borde(d, 'dados_tirada')
        self.auditar(d, 'despues de la tirada')
        self.rama_segunda_tirada(d)
        salida = ComparacionCalles.ir(self, d, self.semilla_menu, 160, 145)
        if salida != 84:
            raise ErrorAnalisis(f'La sala de dados no alcanza84: {salida}.')
        (self.destino / 'dados_solicitud84.ds.bin').write_bytes(d)
        (self.destino / 'dados_solicitud84.semilla.json').write_text(
            json.dumps(list(self.semilla_menu)), encoding='utf-8'
        )
        informe = {
            'resultado': 'coinciden',
            'casos_y_pasos': sum(self.resumen.values()),
            'grupos': self.resumen,
            'capturas': self.capturas,
            'estados_auditados': self.estados_auditados,
            'variantes': self.variantes,
            'alcance': 'Llegada83, escoba141, dados140, tirada y puerta84 con generador compartido. Corredores ySDL pendientes.',
        }
        (self.destino / 'resultado.json').write_text(
            json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8'
        )
        print(
            f'Dados: {informe["casos_y_pasos"]} casos/pasos y {self.capturas} composiciones coincidentes.'
        )


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Contrasta la sala de dados del palacio.')
    parser.add_argument('--biblioteca', type=Path, required=True)
    parser.add_argument('--muestra', action='store_true')
    opciones = parser.parse_args()
    try:
        ComparacionDadosPalacio(
            Path(__file__).resolve().parents[4], opciones.biblioteca, opciones.muestra
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        parser.exit(1, f'Error: {error}\n')
