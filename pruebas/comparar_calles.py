"""Contrasta las calles laterales, la tienda abandonada y la posada desde estados reales."""

import argparse
import json
from pathlib import Path

from comparar_poblacion import ComparacionPoblacion, ErrorAnalisis
from comparar_objetos import palabra


class ComparacionCalles(ComparacionPoblacion):
    # Conserva el estado de cada imagen para diagnosticar diferencias de paso o composición SDL.
    def captura(self, datos, carpeta, nombre):
        super().captura(datos, carpeta, nombre)
        (carpeta / (nombre + '.ds.bin')).write_bytes(datos)

    # Incluye los umbrales junto al borde, que no necesitan el margen central de las cuevas.
    def ir(self, d, sem, x, y):
        for _ in range(1500):
            local = x - palabra(d, 0x3591)
            if 0 <= local <= 319:
                self.clic(d, local, y)
                return self.andar(d, sem, 65535, 192)
            salida = self.andar(d, sem, 8 if local < 0 else 4, 1)
            if salida is not None:
                return salida
        raise ErrorAnalisis(f'No se alcanza el umbral {x},{y}.')

    # Mantiene las instantáneas y recursos originales; cada rama parte de su estado contrastado.
    def ejecutar(self):
        self.destino = self.raiz / 'trabajo/comparacion_calles'
        self.destino.mkdir(parents=True, exist_ok=True)
        self.resumen = {}
        for nombre, salida, puntos, regreso, vuelta in (
            ('fuente', 29, ((260, 180), (310, 100), (319, 70)), 37, ((160, 195), (160, 199))),
            ('fuente', 30, ((60, 180), (8, 100), (0, 70)), 40, ((470, 185), (479, 180))),
            ('maravillas', 33, ((275, 180), (285, 145), (284, 136)), 45, ((104, 148),)),
            ('guardias', 41, ((320, 185), (192, 175), (192, 145)), 50, ((30, 195), (20, 199))),
        ):
            origen = self.raiz / 'trabajo/comparacion_poblacion' / ('poblacion_' + nombre)
            d = bytearray(origen.with_suffix('.ds.bin').read_bytes())
            self.semilla_menu[:] = json.loads(origen.with_suffix('.semilla.json').read_text())
            sem = self.semilla_menu
            self.retomar_recursos(d)
            encontrada = None
            for x, y in puntos:
                encontrada = self.ir(d, sem, x, y)
                if encontrada is not None:
                    break
            if encontrada is None:
                encontrada = self.ir(d, sem, *puntos[-1])
            if encontrada != salida:
                raise ErrorAnalisis(
                    f'Rama {salida}: salida {encontrada}, posición {palabra(d, 0x2C2D)},{palabra(d, 0x2C2F)}.'
                )
            self.cambiar(d, sem, salida)
            self.andar(d, sem, 65535, 128)
            self.captura(d, self.destino, f'calle_{salida}')
            self.auditar(d, f'entrada {salida}')
            (self.destino / f'calle_{salida}.ds.bin').write_bytes(d)
            (self.destino / f'calle_{salida}.semilla.json').write_text(json.dumps(list(sem)))
            # Acercarse al mostrador reproduce también la reacción 541C de la posada.
            if salida == 41:
                self.clic(d, 90, 165)
                self.andar(d, sem, 65535, 8)
                self.captura(d, self.destino, 'posadero')
                self.auditar(d, 'respuesta del posadero')
                encontrada = self.andar(d, sem, 65535, 192)
            else:
                encontrada = None
                for x, y in vuelta:
                    encontrada = self.ir(d, sem, x, y)
                    if encontrada is not None:
                        break
            if encontrada != regreso:
                raise ErrorAnalisis(f'Regreso de {salida}: {encontrada}, esperado {regreso}.')
            self.cambiar(d, sem, regreso)
            self.andar(d, sem, 65535, 128)
            self.captura(d, self.destino, f'regreso_{salida}')
        informe = {
            'resultado': 'coinciden',
            'casos_y_pasos': sum(self.resumen.values()),
            'grupos': self.resumen,
            'estados_auditados': self.estados_auditados,
            'capturas': self.capturas,
        }
        (self.destino / 'resultado.json').write_text(
            json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8'
        )
        print(
            f'Calles y posada: {informe["casos_y_pasos"]} casos/pasos y {self.capturas} composiciones coincidentes.'
        )


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Compara las entradas laterales de la población y sus regresos.'
    )
    parser.add_argument('--biblioteca', type=Path, required=True)
    parser.add_argument('--muestra', action='store_true')
    opciones = parser.parse_args()
    try:
        ComparacionCalles(
            Path(__file__).resolve().parents[1], opciones.biblioteca, opciones.muestra
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        parser.exit(1, f'Error: {error}\n')
