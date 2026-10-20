"""Contrasta GATE67 y la apertura174 hasta solicitar118."""

import argparse
import itertools
import json
from pathlib import Path

from comparar_bosque import ComparacionBosque, ErrorAnalisis
from comparar_calles import ComparacionCalles
from comparar_objetos import palabra


class ComparacionVerja(ComparacionBosque):
    # 4838–48AF: no condiciona la apertura a inventario, fase o canales auxiliares iniciales.
    def variantes_apertura(self, base):
        semilla = list(self.semilla_menu)
        for puntos, camara, auxiliar in itertools.product((0, 65535), (0, 128, 288), (0, 0x1234)):
            cambios = [(0x35C7, puntos, False), (0x3591, camara, False)]
            cambios += [
                (p, auxiliar, False) for p in (0x00A2, 0x00AA, 0x00AC, 0x00B6, 0x00BE, 0x00C0)
            ]
            self.variante(base, 'apertura174', 0x3B53, 174, 2, cambios)
        self.semilla_menu[:] = semilla
        self.retomar_recursos(base)

    # Retoma117 sin fabricar inventario y recorre los canales originales de la verja.
    def ejecutar(self):
        origen = self.raiz / 'apps/juego/trabajo/comparacion_bosque/bosque_solicitud117'
        d = bytearray(origen.with_suffix('.ds.bin').read_bytes())
        self.semilla_menu[:] = json.loads(origen.with_suffix('.semilla.json').read_text())
        self.destino = self.raiz / 'apps/juego/trabajo/comparacion_verja'
        self.destino.mkdir(parents=True, exist_ok=True)
        self.resumen = {}
        self.variantes = {}
        self.referencia_modelo = 0x9000
        self.retomar_recursos(d)
        self.cambiar(d, self.semilla_menu, 117)
        self.hito_borde(d, 'verja_entrada')
        self.esperar(d, 128)
        self.hito_borde(d, 'verja_control')
        self.auditar(d, 'entrada ante la verja')
        self.variantes_apertura(d)
        ComparacionCalles.ir(self, d, self.semilla_menu, 180, 171)
        self.hito_borde(d, 'verja_cerrada')
        self.auditar(d, 'verja cerrada')
        self.elegir_objeto(d, (2, 2), 174)
        self.esperar(d, 32)
        self.hito_borde(d, 'verja_abriendo')
        self.esperar(d, 480)
        self.hito_borde(d, 'verja_abierta')
        self.auditar(d, 'verja abierta')
        salida = None
        for x, y in ((370, 145), (475, 130), (480, 118)):
            salida = ComparacionCalles.ir(self, d, self.semilla_menu, x, y)
            if salida is not None:
                break
        if salida != 118:
            raise ErrorAnalisis(
                f'La verja solicita{salida}, esperada118; posición{palabra(d, 0x2C2D)},{palabra(d, 0x2C2F)}.'
            )
        (self.destino / 'verja_solicitud118.ds.bin').write_bytes(d)
        (self.destino / 'verja_solicitud118.semilla.json').write_text(
            json.dumps(list(self.semilla_menu)) + '\n'
        )
        informe = dict(
            resultado='coinciden',
            casos_y_pasos=sum(self.resumen.values()),
            grupos=self.resumen,
            variantes=self.variantes,
            capturas=self.capturas,
            estados_auditados=self.estados_auditados,
            limite='GATE67 desde117 y apertura174 hasta solicitar118. Destino68 pendiente.',
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
        ComparacionVerja(
            Path(__file__).resolve().parents[4], args.biblioteca, args.muestra
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        parser.exit(1, f'Error: {error}\n')
