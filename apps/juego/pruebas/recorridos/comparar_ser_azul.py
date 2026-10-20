"""Contrasta recogida134, lanzamiento y transformación persistente de SNOWAST4."""

import argparse
import json
from pathlib import Path

from comparar_regreso_benn import ComparacionRegresoBenn, ErrorAnalisis
from comparar_objetos import palabra


class ComparacionSerAzul(ComparacionRegresoBenn):
    # B432: rodeos firmados, presencia/ausencia, rescate6323 y campos diferidos centinela.
    def variantes_bola(self, base):
        self.variantes = {}
        for distancia in (-32768, -1, 0, 1, 32767):
            for altura in (-1, 0, 1):
                for objetos in ((), (134,), (131, 134)):
                    for liberado in (False, True):
                        cambios = [
                            (0x2C2D, palabra(base, 0x2C4E) + distancia, False),
                            (0x2C2F, palabra(base, 0x2C50) + altura, False),
                            (0x490F, 0 if liberado else 1, True),
                            (0x4948, 32 if liberado else 19, True),
                            (0x35C7, 65535, False),
                        ]
                        cambios += [
                            (0x5225 + i * 2, objetos[i] if i < len(objetos) else 65535, False)
                            for i in range(10)
                        ]
                        cambios += [
                            (p, 0xCAFE, False) for p in (0xB8, 0xBA, 0xBC, 0xBE, 0x1A36, 0x1A3C)
                        ]
                        # Los nodos centinela no pueden formar un ciclo de planificación artificial.
                        cambios = [c for c in cambios if c[0] not in (0x1A36, 0x1A3C)]
                        self.variante(base, 'bola_aislada', 0x2FCA, 134, cambios=cambios)
        self.retomar_recursos(base)

    # Retoma exclusivamente el vestido y la solicitud74 verificados; conserva inventario y azar.
    def ejecutar(self):
        origen = self.raiz / 'apps/juego/trabajo/comparacion_regreso_benn/probador_solicitud74'
        if not origen.with_suffix('.ds.bin').exists():
            raise ErrorAnalisis('Falta la partida real: ejecuta comparar:regreso-benn primero.')
        d = bytearray(origen.with_suffix('.ds.bin').read_bytes())
        self.semilla_menu[:] = json.loads(
            origen.with_suffix('.semilla.json').read_text(encoding='utf-8')
        )
        self.destino = self.raiz / 'apps/juego/trabajo/comparacion_ser_azul'
        self.destino.mkdir(parents=True, exist_ok=True)
        self.resumen = {}
        self.retomar_recursos(d)
        self.cambiar(d, self.semilla_menu, 74)
        self.hito_borde(d, 'nieve_entrada74')
        self.esperar(d, 256)
        self.hito_borde(d, 'nieve_control')
        self.auditar(d, 'entrada de SNOWAST4')
        self.ruta_sin_salida(d, 564, 154)
        self.hito_borde(d, 'nieve_ante_bola')
        self.auditar(d, 'ante bola134')
        self.variantes_bola(d)
        self.variantes_objetos(d, (134,))
        self.elegir_objeto(d, (1,), 134)
        self.esperar(d, 192)
        self.exigir_objeto(d, 134)
        self.hito_borde(d, 'nieve_bola_recogida')
        self.auditar(d, 'bola en inventario')
        self.elegir_objeto(d, (2, 5), 134)
        self.esperar(d, 512)
        self.exigir_objeto(d, 134, False)
        self.hito_borde(d, 'nieve_ser_liberado')
        self.auditar(d, 'ser azul despues de la bola')
        informe = {
            'resultado': 'coinciden',
            'casos_y_pasos': sum(self.resumen.values()),
            'grupos': self.resumen,
            'capturas': self.capturas,
            'variantes': self.variantes,
            'estados_auditados': self.estados_auditados,
            'alcance': 'Entrada74, recogida y lanzamiento real134, animacion persistente y menus. Rutas ySDL pendientes.',
        }
        (self.destino / 'resultado.json').write_text(
            json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8'
        )
        print(
            f'Ser azul: {informe["casos_y_pasos"]} casos/pasos y {self.capturas} composiciones coincidentes.'
        )


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Contrasta el puzle del ser azul y la bola de nieve.'
    )
    parser.add_argument('--biblioteca', type=Path, required=True)
    parser.add_argument('--muestra', action='store_true')
    opciones = parser.parse_args()
    try:
        ComparacionSerAzul(
            Path(__file__).resolve().parents[4], opciones.biblioteca, opciones.muestra
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        parser.exit(1, f'Error: {error}\n')
