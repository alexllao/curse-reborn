"""Contrasta la cinta real y contratos aislados de grabación; todavía sin ruta musical completa."""

import argparse
import json
from pathlib import Path

from comparar_correo import ComparacionCorreo, ErrorAnalisis
from comparar_objetos import palabra


class ComparacionGrabacionValle(ComparacionCorreo):
    # Contrasta AB60 en ambos órdenes: límites del lector, cinta ausente y palabraA2 ajena.
    def variantes_insercion(self, base):
        for global_x in (700, 735, 755, 756, 757):
            for y in (118, 119, 120, 123):
                for camara in (0, 640):
                    for puntos in (0, 65535):
                        for presente in (False, True):
                            for id, previo in ((113, 105), (105, 113)):
                                cambios = [
                                    (0x2C2D, global_x - camara, False),
                                    (0x2C2F, y, False),
                                    (0x3591, camara, False),
                                    (0x35C7, puntos, False),
                                    (0x2CF3, 724 - camara, False),
                                    (0x2CF5, 140, False),
                                    (0x2CF7, 40, False),
                                    (0x2CF9, 27, False),
                                    (0x462C, puntos, False),
                                    (0xA2, 0xCAFE, False),
                                ]
                                cambios += [
                                    (0x5225 + i * 2, 105 if presente and i == 0 else 65535, False)
                                    for i in range(10)
                                ]
                                self.contrato(base, id, previo, 1, cambios)
        self.retomar_recursos(base)

    # Contrasta ABD5 en sus dos accesos, bordes735/123/-50 y agotamiento de inventario.
    def variantes_grabacion(self, base):
        for activo in (0, 1, 3):
            for global_x in (700, 713, 714, 734, 735, 763, 764, 765):
                for y in (122, 123, 124):
                    for camara in (0, 640):
                        for puntos in (0, 65535):
                            for inventario in ((), (108,), (3,) * 10, (3,) * 9 + (108,)):
                                cambios = [
                                    (0x2C6D, activo, True),
                                    (0x2C2D, global_x - camara, False),
                                    (0x2C2F, y, False),
                                    (0x3591, camara, False),
                                    (0x35C7, puntos, False),
                                    (0x2CF3, 724 - camara, False),
                                    (0x2CF7, 40, False),
                                    (0xA2, 0xCAFE, False),
                                ]
                                cambios += [
                                    (
                                        0x5225 + i * 2,
                                        inventario[i] if i < len(inventario) else 65535,
                                        False,
                                    )
                                    for i in range(10)
                                ]
                                self.contrato(base, 108, 65535, 2, cambios)
        self.retomar_recursos(base)

    # Recoge105 en CELESTE y lo inserta realmente enPOST; ABD5 conserva además variantes aisladas.
    def ejecutar(self):
        origen = self.raiz / 'trabajo/comparacion_ramas_valle/celeste_entrada'
        if not origen.with_suffix('.ds.bin').exists():
            raise ErrorAnalisis('Prepara primero comparar:ramas-valle --muestra.')
        d = bytearray(origen.with_suffix('.ds.bin').read_bytes())
        self.semilla_menu[:] = json.loads(
            origen.with_suffix('.semilla.json').read_text(encoding='utf-8')
        )
        self.destino = self.raiz / 'trabajo/comparacion_grabacion_valle'
        self.destino.mkdir(parents=True, exist_ok=True)
        self.resumen = {}
        self.variantes = {}
        self.retomar_recursos(d)
        self.variantes_objetos(d, (105,))
        for punto in ((330, 170), (90, 170), (74, 145)):
            self.ruta_sin_salida(d, *punto)
        self.elegir_objeto(d, (1,), 105)
        self.esperar(d, 128)
        if 105 not in [palabra(d, 0x5225 + i * 2) for i in range(10)]:
            raise ErrorAnalisis('La cinta105 no procede de la recogida original.')
        self.hito_borde(d, 'cinta_recogida')
        self.auditar(d, 'CELESTE con cinta')
        self.ruta(d, ((100, 198),), 61)
        for punto in ((550, 140), (620, 145), (704, 145), (704, 185), (760, 185), (760, 171)):
            self.ruta_sin_salida(d, *punto)
        self.elegir_objeto(d, (2, 1), 105)
        self.elegir_objeto(d, (), 113)
        self.esperar(d, 256)
        if 105 in [palabra(d, 0x5225 + i * 2) for i in range(10)] or d[0x2C6D] != 1:
            raise ErrorAnalisis('La cinta no queda insertada mediante el recorrido original.')
        self.hito_borde(d, 'cinta_insertada')
        self.auditar(d, 'POST con cinta insertada')
        origen_post = self.raiz / 'trabajo/comparacion_correo/carta_enviada'
        if not origen_post.with_suffix('.ds.bin').exists():
            raise ErrorAnalisis('Prepara primero comparar:correo --muestra.')
        d = bytearray(origen_post.with_suffix('.ds.bin').read_bytes())
        self.semilla_menu[:] = json.loads(
            origen_post.with_suffix('.semilla.json').read_text(encoding='utf-8')
        )
        self.retomar_recursos(d)
        self.variantes_insercion(d)
        self.variantes_grabacion(d)
        informe = {
            'resultado': 'coinciden',
            'casos_y_pasos': sum(self.resumen.values()),
            'grupos': self.resumen,
            'variantes_aisladas': self.variantes,
            'capturas': self.capturas,
            'estados_auditados': self.estados_auditados,
            'alcance': 'Recogida real105 enCELESTE, transporte61 e inserciónAB60 enPOST. AB60/ABD5 incluyen contratos aislados; falta recogida108 y grabación persistente completa. Sin SDL.',
        }
        (self.destino / 'resultado.json').write_text(
            json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8'
        )
        print(
            f'Grabación del valle: {informe["casos_y_pasos"]} casos/pasos; cinta real y contratos aislados coincidentes.'
        )


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Contrasta cinta y contratos originales de grabación.'
    )
    parser.add_argument('--biblioteca', type=Path, required=True)
    parser.add_argument('--muestra', action='store_true')
    opciones = parser.parse_args()
    try:
        ComparacionGrabacionValle(
            Path(__file__).resolve().parents[1], opciones.biblioteca, opciones.muestra
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        parser.exit(1, f'Error: {error}\n')
