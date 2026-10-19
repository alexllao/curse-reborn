"""Contrasta COUNTRY2: espada, asaltante, dinero y sendero hasta la entrada en DRAGON."""

import argparse
import ctypes
import json
import os
from pathlib import Path

from comparar_campo import ComparacionCampo, ErrorAnalisis
from comparar_objetos import guardar, palabra


class ComparacionAsaltante(ComparacionCampo):
    # Recupera el motor de comparación y mantiene separados los resultados de esta pantalla.
    def __init__(self, raiz, biblioteca, muestra):
        super().__init__(raiz, biblioteca, muestra)
        self.carpeta_resultados = 'comparacion_asaltante'
        self.capturas = 0

    # Cuenta solamente las imágenes de este tramo, cada una contrastada antes de conservarse.
    def captura(self, datos, carpeta, nombre):
        super().captura(datos, carpeta, nombre)
        self.capturas += 1

    # EAD5/EBBD ordena antes del paso lógico; los empates conservan el historial de dibujo.
    def paso(self, datos, semilla, direccion=0xFFFF):
        self.maquina.mem_write(self.segmento, bytes(datos))
        self.ejecutar_original(0xF29A)
        esperado = bytes(self.maquina.mem_read(self.segmento, 65536))
        if (
            not self.nativa.procesar_objetos(
                2,
                (ctypes.c_ubyte * 65536).from_buffer(datos),
                semilla,
                None,
                0,
                0,
                0,
                ctypes.byref(ctypes.c_ushort()),
            )
            or datos != esperado
        ):
            raise ErrorAnalisis('El orden de dibujo previo al paso difiere de F29A.')
        self.resumen['ordenes_dibujo'] = self.resumen.get('ordenes_dibujo', 0) + 1
        return super().paso(datos, semilla, direccion)

    # Contrasta límites de inventario, cambio de modelo y desbordamiento de puntuación al recoger.
    def variantes_recogidas(self, base):
        for objeto in (75, 76):
            for ocupadas in (0, 9, 10):
                for modelo in (20, 255):
                    for puntos in (0, 65535):
                        datos = bytearray(base)
                        for i in range(10):
                            guardar(datos, 0x5225 + i * 2, 3 if i < ocupadas else 65535)
                        datos[0x358C] = modelo
                        guardar(datos, 0x35C7, puntos)
                        guardar(datos, 0xB60C, 0x563A)
                        guardar(datos, 0xB620, objeto + 26)
                        self.seleccionar(datos, 0)
        self.maquina.ctl_flush_tb()

    # Incluso con una sola plaza de entidad o sin ninguna, conserva los 120 consumos de azar.
    def variantes_ataque(self, base):
        semilla = list(self.semilla_menu)
        for numero in range(8):
            for libres in (0, 1, 40):
                for puntos in (0, 65535):
                    datos = bytearray(base)
                    self.semilla_menu[:] = [(n + numero * 7919) & 65535 for n in semilla]
                    primera = palabra(datos, 0x3588)
                    for i in range(primera, 70):
                        datos[0x2C0A + i * 33] = 0 if i - primera < libres else 1
                    guardar(datos, 0x35C7, puntos)
                    guardar(datos, 0xB60C, 0x2FCA)
                    guardar(datos, 0xB620, 75 + 26)
                    self.seleccionar(datos, 0)
        self.semilla_menu[:] = semilla
        self.retomar_recursos(base)
        self.maquina.ctl_flush_tb()

    # Reanuda una partida ya contrastada y sigue las mismas entradas que el recorrido SDL.
    def ejecutar(self):
        origen = self.raiz / 'apps/juego/trabajo/comparacion_campo'
        fichero = origen / 'campo_salida.ds.bin'
        if not fichero.exists():
            ComparacionCampo(self.raiz, self.biblioteca, True).ejecutar()
        datos = bytearray(fichero.read_bytes())
        if len(datos) != 65536 or palabra(datos, 0x8082) != 16 or palabra(datos, 0x3598) != 25:
            raise ErrorAnalisis('La referencia no corresponde a la llegada real a COUNTRY2.')
        semilla = (ctypes.c_ushort * 4)(
            *json.loads((origen / 'campo_salida.semilla.json').read_text(encoding='utf-8'))
        )
        self.semilla_menu = semilla
        self.recursos_menu = True
        self.retomar_recursos(datos)
        carpeta = self.raiz / 'apps/juego/trabajo/comparacion_asaltante'
        carpeta.mkdir(parents=True, exist_ok=True)
        # El orden de empates depende de los dibujos anteriores, ajenos a esta prueba de pantalla.
        # La prueba SDL comparte solo esta permutación de entrada; toda su evolución se ejecuta en DOS.
        referencia_grafica = os.environ.get('CURSE_REBORN_ORDEN_ENTRADA')
        if referencia_grafica:
            entrada = Path(referencia_grafica).read_bytes()
            if (
                len(entrada) != 65536
                or palabra(entrada, 0x8082) != 16
                or palabra(entrada, 0x3598) != 25
            ):
                raise ErrorAnalisis('La entrada gráfica común no corresponde a COUNTRY2.')
            orden = entrada[0xBDD2:0xBEE6]
            entidades = [palabra(entrada, 0xBDD4 + i * 4) for i in range(69)]
            if sorted(entidades) != [0x2C2B + i * 33 for i in range(69)]:
                raise ErrorAnalisis('La permutación de entidades de entrada no es válida.')
            datos[0xBDD2:0xBEE6] = orden
        self.resumen = {}
        self.captura(datos, carpeta, 'asaltante_entrada')
        self.andar(datos, semilla, 65535, 32)
        self.captura(datos, carpeta, 'asaltante_tropiezo')
        self.auditar(datos, 'tropiezo')
        self.andar(datos, semilla, 65535, 32)
        self.captura(datos, carpeta, 'asaltante_espada_suelo')
        self.clic(datos, 227, 178)
        self.andar(datos, semilla, 65535, 96)
        self.auditar(datos, 'espada al alcance')
        self.variantes_recogidas(datos)
        self.elegir_objeto(datos, (1,), 75)
        self.andar(datos, semilla, 65535, 128)
        self.captura(datos, carpeta, 'asaltante_espada')
        self.auditar(datos, 'espada en inventario')
        self.variantes_ataque(datos)
        self.elegir_objeto(datos, (5,), 75)
        self.captura(datos, carpeta, 'asaltante_ataque_inicio')
        for i in range(8):
            self.andar(datos, semilla, 65535, 32)
            if i in (0, 2, 7):
                self.captura(datos, carpeta, f'asaltante_ataque_{i}')
        if datos[0x2C4C] or not datos[0x3594]:
            raise ErrorAnalisis('El ataque no retira al asaltante o no devuelve el control.')
        self.auditar(datos, 'asaltante vencido')
        self.clic(datos, 190, 170)
        self.andar(datos, semilla, 65535, 96)
        self.auditar(datos, 'dinero al alcance')
        self.elegir_objeto(datos, (1,), 76)
        self.andar(datos, semilla, 65535, 128)
        if 76 not in [palabra(datos, 0x5225 + i * 2) for i in range(10)]:
            raise ErrorAnalisis('El dinero no se incorpora al inventario.')
        self.captura(datos, carpeta, 'asaltante_dinero')
        self.auditar(datos, 'dinero recogido')
        for x, y in ((65, 175), (65, 140), (95, 130), (110, 110)):
            self.clic(datos, x, y)
            if self.andar(datos, semilla, 65535, 96) is not None:
                raise ErrorAnalisis('El recorrido abandona prematuramente COUNTRY2.')
        self.captura(datos, carpeta, 'asaltante_camino')
        self.auditar(datos, 'camino liberado')
        self.clic(datos, 175, 94)
        self.andar(datos, semilla, 65535, 32)
        self.captura(datos, carpeta, 'asaltante_sendero')
        for _ in range(512):
            salida = self.andar(datos, semilla, 65535, 1)
            if salida is not None:
                break
        else:
            raise ErrorAnalisis('El sendero no alcanza la salida 26.')
        if salida != 26:
            raise ErrorAnalisis(f'Salida inesperada de COUNTRY2: {salida}.')
        self.cambiar(datos, semilla, salida)
        self.captura(datos, carpeta, 'asaltante_salida')
        informe = {
            'resultado': 'coinciden',
            'casos_y_pasos': sum(self.resumen.values()),
            'grupos': self.resumen,
            'capturas': self.capturas,
            'estados_auditados': self.estados_auditados,
            'orden_inicial_compartido_con_sdl': bool(referencia_grafica),
            'alcance': 'COUNTRY2 hasta la entrada en DRAGON; los puzles de la población siguen pendientes.',
        }
        (carpeta / 'resultado.json').write_text(
            json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8'
        )
        print(
            f'Asaltante: {informe["casos_y_pasos"]} casos/pasos, {self.capturas} composiciones y {self.estados_auditados} estados de menú coincidentes.'
        )


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Compara la pantalla diecisiete con el original.')
    parser.add_argument('--biblioteca', type=Path, required=True)
    parser.add_argument('--muestra', action='store_true')
    opciones = parser.parse_args()
    try:
        ComparacionAsaltante(
            Path(__file__).resolve().parents[3], opciones.biblioteca, opciones.muestra
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        parser.exit(1, f'Error: {error}\n')
