"""Contrasta DROP, su salida automática y la entrada sumergida en LAKEBED con el EXE."""

import argparse
import ctypes
import json
from pathlib import Path

from comparar_corredor import ComparacionCorredor, ErrorAnalisis
from comparar_objetos import ComparacionObjetos, guardar, palabra


class ComparacionCaida(ComparacionCorredor):
    # Añade la entrada con PLUNGE; los recursos siguen en reservas de referencia sin solapamientos.
    def __init__(self, raiz, biblioteca, muestra):
        super().__init__(raiz, biblioteca, muestra)
        self.carpeta_resultados = 'comparacion_caida'
        self.nativa.preparar_transicion.argtypes = [ctypes.POINTER(ctypes.c_ubyte)] + [
            ctypes.c_ushort
        ] * 5
        self.nativa.preparar_transicion.restype = ctypes.c_int

    # Dispone la vista de desarrollo de DROP; no simula la resolución del puzle del corredor.
    def ejecutar(self):
        preparacion = ComparacionObjetos(self.raiz, self.biblioteca, True)
        datos = preparacion.caso('entrada', 4, bytearray(self.base), 'MANA.BRD')
        guardar(datos, 0x2C4A, 0x5400)
        guardar(datos, 0x2C29, 0x7400)
        guardar(datos, 0x8082, 1)
        semilla = (ctypes.c_ushort * 4)(*self.semilla_original)
        self.resumen = {}
        self.cambiar(datos, semilla, 1)
        carpeta = self.raiz / 'apps/juego/trabajo/comparacion_caida'
        carpeta.mkdir(parents=True, exist_ok=True)
        transicion = None
        for paso in range(1, 257):
            transicion = self.paso(datos, semilla)
            if paso in (16, 32):
                self.captura(datos, carpeta, f'caida_{paso}')
            if transicion is not None:
                break
        if transicion != 3:
            raise ErrorAnalisis(f'La caída no solicitó la salida tres: {transicion}.')
        self.cambiar(datos, semilla, 3)
        for paso in range(1, 25 if self.muestra else 129):
            if self.paso(datos, semilla) is not None:
                raise ErrorAnalisis('Se abandonó el agua durante la entrada sumergida.')
            if paso == 24:
                self.captura(datos, carpeta, 'agua_24')
        if palabra(datos, 0x8082) != 3:
            raise ErrorAnalisis('La secuencia no terminó en el fondo del lago.')
        ruta = carpeta / ('muestra.json' if self.muestra else 'resultado.json')
        ruta.write_text(
            json.dumps(
                {
                    'resultado': 'coinciden',
                    'casos_por_grupo': self.resumen,
                    'casos_totales': sum(self.resumen.values()),
                    'entrada_de_desarrollo': True,
                    'salida_automatica': 3,
                    'escena_destino': 3,
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
            f'Total: {sum(self.resumen.values())} casos/pasos. Coinciden caída y entrada sumergida.'
        )
        print(f'Informe local: {ruta}')


if __name__ == '__main__':
    argumentos = argparse.ArgumentParser(description='Compara la caída y la llegada al agua.')
    argumentos.add_argument('--biblioteca', type=Path, required=True)
    argumentos.add_argument('--muestra', action='store_true')
    opciones = argumentos.parse_args()
    try:
        ComparacionCaida(
            Path(__file__).resolve().parents[3], opciones.biblioteca, opciones.muestra
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        argumentos.exit(1, f'Error: {error}\n')
