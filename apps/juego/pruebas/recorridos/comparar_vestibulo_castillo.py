"""Contrasta CASTHALL69 y sus dos escaleras desde el derrumbe del puente."""

import argparse
import json
from pathlib import Path

from comparar_puente_castillo import ComparacionPuenteCastillo, ErrorAnalisis
from comparar_calles import ComparacionCalles
from comparar_objetos import palabra


class ComparacionVestibuloCastillo(ComparacionPuenteCastillo):
    # CS17A3[69]=0: conserva menú, colisiones y ambas salidas reales sin fabricar progreso.
    def ejecutar(self):
        origen = (
            self.raiz
            / 'apps/juego/trabajo/comparacion_puente_castillo/puente_castillo_solicitud119'
        )
        d = bytearray(origen.with_suffix('.ds.bin').read_bytes())
        self.semilla_menu[:] = json.loads(origen.with_suffix('.semilla.json').read_text())
        self.destino = self.raiz / 'apps/juego/trabajo/comparacion_vestibulo_castillo'
        self.destino.mkdir(parents=True, exist_ok=True)
        self.resumen = {}
        self.referencia_modelo = 0x9000
        self.retomar_recursos(d)
        self.cambiar(d, self.semilla_menu, 119)
        self.hito_borde(d, 'vestibulo_entrada')
        self.esperar(d, 128)
        if not d[0x3594]:
            raise ErrorAnalisis('El vestíbulo no devuelve el control.')
        self.hito_borde(d, 'vestibulo_control')
        self.auditar(d, 'vestíbulo tras el puente')
        base, semilla = bytes(d), list(self.semilla_menu)
        for nombre, puntos, salida_esperada in (
            ('izquierda', ((90, 160), (48, 126)), 120),
            ('derecha', ((270, 190), (280, 170), (300, 150)), 122),
        ):
            d = bytearray(base)
            self.semilla_menu[:] = semilla
            self.retomar_recursos(d)
            x, y = puntos[0]
            if ComparacionCalles.ir(self, d, self.semilla_menu, x, y) is not None:
                raise ErrorAnalisis('El vestíbulo se abandona antes del umbral lateral.')
            self.hito_borde(d, 'vestibulo_' + nombre)
            self.auditar(d, 'escalera ' + nombre)
            salida = None
            for x, y in puntos[1:]:
                salida = ComparacionCalles.ir(self, d, self.semilla_menu, x, y)
                if salida is not None:
                    break
            if salida != salida_esperada:
                raise ErrorAnalisis(
                    f'Escalera{nombre}: salida{salida}, esperada{salida_esperada}; posición{palabra(d, 0x2C2D)},{palabra(d, 0x2C2F)}.'
                )
            prefijo = self.destino / ('vestibulo_solicitud' + str(salida))
            prefijo.with_suffix('.ds.bin').write_bytes(d)
            prefijo.with_suffix('.semilla.json').write_text(
                json.dumps(list(self.semilla_menu)) + '\n'
            )
        informe = dict(
            resultado='coinciden',
            casos_y_pasos=sum(self.resumen.values()),
            grupos=self.resumen,
            capturas=self.capturas,
            estados_auditados=self.estados_auditados,
            limite='CASTHALL69 desde119 hasta las solicitudes120/122. STUDY70 y LIBRARY71 pendientes.',
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
        ComparacionVestibuloCastillo(
            Path(__file__).resolve().parents[4], args.biblioteca, args.muestra
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        parser.exit(1, f'Error: {error}\n')
