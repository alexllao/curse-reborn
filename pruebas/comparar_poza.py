"""Continúa la partida desde el lago: algas, botón, puerta y entrada al corredor de las cuevas."""

import argparse
from pathlib import Path

from comparar_lago import ComparacionLago, ErrorAnalisis
from comparar_objetos import guardar, palabra


class ComparacionPoza(ComparacionLago):
    # Conserva todos los pasos anteriores y separa los resultados de esta ampliación.
    def __init__(self, raiz, biblioteca, muestra):
        super().__init__(raiz, biblioteca, muestra)
        self.carpeta_resultados = 'comparacion_poza'

    # Recorre las entradas reales de teclado y menú sin alterar las entidades ni el inventario.
    def despues_lago(self, datos, semilla, carpeta):
        self.opciones_poza(datos)
        for x, y in ((280, 180), (305, 195), (180, 100)):
            self.clic(datos, x, y)
            if self.andar(datos, semilla, 0xFFFF, 96) is not None:
                raise ErrorAnalisis('La aproximación a las algas abandona la poza.')
        self.elegir_objeto(datos, (1,), 49)
        self.andar(datos, semilla, 0xFFFF, 96)
        if (
            49 not in [palabra(datos, 0x5225 + 2 * i) for i in range(10)]
            or palabra(datos, 0x2C7B) != 48
        ):
            raise ErrorAnalisis('Las algas no se conservan o no habilitan el botón.')
        self.captura(datos, carpeta, 'poza_algas')
        self.elegir_objeto(datos, (3,), 50)
        self.clic(datos, palabra(datos, 0x2C6F) - 15, palabra(datos, 0x2C71) + 33)
        self.andar(datos, semilla, 0xFFFF, 96)
        self.elegir_objeto(datos, (2, 2), 48)
        self.andar(datos, semilla, 0xFFFF, 96)
        if datos[0x2C4C] or datos[0x2C4D] != 13:
            raise ErrorAnalisis('La puerta no termina la apertura original.')
        self.captura(datos, carpeta, 'poza_puerta')
        self.opciones_poza(datos)
        self.clic(datos, 90, 110)
        if self.andar(datos, semilla, 0xFFFF, 96) is not None:
            raise ErrorAnalisis('La aproximación a la puerta solicita una salida prematura.')
        self.clic(datos, 80, 80)
        salida = self.andar(datos, semilla, 0xFFFF, 96)
        if salida != 17:
            raise ErrorAnalisis(f'La puerta solicita una salida distinta: {salida}.')
        self.cambiar(datos, semilla, salida)
        self.captura(datos, carpeta, 'cuevas_corredor_entrada')
        self.andar(datos, semilla, 0xFFFF, 96)
        self.captura(datos, carpeta, 'cuevas_corredor')
        if palabra(datos, 0x8082) != 4 or not datos[0x3594]:
            raise ErrorAnalisis('El corredor de las cuevas no devuelve el control.')
        self.resumen['salida_poza'] = 1

    # Compara capacidad y puntuación, ambos temas, verbos sin acción y combinaciones antes/después.
    def opciones_poza(self, base):
        for modelo in (0, 5, 6, 7, 13, 255):
            for puntos in (0, 65535):
                d = bytearray(base)
                d[0x2C4D] = modelo
                guardar(d, 0x35C7, puntos)
                guardar(d, 0xB60C, 0x3B53)
                guardar(d, 0xB620, 48 + 0x1A)
                guardar(d, 0xB634, 2)
                guardar(d, 0xB638, 0xFFFF)
                self.seleccionar(d, 0)
        for ocupadas in (0, 9, 10):
            for puntos in (0, 65535):
                d = bytearray(base)
                for i in range(10):
                    guardar(d, 0x5225 + 2 * i, 3 if i < ocupadas else 0xFFFF)
                guardar(d, 0x35C7, puntos)
                guardar(d, 0xB60C, 0x563A)
                guardar(d, 0xB620, 49 + 0x1A)
                self.seleccionar(d, 0)
        for manejador in (0x384A, 0x2FCA, 0x3143):
            for objeto in (3, 48, 49, 50):
                d = bytearray(base)
                guardar(d, 0xB60C, manejador)
                guardar(d, 0xB620, objeto + 0x1A)
                self.seleccionar(d, 0)
        for tema in (0, 1):
            d = bytearray(base)
            guardar(d, 0xB60C, 0x32DE)
            guardar(d, 0xB620, tema + 0x17)
            self.seleccionar(d, 0)
        for accion in range(8):
            for previo in (0xFFFF, 3, 49):
                for objeto in (3, 48, 49, 50):
                    d = bytearray(base)
                    guardar(d, 0xB60C, 0x3B53)
                    guardar(d, 0xB620, objeto + 0x1A)
                    guardar(d, 0xB634, accion)
                    guardar(d, 0xB638, previo)
                    self.seleccionar(d, 0)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Compara las acciones de POOL con el EXE.')
    parser.add_argument('--biblioteca', type=Path, required=True)
    parser.add_argument('--muestra', action='store_true')
    opciones = parser.parse_args()
    try:
        ComparacionPoza(
            Path(__file__).resolve().parents[1], opciones.biblioteca, opciones.muestra
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        parser.exit(1, f'Error: {error}\n')
