"""Contrasta primeros puzles de EDGE y variantes aisladas de sus manejadores."""

import argparse
import json
from pathlib import Path

from comparar_recorrido_borde import ComparacionRecorridoBorde, ErrorAnalisis
from comparar_objetos import guardar, palabra


class ComparacionAccionesBorde(ComparacionRecorridoBorde):
    # Ejecuta cada selección con el menú original y clasifica los contratos comprobados.
    def variante(self, base, grupo, manejador, id, verbo=0, cambios=()):
        d = bytearray(base)
        for posicion, valor, byte in cambios:
            if byte:
                d[posicion] = valor
            else:
                guardar(d, posicion, valor)
        guardar(d, 0xB60C, manejador)
        guardar(d, 0xB620, id + 26)
        guardar(d, 0xB634, verbo)
        guardar(d, 0xB638, 65535)
        self.seleccionar(d, 0)
        self.variantes[grupo] = self.variantes.get(grupo, 0) + 1

    # Contrasta capacidad, selección del modelo y puntos envolventes de las tres recogidas.
    def variantes_recogidas(self, base):
        for id in (86, 87, 95):
            for ocupadas in (0, 9, 10):
                for modelo in (20, 255):
                    for icono in (0, 0x1F, 0x22):
                        for puntos in (0, 65535):
                            cambios = [
                                (0x358C, modelo, True),
                                (0x524E + id * 2, icono, True),
                                (0x35C7, puntos, False),
                            ]
                            cambios += [
                                (0x5225 + i * 2, 3 if i < ocupadas else 65535, False)
                                for i in range(10)
                            ]
                            self.variante(base, 'recogidas', 0x563A, id, cambios=cambios)

    # Comprueba las distancias exactas y las diferencias entre usar guantes y llevarlos.
    def variantes_ropa(self, base):
        for distancia in (-32768, -41, -31, -30, -1, 0, 30, 31, 40, 41, 32767):
            for camara in (0, 640):
                for puntos in (0, 65535):
                    for manejador, verbo in ((0x2FCA, 0), (0x3B53, 4)):
                        cambios = [
                            (0x2C4E, palabra(base, 0x2C2D) + distancia, False),
                            (0x3591, camara, False),
                            (0x35C7, puntos, False),
                            (0x5225, 86, False),
                        ]
                        self.variante(base, 'guantes', manejador, 86, verbo, cambios)
                    cambios = [
                        (0x2DFB, palabra(base, 0x2C2D) + distancia, False),
                        (0x3591, camara, False),
                        (0x35C7, puntos, False),
                        (0x5225, 87, False),
                    ]
                    self.variante(base, 'caramelo', 0x3B53, 87, 4, cambios)

    # Los cuatro botones deben conservar todos los bits ajenos y distinguir su fotograma actual.
    def variantes_botones(self, base):
        for id in range(88, 92):
            entidad = 0x2D33 + (id - 88) * 33
            for bandera in range(256):
                for imagen in (20, 21):
                    self.variante(
                        base,
                        'botones',
                        0x3B53,
                        id,
                        2,
                        ((0x35C9, bandera, True), (entidad + 1, imagen, True)),
                    )
        for altura in (75, 76, 77):
            for camara in (0, 256):
                for puntos in (0, 65535):
                    cambios = (
                        (0x2CB3, altura, False),
                        (0x3591, camara, False),
                        (0x35C7, puntos, False),
                    )
                    self.variante(base, 'roca_empujar', 0x3B53, 92, 2, cambios)
                    self.variante(base, 'roca_saltar', 0x3143, 92, 0, cambios)

    # Continúa desde la aproximación real y conserva por separado las variantes de capítulos siguientes.
    def ejecutar(self):
        origen = self.raiz / 'trabajo/comparacion_recorrido_borde/borde_roca'
        if not origen.with_suffix('.ds.bin').exists():
            ComparacionRecorridoBorde(self.raiz, self.biblioteca, True).ejecutar()
        d = bytearray(origen.with_suffix('.ds.bin').read_bytes())
        self.semilla_menu[:] = json.loads(origen.with_suffix('.semilla.json').read_text())
        self.resumen = {}
        self.variantes = {}
        self.destino = self.raiz / 'trabajo/comparacion_acciones_borde'
        self.destino.mkdir(parents=True, exist_ok=True)
        self.retomar_recursos(d)
        self.variantes_recogidas(d)
        self.variantes_ropa(d)
        self.variantes_botones(d)
        self.retomar_recursos(d)
        self.auditar(d, 'primera roca antes de empujar')
        self.elegir_objeto(d, (2, 2), 92)
        if self.andar(d, self.semilla_menu, 65535, 192) is not None:
            raise ErrorAnalisis('Empujar la primera roca abandona EDGE.')
        self.hito_borde(d, 'borde_roca_empujada')
        if palabra(d, 0x2CB3) == 76:
            raise ErrorAnalisis('La animación no desplaza la roca.')
        self.auditar(d, 'primera roca después de empujar')
        self.elegir_objeto(d, (6,), 92)
        if self.andar(d, self.semilla_menu, 65535, 192) is not None:
            raise ErrorAnalisis('El salto de la primera roca abandona EDGE.')
        self.hito_borde(d, 'borde_roca_saltada')
        informe = {
            'resultado': 'coinciden',
            'casos_y_pasos': sum(self.resumen.values()),
            'grupos': self.resumen,
            'variantes_aisladas': self.variantes,
            'capturas': self.capturas,
            'estados_auditados': self.estados_auditados,
            'alcance': 'Variantes aisladas de recogidas, guantes, caramelo, botones y roca. Recorrido real de empujar y saltar la primera roca; el resto de EDGE continúa pendiente.',
        }
        (self.destino / 'resultado.json').write_text(
            json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8'
        )
        print(
            f'Primeras acciones de EDGE: {informe["casos_y_pasos"]} casos/pasos y {self.capturas} composiciones coincidentes.'
        )


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Contrasta los primeros contratos de acciones de EDGE.'
    )
    parser.add_argument('--biblioteca', type=Path, required=True)
    parser.add_argument('--muestra', action='store_true')
    opciones = parser.parse_args()
    try:
        ComparacionAccionesBorde(
            Path(__file__).resolve().parents[1], opciones.biblioteca, opciones.muestra
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        parser.exit(1, f'Error: {error}\n')
