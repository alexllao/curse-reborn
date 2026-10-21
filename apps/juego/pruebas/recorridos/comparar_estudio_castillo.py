"""Contrasta STUDY70, la recogida del anillo175 y el regreso121."""

import argparse
import itertools
import json
from pathlib import Path

from comparar_vestibulo_castillo import ComparacionVestibuloCastillo, ErrorAnalisis
from comparar_calles import ComparacionCalles
from comparar_objetos import palabra


class ComparacionEstudioCastillo(ComparacionVestibuloCastillo):
    # 6141/633F/61A7: capacidad, iconos, modelo20, puntos envolventes y conservación de53AD.
    def variantes_anillo(self, base):
        semilla = list(self.semilla_menu)
        for ocupadas, icono, modelo, puntos, bandera in itertools.product(
            (0, 9, 10), (0, 31, 34), (20, 255), (0, 65535), (0, 0xA4)
        ):
            cambios = [
                (0x524E + 175 * 2, icono, True),
                (0x358C, modelo, True),
                (0x35C7, puntos, False),
                (0x53AD, bandera, True),
            ]
            cambios += [
                (0x5225 + i * 2, 76 + i if i < ocupadas else 65535, False) for i in range(10)
            ]
            self.variante(base, 'recoger175', 0x563A, 175, 0, cambios)
        self.semilla_menu[:] = semilla
        self.retomar_recursos(base)

    # Parte de la escalera real120, recoge el anillo y contrasta su persistencia al regresar y volver.
    def ejecutar(self):
        origen = (
            self.raiz / 'apps/juego/trabajo/comparacion_vestibulo_castillo/vestibulo_solicitud120'
        )
        d = bytearray(origen.with_suffix('.ds.bin').read_bytes())
        self.semilla_menu[:] = json.loads(origen.with_suffix('.semilla.json').read_text())
        self.destino = self.raiz / 'apps/juego/trabajo/comparacion_estudio_castillo'
        self.destino.mkdir(parents=True, exist_ok=True)
        self.resumen = {}
        self.variantes = {}
        self.referencia_modelo = 0x9000
        self.retomar_recursos(d)
        self.cambiar(d, self.semilla_menu, 120)
        self.hito_borde(d, 'estudio_entrada')
        self.esperar(d, 128)
        self.hito_borde(d, 'estudio_control')
        self.auditar(d, 'entrada del estudio')
        self.variantes_anillo(d)
        x = palabra(d, 0x2C4E) + 5
        y = palabra(d, 0x2C50) + palabra(d, 0x2C54) + 1
        if ComparacionCalles.ir(self, d, self.semilla_menu, x, y) is not None:
            raise ErrorAnalisis('Se abandona el estudio antes de recoger el anillo.')
        self.hito_borde(d, 'estudio_anillo')
        self.auditar(d, 'ante el anillo')
        self.elegir_objeto(d, (1,), 175)
        self.esperar(d, 96)
        self.exigir_objeto(d, 175)
        self.hito_borde(d, 'estudio_recogido')
        self.auditar(d, 'anillo en el inventario')
        salida = ComparacionCalles.ir(self, d, self.semilla_menu, 319, 180)
        if salida != 121:
            raise ErrorAnalisis(f'El estudio solicita{salida}, esperada121.')
        self.cambiar(d, self.semilla_menu, 121)
        self.esperar(d, 128)
        self.hito_borde(d, 'estudio_regreso')
        self.auditar(d, 'vestíbulo con anillo')
        prefijo = self.destino / 'estudio_vestibulo'
        prefijo.with_suffix('.ds.bin').write_bytes(d)
        prefijo.with_suffix('.semilla.json').write_text(json.dumps(list(self.semilla_menu)) + '\n')
        self.conservar_contexto(self.destino, 'estudio_vestibulo')
        salida = ComparacionCalles.ir(self, d, self.semilla_menu, 48, 126)
        if salida != 120:
            raise ErrorAnalisis(f'La segunda visita solicita{salida}, esperada120.')
        self.cambiar(d, self.semilla_menu, 120)
        self.esperar(d, 128)
        if d[0x2C4C]:
            raise ErrorAnalisis('El anillo reaparece tras recogerlo.')
        self.exigir_objeto(d, 175)
        self.hito_borde(d, 'estudio_segunda_visita')
        informe = dict(
            resultado='coinciden',
            casos_y_pasos=sum(self.resumen.values()),
            grupos=self.resumen,
            variantes=self.variantes,
            capturas=self.capturas,
            estados_auditados=self.estados_auditados,
            limite='STUDY70, anillo175, regreso121 y segunda visita120. Biblioteca71 pendiente.',
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
        ComparacionEstudioCastillo(
            Path(__file__).resolve().parents[4], args.biblioteca, args.muestra
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        parser.exit(1, f'Error: {error}\n')
