"""Contrasta la salida del ataúd y los encuentros del cementerio."""

import argparse
import ctypes
import itertools
import json
from pathlib import Path

from comparar_ataud import ComparacionAtaud, ErrorAnalisis
from comparar_objetos import palabra, guardar
from comparar_calles import ComparacionCalles


class ComparacionCementerio(ComparacionAtaud):
    # 4555–4837/60E1: fases, consumo ausente, puntuación y capacidad con el despacho real.
    def variantes_acciones_cementerio(self, base):
        semilla_recorrido = list(self.semilla_menu)
        for id, manejador, verbo in (
            (168, 0x2FCA, 0),
            (172, 0x3B53, 2),
            (170, 0x3B53, 3),
            (173, 0x2FCA, 0),
            (169, 0x2FCA, 0),
        ):
            for fase, presente, puntos in itertools.product(range(8), (False, True), (0, 65535)):
                cambios = [
                    (0x35CA, (base[0x35CA] & 0xC7) | (fase << 3), True),
                    (0x35C7, puntos, False),
                ]
                cambios += [
                    (0x5225 + i * 2, id if presente and i == 0 else 65535, False) for i in range(10)
                ]
                self.variante(base, 'accion_' + str(id), manejador, id, verbo, cambios)
        for id, ocupadas, modelo in itertools.product(
            (167, 168, 169, 170, 171, 173), (0, 9, 10), (20, 255)
        ):
            cambios = [(0x358C, modelo, True)]
            cambios += [
                (0x5225 + i * 2, 76 + i if i < ocupadas else 65535, False) for i in range(10)
            ]
            self.variante(base, 'recoger_' + str(id), 0x563A, id, 0, cambios)
        self.semilla_menu[:] = semilla_recorrido
        self.retomar_recursos(base)

    # 1BA3–20CC: compara ambos fondos, fases, activaciones, canales y umbrales de aparición.
    def variantes_vampiros(self, base):
        rutina = self.nativa.procesar_rutina_guardia
        rutina.argtypes = [
            ctypes.POINTER(ctypes.c_ubyte),
            ctypes.POINTER(ctypes.c_ushort),
            self.tipo_dimensiones,
        ]
        rutina.restype = ctypes.c_int
        numero = 0
        for escena, fase, actividad, canal, x in itertools.product(
            (65, 77), range(8), (1, 3), (0, 0x14E9, 0x14C4), (0, 120, 200, 270, 350)
        ):
            d = bytearray(base)
            guardar(d, 0x8082, escena)
            d[0x35CA] = (d[0x35CA] & 0xC7) | (fase << 3)
            d[0x2C4C] = actividad
            guardar(d, 0x2C5C, canal)
            guardar(d, 0x2C2D, x)
            semilla = (ctypes.c_ushort * 4)(*((v + numero * 17) & 65535 for v in self.semilla_menu))
            self.disponer(d, self.geometria, semilla)
            self.ejecutar_original(0x1BA3)
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
                    f'Cementerio variante{numero} distinta: {diferencias}, retorno={resultado}.'
                )
            numero += 1
        self.variantes['rutina_vampiros'] = numero
        self.retomar_recursos(base)

    # Retoma la excavación original antes de113, sin preparar vampiros derrotados.
    def ejecutar(self):
        origen = self.raiz / 'apps/juego/trabajo/comparacion_ataud/ataud_solicitud113'
        d = bytearray(origen.with_suffix('.ds.bin').read_bytes())
        self.semilla_menu[:] = json.loads(origen.with_suffix('.semilla.json').read_text())
        self.destino = self.raiz / 'apps/juego/trabajo/comparacion_cementerio'
        self.destino.mkdir(parents=True, exist_ok=True)
        self.resumen = {}
        self.variantes = {}
        # GRAVEYD.SPR supera la reserva5800; separa escena, modelo ySTD en la referencia DOS.
        self.referencia_modelo = 0x9000
        self.retomar_recursos(d)
        self.cambiar(d, self.semilla_menu, 113)
        self.hito_borde(d, 'cementerio_entrada')
        for paso in range(128):
            salida = self.andar(d, self.semilla_menu, 65535, 1)
            if salida is not None:
                raise ErrorAnalisis(
                    f'Llegada65 paso{paso} solicita{salida}; posición{palabra(d, 0x2C2D)},{palabra(d, 0x2C2F)} modelo{d[0x358C]}.'
                )
            if d[0x3594]:
                print('control', paso + 1, flush=True)
                break
        self.hito_borde(d, 'cementerio_control')
        self.variantes_vampiros(d)
        self.variantes_acciones_cementerio(d)
        self.auditar(d, 'salida del ataúd')
        print('cementerio', [palabra(d, p) for p in (0x2C2D, 0x2C2F, 0x3591)], flush=True)
        if self.andar(d, self.semilla_menu, 8, 28) is not None:
            raise ErrorAnalisis('Se abandona el cementerio antes de la pala.')
        self.andar(d, self.semilla_menu, 0, 1)
        print(
            'ante pala',
            [palabra(d, p) for p in (0x2C2D, 0x2C2F)],
            list(d[0x3594:0x3597]),
            flush=True,
        )
        self.elegir_objeto(d, (1,), 168)
        for paso in range(96):
            self.esperar(d, 1)
            if d[0x3594]:
                print('pala control', paso + 1, flush=True)
                break
        self.hito_borde(d, 'cementerio_pala')
        self.auditar(d, 'con la pala')
        self.andar(d, self.semilla_menu, 10, 16)
        self.andar(d, self.semilla_menu, 0, 1)
        for paso in range(256):
            salida = self.andar(d, self.semilla_menu, 65535, 1)
            if paso % 16 == 0:
                print(
                    'escondite',
                    paso,
                    [palabra(d, p) for p in (0x2C2D, 0x2C2F, 0x3591, 0x2C4E, 0x2C50, 0x2C5C)],
                    d[0x2C4C],
                    d[0x358C],
                    flush=True,
                )
            if salida is not None:
                raise ErrorAnalisis(f'El primer vampiro solicita{salida}.')
            if (d[0x35CA] >> 3) & 7:
                self.hito_borde(d, 'cementerio_vampiro_tumba')
                print('vampiro tumbado', paso, flush=True)
                break
        else:
            raise ErrorAnalisis('El primer vampiro no cae en la tumba.')
        self.esperar(d, 64)
        for id in (169, 170, 173):
            entidad = {169: 0x2DD8, 170: 0x2D75, 173: 0x2D54}[id]
            x = palabra(d, entidad + 2) + palabra(d, 0x3591) + 5
            y = palabra(d, entidad + 4) + palabra(d, entidad + 8) - 2
            if (
                ComparacionCalles.ir(self, d, self.semilla_menu, x, 195 if id == 169 else 178)
                is not None
            ):
                raise ErrorAnalisis(f'Salida al aproximarse al útil{id}.')
            if id == 170:
                for xx, yy in ((x + 32, 178), (x + 32, y)):
                    if ComparacionCalles.ir(self, d, self.semilla_menu, xx, yy) is not None:
                        raise ErrorAnalisis('Salida al rodear la piedra del útil170.')
            if ComparacionCalles.ir(self, d, self.semilla_menu, x, y) is not None:
                raise ErrorAnalisis(f'Salida ante el útil{id}.')
            for intento in range(4):
                pie = palabra(d, 0x2C2F) + palabra(d, 0x2C33) - 1
                if abs(pie - y) <= 4:
                    break
                if ComparacionCalles.ir(self, d, self.semilla_menu, x, y) is not None:
                    raise ErrorAnalisis(f'Salida tras el tropiezo ante{id}.')
            print(
                'ante útil',
                id,
                x,
                y,
                [
                    palabra(d, p)
                    for p in (
                        0x2C2D,
                        0x2C2F,
                        0x3591,
                        entidad + 2,
                        entidad + 4,
                        entidad + 6,
                        entidad + 8,
                        entidad + 12,
                    )
                ],
                d[entidad],
                list(d[0x3594:0x3597]),
                flush=True,
            )
            self.elegir_objeto(d, (1,), id)
            self.esperar(d, 96)
            self.exigir_objeto(d, id)
            self.hito_borde(d, 'cementerio_util_' + str(id))
            self.auditar(d, 'útil' + str(id))
            if id == 169:
                self.elegir_objeto(d, (5,), 168)
                self.esperar(d, 192)
                self.hito_borde(d, 'cementerio_pala_vampiro')

        self.continuar(d)

    # Sigue el recorrido con piedra y encuentros, conservando los objetos recogidos de verdad.
    def continuar(self, d):
        ComparacionCalles.ir(self, d, self.semilla_menu, 440, 159)
        self.elegir_objeto(d, (2, 2), 172)
        self.esperar(d, 192)
        self.hito_borde(d, 'cementerio_piedra')
        self.auditar(d, 'piedra desplazada')
        print('fase tras piedra', (d[0x35CA] >> 3) & 7, flush=True)
        self.continuar_piedra(d)

    # 4680 y salida114: vuelve a la tumba y continúa al segundo fondo sin saltarse el camino.
    def continuar_piedra(self, d):
        self.ruta(d, ((550, 178), (606, 195)), 114)
        self.hito_borde(d, 'cementerio_segundo')
        self.auditar(d, 'segundo cementerio')
        self.elegir_objeto(d, (2, 3), 170)
        self.esperar(d, 512)
        self.hito_borde(d, 'cementerio_ajo')
        self.auditar(d, 'ajo y vampiro')
        print('fase tras ajo', (d[0x35CA] >> 3) & 7, flush=True)
        self.continuar_ajo(d)

    # 4730/47D6: sigue las dos acciones de combate en el segundo cementerio.
    def continuar_ajo(self, d):
        self.elegir_objeto(d, (5,), 173)
        self.esperar(d, 512)
        self.hito_borde(d, 'cementerio_cruz')
        self.auditar(d, 'cruz y vampiro')
        self.elegir_objeto(d, (5,), 169)
        self.esperar(d, 512)
        self.hito_borde(d, 'cementerio_platillos')
        self.auditar(d, 'platillos y vampiro')
        print(
            'fase tras platillos',
            (d[0x35CA] >> 3) & 7,
            'control',
            list(d[0x3594:0x3597]),
            flush=True,
        )
        self.continuar_platillos(d)

    # Recoge171 por su posición real y sigue hacia la puerta del cementerio.
    def continuar_platillos(self, d):
        x = palabra(d, 0x2D35) + palabra(d, 0x3591) + 5
        y = palabra(d, 0x2D37) + palabra(d, 0x2D3B) - 2
        if ComparacionCalles.ir(self, d, self.semilla_menu, x, y) is not None:
            raise ErrorAnalisis('Salida antes del útil171.')
        self.elegir_objeto(d, (1,), 171)
        self.esperar(d, 128)
        self.exigir_objeto(d, 171)
        self.hito_borde(d, 'cementerio_util_171')
        self.auditar(d, 'útil171')
        self.regresar(d)

    # Rodea la tumba de la esquina antes de cruzar115, conservando sus colisiones originales.
    def regresar(self, d):
        self.ruta(d, ((200, 195), (0, 195)), 115)
        self.hito_borde(d, 'cementerio_regreso')
        self.auditar(d, 'regreso al primer fondo')
        self.recoger_restante(d)

    # Vuelve al útil167 tras despejar los encuentros, sin alterar los descriptores persistentes.
    def recoger_restante(self, d):
        if ComparacionCalles.ir(self, d, self.semilla_menu, 550, 195) is not None:
            raise ErrorAnalisis('Salida al apartarse de los tropiezos del primer cementerio.')
        x = (palabra(d, 0x2D98) + palabra(d, 0x3591) + 5) & 65535
        y = palabra(d, 0x2D9A) + palabra(d, 0x2D9E) - 2
        salida = ComparacionCalles.ir(self, d, self.semilla_menu, x, y)
        if salida is not None:
            raise ErrorAnalisis(
                f'Salida{salida} antes del útil167; destino{x},{y}; jugador{palabra(d, 0x2C2D)},{palabra(d, 0x2C2F)} cámara{palabra(d, 0x3591)}.'
            )
        self.elegir_objeto(d, (1,), 167)
        self.esperar(d, 96)
        self.exigir_objeto(d, 167)
        self.hito_borde(d, 'cementerio_util_167')
        self.auditar(d, 'útil167')
        self.puerta(d)

    # 1FF5–20CC: el objeto171 y la cámara activan la huida que abre la verja116.
    def puerta(self, d):
        self.ruta(d, ((550, 178), (606, 195)), 114)
        self.avanzar_puerta(d)

    # Pasa por debajo de la tumba306–324 y del sendero de tropiezos de la segunda zona.
    def avanzar_puerta(self, d):
        for x, y in ((110, 170), (250, 178), (250, 190), (450, 190), (500, 170)):
            try:
                salida = ComparacionCalles.ir(self, d, self.semilla_menu, x, y)
            except ErrorAnalisis:
                self.hito_borde(d, 'cementerio_diagnostico_camino')
                print(
                    'camino',
                    x,
                    y,
                    [palabra(d, p) for p in (0x2C2D, 0x2C2F, 0x3591, 0x18F5, 0x18F9)],
                    list(d[0x3594:0x3598]),
                    flush=True,
                )
                raise
            if salida is not None:
                raise ErrorAnalisis(f'Salida{salida} antes de la verja.')
        self.esperar(d, 512)
        self.hito_borde(d, 'cementerio_puerta')
        self.auditar(d, 'verja del castillo')
        self.cruzar_verja(d)

    # Ajusta el clic al ancho de Brad, sin atravesar la pared situada a la derecha de518.
    def cruzar_verja(self, d):
        salida = ComparacionCalles.ir(self, d, self.semilla_menu, 500, 138)
        print('verja', salida, 'actor', d[0x2C4C], 'puerta', d[0x2CD1], flush=True)
        if salida != 116:
            raise ErrorAnalisis(f'La verja solicita{salida}; esperada116.')
        (self.destino / 'cementerio_solicitud116.ds.bin').write_bytes(d)
        (self.destino / 'cementerio_solicitud116.semilla.json').write_text(
            json.dumps(list(self.semilla_menu))
        )
        self.conservar_contexto(self.destino, 'cementerio_solicitud116')
        informe = dict(
            resultado='coinciden',
            casos_y_pasos=sum(self.resumen.values()),
            grupos=self.resumen,
            variantes=self.variantes,
            capturas=self.capturas,
            estados_auditados=self.estados_auditados,
            limite='Cementerios65/77, seis útiles, fases0–6, rutas113/114/115 y petición116. Destino66 pendiente.',
        )
        (self.destino / 'resultado.json').write_text(
            json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8'
        )
        print(
            'Cementerio:',
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
        ComparacionCementerio(
            Path(__file__).resolve().parents[4], args.biblioteca, args.muestra
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        parser.exit(1, f'Error: {error}\n')
