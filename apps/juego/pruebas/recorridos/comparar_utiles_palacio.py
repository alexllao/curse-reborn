"""Contrasta puertas elegidas por dados, gato149, lupa150 y sus regresos persistentes."""

import argparse
import json
from pathlib import Path

from comparar_corredores_palacio import ComparacionCorredoresPalacio, ErrorAnalisis
from comparar_objetos import palabra


class ComparacionUtilesPalacio(ComparacionCorredoresPalacio):
    # 5FC4/6062: conserva altura, capacidad, modelos y condición original de oferta147.
    def variantes_recogida(self, base, id):
        for ocupadas in (0, 9, 10):
            for modelo in (20, 255):
                for icono in (0, 31, 34):
                    for puntos in (0, 65535):
                        cambios = [
                            (0x358C, modelo, True),
                            (0x524E + id * 2, icono, True),
                            (0x35C7, puntos, False),
                        ]
                        cambios += [
                            (0x5225 + i * 2, 76 if i < ocupadas else 65535, False)
                            for i in range(10)
                        ]
                        self.variante(base, f'recoger{id}', 0x563A, id, cambios=cambios)
        requeridos = (153, 154, 152, 149, 150, 144, 145)
        for mascara in range(128):
            objetos = [objeto for i, objeto in enumerate(requeridos) if mascara & (1 << i)] + [141]
            cambios = [(0x4BAE, 3, True)]
            cambios += [
                (0x5225 + i * 2, objetos[i] if i < len(objetos) else 65535, False)
                for i in range(10)
            ]
            self.variante(base, f'oferta147_recoger{id}', 0x563A, id, cambios=cambios)
        self.retomar_recursos(base)

    # Repite recogida y lanzamiento con el mismo azar, hasta que la tirada abra la puerta buscada.
    def abrir_puerta(self, d, escena, bandera):
        tiradas = 0
        while d[bandera] != 1:
            if tiradas >= 32:
                raise ErrorAnalisis(f'No se abre la puerta de{escena} tras32 tiradas reales.')
            self.ruta_sin_salida(d, 259, 165)
            self.elegir_objeto(d, (1,), 140)
            self.esperar(d, 192)
            self.exigir_objeto(d, 140)
            self.elegir_objeto(d, (2, 5), 140)
            self.esperar(d, 512)
            tiradas += 1
            print(f'Tirada{tiradas} hacia{escena}: puerta={d[bandera]}.', flush=True)
        self.tiradas[str(escena)] = tiradas

    # Continúa la partida de los corredores sin inventario ni banderas añadidos.
    def ejecutar(self):
        origen = (
            self.raiz / 'apps/juego/trabajo/comparacion_corredores_palacio/hielo_fin_corredores'
        )
        d = bytearray(origen.with_suffix('.ds.bin').read_bytes())
        self.semilla_menu[:] = json.loads(
            origen.with_suffix('.semilla.json').read_text(encoding='utf-8')
        )
        self.destino = self.raiz / 'apps/juego/trabajo/comparacion_utiles_palacio'
        self.destino.mkdir(parents=True, exist_ok=True)
        self.resumen = {}
        self.variantes = {}
        self.tiradas = {}
        self.retomar_recursos(d)
        self.ruta(d, ((160, 199),), 89)
        self.ruta(d, ((160, 199),), 87)
        self.ruta(d, ((160, 199),), 85)
        for escena, bandera, entrada, vuelta, x, id, nombre in (
            (50, 0x4B2C, 91, 92, 116, 149, 'gato'),
            (51, 0x4B35, 93, 94, 242, 150, 'lupa'),
        ):
            self.abrir_puerta(d, escena, bandera)
            self.hito_borde(d, nombre + '_tirada')
            self.auditar(d, nombre + ': dados antes de entrar')
            self.ruta(d, ((160, 145),), 84)
            self.hito_borde(d, nombre + '_puerta')
            puntos = (
                ((80, 182), (49, 182), (70, 190), (50, 190), (40, 182))
                if escena == 50
                else ((220, 175), (242, 172))
            )
            self.ruta(d, puntos, entrada)
            self.hito_borde(d, nombre + '_entrada')
            self.auditar(d, nombre + ': entrada')
            self.variantes_recogida(d, id)
            self.ruta_sin_salida(d, x, 180)
            if escena == 51:
                self.ruta_sin_salida(d, x, 165)
            self.hito_borde(d, nombre + '_ante_objeto')
            self.auditar(d, nombre + ': proximidad')
            self.elegir_objeto(d, (1,), id)
            self.esperar(d, 192)
            self.exigir_objeto(d, id)
            self.hito_borde(d, nombre + '_recogido')
            self.auditar(d, nombre + ': recogido')
            self.ruta(d, ((280 if escena == 50 else 41, 165),), vuelta)
            self.hito_borde(d, nombre + '_regreso')
            self.exigir_objeto(d, id)
            self.auditar(d, nombre + ': regreso al corredor')
            self.ruta(d, ((160, 199),), 85)
        if palabra(d, 0x8082) != 46:
            raise ErrorAnalisis('El recorrido no ha regresado a los dados.')
        self.hito_borde(d, 'utiles_dados')
        informe = dict(
            resultado='coinciden',
            casos_y_pasos=sum(self.resumen.values()),
            grupos=self.resumen,
            capturas=self.capturas,
            estados_auditados=self.estados_auditados,
            variantes=self.variantes,
            tiradas_reales=self.tiradas,
            alcance='Dos habitaciones laterales, gato149 y lupa150, puertas aleatorias sin forzar, regresos e inventario. Resto de útiles y solución final del palacio pendientes.',
        )
        (self.destino / 'resultado.json').write_text(
            json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8'
        )
        print(
            f'Útiles del palacio: {informe["casos_y_pasos"]} casos/pasos y{self.capturas} imágenes coincidentes.'
        )


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Contrasta gato y lupa del palacio con tiradas reales.'
    )
    parser.add_argument('--biblioteca', type=Path, required=True)
    parser.add_argument('--muestra', action='store_true')
    opciones = parser.parse_args()
    try:
        ComparacionUtilesPalacio(
            Path(__file__).resolve().parents[4], opciones.biblioteca, opciones.muestra
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        parser.exit(1, f'Error: {error}\n')
