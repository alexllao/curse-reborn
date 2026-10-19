"""Contrasta los tres corredores, sus regresos, poleas y ventana sin fabricar útiles posteriores."""

import argparse
import json
from pathlib import Path

from comparar_dados_palacio import ComparacionDadosPalacio, ErrorAnalisis


class ComparacionCorredoresPalacio(ComparacionDadosPalacio):
    # B93D/B968 y6285: incluye ambos órdenes de inserción y todas las máscaras de los siete útiles.
    def variantes_poleas(self, base):
        self.variantes = {}
        for imagen in (8, 9, 10):
            for actividad in (0, 1, 3):
                self.variante(
                    base,
                    'examinar143',
                    0x384A,
                    143,
                    cambios=((0x2C4D, imagen, True), (0x2C4C, actividad, True)),
                )
        for id, previo in ((142, 151), (151, 142)):
            for presente in (False, True):
                for puntos in (0, 65535):
                    cambios = [(0x35C7, puntos, False)]
                    cambios += [
                        (0x5225 + i * 2, 151 if presente and i == 0 else 65535, False)
                        for i in range(10)
                    ]
                    cambios += [(p, 0xCAFE, False) for p in (0xA2, 0xA4, 0xAA, 0xAC)]
                    self.contrato(base, id, previo, 1, cambios)
        requeridos = (153, 154, 152, 149, 150, 144, 145)
        for id in (144, 145):
            for mascara in range(128):
                objetos = [id for i, id in enumerate(requeridos) if mascara & (1 << i)] + [141]
                cambios = [(0x4BAE, 3, True)]
                cambios += [
                    (0x5225 + i * 2, objetos[i] if i < len(objetos) else 65535, False)
                    for i in range(10)
                ]
                self.variante(base, f'oferta147_recoger{id}', 0x563A, id, cambios=cambios)
        self.retomar_recursos(base)

    # Las ramas abiertas de la ventana se comparan aisladamente; no se añaden148/152 a la partida.
    def variantes_ventana(self, base):
        for actividad in (0, 1, 3):
            for presente in (False, True):
                for puntos in (0, 65535):
                    cambios = [(0x2C4C, actividad, True), (0x35C7, puntos, False)]
                    cambios += [
                        (0x5225 + i * 2, 148 if presente and i == 0 else 65535, False)
                        for i in range(10)
                    ]
                    self.variante(base, 'atacar148', 0x2FCA, 148, cambios=cambios)
                    self.variante(base, 'saltar146', 0x3143, 146, cambios=cambios)
        for objetos in ((), (147,), (152,), (147, 152), (147, 152, 148)):
            cambios = [
                (0x5225 + i * 2, objetos[i] if i < len(objetos) else 65535, False)
                for i in range(10)
            ]
            self.contrato(base, 147, 152, 7, cambios)
            self.contrato(base, 152, 147, 7, cambios)
        for capacidad in (0, 9, 10):
            for icono in (0, 31, 34):
                for puntos in (0, 65535):
                    cambios = [(0x524E + 147 * 2, icono, True), (0x35C7, puntos, False)]
                    cambios += [
                        (0x5225 + i * 2, 76 if i < capacidad else 65535, False) for i in range(10)
                    ]
                    self.variante(base, 'recoger147_aislado', 0x563A, 147, cambios=cambios)
        self.retomar_recursos(base)

    # Recorre84/85/86/87/88/89 y las opciones ofrecidas por la partida real posterior a los dados.
    def ejecutar(self):
        origen = self.raiz / 'apps/juego/trabajo/comparacion_dados_palacio/dados_solicitud84'
        d = bytearray(origen.with_suffix('.ds.bin').read_bytes())
        self.semilla_menu[:] = json.loads(
            origen.with_suffix('.semilla.json').read_text(encoding='utf-8')
        )
        self.destino = self.raiz / 'apps/juego/trabajo/comparacion_corredores_palacio'
        self.destino.mkdir(parents=True, exist_ok=True)
        self.resumen = {}
        self.retomar_recursos(d)
        self.cambiar(d, self.semilla_menu, 84)
        self.esperar(d, 128)
        self.hito_borde(d, 'poleas_entrada')
        self.auditar(d, 'primer corredor')
        self.variantes_poleas(d)
        self.ruta_sin_salida(d, 231, 182)
        self.hito_borde(d, 'poleas_ante_inspeccion')
        self.auditar(d, 'ante soporte143')
        self.elegir_objeto(d, (3,), 143)
        self.esperar(d, 256)
        self.hito_borde(d, 'poleas_inspeccion')
        self.auditar(d, 'soporte inspeccionado')
        self.ruta(d, ((160, 199),), 85)
        self.hito_borde(d, 'poleas_regreso_dados')
        self.auditar(d, 'regreso a dados')
        self.ruta(d, ((160, 145),), 84)
        self.hito_borde(d, 'poleas_regreso')
        self.ruta(d, ((160, 166),), 86)
        self.hito_borde(d, 'hielo_pasillo2')
        self.auditar(d, 'segundo corredor')
        self.ruta(d, ((160, 199),), 87)
        self.hito_borde(d, 'hielo_vuelta1')
        self.ruta(d, ((160, 166),), 86)
        self.ruta(d, ((160, 166),), 88)
        self.hito_borde(d, 'hielo_pasillo3')
        self.auditar(d, 'tercer corredor')
        self.variantes_ventana(d)
        self.ruta_sin_salida(d, 159, 164)
        self.hito_borde(d, 'hielo_ante_ventana')
        self.auditar(d, 'ante ventana146')
        self.elegir_objeto(d, (3,), 146)
        self.esperar(d, 256)
        self.hito_borde(d, 'hielo_ventana')
        self.auditar(d, 'ventana inspeccionada')
        self.elegir_objeto(d, (6,), 146)
        self.esperar(d, 64)
        if not d[0x2C4C] or d[0x4BAE] != 3:
            raise ErrorAnalisis('La partida ha abierto la ventana o el silbato sin sus útiles.')
        self.hito_borde(d, 'hielo_ventana_cerrada')
        self.auditar(d, 'salto rechazado con ventana cerrada')
        self.ruta(d, ((160, 199),), 89)
        self.hito_borde(d, 'hielo_vuelta2')
        self.auditar(d, 'regreso al segundo corredor')
        self.ruta(d, ((160, 166),), 88)
        self.hito_borde(d, 'hielo_fin_corredores')
        informe = {
            'resultado': 'coinciden',
            'casos_y_pasos': sum(self.resumen.values()),
            'grupos': self.resumen,
            'capturas': self.capturas,
            'estados_auditados': self.estados_auditados,
            'variantes': self.variantes,
            'alcance': 'Tres corredores47/48/49, seis enlaces y regresos, inspecciones, rechazo de salto y oferta147 cerrada. Arma151, útiles laterales y solución real de ventana siguen pendientes; sus manejadores se contrastan como variantes.',
        }
        (self.destino / 'resultado.json').write_text(
            json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8'
        )
        print(
            f'Corredores del palacio: {informe["casos_y_pasos"]} casos/pasos y {self.capturas} composiciones coincidentes.'
        )


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Contrasta los tres primeros corredores del palacio.'
    )
    parser.add_argument('--biblioteca', type=Path, required=True)
    parser.add_argument('--muestra', action='store_true')
    opciones = parser.parse_args()
    try:
        ComparacionCorredoresPalacio(
            Path(__file__).resolve().parents[3], opciones.biblioteca, opciones.muestra
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        parser.exit(1, f'Error: {error}\n')
