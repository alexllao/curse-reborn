"""Contrasta el recorrido real por las cuatro zonas de nieve hasta solicitar el lago81."""

import argparse
import ctypes
import json
from pathlib import Path

from comparar_regreso_benn import ComparacionRegresoBenn, ErrorAnalisis
from comparar_objetos import palabra, guardar


class ComparacionMorsa(ComparacionRegresoBenn):
    # Contrasta CBD0/CC4B/CDA6 sobre segmentos completos sin ejecutar guiones inventados.
    def variantes_rutinas(self, base):
        funcion = self.nativa.procesar_rutina_nieve
        funcion.argtypes = [ctypes.POINTER(ctypes.c_ubyte)]
        funcion.restype = ctypes.c_int
        numero = 0
        for escena, direccion in ((40, 0xCBD0), (41, 0xCC4B), (42, 0xCDA6)):
            for i in range(384):
                d = bytearray(base)
                guardar(d, 0x8082, escena)
                for p in (0x2C2D, 0x2C4E, 0x2CB1):
                    guardar(d, p, (0, 164, 165, 166, 899, 900, 901, 32768, 65535)[(i // 3) % 9])
                guardar(d, 0x3591, (0, 480, 65535)[i % 3])
                guardar(d, 0x2C2F, (71, 72, 100, 101, 65535)[(i // 9) % 5])
                d[0x2C4C] = (0, 1, 3)[i % 3]
                d[0x3594] = (i // 45) % 2
                d[0x35C9] = (0, 128)[(i // 4) % 2]
                d[0x2C4D] = (17, 18)[i % 2]
                d[0x2C6E] = (38, 39)[(i // 2) % 2]
                d[0x2C2C] = (27, 28)[(i // 3) % 2]
                guardar(d, 0x2C3B, (0, 0xCAFE)[(i // 5) % 2])
                d[0x2CAF] = (0, 1, 3)[(i // 3) % 3]
                d[0x2CB0] = (35, 36, 37)[(i // 5) % 3]
                if escena == 41:
                    guardar(d, 0x2C4E, 100)
                    guardar(
                        d,
                        0x2C2D,
                        100 + (-32768, -121, -120, -119, 0, 119, 120, 121, 32767)[(i // 3) % 9],
                    )
                for p in (0x18F5, 0x18F7, 0x18F9, 0x18FB):
                    guardar(d, p, 0xBEEF)
                self.disponer(d, self.geometria, self.semilla_menu)
                self.ejecutar_original(direccion)
                esperado = bytes(self.maquina.mem_read(self.segmento, 65536))
                if not funcion((ctypes.c_ubyte * 65536).from_buffer(d)) or bytes(d) != esperado:
                    diferencias = [f'{p:04X}' for p in range(65536) if d[p] != esperado[p]][:8]
                    raise ErrorAnalisis(f'Rutina{escena}/{i}: {diferencias}.')
                numero += 1
        self.resumen['rutinas_nieve'] = numero
        self.retomar_recursos(base)

    # B1F8/B350: inventario, puntuación y límites de los rodeos; no altera el recorrido principal.
    def variantes_hielo(self, base):
        for objeto in (127, 132):
            for objetos in ((), (objeto,), (131, objeto)):
                for puntos in (0, 65535):
                    for desplazamiento in (-32768, -1, 0, 1, 32767):
                        cambios = [
                            (0x35C7, puntos, False),
                            (0x2C2D, palabra(base, 0x2CB1) + desplazamiento, False),
                            (0x2C2F, (90, 120)[puntos == 0], False),
                        ]
                        cambios += [
                            (0x5225 + i * 2, objetos[i] if i < len(objetos) else 65535, False)
                            for i in range(10)
                        ]
                        if objeto == 127:
                            self.variante(base, 'romper127', 0x2FCA, 127, cambios=cambios)
                        else:
                            self.contrato(base, 132, 65535, 4, cambios)
        self.retomar_recursos(base)

    # B265: rechazos de perfume/postura y los tres rodeos verticales y horizontales.
    def variantes_entrega(self, base):
        for perfume in (0, 128, 255):
            for postura in (16, 17, 18):
                for distancia in (-70, -1, 0, 48, 70):
                    for altura in (0, 100, 125, 190):
                        cambios = [
                            (0x35C9, perfume, True),
                            (0x2C4D, postura, True),
                            (0x2C2D, palabra(base, 0x2C4E) + distancia, False),
                            (0x2C2F, altura, False),
                            (0x35C7, 65535, False),
                        ]
                        self.contrato(base, 126, 128, 6, cambios)
        self.retomar_recursos(base)

    # Entra42 desde la liberación real del ser azul y conserva su inventario y azar.
    def ejecutar(self):
        origen = self.raiz / 'apps/juego/trabajo/comparacion_ser_azul/nieve_ser_liberado'
        d = bytearray(origen.with_suffix('.ds.bin').read_bytes())
        self.semilla_menu[:] = json.loads(
            origen.with_suffix('.semilla.json').read_text(encoding='utf-8')
        )
        self.destino = self.raiz / 'apps/juego/trabajo/comparacion_morsa'
        self.destino.mkdir(parents=True, exist_ok=True)
        self.resumen = {}
        self.retomar_recursos(d)
        self.variantes = {}
        self.variantes_rutinas(d)
        self.ruta(d, ((620, 170), (620, 95)), 80)
        self.hito_borde(d, 'morsa_entrada')
        self.auditar(d, 'entrada42')
        self.variantes_hielo(d)
        self.ruta_sin_salida(d, 600, 143)
        self.hito_borde(d, 'morsa_ante_desodorante')
        self.auditar(d, 'ante desodorante')
        self.elegir_objeto(d, (1,), 132)
        self.esperar(d, 192)
        self.exigir_objeto(d, 132)
        self.hito_borde(d, 'morsa_desodorante')
        self.elegir_objeto(d, (2, 4), 132)
        self.esperar(d, 256)
        self.exigir_objeto(d, 132, False)
        self.hito_borde(d, 'morsa_perfumado')
        self.auditar(d, 'perfumado')
        self.ruta_sin_salida(d, 1000, 175)
        self.hito_borde(d, 'morsa_ante_saludo')
        self.menu(d, (4, 0))
        self.esperar(d, 256)
        if d[0x490F]:
            raise ErrorAnalisis('El saludo no duerme a la morsa.')
        self.hito_borde(d, 'morsa_saludada')
        self.auditar(d, 'morsa tras saludo')
        self.ruta(d, ((620, 175), (620, 95)), 78)
        self.hito_borde(d, 'esquimal_entrada')
        self.auditar(d, 'entrada41')
        self.ruta(d, ((620, 175), (620, 95)), 76)
        self.hito_borde(d, 'rescate_entrada')
        self.auditar(d, 'entrada40')
        self.ruta_sin_salida(d, 533, 148)
        self.hito_borde(d, 'rescate_ante_tablon')
        self.auditar(d, 'ante tablon125')
        self.elegir_objeto(d, (1,), 125)
        self.esperar(d, 256)
        self.exigir_objeto(d, 125)
        self.hito_borde(d, 'rescate_tablon')
        self.auditar(d, 'tablon125 en inventario')
        self.ruta(d, ((620, 180), (620, 199)), 75)
        self.hito_borde(d, 'esquimal_con_tablon')
        self.ruta_sin_salida(d, 274, 150)
        self.auditar(d, 'ante agujero128')
        self.elegir_objeto(d, (2, 5), 125)
        self.esperar(d, 256)
        self.hito_borde(d, 'esquimal_tablon_colocado')
        self.auditar(d, 'tablon colocado')
        self.elegir_objeto(d, (1,), 128)
        self.esperar(d, 192)
        self.exigir_objeto(d, 128)
        self.hito_borde(d, 'esquimal_pez')
        self.auditar(d, 'pez128 en inventario')
        self.ruta_sin_salida(d, 850, 175)
        self.auditar(d, 'ante esquimal126')
        self.variantes_entrega(d)
        self.elegir_objeto(d, (2, 6), 128)
        self.elegir_objeto(d, (), 126)
        self.esperar(d, 512)
        self.hito_borde(d, 'esquimal_entrega')
        self.auditar(d, 'esquimal tras entrega')
        self.elegir_objeto(d, (1,), 127)
        self.esperar(d, 192)
        self.exigir_objeto(d, 127)
        self.hito_borde(d, 'esquimal_cana')
        self.auditar(d, 'cana127 en inventario')
        self.ruta(d, ((620, 180), (620, 199)), 77)
        self.hito_borde(d, 'morsa_con_cana')
        self.ruta_sin_salida(d, 550, 166)
        self.auditar(d, 'ante hielo')
        self.elegir_objeto(d, (5,), 127)
        self.esperar(d, 1024)
        self.hito_borde(d, 'morsa_amigo_liberado')
        self.auditar(d, 'amigo liberado del hielo')
        self.elegir_objeto(d, (1,), 131)
        self.esperar(d, 192)
        self.exigir_objeto(d, 131)
        self.hito_borde(d, 'morsa_cenizas')
        self.auditar(d, 'cenizas131 en inventario')
        self.ruta(d, ((620, 175), (620, 95)), 78)
        self.ruta(d, ((620, 175), (620, 95)), 76)
        self.hito_borde(d, 'rescate_amigos_listos')
        self.ruta_sin_salida(d, 165, 114)
        for direccion, pasos in ((1, 8), (8, 4), (2, 16), (0, 128)):
            self.esperar(d, pasos, direccion)
        if palabra(d, 0x2C2F) > 72 or not d[0x3594]:
            raise ErrorAnalisis('La aproximacion no sube a la cornisa original.')
        self.hito_borde(d, 'rescate_cornisa')
        self.auditar(d, 'en cornisa con amigos')
        self.menu(d, (4, 1))
        self.hito_borde(d, 'rescate_ayuda')
        salida = self.andar(d, self.semilla_menu, 65535, 1024)
        if salida != 81:
            raise ErrorAnalisis(f'El rescate no alcanza81: {salida}.')
        # La solicitud conserva dimensiones F6 acotadas; la sala44 se comprobará al cargarla.
        (self.destino / 'rescate_solicitud81.ds.bin').write_bytes(d)
        (self.destino / 'rescate_solicitud81.semilla.json').write_text(
            json.dumps(list(self.semilla_menu)), encoding='utf-8'
        )
        self.conservar_contexto(self.destino, 'rescate_solicitud81')
        informe = {
            'resultado': 'coinciden',
            'casos_y_pasos': sum(self.resumen.values()),
            'grupos': self.resumen,
            'capturas': self.capturas,
            'variantes': self.variantes,
            'estados_auditados': self.estados_auditados,
            'alcance': 'Recorrido real43/42/41/40: desodorante, morsa, tablon, pez, esquimal, cana, hielo, cenizas y rescate hasta solicitar81. SDL pendiente.',
        }
        (self.destino / 'resultado.json').write_text(
            json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8'
        )
        print(
            f'Nieve: {informe["casos_y_pasos"]} casos/pasos y {self.capturas} composiciones coincidentes.'
        )


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Contrasta las cuatro zonas nevadas y el rescate de la cornisa.'
    )
    parser.add_argument('--biblioteca', type=Path, required=True)
    parser.add_argument('--muestra', action='store_true')
    opciones = parser.parse_args()
    try:
        ComparacionMorsa(
            Path(__file__).resolve().parents[4], opciones.biblioteca, opciones.muestra
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        parser.exit(1, f'Error: {error}\n')
