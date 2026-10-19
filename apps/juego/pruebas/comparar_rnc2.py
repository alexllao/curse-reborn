"""Contrasta el método dos con 67BC, incluidos los fondos de la mazmorra inicial."""

import argparse
import hashlib
from itertools import product
import json
from pathlib import Path
import struct

from comparar_rnc import ComparacionRnc, ErrorAnalisis


class Escritor:
    # Reserva el primer byte de bits y descarta los dos bits iniciales del formato.
    def __init__(self):
        self.datos = bytearray()
        self.libres = 0
        self.posicion = 0
        self.bits(0, 2)

    # Intercala los bits de control con los literales, de mayor a menor peso.
    def bits(self, valor, cantidad):
        for i in range(cantidad - 1, -1, -1):
            if self.libres == 0:
                self.posicion = len(self.datos)
                self.datos.append(0)
                self.libres = 8
            self.libres -= 1
            self.datos[self.posicion] |= ((valor >> i) & 1) << self.libres

    # Codifica un literal individual con el prefijo observado en 68F6.
    def literal(self, valor):
        self.bits(0, 1)
        self.datos.append(valor)

    # Codifica una coincidencia con cualquiera de los dieciséis prefijos de distancia.
    def coincidencia(self, cantidad, distancia):
        if cantidad == 2:
            self.bits(0b110, 3)
        elif cantidad == 3:
            self.bits(0b1110, 4)
        elif cantidad in (4, 5):
            self.bits(0b1000 | ((cantidad - 4) << 1), 4)
        elif cantidad in (6, 7):
            self.bits(0b10010 | (cantidad - 6), 5)
        elif cantidad == 8:
            self.bits(0b10110, 5)
        else:
            self.bits(0b1111, 4)
            self.datos.append(cantidad - 8)
        alto = (distancia - 1) >> 8
        if cantidad != 2:
            if alto == 0:
                self.bits(0, 1)
            elif alto == 1:
                self.bits(0b110, 3)
            elif alto < 4:
                self.bits(0b1000 | (alto & 1), 4)
            elif alto < 8:
                self.bits(0b10101 | (((alto >> 1) & 1) << 3) | ((alto & 1) << 1), 5)
            else:
                self.bits(
                    0b101000 | (((alto >> 2) & 1) << 4) | (((alto >> 1) & 1) << 2) | (alto & 1), 6
                )
        self.datos.append((distancia - 1) & 255)

    # Emite el final de bloque o de flujo, según el bit final de 697B.
    def terminar(self, continuar=False):
        self.bits(0b1111, 4)
        self.datos.append(0)
        self.bits(int(continuar), 1)

    # Añade la cabecera de tamaños; el original no verifica sus campos CRC.
    def paquete(self, longitud):
        return (
            b'RNC\x02' + struct.pack('>2I2H2B', longitud, len(self.datos), 0, 0, 0, 1) + self.datos
        )


