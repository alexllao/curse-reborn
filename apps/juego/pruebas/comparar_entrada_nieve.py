"""Contrasta la carga real74 y los primeros pasos de SNOWAST4; no integra SDL."""

import argparse
import json
from pathlib import Path

from comparar_regreso_benn import ComparacionRegresoBenn, ErrorAnalisis


class ComparacionEntradaNieve(ComparacionRegresoBenn):
    # Retoma exclusivamente el vestido y la solicitud74 verificados; conserva inventario y azar.
    def ejecutar(self):
        origen = self.raiz / 'apps/juego/trabajo/comparacion_regreso_benn/probador_solicitud74'
        if not origen.with_suffix('.ds.bin').exists():
            raise ErrorAnalisis('Falta la partida real: ejecuta comparar:regreso-benn primero.')
        d = bytearray(origen.with_suffix('.ds.bin').read_bytes())
        self.semilla_menu[:] = json.loads(
            origen.with_suffix('.semilla.json').read_text(encoding='utf-8')
        )
        self.destino = self.raiz / 'apps/juego/trabajo/comparacion_entrada_nieve'
        self.destino.mkdir(parents=True, exist_ok=True)
        self.resumen = {}
        self.retomar_recursos(d)
        self.cambiar(d, self.semilla_menu, 74)
        self.hito_borde(d, 'nieve_entrada74')
        self.esperar(d, 256)
        self.hito_borde(d, 'nieve_control')
        self.auditar(d, 'entrada de SNOWAST4')
        informe = {
            'resultado': 'coinciden',
            'casos_y_pasos': sum(self.resumen.values()),
            'grupos': self.resumen,
            'capturas': self.capturas,
            'estados_auditados': self.estados_auditados,
            'alcance': 'Carga74 desde vestido real, entidades, cámara inicial, 256 pasos, composición y opciones de entrada. No acredita todo el puzle, salidas ni SDL.',
        }
        (self.destino / 'resultado.json').write_text(
            json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8'
        )
        print(
            f'Entrada nieve: {informe["casos_y_pasos"]} casos/pasos y {self.capturas} composiciones coincidentes.'
        )


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Contrasta la entrada74 real de SNOWAST4.')
    parser.add_argument('--biblioteca', type=Path, required=True)
    parser.add_argument('--muestra', action='store_true')
    opciones = parser.parse_args()
    try:
        ComparacionEntradaNieve(
            Path(__file__).resolve().parents[3], opciones.biblioteca, opciones.muestra
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        parser.exit(1, f'Error: {error}\n')
