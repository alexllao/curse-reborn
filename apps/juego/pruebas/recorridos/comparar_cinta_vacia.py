"""Contrasta la respuesta a105 desde una rama real anterior a la grabación y el correo."""

import argparse
import json
from pathlib import Path

from comparar_entrega_grabacion import ComparacionEntregaGrabacion, ErrorAnalisis
from comparar_grabacion_completa import ComparacionGrabacionCompleta
from comparar_objetos import palabra


class ComparacionCintaVacia(ComparacionEntregaGrabacion):
    # Retoma105 realmente recogida, camina hasta98 y no asigna objetos, banderas ni fotogramas.
    def ejecutar(self):
        origen = self.raiz / 'apps/juego/trabajo/comparacion_grabacion_completa/cinta_y_mando'
        if not origen.with_suffix('.ds.bin').exists():
            ComparacionGrabacionCompleta(self.raiz, self.biblioteca, True).ejecutar()
        d = bytearray(origen.with_suffix('.ds.bin').read_bytes())
        self.semilla_menu[:] = json.loads(
            origen.with_suffix('.semilla.json').read_text(encoding='utf-8')
        )
        self.destino = self.raiz / 'apps/juego/trabajo/comparacion_cinta_vacia'
        self.destino.mkdir(parents=True, exist_ok=True)
        self.resumen = {}
        self.retomar_recursos(d)
        if palabra(d, 0x8082) != 33:
            raise ErrorAnalisis('La rama no parte de CELESTE con cinta y mando reales.')
        self.ruta(d, ((100, 198),), 61)
        self.ruta(d, ((550, 140), (620, 145), (704, 145), (850, 190), (850, 199)), 57)
        self.ruta_sin_salida(d, 150, 165)
        inventario = bytes(d[0x5225:0x5239])
        objetos = [palabra(d, 0x5225 + i * 2) for i in range(10)]
        if (
            not all(id in objetos for id in (105, 108))
            or any(id in objetos for id in (99, 106, 111, 112))
            or not d[0x2C4C]
            or d[0x2C4D] == 4
        ):
            raise ErrorAnalisis('La rama no conserva cinta vacía, mando y destinatario sin correo.')
        self.hito_borde(d, 'cinta_vacia_antes_entrega')
        puntos = palabra(d, 0x35C7)
        correo = d[0x44F9]
        self.elegir_objeto(d, (2, 6), 105)
        self.elegir_objeto(d, (), 98)
        self.esperar(d, 256)
        if bytes(d[0x5225:0x5239]) != inventario or palabra(d, 0x35C7) != puntos:
            raise ErrorAnalisis('La respuesta a105 altera inventario o puntuación.')
        if not d[0x2C4C] or d[0x44F9] != correo or not d[0x3595]:
            raise ErrorAnalisis(
                'La respuesta vacía retira al destinatario o altera su correo/control.'
            )
        self.hito_borde(d, 'cinta_vacia_respuesta')
        informe = {
            'resultado': 'coinciden',
            'casos_y_pasos': sum(self.resumen.values()),
            'grupos': self.resumen,
            'capturas': self.capturas,
            'puntuacion_conservada': puntos,
            'inventario_conservado': objetos,
            'referencia_modelo': self.referencia_modelo,
            'limite': 'Respuesta real A8AE a105 sin grabación ni correo;256 pasos posteriores. No incorpora esta rama negativa a la solución principal ni agota todos los estados del destinatario.',
        }
        (self.destino / 'resultado.json').write_text(
            json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8'
        )
        print(
            f'Cinta vacía: {informe["casos_y_pasos"]} casos/pasos y dos composiciones coincidentes.'
        )


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Contrasta la respuesta original a la cinta vacía.'
    )
    parser.add_argument('--biblioteca', type=Path, required=True)
    parser.add_argument('--muestra', action='store_true')
    opciones = parser.parse_args()
    try:
        ComparacionCintaVacia(
            Path(__file__).resolve().parents[4], opciones.biblioteca, opciones.muestra
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        parser.exit(1, f'Error: {error}\n')
