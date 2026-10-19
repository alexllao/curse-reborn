"""Contrasta entrada64, petición de ayuda, detergente120 y regreso65 desde la misma partida."""

import argparse
import json
from pathlib import Path

from comparar_nube import ComparacionNube, ErrorAnalisis
from comparar_objetos import palabra


class ComparacionCuevaValle(ComparacionNube):
    # 337D–34D6: estado3 requerido, destino, retardos, preservación de fases y puntuación.
    def variantes_ayuda(self, base):
        self.variantes = {}
        for activo in (0, 1, 3, 255):
            for x in (10, 150, 300):
                for y in (100, 176):
                    for puntos in (0, 65535):
                        self.variante(
                            base,
                            'ayuda',
                            0x32DE,
                            0,
                            cambios=(
                                (0x2DF9, activo, True),
                                (0x2C2D, x, False),
                                (0x2C2F, y, False),
                                (0x35C7, puntos, False),
                                (0x2E0B, 7, True),
                                (0x2C5E, 3, True),
                            ),
                        )
        self.retomar_recursos(base)

    # 563A/5E57: contrasta límite de inventario e iconos de puntos de la botella.
    def variantes_detergente(self, base):
        for ocupadas in (0, 9, 10):
            for icono in (0, 0x1F, 0x22):
                for puntos in (0, 65535):
                    cambios = [(0x524E + 120 * 2, icono, True), (0x35C7, puntos, False)]
                    cambios += [
                        (0x5225 + i * 2, 3 if i < ocupadas else 65535, False) for i in range(10)
                    ]
                    self.variante(base, 'detergente', 0x563A, 120, cambios=cambios)
        self.retomar_recursos(base)

    # Continúa nube→POST→DINO→GHETTO y recoge120 sin preparar inventarios ni posiciones.
    def ejecutar(self):
        origen = self.raiz / 'trabajo/comparacion_nube/nube_regreso_post'
        if not origen.with_suffix('.ds.bin').exists():
            ComparacionNube(self.raiz, self.biblioteca, True).ejecutar()
        d = bytearray(origen.with_suffix('.ds.bin').read_bytes())
        self.semilla_menu[:] = json.loads(
            origen.with_suffix('.semilla.json').read_text(encoding='utf-8')
        )
        self.destino = self.raiz / 'trabajo/comparacion_cueva_valle'
        self.destino.mkdir(parents=True, exist_ok=True)
        self.resumen = {}
        self.retomar_recursos(d)
        self.ruta(d, ((850, 190), (850, 199)), 57)
        self.ruta(d, ((250, 170), (150, 165), (133, 148)), 64)
        if palabra(d, 0x8082) != 37 or d[0x2DF9] != 3:
            raise ErrorAnalisis('La entrada64 no conserva las rocas previas a la ayuda.')
        self.hito_borde(d, 'cueva_entrada')
        self.auditar(d, 'GHETTO antes de pedir ayuda')
        self.variantes_ayuda(d)
        puntos = palabra(d, 0x35C7)
        self.menu(d, (4,))
        if palabra(d, 0xB622) != 0x17:
            raise ErrorAnalisis('La casilla de ayuda no contiene el identificador original0.')
        self.seleccionar(d, 1)
        self.esperar(d, 512)
        if d[0x2DF9] != 1 or d[0x4794] != 1 or palabra(d, 0x35C7) != puntos:
            raise ErrorAnalisis('La ayuda no revela el detergente sin cambiar la puntuación.')
        self.hito_borde(d, 'cueva_derrumbe')
        self.ruta_sin_salida(d, 171, 145)
        self.auditar(d, 'GHETTO con detergente expuesto')
        self.variantes_detergente(d)
        self.elegir_objeto(d, (1,), 120)
        self.esperar(d, 192)
        if 120 not in [palabra(d, 0x5225 + i * 2) for i in range(10)] or d[0x2DF9]:
            raise ErrorAnalisis('El detergente120 no llega al inventario tras recogerlo.')
        self.hito_borde(d, 'cueva_detergente')
        self.ruta(d, ((150, 199),), 65)
        self.hito_borde(d, 'cueva_regreso_dino')
        self.auditar(d, 'DINO después de recoger detergente')
        informe = {
            'resultado': 'coinciden',
            'casos_y_pasos': sum(self.resumen.values()),
            'grupos': self.resumen,
            'variantes': self.variantes,
            'capturas': self.capturas,
            'estados_auditados': self.estados_auditados,
            'alcance': 'Entrada64, ayuda y derrumbe, recogida120 y regreso65 en la misma partida tras la nube. No acredita otras rutas del valle ni su integración SDL.',
        }
        (self.destino / 'resultado.json').write_text(
            json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8'
        )
        print(
            f'Cueva del valle: {informe["casos_y_pasos"]} casos/pasos y {self.capturas} composiciones coincidentes.'
        )


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Contrasta GHETTO y la petición de ayuda original.'
    )
    parser.add_argument('--biblioteca', type=Path, required=True)
    parser.add_argument('--muestra', action='store_true')
    opciones = parser.parse_args()
    try:
        ComparacionCuevaValle(
            Path(__file__).resolve().parents[1], opciones.biblioteca, opciones.muestra
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        parser.exit(1, f'Error: {error}\n')
