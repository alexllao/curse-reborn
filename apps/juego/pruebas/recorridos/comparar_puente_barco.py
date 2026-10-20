"""Contrasta los dos tablones, colocación del puente, trapo119 y regreso63 en la misma partida."""

import argparse
import json
from pathlib import Path

from comparar_guardia_barco import ComparacionGuardiaBarco, ErrorAnalisis
from comparar_objetos import palabra


class ComparacionPuenteBarco(ComparacionGuardiaBarco):
    # 563A/5DDC–5E54: capacidad, exclusión mutua, duplicados e iconos con puntos envolventes.
    def variantes_recogidas_barco(self, base):
        self.variantes = {}
        for id in (117, 118, 119):
            inventarios = ((), (id,), (3,) * 10, (3,) * 9 + (id,))
            if id != 119:
                otro = 118 if id == 117 else 117
                inventarios += ((otro,), (id, otro))
            for objetos in inventarios:
                for icono in (0, 0x1F, 0x22):
                    for puntos in (0, 65535):
                        cambios = [(0x524E + id * 2, icono, True), (0x35C7, puntos, False)]
                        cambios += [
                            (0x5225 + i * 2, objetos[i] if i < len(objetos) else 65535, False)
                            for i in range(10)
                        ]
                        self.variante(base, f'recoger{id}', 0x563A, id, cambios=cambios)
        self.retomar_recursos(base)

    # AC8F–AFBB: ambos identificadores/verbos, sustitución del tablón llevado y bordes de colocación.
    def variantes_colocacion(self, base):
        posiciones = (
            (118, 95),
            (119, 95),
            (139, 95),
            (140, 95),
            (130, 89),
            (130, 90),
            (130, 100),
            (130, 101),
            (155, 95),
            (167, 95),
        )
        for id in (117, 118):
            for verbo in (2, 5):
                for objetos in ((), (117,), (118,), (117, 118)):
                    for x, y in posiciones:
                        for otro in (0, 2):
                            bandera, par = (0x5337, 0x5339) if id == 117 else (0x5339, 0x5337)
                            cambios = [
                                (0x2C2D, x, False),
                                (0x2C2F, y, False),
                                (bandera, 0xA7, True),
                                (par, 0xA4 | otro, True),
                                (0x46ED, 0xCAFE, False),
                                (0x46F6, 0xBEEF, False),
                            ]
                            cambios += [
                                (0x5225 + i * 2, objetos[i] if i < len(objetos) else 65535, False)
                                for i in range(10)
                            ]
                            self.variante(
                                base, 'colocacion', 0x3B53, id, verbo=verbo, cambios=cambios
                            )
        self.retomar_recursos(base)

    # Usa Empujar desde el inventario real, sin sustituir los canales del personaje.
    def empujar_tablon(self, d, id):
        self.menu(d, (2, 2))
        self.seleccionar(d, 0)
        self.elegir_objeto(d, (), id)
        self.esperar(d, 256)

    # Recorre HOLD desde la llegada62, monta ambos tablones y vuelve con119 por63.
    def ejecutar(self):
        origen = self.raiz / 'apps/juego/trabajo/comparacion_guardia_barco/barco_interior_entrada'
        if not origen.with_suffix('.ds.bin').exists():
            ComparacionGuardiaBarco(self.raiz, self.biblioteca, True).ejecutar()
        d = bytearray(origen.with_suffix('.ds.bin').read_bytes())
        self.semilla_menu[:] = json.loads(
            origen.with_suffix('.semilla.json').read_text(encoding='utf-8')
        )
        self.destino = self.raiz / 'apps/juego/trabajo/comparacion_puente_barco'
        self.destino.mkdir(parents=True, exist_ok=True)
        self.resumen = {}
        self.retomar_recursos(d)
        puntos = palabra(d, 0x35C7)
        for punto in ((225, 185), (160, 185), (85, 180), (70, 173)):
            self.ruta_sin_salida(d, *punto)
        self.auditar(d, 'HOLD junto a los tablones iniciales')
        self.variantes_recogidas_barco(d)
        self.elegir_objeto(d, (1,), 117)
        self.esperar(d, 192)
        if d[0x358C] != 254 or 117 not in [palabra(d, 0x5225 + i * 2) for i in range(10)]:
            raise ErrorAnalisis('El primer tablón no se lleva con el modelo originalFE.')
        self.hito_borde(d, 'barco_primer_tablon')
        self.variantes_colocacion(d)
        for punto in ((100, 180), (170, 180), (136, 166)):
            self.ruta_sin_salida(d, *punto)
        self.empujar_tablon(d, 117)
        if d[0x2C4D] != 9 or d[0x5337] & 2:
            raise ErrorAnalisis('El primer tablón no queda horizontal en su emplazamiento.')
        self.hito_borde(d, 'barco_tablon_horizontal')
        for punto in ((170, 180), (95, 180), (91, 171)):
            self.ruta_sin_salida(d, *punto)
        self.elegir_objeto(d, (1,), 118)
        self.esperar(d, 192)
        for punto in ((170, 180), (175, 166), (163, 166)):
            self.ruta_sin_salida(d, *punto)
        self.empujar_tablon(d, 118)
        if d[0x2C6E] != 10 or d[0x5339] & 2:
            raise ErrorAnalisis('El segundo tablón no completa el puente vertical.')
        self.hito_borde(d, 'barco_puente_completo')
        for punto in ((170, 165), (170, 150), (170, 120), (230, 105), (281, 102)):
            self.ruta_sin_salida(d, *punto)
        self.auditar(d, 'HOLD después de cruzar el puente')
        self.elegir_objeto(d, (1,), 119)
        self.esperar(d, 192)
        inventario = [palabra(d, 0x5225 + i * 2) for i in range(10)]
        if (
            119 not in inventario
            or 117 in inventario
            or 118 in inventario
            or d[0x2C8E]
            or palabra(d, 0x35C7) != puntos
        ):
            raise ErrorAnalisis('El trapo no se recoge conservando el puente y la puntuación.')
        self.hito_borde(d, 'barco_trapo_recogido')
        self.ruta(d, ((170, 120), (170, 175), (280, 199)), 63)
        self.hito_borde(d, 'barco_regreso_celeste')
        self.auditar(d, 'CELESTE con el trapo del barco')
        informe = {
            'resultado': 'coinciden',
            'casos_y_pasos': sum(self.resumen.values()),
            'grupos': self.resumen,
            'variantes': self.variantes,
            'capturas': self.capturas,
            'estados_auditados': self.estados_auditados,
            'alcance': 'Ambos tablones reales, puente, cruce, trapo119 y regreso63 en la misma partida del guardia. Otros puzles/rutas del valle y SDL siguen pendientes.',
        }
        (self.destino / 'resultado.json').write_text(
            json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8'
        )
        print(
            f'Puente del barco: {informe["casos_y_pasos"]} casos/pasos y {self.capturas} composiciones coincidentes.'
        )


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Contrasta los tablones y el trapo deHOLD36.')
    parser.add_argument('--biblioteca', type=Path, required=True)
    parser.add_argument('--muestra', action='store_true')
    opciones = parser.parse_args()
    try:
        ComparacionPuenteBarco(
            Path(__file__).resolve().parents[4], opciones.biblioteca, opciones.muestra
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        parser.exit(1, f'Error: {error}\n')
