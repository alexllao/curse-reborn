"""Contrasta LIBRARY71, el libro176 y el pasadizo hacia la batalla final."""

import argparse
import itertools
import json
from pathlib import Path

from comparar_estudio_castillo import ComparacionEstudioCastillo, ErrorAnalisis
from comparar_calles import ComparacionCalles
from comparar_objetos import palabra


class ComparacionBibliotecaCastillo(ComparacionEstudioCastillo):
    # 48B0: frontera187, cámara, signo de Y y campos auxiliares conservados por la acción.
    def variantes_libro(self, base):
        semilla = list(self.semilla_menu)
        for global_x, camara, y, auxiliar in itertools.product(
            (186, 187, 188), (0, 64), (32768, 131, 132, 32767), (0, 0x1234)
        ):
            cambios = [
                (0x2C6F, (global_x - camara) & 65535, False),
                (0x3591, camara, False),
                (0x2C2F, y, False),
                (0x00A2, auxiliar, False),
                (0x00A4, auxiliar, False),
                (0x00AE, auxiliar, False),
            ]
            self.variante(base, 'empujar176', 0x3B53, 176, 2, cambios)
        self.semilla_menu[:] = semilla
        self.retomar_recursos(base)

    # Continúa con el anillo real desde el estudio, sin preparar la puerta secreta abierta.
    def ejecutar(self):
        origen = self.raiz / 'apps/juego/trabajo/comparacion_estudio_castillo/estudio_vestibulo'
        d = bytearray(origen.with_suffix('.ds.bin').read_bytes())
        self.semilla_menu[:] = json.loads(origen.with_suffix('.semilla.json').read_text())
        self.destino = self.raiz / 'apps/juego/trabajo/comparacion_biblioteca_castillo'
        self.destino.mkdir(parents=True, exist_ok=True)
        self.resumen = {}
        self.variantes = {}
        self.referencia_modelo = 0x9000
        self.retomar_recursos(d)
        salida = None
        for x, y in ((80, 170), (140, 190), (270, 190), (280, 170), (300, 150)):
            salida = ComparacionCalles.ir(self, d, self.semilla_menu, x, y)
            if salida is not None:
                break
        if salida != 122:
            raise ErrorAnalisis(f'Camino de la biblioteca solicita{salida}, esperada122.')
        self.cambiar(d, self.semilla_menu, 122)
        self.hito_borde(d, 'biblioteca_entrada')
        self.esperar(d, 128)
        self.hito_borde(d, 'biblioteca_control')
        self.auditar(d, 'entrada de la biblioteca')
        salida = ComparacionCalles.ir(self, d, self.semilla_menu, 24, 164)
        if salida != 123:
            raise ErrorAnalisis(f'Regreso de la biblioteca solicita{salida}, esperada123.')
        self.cambiar(d, self.semilla_menu, 123)
        self.esperar(d, 128)
        self.hito_borde(d, 'biblioteca_regreso')
        self.auditar(d, 'vestíbulo tras la biblioteca')
        salida = None
        for x, y in ((270, 190), (280, 170), (300, 150)):
            salida = ComparacionCalles.ir(self, d, self.semilla_menu, x, y)
            if salida is not None:
                break
        if salida != 122:
            raise ErrorAnalisis(f'Segunda entrada en biblioteca solicita{salida}, esperada122.')
        self.cambiar(d, self.semilla_menu, 122)
        self.esperar(d, 128)
        self.hito_borde(d, 'biblioteca_segunda_visita')
        self.variantes_libro(d)
        for x, y in ((80, 190), (140, 190), (142, 159)):
            if ComparacionCalles.ir(self, d, self.semilla_menu, x, y) is not None:
                raise ErrorAnalisis('Se abandona la biblioteca antes del libro.')
        self.hito_borde(d, 'biblioteca_libro')
        self.auditar(d, 'ante el libro')
        print('Libro global', (palabra(d, 0x2C6F) + palabra(d, 0x3591)) & 65535, flush=True)
        self.elegir_objeto(d, (3,), 176)
        self.esperar(d, 32)
        self.hito_borde(d, 'biblioteca_examinada')
        self.elegir_objeto(d, (2, 2), 176)
        self.esperar(d, 32)
        self.hito_borde(d, 'biblioteca_abriendo')
        self.esperar(d, 224)
        self.hito_borde(d, 'biblioteca_abierta')
        self.auditar(d, 'pasadizo abierto')
        salida = None
        for x, y in ((142, 190), (205, 190), (205, 154)):
            salida = ComparacionCalles.ir(self, d, self.semilla_menu, x, y)
            if salida is not None:
                break
        if salida != 124:
            raise ErrorAnalisis(
                f'Pasadizo solicita{salida}, esperada124; libro{palabra(d, 0x2C6F)}, posición{palabra(d, 0x2C2D)},{palabra(d, 0x2C2F)}.'
            )
        prefijo = self.destino / 'biblioteca_solicitud124'
        prefijo.with_suffix('.ds.bin').write_bytes(d)
        prefijo.with_suffix('.semilla.json').write_text(json.dumps(list(self.semilla_menu)) + '\n')
        self.conservar_contexto(self.destino, 'biblioteca_solicitud124')
        informe = dict(
            resultado='coinciden',
            casos_y_pasos=sum(self.resumen.values()),
            grupos=self.resumen,
            variantes=self.variantes,
            capturas=self.capturas,
            estados_auditados=self.estados_auditados,
            limite='LIBRARY71 con anillo175, libro176 y solicitud124. Batalla72 pendiente.',
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
        ComparacionBibliotecaCastillo(
            Path(__file__).resolve().parents[4], args.biblioteca, args.muestra
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        parser.exit(1, f'Error: {error}\n')
