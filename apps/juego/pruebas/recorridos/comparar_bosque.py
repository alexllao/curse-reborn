"""Contrasta COPSE66 desde la verja del cementerio hasta solicitar117."""

import argparse
import json
from pathlib import Path

from comparar_cementerio import ComparacionCementerio, ErrorAnalisis
from comparar_calles import ComparacionCalles
from comparar_objetos import palabra


class ComparacionBosque(ComparacionCementerio):
    # CS17A3[66]=0: la entrada116 y el sendero usan los servicios comunes sin rutina propia.
    def ejecutar(self):
        origen = self.raiz / 'apps/juego/trabajo/comparacion_cementerio/cementerio_solicitud116'
        d = bytearray(origen.with_suffix('.ds.bin').read_bytes())
        self.semilla_menu[:] = json.loads(origen.with_suffix('.semilla.json').read_text())
        self.destino = self.raiz / 'apps/juego/trabajo/comparacion_bosque'
        self.destino.mkdir(parents=True, exist_ok=True)
        self.resumen = {}
        self.referencia_modelo = 0x9000
        self.retomar_recursos(d)
        self.cambiar(d, self.semilla_menu, 116)
        self.hito_borde(d, 'bosque_entrada')
        self.esperar(d, 128)
        if not d[0x3594]:
            raise ErrorAnalisis('La entrada del bosque no devuelve el control.')
        self.hito_borde(d, 'bosque_control')
        self.auditar(d, 'sendero del bosque')
        base, semilla = bytes(d), list(self.semilla_menu)
        for x, y in ((230, 190), (205, 120), (214, 78)):
            salida = ComparacionCalles.ir(self, d, self.semilla_menu, x, y)
            if salida is not None:
                break
        if salida != 117:
            raise ErrorAnalisis(
                f'El sendero derecho solicita{salida}, esperada117; posición{palabra(d, 0x2C2D)},{palabra(d, 0x2C2F)}.'
            )
        d = bytearray(base)
        self.semilla_menu[:] = semilla
        self.retomar_recursos(d)
        if ComparacionCalles.ir(self, d, self.semilla_menu, 90, 120) is not None:
            raise ErrorAnalisis('El sendero izquierdo se abandona antes del umbral.')
        self.hito_borde(d, 'bosque_sendero')
        self.auditar(d, 'sendero izquierdo')
        salida = ComparacionCalles.ir(self, d, self.semilla_menu, 90, 78)
        if salida != 117:
            raise ErrorAnalisis(f'El bosque solicita{salida}, esperada117.')
        (self.destino / 'bosque_solicitud117.ds.bin').write_bytes(d)
        (self.destino / 'bosque_solicitud117.semilla.json').write_text(
            json.dumps(list(self.semilla_menu)) + '\n'
        )
        informe = dict(
            resultado='coinciden',
            casos_y_pasos=sum(self.resumen.values()),
            grupos=self.resumen,
            capturas=self.capturas,
            estados_auditados=self.estados_auditados,
            limite='COPSE66 desde116 hasta solicitar117. Verja67 pendiente.',
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
        ComparacionBosque(
            Path(__file__).resolve().parents[4], args.biblioteca, args.muestra
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        parser.exit(1, f'Error: {error}\n')
