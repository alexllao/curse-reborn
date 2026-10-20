"""Contrasta el duelo FINAL72 desde la biblioteca hasta solicitar125."""

import argparse
import ctypes
import itertools
import json
from pathlib import Path

from comparar_biblioteca_castillo import ComparacionBibliotecaCastillo, ErrorAnalisis
from comparar_objetos import palabra, guardar


class ComparacionBruja(ComparacionBibliotecaCastillo):
    # 20CD–2549: contrasta fases, fronteras del fantasma, fracasos, partículas y falta de registros.
    def variantes_duelo(self, base):
        rutina = self.nativa.procesar_rutina_guardia
        rutina.argtypes = [
            ctypes.POINTER(ctypes.c_ubyte),
            ctypes.POINTER(ctypes.c_ushort),
            self.tipo_dimensiones,
        ]
        rutina.restype = ctypes.c_int
        numero = 0
        for fase, modelo, actividad, x, ocupado, bajos in itertools.product(
            range(4), (255, 27), (0, 1, 3), (84, 85, 120, 121, 32768), (False, True), (0, 63)
        ):
            d = bytearray(base)
            d[0x35CA] = fase * 64 + bajos
            d[0x358C] = modelo
            d[0x3595] = 1
            d[0x2C6D] = d[0x2C8E] = d[0x2D33] = actividad
            guardar(d, 0x2D14, x)
            guardar(d, 0x2C3B, 0x1A2C if ocupado else 0)
            guardar(d, 0x2C5C, 0x1A2C if ocupado else 0)
            guardar(d, 0x2D22, 0x16A2 if actividad else 0)
            guardar(d, 0x2CE0, 0)
            d[0x2C5F] = actividad
            d[0x2D03] = 4 if actividad else 0
            if x == 32768:
                for p in range(0x2C0A + palabra(d, 0x3588) * 33, 0x3510, 33):
                    d[p] = 1
            semilla = (ctypes.c_ushort * 4)(*((v + numero * 17) & 65535 for v in self.semilla_menu))
            self.disponer(d, self.geometria, semilla)
            self.ejecutar_original(0x20CD)
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
                    f'Bruja variante{numero} fase{fase}: {diferencias}, retorno{resultado}.'
                )
            numero += 1
        self.variantes['rutina_duelo'] = numero
        semilla = list(self.semilla_menu)
        for id, manejador, verbo in ((171, 0x2FCA, 0), (164, 0x3B53, 2), (124, 0x2FCA, 0)):
            for fase, puntos, presente in itertools.product(range(4), (0, 65535), (False, True)):
                cambios = [(0x35CA, fase * 64 + 63, True), (0x35C7, puntos, False)]
                cambios += [
                    (0x5225 + i * 2, id if presente and i == 0 else 65535, False) for i in range(10)
                ]
                self.variante(base, 'defensa_' + str(id), manejador, id, verbo, cambios)
        for fase, puntos, verbo, invertido in itertools.product(
            range(4), (0, 65535), (0, 7), (False, True)
        ):
            cambios = [(0x35CA, fase * 64 + 63, True), (0x35C7, puntos, False)]
            self.contrato(
                base, 175 if invertido else 177, 177 if invertido else 175, verbo, cambios
            )
        self.variantes['anillo_orden_y_verbo'] = 32
        self.semilla_menu[:] = semilla
        self.retomar_recursos(base)

    # Conserva cada fase y su semilla para continuar desde estados reales, sin escribir progreso.
    def guardar_hito(self, d, nombre):
        self.hito_borde(d, 'bruja_' + nombre)

    # Sigue un límite de pasos y rechaza salidas prematuras antes de la condición original.
    def esperar_duelo(self, d, nombre, condicion, limite=512):
        for paso in range(limite):
            if condicion():
                print(
                    nombre,
                    paso,
                    'fase',
                    d[0x35CA] >> 6,
                    'control',
                    list(d[0x3594:0x3597]),
                    flush=True,
                )
                self.guardar_hito(d, nombre)
                return
            salida = self.andar(d, self.semilla_menu, 65535, 1)
            if salida is not None:
                raise ErrorAnalisis(f'Duelo{nombre}: salida prematura{salida}, paso{paso}.')
        raise ErrorAnalisis(
            f'Duelo{nombre} no alcanza condición; fase{d[0x35CA] >> 6}, modelo{d[0x358C]}, fantasma{palabra(d, 0x2D14)}, canales{palabra(d, 0x2C3B):04X}/{palabra(d, 0x2C5C):04X}.'
        )

    # Reanuda124 con los cuatro útiles obtenidos en el recorrido real de la guía.
    def ejecutar(self):
        origen = (
            self.raiz / 'apps/juego/trabajo/comparacion_biblioteca_castillo/biblioteca_solicitud124'
        )
        d = bytearray(origen.with_suffix('.ds.bin').read_bytes())
        self.semilla_menu[:] = json.loads(origen.with_suffix('.semilla.json').read_text())
        self.destino = self.raiz / 'apps/juego/trabajo/comparacion_bruja'
        self.destino.mkdir(parents=True, exist_ok=True)
        self.resumen = {}
        self.variantes = {}
        self.referencia_modelo = 0x9000
        self.retomar_recursos(d)
        self.cambiar(d, self.semilla_menu, 124)
        self.variantes_duelo(d)
        self.guardar_hito(d, 'entrada')
        self.esperar_duelo(
            d,
            'fantasma',
            lambda: d[0x3595] and palabra(d, 0x2D22) and 120 < palabra(d, 0x2D14) <= 180,
        )
        self.auditar(d, 'fantasma de la bruja')
        self.elegir_objeto(d, (5,), 171)
        self.esperar_duelo(d, 'aspiradora', lambda: d[0x35CA] >> 6 == 1 and d[0x3595])
        self.auditar(d, 'defensa del fantasma')
        self.elegir_objeto(d, (2, 2, 0), 164)
        self.esperar_duelo(d, 'extintor', lambda: d[0x35CA] >> 6 == 2 and d[0x3595])
        self.auditar(d, 'defensa del fuego')
        self.elegir_objeto(d, (5,), 124)
        self.esperar_duelo(d, 'ventilador', lambda: d[0x35CA] >> 6 == 3 and d[0x3595], 1024)
        self.auditar(d, 'bruja derrotada')
        self.elegir_objeto(d, (2, 7), 175)
        self.elegir_objeto(d, (), 177)
        for paso in range(1024):
            salida = self.andar(d, self.semilla_menu, 65535, 1)
            if salida is not None:
                if salida != 125:
                    raise ErrorAnalisis(f'El anillo solicita{salida}, esperada125.')
                prefijo = self.destino / 'bruja_solicitud125'
                prefijo.with_suffix('.ds.bin').write_bytes(d)
                prefijo.with_suffix('.semilla.json').write_text(
                    json.dumps(list(self.semilla_menu)) + '\n'
                )
                break
            if paso in (31, 63, 95):
                self.guardar_hito(d, 'anillo_' + str(paso + 1))
        else:
            raise ErrorAnalisis('El anillo no entrega125.')
        informe = dict(
            resultado='coinciden',
            casos_y_pasos=sum(self.resumen.values()),
            grupos=self.resumen,
            variantes=self.variantes,
            capturas=self.capturas,
            estados_auditados=self.estados_auditados,
            limite='Duelo72, aspiradora171, extintor164, ventilador124 y anillo175 hasta125; desenlace73 pendiente.',
        )
        (self.destino / 'resultado.json').write_text(
            json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8'
        )
        print(json.dumps(informe, ensure_ascii=False), flush=True)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--biblioteca', type=Path, required=True)
    parser.add_argument('--muestra', action='store_true')
    args = parser.parse_args()
    try:
        ComparacionBruja(
            Path(__file__).resolve().parents[4], args.biblioteca, args.muestra
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        parser.exit(1, f'Error: {error}\n')
