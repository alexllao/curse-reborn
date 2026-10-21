"""Contrasta la entrada104, la mano que atrapa y la liberación con aceite144."""

import argparse
import ctypes
import itertools
import json
from pathlib import Path

from comparar_reina_hielo import ComparacionReinaHielo, ErrorAnalisis
from comparar_objetos import guardar, palabra


class ComparacionManoPalacio(ComparacionReinaHielo):
    # D36C–D41B: contrasta umbral vertical, control, actividad y orden de profundidad.
    def variantes_mano(self, base):
        rutina = self.nativa.procesar_rutina_mano
        rutina.argtypes = [ctypes.POINTER(ctypes.c_ubyte)]
        rutina.restype = ctypes.c_int
        numero = 0
        for activo, y, control, banderas in itertools.product(
            (0, 1, 3), (129, 130, 131, 65535), (0, 1), (1, 3)
        ):
            d = bytearray(base)
            d[0x2C4C] = activo
            guardar(d, 0x2C2F, y)
            d[0x3594] = control
            d[0x2C2B] = banderas
            self.disponer(d, self.geometria, self.semilla_menu)
            self.ejecutar_original(0xD36C)
            esperado = bytes(self.maquina.mem_read(self.segmento, 65536))
            if rutina((ctypes.c_ubyte * 65536).from_buffer(d)) != 1 or bytes(d) != esperado:
                diferencias = [
                    f'{i:04X}:{d[i]:02X}/{esperado[i]:02X}'
                    for i in range(65536)
                    if d[i] != esperado[i]
                ][:8]
                raise ErrorAnalisis(f'Mano variante{numero} distinta: {diferencias}')
            numero += 1
        self.variantes['rutina_mano'] = numero
        self.retomar_recursos(base)

    # Recibe la partida expulsada por el trono, sin añadir el aceite ni alterar la captura.
    def ejecutar(self):
        origen = self.raiz / 'apps/juego/trabajo/comparacion_reina_hielo/reina_solicitud104'
        d = bytearray(origen.with_suffix('.ds.bin').read_bytes())
        self.semilla_menu[:] = json.loads(
            origen.with_suffix('.semilla.json').read_text(encoding='utf-8')
        )
        self.destino = self.raiz / 'apps/juego/trabajo/comparacion_mano_palacio'
        self.destino.mkdir(parents=True, exist_ok=True)
        self.resumen = {}
        self.variantes = {}
        self.retomar_recursos(d)
        self.cambiar(d, self.semilla_menu, 104)
        self.hito_borde(d, 'mano_entrada')
        self.variantes_mano(d)
        self.exigir_objeto(d, 144)
        for paso in range(192):
            self.esperar(d, 1)
            if d[0x3594]:
                break
        print(
            'control',
            paso,
            palabra(d, 0x2C2D),
            palabra(d, 0x2C2F),
            d[0x358C],
            d[0x2C2C],
            flush=True,
        )
        self.hito_borde(d, 'mano_control')
        self.auditar(d, 'antes de la mano')
        self.clic(d, 160, 125)
        for paso in range(192):
            self.esperar(d, 1)
            if d[0x358C] == 23 and d[0x2C2C] == 29 and d[0x3595]:
                break
        else:
            raise ErrorAnalisis('La mano no atrapa o no devuelve el menú a Brad.')
        print('captura', paso, palabra(d, 0x2C2D), palabra(d, 0x2C2F), flush=True)
        self.hito_borde(d, 'mano_atrapado')
        self.auditar(d, 'atrapado por la mano')
        for modelo, cuadro, presente, puntos in itertools.product(
            (22, 23, 255), (28, 29, 30), (False, True), (0, 65535)
        ):
            cambios = [(0x358C, modelo, True), (0x2C2C, cuadro, True), (0x35C7, puntos, False)]
            cambios += [
                (0x5225 + i * 2, 144 if presente and i == 0 else 65535, False) for i in range(10)
            ]
            self.variante(d, 'aceite144', 0x3B53, 144, 4, cambios)
        self.retomar_recursos(d)
        self.elegir_objeto(d, (2, 4), 144)
        for paso in range(64):
            self.esperar(d, 1)
            if paso in (7, 23, 39):
                self.hito_borde(d, 'mano_libera_' + str(paso + 1))
        if not d[0x3594] or d[0x358C] != 255 or d[0x2C4C] != 0:
            raise ErrorAnalisis('El aceite no libera a Brad y retira la mano.')
        if 144 in [palabra(d, 0x5225 + i * 2) for i in range(10)]:
            raise ErrorAnalisis('La liberación no consume el aceite144.')
        self.hito_borde(d, 'mano_liberado')
        self.auditar(d, 'liberado de la mano')
        self.clic(d, 160, 140)
        for paso in range(192):
            salida = self.andar(d, self.semilla_menu, 65535, 1)
            if salida is not None:
                if salida != 105:
                    raise ErrorAnalisis(f'Salida inesperada de la mano: {salida}.')
                (self.destino / 'mano_solicitud105.ds.bin').write_bytes(d)
                (self.destino / 'mano_solicitud105.semilla.json').write_text(
                    json.dumps(list(self.semilla_menu)), encoding='utf-8'
                )
                break
        else:
            raise ErrorAnalisis('La puerta no solicita105 tras liberar a Brad.')
        self.conservar_contexto(self.destino, 'mano_solicitud105')
        informe = dict(
            resultado='coinciden',
            casos_y_pasos=sum(self.resumen.values()) + self.variantes['rutina_mano'],
            grupos=self.resumen,
            variantes=self.variantes,
            capturas=self.capturas,
            estados_auditados=self.estados_auditados,
            limite='Salida105 hacia escena59 solicitada; destino pendiente. DS completo y azar sin normalizar.',
        )
        (self.destino / 'resultado.json').write_text(
            json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8'
        )
        print(
            'Mano:',
            informe['casos_y_pasos'],
            'casos/pasos;',
            self.capturas,
            'imágenes; salida105 comprobada.',
            flush=True,
        )


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--biblioteca', type=Path, required=True)
    parser.add_argument('--muestra', action='store_true')
    args = parser.parse_args()
    try:
        ComparacionManoPalacio(
            Path(__file__).resolve().parents[4], args.biblioteca, args.muestra
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        parser.exit(1, f'Error: {error}\n')
