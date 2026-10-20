"""Contrasta el puente CASTLENT68 y su derrumbe desde la verja."""

import argparse
import json
from pathlib import Path

from comparar_verja import ComparacionVerja, ErrorAnalisis


class ComparacionPuenteCastillo(ComparacionVerja):
    # Entrada118: los canales84E7/8503/851E escenifican el derrumbe; CS17A3[68]=0.
    def ejecutar(self):
        origen = self.raiz / 'apps/juego/trabajo/comparacion_verja/verja_solicitud118'
        d = bytearray(origen.with_suffix('.ds.bin').read_bytes())
        self.semilla_menu[:] = json.loads(origen.with_suffix('.semilla.json').read_text())
        self.destino = self.raiz / 'apps/juego/trabajo/comparacion_puente_castillo'
        self.destino.mkdir(parents=True, exist_ok=True)
        self.resumen = {}
        self.referencia_modelo = 0x9000
        self.retomar_recursos(d)
        self.cambiar(d, self.semilla_menu, 118)
        self.hito_borde(d, 'puente_castillo_entrada')
        for paso in range(1, 257):
            salida = self.andar(d, self.semilla_menu, 65535, 1)
            if salida is not None:
                if salida != 119:
                    raise ErrorAnalisis(f'El puente solicita{salida}, esperada119.')
                print('Transporte automático119 tras', paso, 'pasos.', flush=True)
                break
            if paso in (32, 64, 80):
                self.hito_borde(d, 'puente_castillo_' + str(paso))
        else:
            raise ErrorAnalisis('El derrumbe no entrega119.')
        (self.destino / 'puente_castillo_solicitud119.ds.bin').write_bytes(d)
        (self.destino / 'puente_castillo_solicitud119.semilla.json').write_text(
            json.dumps(list(self.semilla_menu)) + '\n'
        )
        informe = dict(
            resultado='coinciden',
            casos_y_pasos=sum(self.resumen.values()),
            grupos=self.resumen,
            capturas=self.capturas,
            estados_auditados=self.estados_auditados,
            limite='CASTLENT68 desde118, derrumbe y solicitud119. Destino69 pendiente.',
        )
        (self.destino / 'resultado.json').write_text(
            json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8'
        )
        print(json.dumps(informe, ensure_ascii=False), flush=True)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--biblioteca', type=Path, required=True)
    parser.add_argument('--muestra', action='store_true')
    args = parser.parse_args()
    try:
        ComparacionPuenteCastillo(
            Path(__file__).resolve().parents[4], args.biblioteca, args.muestra
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        parser.exit(1, f'Error: {error}\n')