# Produce una referencia conocida, con copia solapada o distancia larga y varios bloques.
def repeticion(cantidad, distancia, continuar=False):
    escritor = Escritor()
    inicio = bytes((i * 29 + 7) & 255 for i in range(distancia))
    for valor in inicio:
        escritor.literal(valor)
    if continuar:
        escritor.terminar(True)
    escritor.coincidencia(cantidad, distancia)
    escritor.terminar()
    esperado = (inicio * (1 + cantidad // distancia + 1))[: distancia + cantidad]
    return escritor.paquete(len(esperado)), esperado


class ComparacionRnc2(ComparacionRnc):
    metodo = 2
    entrada_original = 0x67BC

    # Contrasta datos reales y programas que fuerzan los códigos y límites de la API.
    def ejecutar(self):
        carpeta = self.raiz / 'datos_originales/CURSE/DATA'
        reales = [
            (p.name, p.read_bytes())
            for p in sorted(carpeta.glob('*.DAT'))
            if p.read_bytes()[:4] == b'RNC\x02'
        ]
        mapa = (carpeta / 'DUNGEON.MAP').read_bytes()
        tiras = []
        for i in range(10):
            posicion = 578 + int.from_bytes(mapa[578 + i * 2 : 580 + i * 2], 'little') * 16
            longitud = int.from_bytes(mapa[posicion + 8 : posicion + 12], 'big') + 18
            tiras.append((f'DUNGEON.MAP:{i}', mapa[posicion : posicion + longitud]))
        for nombre, paquete in reales[:2] + tiras[:3] if self.muestra else reales + tiras:
            for patron, compartido in product(
                (0xCD,) if self.muestra else (0, 0xCD), (False, True)
            ):
                salida = self.contrastar(
                    'archivos_reales',
                    paquete,
                    patron,
                    compartido,
                    int.from_bytes(paquete[4:8], 'big'),
                )
                self.archivos[nombre] = {
                    'longitud': len(salida),
                    'sha256_descomprimido': hashlib.sha256(salida).hexdigest(),
                }
        distancias = (
            (1, 257, 4096) if self.muestra else tuple(1 + i * 256 for i in range(16)) + (256, 4096)
        )
        for cantidad, distancia, continuar in product(
            (2, 3, 4, 5, 6, 7, 8, 9, 263), distancias, (False, True)
        ):
            if cantidad == 2 and distancia > 256:
                continue
            paquete, esperado = repeticion(cantidad, distancia, continuar)
            self.contrastar('codigos_y_bloques', paquete, 0xCD, False, len(esperado), esperado)
        for valor in (0, 15) if self.muestra else range(16):
            escritor = Escritor()
            escritor.bits(0b10111, 5)
            escritor.bits(valor, 4)
            esperado = bytes(range((valor + 3) * 4))
            escritor.datos.extend(esperado)
            escritor.terminar()
            self.contrastar(
                'literales_agrupados',
                escritor.paquete(len(esperado)),
                0xCD,
                False,
                len(esperado),
                esperado,
            )
        valido, _ = repeticion(16, 1)
        variantes = [(valido[:n], 65536) for n in (4, 17, len(valido) - 1)]
        variantes += [(valido, 16), (valido[:8] + b'\xff' * 4 + valido[12:], 65536)]
        for paquete, capacidad in variantes:
            destino = bytearray([0xCD] * 65536)
            estado, escritos = self.nativo(paquete, destino, capacidad)
            if estado != 3 or escritos != 0 or destino != bytes([0xCD] * 65536):
                raise ErrorAnalisis(
                    'Cabecera o capacidad inválidas no rechazadas antes de escribir.'
                )
            self.resumen['limites_api'] = self.resumen.get('limites_api', 0) + 1
        informe = {
            'sha256_ejecutable': self.cabecera['sha256'],
            'casos_por_grupo': self.resumen,
            'casos_totales': sum(self.resumen.values()),
            'archivos': self.archivos,
            'resultado': 'coinciden',
            'muestra': self.muestra,
            'alcance': 'Salida y cola, registros y pila del original. Método dos con entrada original '
            'separada o compartida; C++ con búferes separados. Sin CRC.',
        }
        ruta = (
            self.raiz
            / 'apps/juego/trabajo/comparacion_rnc2'
            / ('muestra.json' if self.muestra else 'resultado.json')
        )
        ruta.parent.mkdir(parents=True, exist_ok=True)
        ruta.write_text(json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
        for grupo, cantidad in self.resumen.items():
            print(f'{grupo}: {cantidad} casos coincidentes.')
        print(
            f'Total: {informe["casos_totales"]} casos. Coinciden las salidas y los contratos comprobados.'
        )
        print(f'Informe local: {ruta}')


if __name__ == '__main__':
    argumentos = argparse.ArgumentParser(description='Compara el segundo método RNC con 67BC.')
    argumentos.add_argument('--biblioteca', type=Path, required=True)
    argumentos.add_argument('--muestra', action='store_true')
    opciones = argumentos.parse_args()
    try:
        ComparacionRnc2(
            Path(__file__).resolve().parents[3], opciones.biblioteca, opciones.muestra
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        argumentos.exit(1, f'Error: {error}\n')
