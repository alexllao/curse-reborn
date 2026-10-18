"""Contrasta entrada, pasos y composición de EDGE con bancos DOS independientes."""

import argparse
import ctypes
import json
from pathlib import Path

from comparar_poblacion import ComparacionPoblacion, ErrorAnalisis
from comparar_objetos import palabra


class ComparacionRecorridoBorde(ComparacionPoblacion):
    # SMALL se carga en5800 para conservar completos EDGE, SMALL y STD simultáneamente.
    def __init__(self, raiz, biblioteca, muestra):
        super().__init__(raiz, biblioteca, muestra)
        self.referencia_modelo = 0x5800
        self.nativa.avanzar_jugabilidad_bancos.argtypes = (
            self.nativa.avanzar_jugabilidad.argtypes + [ctypes.c_ushort]
        )
        self.nativa.avanzar_jugabilidad_bancos.restype = ctypes.c_int
        self.nativa.avanzar_jugabilidad = self.avanzar_bancos
        self.recursos_menu = True

    # Aporta al núcleo la misma referencia que usa el modelo cargado por el EXE.
    def avanzar_bancos(self, *argumentos):
        return self.nativa.avanzar_jugabilidad_bancos(*argumentos, self.referencia_modelo)

    # Conserva cada imagen original contrastada junto con su estado y semilla para SDL.
    def hito_borde(self, d, nombre):
        self.captura(d, self.destino, nombre)

    # Retoma el transporte real del mago y comprueba la carga y los pasos de entrada de EDGE.
    def ejecutar(self):
        origen = self.raiz / 'trabajo/comparacion_poblacion/poblacion_transporte'
        if not origen.with_suffix('.ds.bin').exists():
            ComparacionPoblacion(self.raiz, self.biblioteca, True).ejecutar()
        d = bytearray(origen.with_suffix('.ds.bin').read_bytes())
        self.semilla_menu[:] = json.loads(origen.with_suffix('.semilla.json').read_text())
        self.resumen = {}
        self.destino = self.raiz / 'trabajo/comparacion_recorrido_borde'
        self.destino.mkdir(parents=True, exist_ok=True)
        self.retomar_recursos(d)
        salida = self.andar(d, self.semilla_menu, 65535, 768)
        if salida != 47:
            raise ErrorAnalisis(f'El transporte solicita {salida}, esperada47.')
        self.cambiar(d, self.semilla_menu, 47)
        self.hito_borde(d, 'borde_cargado')
        anterior = 0
        for paso in (32, 64, 128, 256):
            salida = self.andar(d, self.semilla_menu, 65535, paso - anterior)
            if salida is not None:
                raise ErrorAnalisis(f'Salida inesperada durante la entrada: {salida}.')
            self.hito_borde(d, f'borde_entrada_{paso}')
            anterior = paso
        if not d[0x3594] or palabra(d, 0x8082) != 30:
            raise ErrorAnalisis('La entrada de EDGE no devuelve el control.')
        informe = {
            'resultado': 'coinciden',
            'casos_y_pasos': sum(self.resumen.values()),
            'grupos': self.resumen,
            'capturas': self.capturas,
            'referencia_modelo': self.referencia_modelo,
            'alcance': 'Transporte real, entrada,256 pasos de la escena y cinco composiciones. Interacciones y puzles todavía pendientes.',
        }
        (self.destino / 'resultado.json').write_text(
            json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8'
        )
        print(
            f'Recorrido inicial de EDGE: {informe["casos_y_pasos"]} casos/pasos y {self.capturas} composiciones coincidentes.'
        )


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Compara el recorrido inicial de EDGE con el EXE.')
    parser.add_argument('--biblioteca', type=Path, required=True)
    parser.add_argument('--muestra', action='store_true')
    opciones = parser.parse_args()
    try:
        ComparacionRecorridoBorde(
            Path(__file__).resolve().parents[1], opciones.biblioteca, opciones.muestra
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        parser.exit(1, f'Error: {error}\n')
