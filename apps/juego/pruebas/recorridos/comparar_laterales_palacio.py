"""Contrasta los otros cuatro laterales, escoba, útiles y resolución de poleas y ventana."""

import argparse
import json
from pathlib import Path

from comparar_utiles_palacio import ComparacionUtilesPalacio, ErrorAnalisis
from comparar_objetos import palabra


class ComparacionLateralesPalacio(ComparacionUtilesPalacio):
    # BAA6/BB18: exige actividad1 eY89, conserva canales omitidos y puntuación envolvente.
    def variantes_escoba(self, base):
        for actividad in (0, 1, 3):
            for y in (88, 89, 90, 65535):
                for puntos in (0, 65535):
                    cambios = [
                        (0x2C4C, actividad, True),
                        (0x2C50, y, False),
                        (0x35C7, puntos, False),
                    ]
                    cambios += [(p, 0xCAFE, False) for p in (0xA2, 0xAC)]
                    self.variante(
                        base, 'escoba' + str(palabra(base, 0x8082)), 0x2FCA, 141, cambios=cambios
                    )
        self.retomar_recursos(base)

    # Regresa a la mesa atravesando solo los enlaces originales, conservando el inventario.
    def volver_dados(self, d):
        if palabra(d, 0x8082) == 49:
            self.ruta(d, ((160, 199),), 89)
        self.ruta(d, ((160, 199),), 87)
        self.ruta(d, ((160, 199),), 85)

    # Obtiene151–154 con tiradas reales, incluidos los dos objetos altos que debe bajar la escoba.
    def ejecutar(self):
        origen = self.raiz / 'apps/juego/trabajo/comparacion_utiles_palacio/utiles_dados'
        d = bytearray(origen.with_suffix('.ds.bin').read_bytes())
        self.semilla_menu[:] = json.loads(
            origen.with_suffix('.semilla.json').read_text(encoding='utf-8')
        )
        self.destino = self.raiz / 'apps/juego/trabajo/comparacion_laterales_palacio'
        self.destino.mkdir(parents=True, exist_ok=True)
        self.resumen = {}
        self.variantes = {}
        self.tiradas = {}
        self.retomar_recursos(d)
        for escena, bandera, entrada, vuelta, id, nombre in (
            (52, 0x4B3E, 95, 96, 151, 'arma'),
            (53, 0x4B47, 97, 98, 152, 'megafono'),
            (54, 0x4B50, 99, 100, 153, 'hielo'),
            (55, 0x4B59, 101, 102, 154, 'carambano'),
        ):
            self.abrir_puerta(d, escena, bandera)
            self.hito_borde(d, nombre + '_tirada')
            self.ruta(d, ((160, 145),), 84)
            self.ruta(d, ((160, 166),), 86)
            if escena >= 54:
                self.ruta(d, ((160, 166),), 88)
            self.hito_borde(d, nombre + '_puerta')
            puntos = (
                ((80, 182), (49, 182), (70, 190), (50, 190), (40, 182))
                if escena % 2 == 0
                else ((220, 175), (242, 172))
            )
            self.ruta(d, puntos, entrada)
            self.hito_borde(d, nombre + '_entrada')
            self.auditar(d, nombre + ': entrada')
            self.variantes_recogida(d, id)
            if escena in (52, 55):
                self.variantes_escoba(d)
                self.elegir_objeto(d, (5,), 141)
                self.esperar(d, 512)
                self.hito_borde(d, nombre + '_bajado')
                self.auditar(d, nombre + ': bajado con escoba')
            x = palabra(d, 0x2C4E) + 5
            y = (
                palabra(d, 0x2C50)
                + palabra(d, 0x2C54)
                - (5 if id == 153 else 2 if id == 154 else 4)
            )
            self.ruta_sin_salida(d, x, y)
            self.hito_borde(d, nombre + '_ante_objeto')
            self.auditar(d, nombre + ': proximidad')
            self.elegir_objeto(d, (1,), id)
            self.esperar(d, 192)
            self.exigir_objeto(d, id)
            self.hito_borde(d, nombre + '_recogido')
            self.auditar(d, nombre + ': recogido')
            self.ruta(d, ((280 if escena % 2 == 0 else 41, 165),), vuelta)
            self.hito_borde(d, nombre + '_regreso')
            self.auditar(d, nombre + ': regreso')
            self.volver_dados(d)
            self.hito_borde(d, nombre + '_dados')
        self.hito_borde(d, 'laterales_utiles')
        informe = dict(
            resultado='coinciden',
            casos_y_pasos=sum(self.resumen.values()),
            grupos=self.resumen,
            capturas=self.capturas,
            estados_auditados=self.estados_auditados,
            variantes=self.variantes,
            tiradas_reales=self.tiradas,
        )
        (self.destino / 'resultado.json').write_text(
            json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8'
        )
        print(
            f'Laterales: {informe["casos_y_pasos"]} casos/pasos y{self.capturas} imágenes coincidentes.'
        )


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Contrasta los cuatro laterales restantes del palacio.'
    )
    parser.add_argument('--biblioteca', type=Path, required=True)
    parser.add_argument('--muestra', action='store_true')
    opciones = parser.parse_args()
    try:
        ComparacionLateralesPalacio(
            Path(__file__).resolve().parents[4], opciones.biblioteca, opciones.muestra
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        parser.exit(1, f'Error: {error}\n')
