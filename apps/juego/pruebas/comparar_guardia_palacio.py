"""Contrasta el guardia, la cerilla y el regreso desde el palacio del hielo."""

import argparse
import ctypes
import itertools
import json
from pathlib import Path

from comparar_laberinto_palacio import ComparacionLaberintoPalacio, ErrorAnalisis
from comparar_objetos import palabra, guardar


class ComparacionGuardiaPalacio(ComparacionLaberintoPalacio):
    # BD73/BDD8: ambos órdenes, dos verbos de encendido, ausencia de útiles y plazo insuficiente.
    def variantes_cerillas(self, base):
        numero = 0
        for cuenta, x, presente, puntos in itertools.product(
            (65516, 0, 1, 6, 7, 49, 50, 100, 32767, 32768),
            (48, 164, 230),
            (False, True),
            (0, 65535),
        ):
            cambios = [(0x35CE, cuenta, False), (0x2C2D, x, False), (0x35C7, puntos, False)]
            cambios += [
                (0x5225 + i * 2, (161 if i == 0 else 162) if presente and i < 2 else 65535, False)
                for i in range(10)
            ]
            self.contrato(base, 162, 163, 1, cambios)
            self.contrato(base, 163, 162, 1, cambios)
            self.contrato(base, 161, 65535, 2, cambios)
            self.variante(base, 'encender_caja', 0x2FCA, 161, 0, cambios)
            numero += 3
        self.variantes['cerilla_y_uso'] = numero
        for ocupadas, icono, progreso in itertools.product((0, 9, 10), (0, 31, 34), (0, 3, 4, 255)):
            cambios = [
                (0x35C9, progreso, True),
                (0x524E + 164 * 2, icono, True),
                (0x35C7, 65535, False),
            ]
            cambios += [
                (0x5225 + i * 2, 76 + i if i < ocupadas else 65535, False) for i in range(10)
            ]
            self.variante(base, 'extintor164', 0x563A, 164, 0, cambios)
        self.retomar_recursos(base)

    # D41C: contrasta límites con signo, enfriamiento, despertar y agotamiento de partículas.
    def variantes_guardia(self, base):
        rutina = self.nativa.procesar_rutina_guardia
        rutina.argtypes = [
            ctypes.POINTER(ctypes.c_ubyte),
            ctypes.POINTER(ctypes.c_ushort),
            self.tipo_dimensiones,
        ]
        rutina.restype = ctypes.c_int
        casos = []
        for x, y, cuenta in itertools.product(
            (79, 80, 81, 229, 230, 65535),
            (117, 118, 122, 126, 127, 32890),
            (0, 1, 100, 101, 32767, 32768, 65516, 65535),
        ):
            d = bytearray(base)
            guardar(d, 0x2C2D, x)
            guardar(d, 0x2C2F, y)
            guardar(d, 0x35CE, cuenta)
            casos.append(d)
        for actividad, imagen, cuenta, libres in itertools.product(
            (0, 1), (29, 30, 36, 37), (7, 8, 127, 128), (0, 1, 10)
        ):
            d = bytearray(base)
            d[0x2D33], d[0x2D34], d[0x2D45] = actividad, imagen, cuenta
            for i in range(palabra(d, 0x3588), 70):
                d[0x2C0A + i * 33] = 0 if i - palabra(d, 0x3588) < libres else 1
            casos.append(d)
        for numero, d in enumerate(casos):
            semilla = (ctypes.c_ushort * 4)(*self.semilla_menu)
            self.disponer(d, self.geometria, semilla)
            self.ejecutar_original(0xD41C)
            esperado = bytes(self.maquina.mem_read(self.segmento, 65536))
            azar = bytes(self.maquina.mem_read(0x78DE, 8))
            resultado = rutina(
                (ctypes.c_ubyte * 65536).from_buffer(d), semilla, self.callback_dimensiones
            )
            if resultado != 1 or bytes(d) != esperado or bytes(semilla) != azar:
                diferencias = [
                    f'{i:04X}:{d[i]:02X}/{esperado[i]:02X}'
                    for i in range(65536)
                    if d[i] != esperado[i]
                ][:8]
                raise ErrorAnalisis(
                    f'Guardia variante{numero} distinta: {diferencias}, retorno={resultado}.'
                )
        self.variantes['rutina_guardia'] = len(casos)
        self.retomar_recursos(base)

    # Identifica el objeto de cualquier discrepancia del selector original.
    def seleccionar(self, datos, casilla):
        id = palabra(datos, 0xB620 + casilla * 2)
        try:
            return super().seleccionar(datos, casilla)
        except ErrorAnalisis as error:
            raise ErrorAnalisis(f'{error} Identificador={id - 26}.') from error

    # Continúa la partida real con sus cerillas y el azar conservado por las veinte puertas.
    def ejecutar(self):
        origen = (
            self.raiz / 'apps/juego/trabajo/comparacion_laberinto_palacio/laberinto_solicitud107'
        )
        d = bytearray(origen.with_suffix('.ds.bin').read_bytes())
        self.semilla_menu[:] = json.loads(origen.with_suffix('.semilla.json').read_text())
        self.destino = self.raiz / 'apps/juego/trabajo/comparacion_guardia_palacio'
        self.destino.mkdir(parents=True, exist_ok=True)
        self.resumen = {}
        self.variantes = {}
        self.retomar_recursos(d)
        self.cambiar(d, self.semilla_menu, 107)
        self.hito_borde(d, 'guardia_entrada')
        self.variantes_guardia(d)
        self.variantes_cerillas(d)
        self.esperar(d, 32)
        self.hito_borde(d, 'guardia_control')
        self.auditar(d, 'guardia despierto')
        self.ruta_sin_salida(d, 60, 169)
        self.hito_borde(d, 'guardia_columna')
        print('oculto', [palabra(d, p) for p in (0x2C2D, 0x2C2F, 0x35CE)], flush=True)
        self.esperar(d, 128)
        self.hito_borde(d, 'guardia_dormido')
        self.auditar(d, 'guardia dormido')
        self.clic(d, 242, 163)
        self.esperar(d, 60)
        self.elegir_objeto(d, (2, 1), 162)
        self.elegir_objeto(d, (), 163)
        self.esperar(d, 96)
        self.exigir_objeto(d, 162, False)
        self.hito_borde(d, 'guardia_cerilla')
        self.auditar(d, 'cerilla colocada')
        print(
            'cerilla lista',
            [palabra(d, p) for p in (0x2C2D, 0x2C2F, 0x35CE)],
            list(d[0x3594:0x3597]),
            flush=True,
        )
        self.ruta_sin_salida(d, 160, 180)
        self.ruta_sin_salida(d, 60, 180)
        self.ruta_sin_salida(d, 60, 169)
        self.esperar(d, 128)
        print(
            'segunda columna',
            [palabra(d, p) for p in (0x2C2D, 0x2C2F, 0x35CE)],
            list(d[0x3594:0x3597]),
            flush=True,
        )
        self.clic(d, 242, 163)
        self.esperar(d, 60)
        print(
            'encender',
            [palabra(d, p) for p in (0x2C2D, 0x2C2F, 0x35CE, 0x359A)],
            list(d[0x3594:0x3597]),
            flush=True,
        )
        self.elegir_objeto(d, (5,), 161)
        print(
            'caja elegida',
            [palabra(d, p) for p in (0x0AD7, 0x35CE, 0x1A38, 0x1A3A)],
            [palabra(d, 0x5225 + i * 2) for i in range(10)],
            flush=True,
        )
        for paso in range(192):
            self.esperar(d, 1)
            if paso in (47, 95, 191):
                self.hito_borde(d, 'guardia_fuego_' + str(paso + 1))
        self.auditar(d, 'guardia tras el fuego')
        self.exigir_objeto(d, 161, False)
        self.ruta_sin_salida(d, palabra(d, 0x2CB1) + 2, palabra(d, 0x2CB3) + palabra(d, 0x2CB7))
        self.elegir_objeto(d, (1,), 164)
        for paso in range(768):
            salida = self.andar(d, self.semilla_menu, 65535, 1)
            if salida is not None:
                if salida != 111:
                    raise ErrorAnalisis(f'El guardia entrega salida{salida}, esperada111.')
                self.hito_borde(d, 'guardia_solicitud111')
                (self.destino / 'guardia_solicitud111.ds.bin').write_bytes(d)
                (self.destino / 'guardia_solicitud111.semilla.json').write_text(
                    json.dumps(list(self.semilla_menu))
                )
                break
        else:
            raise ErrorAnalisis('No aparece el transporte111 tras recoger el extintor.')
        self.exigir_objeto(d, 164)
        self.cambiar(d, self.semilla_menu, 111)
        self.hito_borde(d, 'guardia_regreso_benn')
        self.esperar(d, 192)
        self.hito_borde(d, 'guardia_benn_control')
        self.auditar(d, 'Benn después del palacio')
        informe = dict(
            resultado='coinciden',
            casos_y_pasos=sum(self.resumen.values()) + self.variantes['rutina_guardia'],
            grupos=self.resumen,
            variantes=self.variantes,
            capturas=self.capturas,
            estados_auditados=self.estados_auditados,
            limite='Sala63, columna, sueño, cerillas, extintor y regreso111 a Benn24 contrastados. El cementerio y final siguen pendientes; no se acredita una partida completa.',
        )
        (self.destino / 'resultado.json').write_text(
            json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8'
        )
        print(
            'Guardia:',
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
        ComparacionGuardiaPalacio(
            Path(__file__).resolve().parents[3], args.biblioteca, args.muestra
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        parser.exit(1, f'Error: {error}\n')
