"""Contrasta la resolución real de poleas, silbato, megáfono y salto de la ventana."""

import argparse
import json
from pathlib import Path

from comparar_laterales_palacio import ComparacionLateralesPalacio, ErrorAnalisis
from comparar_objetos import palabra


class ComparacionResolucionPalacio(ComparacionLateralesPalacio):
    # Usa los útiles obtenidos en los seis laterales, sin simular inventario ni abrir la ventana.
    def ejecutar(self):
        origen = self.raiz / 'apps/juego/trabajo/comparacion_laterales_palacio/laterales_utiles'
        d = bytearray(origen.with_suffix('.ds.bin').read_bytes())
        self.semilla_menu[:] = json.loads(
            origen.with_suffix('.semilla.json').read_text(encoding='utf-8')
        )
        self.destino = self.raiz / 'apps/juego/trabajo/comparacion_resolucion_palacio'
        self.destino.mkdir(parents=True, exist_ok=True)
        self.resumen = {}
        self.retomar_recursos(d)
        for id in (141, 149, 150, 151, 152, 153, 154):
            self.exigir_objeto(d, id)
        self.ruta(d, ((160, 145),), 84)
        self.ruta_sin_salida(d, 89, 180)
        self.hito_borde(d, 'poleas_con_arma')
        self.auditar(d, 'ante funda142 con arma151')
        self.elegir_objeto(d, (2, 1), 151)
        self.elegir_objeto(d, (), 142)
        self.esperar(d, 512)
        self.hito_borde(d, 'poleas_bajadas')
        self.auditar(d, 'poleas bajadas')
        if 151 in [palabra(d, 0x5225 + i * 2) for i in range(10)]:
            raise ErrorAnalisis('El arma no se consumió en la funda.')
        self.ruta_sin_salida(d, 238, 190)
        self.ruta_sin_salida(d, 238, 150)
        self.hito_borde(d, 'poleas_ante_utiles')
        self.auditar(d, 'ante aceite144 y cristal145')
        self.elegir_objeto(d, (1,), 144)
        self.esperar(d, 192)
        self.exigir_objeto(d, 144)
        self.hito_borde(d, 'poleas_aceite')
        self.auditar(d, 'aceite recogido')
        self.ruta_sin_salida(d, 238, 150)
        self.elegir_objeto(d, (1,), 145)
        self.esperar(d, 192)
        self.exigir_objeto(d, 145)
        self.hito_borde(d, 'poleas_cristal')
        self.auditar(d, 'cristal recogido y oferta de silbato')
        if d[0x4BAE] != 1 or 141 in [palabra(d, 0x5225 + i * 2) for i in range(10)]:
            raise ErrorAnalisis('6285 no ha ofrecido el silbato o retirado la escoba.')
        self.ruta(d, ((160, 166),), 86)
        self.ruta(d, ((160, 166),), 88)
        self.hito_borde(d, 'ventana_con_utiles')
        self.ruta_sin_salida(d, 261, 188)
        self.hito_borde(d, 'ventana_ante_silbato')
        self.auditar(d, 'silbato ofrecido')
        self.elegir_objeto(d, (1,), 147)
        self.esperar(d, 192)
        self.exigir_objeto(d, 147)
        self.hito_borde(d, 'ventana_silbato')
        self.auditar(d, 'silbato recogido')
        self.elegir_objeto(d, (2, 7), 147)
        self.seleccionar(d, 0)
        self.elegir_objeto(d, (), 152)
        self.esperar(d, 192)
        self.exigir_objeto(d, 148)
        self.hito_borde(d, 'ventana_megafono')
        self.auditar(d, 'silbato y megáfono combinados')
        self.elegir_objeto(d, (5,), 148)
        self.esperar(d, 512)
        if d[0x2C4C] != 0:
            raise ErrorAnalisis('La ventana no ha quedado abierta.')
        self.hito_borde(d, 'ventana_abierta')
        self.auditar(d, 'ventana abierta')
        self.elegir_objeto(d, (6,), 146)
        for paso in range(768):
            salida = self.andar(d, self.semilla_menu, 65535, 1)
            if salida is not None:
                if salida != 90:
                    raise ErrorAnalisis(f'La ventana solicita{salida}, esperada90.')
                (self.destino / 'ventana_solicitud90.ds.bin').write_bytes(d)
                (self.destino / 'ventana_solicitud90.semilla.json').write_text(
                    json.dumps(list(self.semilla_menu)), encoding='utf-8'
                )
                break
        else:
            raise ErrorAnalisis('El salto no ha solicitado la continuación90.')
        informe = dict(
            resultado='coinciden',
            casos_y_pasos=sum(self.resumen.values()),
            grupos=self.resumen,
            capturas=self.capturas,
            estados_auditados=self.estados_auditados,
            alcance='Poleas y todos sus útiles reales, silbato habilitado por6285, combinación, ventana rota y solicitud90 hacia56. Sala del láser56 aún por contrastar.',
        )
        (self.destino / 'resultado.json').write_text(
            json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8'
        )
        print(
            f'Resolución: {informe["casos_y_pasos"]} casos/pasos y{self.capturas} imágenes coincidentes.'
        )


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Contrasta la resolución de los laterales del palacio.'
    )
    parser.add_argument('--biblioteca', type=Path, required=True)
    parser.add_argument('--muestra', action='store_true')
    opciones = parser.parse_args()
    try:
        ComparacionResolucionPalacio(
            Path(__file__).resolve().parents[4], opciones.biblioteca, opciones.muestra
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        parser.exit(1, f'Error: {error}\n')
