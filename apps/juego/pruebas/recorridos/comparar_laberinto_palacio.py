"""Contrasta las salas59–62, sus desvíos originales y la caja de cerillas."""

import argparse
import json
from pathlib import Path
from comparar_mano_palacio import ComparacionManoPalacio, ErrorAnalisis
from comparar_objetos import palabra
from comparar_calles import ComparacionCalles


class ComparacionLaberintoPalacio(ComparacionManoPalacio):
    # Recorre ambas puertas108 con clics reales hasta61/62 y el regreso110 a58, conservando azar e inventario.
    def comprobar_puertas_60(self, base):
        semilla = list(self.semilla_menu)
        inventario = bytes(base[0x5225:0x5239])
        for x in (115, 204):
            d = bytearray(base)
            self.semilla_menu[:] = semilla
            self.retomar_recursos(d)
            salida = ComparacionCalles.ir(self, d, self.semilla_menu, x, 140)
            if salida not in (108, 109):
                raise ErrorAnalisis(f'Puerta60 en{x}: salida inesperada{salida}.')
            self.cambiar(d, self.semilla_menu, salida)
            self.esperar(d, 16)
            escena = palabra(d, 0x8082)
            if escena not in (61, 62) or bytes(d[0x5225:0x5239]) != inventario:
                raise ErrorAnalisis('El desvío60 cambia el inventario o entrega otra escena.')
            self.hito_borde(d, f'laberinto_alternativa_{x}_destino')
            regreso = ComparacionCalles.ir(
                self, d, self.semilla_menu, 160 if escena == 61 else 204, 140
            )
            if regreso != 110:
                raise ErrorAnalisis(f'El desvío de la puerta{x} no permite regresar110.')
            self.cambiar(d, self.semilla_menu, regreso)
            self.esperar(d, 16)
            if palabra(d, 0x8082) != 58 or bytes(d[0x5225:0x5239]) != inventario:
                raise ErrorAnalisis('El regreso del laberinto altera inventario o destino58.')
            self.hito_borde(d, f'laberinto_alternativa_{x}_regreso')
            self.variantes['puertas60_con_regreso'] = (
                self.variantes.get('puertas60_con_regreso', 0) + 1
            )
        self.semilla_menu[:] = semilla
        self.retomar_recursos(base)

    # Recorre puertas reales con su azar original; no elige destinos cambiando la semilla.
    def ejecutar(self):
        origen = self.raiz / 'apps/juego/trabajo/comparacion_mano_palacio/mano_solicitud105'
        d = bytearray(origen.with_suffix('.ds.bin').read_bytes())
        self.semilla_menu[:] = json.loads(
            origen.with_suffix('.semilla.json').read_text(encoding='utf-8')
        )
        self.destino = self.raiz / 'apps/juego/trabajo/comparacion_laberinto_palacio'
        self.destino.mkdir(parents=True, exist_ok=True)
        self.resumen = {}
        self.variantes = {}
        self.retomar_recursos(d)
        self.cambiar(d, self.semilla_menu, 105)
        self.hito_borde(d, 'laberinto_59_entrada')
        self.esperar(d, 16)
        self.hito_borde(d, 'laberinto_59_control')
        self.auditar(d, 'sala59: entrada')
        vistos = {59}
        recorrido = []
        con_cerillas = False
        for intento in range(64):
            escena = palabra(d, 0x8082)
            if escena == 62 and not con_cerillas:
                self.variantes_recogida(d, 161)
                self.ruta_sin_salida(
                    d, palabra(d, 0x2C4E) + 5, palabra(d, 0x2C50) + palabra(d, 0x2C54) - 2
                )
                self.hito_borde(d, 'laberinto_62_ante_cerillas')
                self.auditar(d, 'ante caja de cerillas')
                self.elegir_objeto(d, (1,), 161)
                self.esperar(d, 192)
                self.exigir_objeto(d, 161)
                self.exigir_objeto(d, 162)
                con_cerillas = True
                self.hito_borde(d, 'laberinto_62_cerillas')
                self.auditar(d, 'caja y cerilla recogidas')
            if escena == 59 and con_cerillas:
                self.hito_borde(d, 'laberinto_59_con_cerillas')
                self.auditar(d, 'sala59 con cerillas')
            x = {58: 160, 59: 115, 60: 115, 61: 160, 62: 204}[escena]
            self.clic(d, x, 140)
            for paso in range(192):
                salida = self.andar(d, self.semilla_menu, 65535, 1)
                if salida is not None:
                    break
            else:
                raise ErrorAnalisis(f'La puerta no entrega salida en sala{escena}.')
            recorrido.append([escena, salida])
            print('paso', recorrido[-1], flush=True)
            if salida == 107:
                if not con_cerillas or not vistos.issuperset({59, 60, 61, 62}):
                    raise ErrorAnalisis(
                        'La ruta llega al guardia sin haber contrastado los cuatro desvíos.'
                    )
                (self.destino / 'laberinto_solicitud107.ds.bin').write_bytes(d)
                (self.destino / 'laberinto_solicitud107.semilla.json').write_text(
                    json.dumps(list(self.semilla_menu)), encoding='utf-8'
                )
                break
            self.cambiar(d, self.semilla_menu, salida)
            escena = palabra(d, 0x8082)
            if escena not in vistos:
                self.hito_borde(d, 'laberinto_' + str(escena) + '_entrada')
            self.esperar(d, 16)
            if escena not in vistos:
                self.hito_borde(d, 'laberinto_' + str(escena) + '_control')
                self.auditar(d, 'laberinto sala' + str(escena))
                if escena == 60:
                    self.comprobar_puertas_60(d)
                vistos.add(escena)
        else:
            raise ErrorAnalisis('El laberinto no entrega107 en64 puertas.')
        informe = dict(
            resultado='coinciden',
            casos_y_pasos=sum(self.resumen.values()),
            grupos=self.resumen,
            variantes=self.variantes,
            capturas=self.capturas,
            estados_auditados=self.estados_auditados,
            recorrido=recorrido,
            limite='Cuatro salas, ambas puertas60 hasta61/62 y regreso110 a58 contrastados; no agota todos los estados ni semillas.',
        )
        (self.destino / 'resultado.json').write_text(
            json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8'
        )
        print(
            'Laberinto:',
            informe['casos_y_pasos'],
            'casos/pasos;',
            self.capturas,
            'imágenes.',
            flush=True,
        )


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--biblioteca', type=Path, required=True)
    parser.add_argument('--muestra', action='store_true')
    args = parser.parse_args()
    try:
        ComparacionLaberintoPalacio(
            Path(__file__).resolve().parents[4], args.biblioteca, args.muestra
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        parser.exit(1, f'Error: {error}\n')
