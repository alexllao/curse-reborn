"""Contrasta el mando y una grabación persistente desde el estornudo real de DINO."""

import argparse
import json
from pathlib import Path

from comparar_grabacion_valle import ComparacionGrabacionValle, ErrorAnalisis
from comparar_dinosaurio import ComparacionDinosaurio
from comparar_objetos import palabra


class ComparacionGrabacionCompleta(ComparacionGrabacionValle):
    # Sigue salidas59/56/61 sin conceder objetos y graba106 con el mando realmente recogido.
    def ejecutar(self):
        origen = self.raiz / 'trabajo/comparacion_dinosaurio/dinosaurio_sello_boligrafo'
        if not origen.with_suffix('.ds.bin').exists():
            ComparacionDinosaurio(self.raiz, self.biblioteca, True).ejecutar()
        d = bytearray(origen.with_suffix('.ds.bin').read_bytes())
        self.semilla_menu[:] = json.loads(
            origen.with_suffix('.semilla.json').read_text(encoding='utf-8')
        )
        self.destino = self.raiz / 'trabajo/comparacion_grabacion_completa'
        self.destino.mkdir(parents=True, exist_ok=True)
        self.resumen = {}
        self.retomar_recursos(d)
        self.ruta(d, ((400, 185), (550, 189)), 59)
        self.esperar(d, 128)
        if d[0x2C4C]:
            raise ErrorAnalisis('El protector de REMOTE reaparece después del estornudo.')
        self.hito_borde(d, 'mando_entrada59')
        self.ruta_sin_salida(d, 110, 165)
        self.elegir_objeto(d, (1,), 108)
        self.esperar(d, 128)
        if 108 not in [palabra(d, 0x5225 + i * 2) for i in range(10)]:
            raise ErrorAnalisis('El mando108 no procede de la recogida real.')
        self.hito_borde(d, 'mando_recogido')
        self.auditar(d, 'REMOTE después de recoger108')
        self.ruta(d, ((170, 122),), 56)
        for punto in ((330, 170), (90, 170), (74, 145)):
            self.ruta_sin_salida(d, *punto)
        self.elegir_objeto(d, (1,), 105)
        self.esperar(d, 128)
        if 105 not in [palabra(d, 0x5225 + i * 2) for i in range(10)]:
            raise ErrorAnalisis('La cinta105 no se recoge con el mando en esta misma partida.')
        self.hito_borde(d, 'cinta_y_mando')
        self.auditar(d, 'CELESTE con cinta y mando')
        self.ruta(d, ((100, 198),), 61)
        for punto in ((550, 140), (620, 145), (704, 145), (704, 185), (760, 185), (760, 171)):
            self.ruta_sin_salida(d, *punto)
        self.elegir_objeto(d, (2, 1), 105)
        self.elegir_objeto(d, (), 113)
        self.esperar(d, 256)
        if d[0x2C6D] != 1 or 105 in [palabra(d, 0x5225 + i * 2) for i in range(10)]:
            raise ErrorAnalisis('El equipo no conserva la cinta insertada original.')
        self.hito_borde(d, 'cinta_insertada')
        self.auditar(d, 'POST antes de grabar')
        self.menu(d, (2, 2))
        self.seleccionar(d, 0)
        self.elegir_objeto(d, (), 108)
        self.esperar(d, 768)
        inventario = [palabra(d, 0x5225 + i * 2) for i in range(10)]
        if 106 not in inventario or 108 in inventario or 105 in inventario:
            raise ErrorAnalisis('La grabación no sustituye el mando y la cinta por106.')
        self.hito_borde(d, 'grabacion_terminada')
        self.auditar(d, 'POST con grabación106 real')
        # Une el correo con esta misma grabación real; no recupera el sello de otra instantánea.
        for punto in ((704, 185), (704, 145), (620, 145), (550, 140), (484, 104)):
            self.ruta_sin_salida(d, *punto)
        self.elegir_objeto(d, (1,), 111)
        self.esperar(d, 128)
        self.hito_borde(d, 'grabacion_y_carta')
        self.auditar(d, 'carta y grabación en la misma partida')
        self.elegir_objeto(d, (2, 7), 109)
        self.seleccionar(d, 0)
        self.elegir_objeto(d, (), 111)
        self.esperar(d, 128)
        self.hito_borde(d, 'grabacion_y_carta_franqueada')
        for punto in ((450, 175), (435, 175)):
            self.ruta_sin_salida(d, *punto)
        self.elegir_objeto(d, (2, 1), 112)
        self.elegir_objeto(d, (), 114)
        self.esperar(d, 256)
        inventario = [palabra(d, 0x5225 + i * 2) for i in range(10)]
        if 106 not in inventario or any(id in inventario for id in (108, 105, 109, 111, 112)):
            raise ErrorAnalisis(
                'El correo no conserva la grabación y los consumos de esta misma partida.'
            )
        self.hito_borde(d, 'grabacion_y_correo_enviado')
        self.auditar(d, 'correo enviado con grabación real')
        informe = {
            'resultado': 'coinciden',
            'casos_y_pasos': sum(self.resumen.values()),
            'grupos': self.resumen,
            'capturas': self.capturas,
            'estados_auditados': self.estados_auditados,
            'alcance': 'Recorrido persistente desde DINO: salida59, mando108 libre trasA81A, cinta105, regreso61, inserciónAB60 y grabaciónABD5 hasta106, carta111 y correoAA6C/AAA3 en la misma partida. Entrega a98, demás puzles y SDL pendientes.',
        }
        (self.destino / 'resultado.json').write_text(
            json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8'
        )
        print(
            f'Grabación persistente: {informe["casos_y_pasos"]} casos/pasos y {self.capturas} composiciones coincidentes.'
        )


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Contrasta el recorrido real del mando y la grabación.'
    )
    parser.add_argument('--biblioteca', type=Path, required=True)
    parser.add_argument('--muestra', action='store_true')
    opciones = parser.parse_args()
    try:
        ComparacionGrabacionCompleta(
            Path(__file__).resolve().parents[1], opciones.biblioteca, opciones.muestra
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        parser.exit(1, f'Error: {error}\n')
