"""Contrasta C8F9 y la entrada real66 desde el pase; no acredita aún el puzle de CLOUD."""

import argparse
import ctypes
import json
from pathlib import Path

from comparar_entrega_grabacion import ComparacionEntregaGrabacion, ErrorAnalisis
from comparar_objetos import guardar, palabra


class ComparacionNube(ComparacionEntregaGrabacion):
    # Compara DS completo y azar con C8F9, incluidos umbrales firmados y orden de reciclado.
    def contrastar_nube(self, base, semilla):
        d = bytearray(base)
        azar = (ctypes.c_ushort * 4)(*semilla)
        self.disponer(d, self.geometria, azar)
        self.ejecutar_original(0xC8F9)
        esperado = bytes(self.maquina.mem_read(self.segmento, 65536))
        azar_esperado = bytes(self.maquina.mem_read(0x78DE, 8))
        if not self.nativa.procesar_nube((ctypes.c_ubyte * 65536).from_buffer(d), azar):
            raise ErrorAnalisis('C8F9 nativa rechazada.')
        if bytes(d) != esperado or bytes(azar) != azar_esperado:
            diferencias = [f'{i:04X}' for i in range(65536) if d[i] != esperado[i]][:10]
            raise ErrorAnalisis(f'C8F9 distinta: {diferencias}; azar={bytes(azar).hex()}.')
        return d, list(azar)

    # Aísla inversión, pausa, envoltura y límites; las variantes no conceden objetos a la partida.
    def variantes_nube(self, base):
        funcion = self.nativa.procesar_nube
        funcion.argtypes = [ctypes.POINTER(ctypes.c_ubyte), ctypes.POINTER(ctypes.c_ushort)]
        funcion.restype = ctypes.c_int
        numero = 0
        posiciones = (-32768, -761, -161, -160, -159, 0, 480, 481, 1081, 32767)
        vientos = (0, 109, 110, 111, 149, 150, 151, 189, 190, 191, 32767, 32768, 32917, 65535)
        for cuadro in (0, 8, 9, 127, 128, 136, 255):
            for viento in vientos:
                for modelo, canal in ((255, 0), (255, 0x9808), (23, 0)):
                    d = bytearray(base)
                    d[0x358C] = modelo
                    d[0x2C2C] = cuadro
                    guardar(d, 0x2C3B, canal)
                    guardar(d, 0x2C40, 0xCAFE)
                    guardar(d, 0x2C45, 0xBEEF)
                    guardar(d, 0x359A, 0xFFFF)
                    guardar(d, 0x0DFA, 0x1A36)
                    guardar(d, 0x35C5, viento)
                    guardar(d, 0x35B7, vientos[(numero * 5) % len(vientos)])
                    for i in range(4):
                        guardar(d, 0x2C6F + i * 33, posiciones[(numero + i * 3) % len(posiciones)])
                    self.contrastar_nube(d, (numero, 65535 - numero, 0x1357, 0x2468))
                    numero += 1
        self.resumen['variantes_C8F9'] = numero
        d = bytearray(base)
        semilla = list(self.semilla_menu)
        for paso in range(256):
            guardar(d, 0x35C5, (90, 150, 210)[(paso // 32) % 3])
            guardar(d, 0x35B7, (50, 250)[(paso // 64) % 2])
            d, semilla = self.contrastar_nube(d, semilla)
        self.resumen['secuencia_C8F9_aislada'] = 256
        self.retomar_recursos(base)

    # Continúa la solicitud66 real sin alterar inventario, carga CLOUD y compara sus primeros pasos.
    def ejecutar(self):
        origen = self.raiz / 'trabajo/comparacion_entrega_grabacion/puerta_solicitud66'
        if not origen.with_suffix('.ds.bin').exists():
            ComparacionEntregaGrabacion(self.raiz, self.biblioteca, True).ejecutar()
        d = bytearray(origen.with_suffix('.ds.bin').read_bytes())
        self.semilla_menu[:] = json.loads(
            origen.with_suffix('.semilla.json').read_text(encoding='utf-8')
        )
        self.destino = self.raiz / 'trabajo/comparacion_nube'
        self.destino.mkdir(parents=True, exist_ok=True)
        self.resumen = {}
        self.retomar_recursos(d)
        if palabra(d, 0x8082) != 35 or d[0x2CD0] != 1:
            raise ErrorAnalisis(
                'La instantánea no conserva POST con la puerta abierta por el pase.'
            )
        # La petición66 es el retorno del paso, no DS:3598 (última entrada58).
        # El comparador anterior solo guarda esta instantánea tras comprobar ese retorno.
        self.variantes_nube(d)
        self.cambiar(d, self.semilla_menu, 66)
        self.esperar(d, 128)
        self.hito_borde(d, 'nube_entrada')
        for direccion in (8, 4, 8, 4):
            self.esperar(d, 64, direccion)
        self.esperar(d, 32, 0)
        self.hito_borde(d, 'nube_impulso_alternado')
        informe = {
            'resultado': 'coinciden',
            'casos_y_pasos': sum(self.resumen.values()),
            'grupos': self.resumen,
            'capturas': self.capturas,
            'alcance': 'C8F9 aislada y entrada66 con primeros pasos e impulso alternado real. No acredita recogida121, regreso67 ni SDL del valle.',
        }
        (self.destino / 'resultado.json').write_text(
            json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8'
        )
        print(
            f'Nube: {informe["casos_y_pasos"]} casos/pasos y {self.capturas} composiciones coincidentes.'
        )


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Contrasta la deriva y la entrada original de CLOUD38.'
    )
    parser.add_argument('--biblioteca', type=Path, required=True)
    parser.add_argument('--muestra', action='store_true')
    opciones = parser.parse_args()
    try:
        ComparacionNube(
            Path(__file__).resolve().parents[1], opciones.biblioteca, opciones.muestra
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        parser.exit(1, f'Error: {error}\n')
