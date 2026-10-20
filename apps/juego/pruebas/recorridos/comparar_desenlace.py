"""Contrasta BASEBAT73/OUTRO desde la derrota de la bruja hasta el final de sus canales."""

import argparse
import json
from pathlib import Path

from comparar_bruja import ComparacionBruja, ErrorAnalisis
from comparar_objetos import palabra


class ComparacionDesenlace(ComparacionBruja):
    # 4D40 y tres actores de5206: vaciado original, golpe de bate, pelota y canales hasta reposo.
    def ejecutar(self):
        origen = self.raiz / 'apps/juego/trabajo/comparacion_bruja/bruja_solicitud125'
        d = bytearray(origen.with_suffix('.ds.bin').read_bytes())
        self.semilla_menu[:] = json.loads(origen.with_suffix('.semilla.json').read_text())
        self.destino = self.raiz / 'apps/juego/trabajo/comparacion_desenlace'
        self.destino.mkdir(parents=True, exist_ok=True)
        self.resumen = {}
        self.referencia_modelo = 0x9000
        self.retomar_recursos(d)
        self.cambiar(d, self.semilla_menu, 125)
        if any(palabra(d, 0x5225 + i * 2) != 65535 for i in range(12)):
            raise ErrorAnalisis('El desenlace conserva objetos o temas que4D40 vacía.')
        self.hito_borde(d, 'desenlace_entrada')
        canales = [base + canal * 5 for base in (0x2C5C, 0x2C7D, 0x2C9E) for canal in range(3)]
        for paso in range(1, 513):
            salida = self.andar(d, self.semilla_menu, 65535, 1)
            if salida is not None:
                raise ErrorAnalisis(f'El desenlace solicita una salida inesperada{salida}.')
            if paso in (16, 32, 48, 64):
                self.hito_borde(d, 'desenlace_' + str(paso))
            if not any(palabra(d, p) for p in canales):
                print('Desenlace en reposo tras', paso, 'pasos.', flush=True)
                self.hito_borde(d, 'desenlace_reposo')
                break
        else:
            raise ErrorAnalisis('Los canales del desenlace no terminan.')
        self.auditar(d, 'campo de béisbol después del final')
        self.esperar(d, 128)
        self.hito_borde(d, 'desenlace_final')
        if any(palabra(d, p) for p in canales) or not d[0x3595]:
            raise ErrorAnalisis('El final no conserva el reposo y el menú originales.')
        informe = dict(
            resultado='coinciden',
            casos_y_pasos=sum(self.resumen.values()),
            grupos=self.resumen,
            capturas=self.capturas,
            estados_auditados=self.estados_auditados,
            limite='Desenlace73 completo hasta fin de los nueve canales y128 pasos de reposo. Revisión integral de la aventura aún pendiente.',
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
        ComparacionDesenlace(
            Path(__file__).resolve().parents[4], args.biblioteca, args.muestra
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        parser.exit(1, f'Error: {error}\n')
