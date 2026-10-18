"""Contrasta la visita al loro desde la salida real de EDGE y el regreso a la casa."""

import argparse
import json
from pathlib import Path

from comparar_acciones_borde import ComparacionAccionesBorde, ErrorAnalisis
from comparar_objetos import palabra


class ComparacionLoro(ComparacionAccionesBorde):
    # Comprueba el saludo inicial y repetido para cada combinación de banderas, sin fingir una visita.
    def variantes_saludo(self, base):
        for banderas in range(256):
            self.variante(base, 'saludos', 0x32DE, 1, cambios=((0x35C9, banderas, True),))

    # Reproduce entrada72, diálogo, salida73 y regreso real sin conceder objetos de la guía.
    def ejecutar(self):
        origen = self.raiz / 'trabajo/comparacion_acciones_borde/borde_salida'
        if not origen.with_suffix('.ds.bin').exists():
            ComparacionAccionesBorde(self.raiz, self.biblioteca, True).ejecutar()
        d = bytearray(origen.with_suffix('.ds.bin').read_bytes())
        self.semilla_menu[:] = json.loads(
            origen.with_suffix('.semilla.json').read_text(encoding='utf-8')
        )
        self.destino = self.raiz / 'trabajo/comparacion_loro'
        self.destino.mkdir(parents=True, exist_ok=True)
        self.resumen = {}
        self.variantes = {}
        self.retomar_recursos(d)
        self.cambiar(d, self.semilla_menu, 72)
        self.hito_borde(d, 'loro_cargado')
        if self.andar(d, self.semilla_menu, 65535, 128) is not None:
            raise ErrorAnalisis('La entrada del loro abandona la habitación.')
        self.hito_borde(d, 'loro_entrada')
        self.variantes_saludo(d)
        self.auditar(d, 'llegada al loro')
        self.menu(d, (4,))
        self.seleccionar(d, 0)
        salida = None
        for paso in range(32):
            salida = self.andar(d, self.semilla_menu, 65535, 32)
            if paso in (1, 3):
                self.hito_borde(d, f'loro_saludo_{(paso + 1) * 32}')
            if salida is not None:
                break
        if salida != 73:
            raise ErrorAnalisis(f'El saludo solicita {salida}, esperada73.')
        self.hito_borde(d, 'loro_salida')
        self.cambiar(d, self.semilla_menu, 73)
        if self.andar(d, self.semilla_menu, 65535, 256) is not None or palabra(d, 0x8082) != 27:
            raise ErrorAnalisis('El regreso del loro no alcanza DERELICT27.')
        self.hito_borde(d, 'loro_regreso')
        self.auditar(d, 'regreso del loro')
        informe = {
            'resultado': 'coinciden',
            'casos_y_pasos': sum(self.resumen.values()),
            'grupos': self.resumen,
            'capturas': self.capturas,
            'estados_auditados': self.estados_auditados,
            'variantes_aisladas': self.variantes,
            'alcance': 'Visita inicial desde EDGE, diálogo y regreso73 a DERELICT27; saludo repetido en variantes aisladas. La guía no sustituye las condiciones del EXE.',
        }
        (self.destino / 'resultado.json').write_text(
            json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8'
        )
        print(
            f'Loro: {informe["casos_y_pasos"]} casos/pasos y {self.capturas} composiciones coincidentes.'
        )


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Contrasta la visita original a WISE31.')
    parser.add_argument('--biblioteca', type=Path, required=True)
    parser.add_argument('--muestra', action='store_true')
    opciones = parser.parse_args()
    try:
        ComparacionLoro(
            Path(__file__).resolve().parents[1], opciones.biblioteca, opciones.muestra
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        parser.exit(1, f'Error: {error}\n')
