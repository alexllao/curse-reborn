"""Prolonga la partida de las cuevas hasta catapultarse, recoger el imÃƒÆ’Ã‚Â¡n y obtener el alambre."""

import argparse
from pathlib import Path

from comparar_cuevas import ComparacionCuevas, ErrorAnalisis
from comparar_objetos import guardar, palabra


class ComparacionCatapulta(ComparacionCuevas):
    # Reutiliza la ruta persistente completa y separa las referencias de los nuevos puzles.
    def __init__(self, raiz, biblioteca, muestra):
        super().__init__(raiz, biblioteca, muestra)
        self.carpeta_resultados = 'comparacion_catapulta'

    # Comprueba cada verbo, las respuestas negativas y las combinaciones en copias de la escena.
    def opciones_catapulta(self, base, objetos):
        for manejador in (0x384A, 0x2FCA, 0x3143):
            for objeto in objetos:
                d = bytearray(base)
                guardar(d, 0xB60C, manejador)
                guardar(d, 0xB620, objeto + 0x1A)
                self.seleccionar(d, 0)
        for accion in range(8):
            for previo in (0xFFFF, 55, 56, 57, 61, 62, 70):
                for objeto in objetos:
                    d = bytearray(base)
                    guardar(d, 0xB60C, 0x3B53)
                    guardar(d, 0xB620, objeto + 0x1A)
                    guardar(d, 0xB634, accion)
                    guardar(d, 0xB638, previo)
                    self.seleccionar(d, 0)
        for objeto in (56, 57) if palabra(base, 0x8082) == 11 else (61,):
            for ocupadas in (0, 9, 10):
                for puntos in (0, 65535):
                    d = bytearray(base)
                    for i in range(10):
                        guardar(d, 0x5225 + 2 * i, 3 if i < ocupadas else 0xFFFF)
                    guardar(d, 0x35C7, puntos)
                    guardar(d, 0xB60C, 0x563A)
                    guardar(d, 0xB620, objeto + 0x1A)
                    self.seleccionar(d, 0)

    # Llega al monitor por las salidas BOXDET y ejecuta sus menÃƒÆ’Ã‚Âºs sin modificar el inventario.
    def despues_cuevas(self, datos, semilla, carpeta):
        self.cruzar_cueva(datos, semilla, 150, 199, 19)
        self.cruzar_cueva(datos, semilla, 148, 90, 7)
        self.cruzar_cueva(datos, semilla, 670, 199, 11)
        self.captura(datos, carpeta, 'catapulta_corredor')
        self.cruzar_cueva(datos, semilla, 160, 199, 15)
        self.opciones_catapulta(datos, (3, 55, 61, 62, 70))
        self.caminar_hasta(datos, semilla, 255, 180)
        self.elegir_objeto(datos, (1,), 61)
        self.andar(datos, semilla, 0xFFFF, 192)
        self.captura(datos, carpeta, 'catapulta_monitor')
        self.cruzar_cueva(datos, semilla, 160, 199, 22)
        self.caminar_hasta(datos, semilla, 348, 130)
        self.cruzar_cueva(datos, semilla, 348, 80, 14)
        self.cruzar_cueva(datos, semilla, 148, 90, 12)
        self.opciones_catapulta(datos, (3, 55, 56, 57, 58, 59, 60, 61, 70))
        self.caminar_hasta(datos, semilla, 75, 162)
        self.elegir_objeto(datos, (1,), 56)
        self.andar(datos, semilla, 0xFFFF, 192)
        self.elegir_objeto(datos, (2, 7), 56)
        self.elegir_objeto(datos, (), 58)
        self.andar(datos, semilla, 0xFFFF, 192)
        self.captura(datos, carpeta, 'catapulta_tablon')
        self.caminar_hasta(datos, semilla, 123, 160)
        self.elegir_objeto(datos, (2, 5), 61)
        self.andar(datos, semilla, 0xFFFF, 256)
        self.captura(datos, carpeta, 'catapulta_cornisa')
        self.elegir_objeto(datos, (1,), 57)
        self.andar(datos, semilla, 0xFFFF, 192)
        self.elegir_objeto(datos, (6,), 59)
        self.andar(datos, semilla, 0xFFFF, 192)
        self.caminar_hasta(datos, semilla, 200, 180)
        self.captura(datos, carpeta, 'catapulta_iman')
        self.cruzar_cueva(datos, semilla, 160, 199, 20)
        self.cruzar_cueva(datos, semilla, 670, 199, 11)
        self.cruzar_cueva(datos, semilla, 160, 199, 15)
        self.elegir_objeto(datos, (2, 7), 55)
        self.seleccionar(datos, 0)
        self.elegir_objeto(datos, (), 57)
        self.andar(datos, semilla, 0xFFFF, 96)
        self.elegir_objeto(datos, (2, 5), 70)
        self.andar(datos, semilla, 0xFFFF, 192)
        if 63 not in [palabra(datos, 0x5225 + i * 2) for i in range(10)]:
            raise ErrorAnalisis('El imÃƒÆ’Ã‚Â¡n con cordel no recupera el alambre del agujero.')
        self.captura(datos, carpeta, 'catapulta_alambre')
        self.opciones_catapulta(datos, (3, 55, 61, 62, 70))
        self.resumen['alambre_recuperado'] = 1
        self.despues_catapulta(datos, semilla, carpeta)

    # Abre la ampliación siguiente sobre la misma partida y sus bancos reales.
    def despues_catapulta(self, datos, semilla, carpeta):
        pass


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Compara catapulta, imÃƒÆ’Ã‚Â¡n y alambre con DOS.'
    )
    parser.add_argument('--biblioteca', type=Path, required=True)
    parser.add_argument('--muestra', action='store_true')
    opciones = parser.parse_args()
    try:
        ComparacionCatapulta(
            Path(__file__).resolve().parents[1], opciones.biblioteca, opciones.muestra
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        parser.exit(1, f'Error: {error}\n')
