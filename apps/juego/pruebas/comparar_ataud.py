"""Contrasta el tercer pago al mago, el ataúd y la salida al cementerio."""

import argparse
import itertools
import json
from pathlib import Path

from comparar_guardia_palacio import ComparacionGuardiaPalacio, ErrorAnalisis


class ComparacionAtaud(ComparacionGuardiaPalacio):
    # 60CE/4502: comprueba capacidad, iconos, consumo ausente, canales y puntuación envolvente.
    def variantes_hueso(self, base):
        for ocupadas, icono, puntos in itertools.product((0, 9, 10), (0, 31, 34), (0, 65535)):
            cambios = [(0x524E + 165 * 2, icono, True), (0x35C7, puntos, False)]
            cambios += [
                (0x5225 + i * 2, 76 + i if i < ocupadas else 65535, False) for i in range(10)
            ]
            self.variante(base, 'recoger165', 0x563A, 165, 0, cambios)
        for presente, puntos, actividad in itertools.product((False, True), (0, 65535), (0, 1, 3)):
            cambios = [(0x35C7, puntos, False), (0x2C4C, actividad, True)]
            cambios += [
                (0x5225 + i * 2, 165 if presente and i == 0 else 65535, False) for i in range(10)
            ]
            self.variante(base, 'excavar165', 0x2FCA, 165, 0, cambios)
        self.retomar_recursos(base)

    # Continúa desde Benn con el extintor y las banderas reales del palacio.
    def ejecutar(self):
        origen = self.raiz / 'apps/juego/trabajo/comparacion_guardia_palacio/guardia_benn_control'
        d = bytearray(origen.with_suffix('.ds.bin').read_bytes())
        self.semilla_menu[:] = json.loads(origen.with_suffix('.semilla.json').read_text())
        self.destino = self.raiz / 'apps/juego/trabajo/comparacion_ataud'
        self.destino.mkdir(parents=True, exist_ok=True)
        self.resumen = {}
        self.variantes = {}
        self.retomar_recursos(d)
        self.ruta(d, ((200, 155), (210, 145)), 46)
        self.ruta(d, ((200, 180), (160, 199)), 37)
        self.ruta(d, ((220, 180), (235, 120), (225, 70), (170, 50), (170, 5)), 28)
        self.ruta(d, ((115, 170), (110, 130)), 32)
        self.hito_borde(d, 'ataud_mago')
        self.auditar(d, 'mago después del palacio')
        self.elegir_objeto(d, (2, 6), 76)
        self.elegir_objeto(d, (), 77)
        for paso in range(768):
            salida = self.andar(d, self.semilla_menu, 65535, 1)
            if salida is not None:
                print('transporte', salida, paso, flush=True)
                (self.destino / 'ataud_transporte.ds.bin').write_bytes(d)
                (self.destino / 'ataud_transporte.semilla.json').write_text(
                    json.dumps(list(self.semilla_menu))
                )
                break
        else:
            raise ErrorAnalisis('El tercer pago no produce transporte.')
        if salida != 112:
            raise ErrorAnalisis(f'El tercer pago solicita{salida}, esperada112.')
        self.cambiar(d, self.semilla_menu, 112)
        self.hito_borde(d, 'ataud_entrada')
        self.esperar(d, 192)
        self.hito_borde(d, 'ataud_control')
        self.auditar(d, 'dentro del ataúd')
        self.variantes_hueso(d)
        self.elegir_objeto(d, (1,), 165)
        self.esperar(d, 96)
        self.exigir_objeto(d, 165)
        self.hito_borde(d, 'ataud_hueso')
        self.auditar(d, 'hueso en el ataúd')
        self.elegir_objeto(d, (5,), 165)
        for paso in range(768):
            salida = self.andar(d, self.semilla_menu, 65535, 1)
            if salida is not None:
                if salida != 113:
                    raise ErrorAnalisis(f'El hueso solicita{salida}, esperada113.')
                (self.destino / 'ataud_solicitud113.ds.bin').write_bytes(d)
                (self.destino / 'ataud_solicitud113.semilla.json').write_text(
                    json.dumps(list(self.semilla_menu))
                )
                print('Ataúd resuelto; salida113, paso', paso, flush=True)
                informe = dict(
                    resultado='coinciden',
                    casos_y_pasos=sum(self.resumen.values()),
                    grupos=self.resumen,
                    variantes=self.variantes,
                    capturas=self.capturas,
                    estados_auditados=self.estados_auditados,
                    limite='Tercer pago, transporte112 desde MAGE22, ataúd64 y hueso165 hasta solicitar113. Cementerio65 pendiente.',
                )
                (self.destino / 'resultado.json').write_text(
                    json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8'
                )
                return
            if paso in (31, 63, 95):
                self.hito_borde(d, 'ataud_excavar_' + str(paso + 1))
        raise ErrorAnalisis('El hueso no entrega113.')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--biblioteca', type=Path, required=True)
    parser.add_argument('--muestra', action='store_true')
    args = parser.parse_args()
    try:
        ComparacionAtaud(
            Path(__file__).resolve().parents[3], args.biblioteca, args.muestra
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        parser.exit(1, f'Error: {error}\n')
