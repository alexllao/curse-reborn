"""Contrasta carta, sello y buzón desde el regreso real del dinosaurio."""

import argparse
import json
from pathlib import Path

from comparar_dinosaurio import ComparacionDinosaurio, ErrorAnalisis
from comparar_objetos import guardar, palabra


class ComparacionCorreo(ComparacionDinosaurio):
    # Prepara un contrato aislado del selector; el resultado sigue comparándose con el EXE completo.
    def contrato(self, base, id, previo, verbo, cambios=()):
        d = bytearray(base)
        for posicion, valor, byte in cambios:
            if byte:
                d[posicion] = valor
            else:
                guardar(d, posicion, valor)
        guardar(d, 0xB60C, 0x3B53)
        guardar(d, 0xB620, id + 26)
        guardar(d, 0xB638, previo)
        guardar(d, 0xB634, verbo)
        self.seleccionar(d, 0)

    # Contrasta falta de carta, sello ausente, duplicados, capacidad y puntuación envolvente deAA6C.
    def variantes_franqueo(self, base):
        for objetos in ((), (109,), (111,), (111, 109), (111, 112, 109), (112,), (111, 111, 109)):
            for lleno in (False, True):
                for puntos in (0, 65535):
                    for id, previo in ((111, 109), (109, 111)):
                        cambios = [(0x35C7, puntos, False)]
                        cambios += [
                            (
                                0x5225 + i * 2,
                                objetos[i] if i < len(objetos) else (3 if lleno else 65535),
                                False,
                            )
                            for i in range(10)
                        ]
                        self.contrato(base, id, previo, 7, cambios)
        self.retomar_recursos(base)

    # Revisa las ramas de dos/tres nodos deAAA3, ambos órdenes y retirada con/sin carta112.
    def variantes_buzon(self, base):
        x_buzon = palabra(base, 0x2D14)
        for x in (x_buzon - 33, x_buzon - 32, x_buzon - 1, x_buzon + 31, x_buzon + 32):
            for y in (100, 160, 161):
                for delta in (-1, 0, 1):
                    for camara in (0, 256):
                        for puntos in (0, 65535):
                            for activo in (0, 1):
                                for presente in (False, True):
                                    for id, previo in ((114, 112), (112, 114)):
                                        cambios = [
                                            (0x2C2D, x, False),
                                            (0x2C2F, y, False),
                                            (
                                                0x2D16,
                                                y
                                                + palabra(base, 0x2C33)
                                                + delta
                                                - palabra(base, 0x2D1A),
                                                False,
                                            ),
                                            (0x3591, camara, False),
                                            (0x35C7, puntos, False),
                                            (0x44F9, activo, True),
                                            (0x1A36, 0, False),
                                            (0x1A3C, 0, False),
                                            (0x1A42, 0, False),
                                        ]
                                        cambios += [
                                            (
                                                0x5225 + i * 2,
                                                112 if presente and i == 0 else 65535,
                                                False,
                                            )
                                            for i in range(10)
                                        ]
                                        self.contrato(base, id, previo, 1, cambios)
        self.retomar_recursos(base)

    # Continúa con el sello real: recoge111, combina por el menú y envía112 al buzón114.
    def ejecutar(self):
        origen = self.raiz / 'apps/juego/trabajo/comparacion_dinosaurio/dinosaurio_regreso_post'
        if not origen.with_suffix('.ds.bin').exists():
            ComparacionDinosaurio(self.raiz, self.biblioteca, True).ejecutar()
        d = bytearray(origen.with_suffix('.ds.bin').read_bytes())
        self.semilla_menu[:] = json.loads(
            origen.with_suffix('.semilla.json').read_text(encoding='utf-8')
        )
        self.destino = self.raiz / 'apps/juego/trabajo/comparacion_correo'
        self.destino.mkdir(parents=True, exist_ok=True)
        self.resumen = {}
        self.retomar_recursos(d)
        for punto in ((800, 145), (700, 145), (620, 145), (550, 140), (484, 104)):
            self.ruta_sin_salida(d, *punto)
        self.elegir_objeto(d, (1,), 111)
        self.esperar(d, 128)
        if not all(id in [palabra(d, 0x5225 + i * 2) for i in range(10)] for id in (109, 111)):
            raise ErrorAnalisis('La carta y el sello no proceden del recorrido real.')
        self.hito_borde(d, 'carta_recogida')
        self.auditar(d, 'carta y sello reales')
        self.variantes_franqueo(d)
        self.variantes_buzon(d)
        self.elegir_objeto(d, (2, 7), 109)
        self.seleccionar(d, 0)
        self.elegir_objeto(d, (), 111)
        self.esperar(d, 128)
        inventario = [palabra(d, 0x5225 + i * 2) for i in range(10)]
        if 112 not in inventario or 109 in inventario or 111 in inventario:
            raise ErrorAnalisis('El franqueo no sustituye carta y sello por112.')
        self.hito_borde(d, 'carta_franqueada')
        self.auditar(d, 'carta franqueada')
        for punto in ((450, 175), (435, 175)):
            self.ruta_sin_salida(d, *punto)
        self.elegir_objeto(d, (2, 1), 112)
        self.elegir_objeto(d, (), 114)
        self.esperar(d, 256)
        if 112 in [palabra(d, 0x5225 + i * 2) for i in range(10)]:
            raise ErrorAnalisis('La carta franqueada permanece tras enviarla.')
        self.hito_borde(d, 'carta_enviada')
        self.auditar(d, 'carta enviada')
        informe = {
            'resultado': 'coinciden',
            'casos_y_pasos': sum(self.resumen.values()),
            'grupos': self.resumen,
            'capturas': self.capturas,
            'estados_auditados': self.estados_auditados,
            'alcance': 'Recogida111, franqueoAA6C y envíoAAA3 desde el sello real de DINO. No acredita la devolución posterior, la grabación ni SDL del valle.',
        }
        (self.destino / 'resultado.json').write_text(
            json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8'
        )
        print(
            f'Correo: {informe["casos_y_pasos"]} casos/pasos y {self.capturas} composiciones coincidentes.'
        )


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Contrasta el franqueo y envío originales.')
    parser.add_argument('--biblioteca', type=Path, required=True)
    parser.add_argument('--muestra', action='store_true')
    opciones = parser.parse_args()
    try:
        ComparacionCorreo(
            Path(__file__).resolve().parents[4], opciones.biblioteca, opciones.muestra
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        parser.exit(1, f'Error: {error}\n')
