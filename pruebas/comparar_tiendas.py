"""Contrasta los interiores de Benn, Sally y la segunda casa abandonada con el EXE."""

import argparse
import ctypes
import json
from pathlib import Path

from comparar_calles import ComparacionCalles, ErrorAnalisis
from comparar_objetos import guardar, palabra
from comparar_poblacion import ComparacionPoblacion


class ComparacionTiendas(ComparacionCalles):
    # Revisa las fases de oferta de Benn sin trasladar estas variantes a la partida persistente.
    def variantes_benn(self, base):
        for fase in (0, 1, 2, 3, 4):
            for puntos in (0, 65535):
                d = bytearray(base)
                d[0x2C6D] = fase
                guardar(d, 0x35C7, puntos)
                guardar(d, 0xB60C, 0x3B53)
                guardar(d, 0xB620, 78 + 26)
                guardar(d, 0xB634, 6)
                guardar(d, 0xB638, 76)
                self.seleccionar(d, 0)
        # 19DF–1A73 también debe conservar las emisiones aleatorias de las visitas posteriores.
        for progreso in (0, 4):
            for activo in (0, 1, 3):
                for secuencia in (0, 0x1383, 0x1A2F):
                    d = bytearray(base)
                    d[0x35C9] = progreso
                    d[0x2C4C] = activo
                    guardar(d, 0x2C5C, secuencia)
                    d[0x2C5E] = 0
                    d[0x2C5F] = 40
                    d[0x2C60] = 2
                    sem = (ctypes.c_ushort * 4)(*self.semilla_menu)
                    self.retomar_recursos(d)
                    self.paso(d, sem)
        self.retomar_recursos(base)
        self.maquina.ctl_flush_tb()

    # Conserva captura, estado y semilla de cada hito para reproducirlo también en SDL.
    def hito_tienda(self, datos, nombre):
        self.captura(datos, self.destino, nombre)
        (self.destino / (nombre + '.semilla.json')).write_text(json.dumps(list(self.semilla_menu)))
        self.auditar(datos, nombre)

    # Cada visita parte de una calle contrastada; entra y regresa caminando por sus umbrales.
    def ejecutar(self):
        if not (self.raiz / 'trabajo/comparacion_calles/calle_30.ds.bin').exists():
            if not (self.raiz / 'trabajo/comparacion_poblacion/poblacion_fuente.ds.bin').exists():
                ComparacionPoblacion(self.raiz, self.biblioteca, True).ejecutar()
            ComparacionCalles(self.raiz, self.biblioteca, True).ejecutar()
        self.destino = self.raiz / 'trabajo/comparacion_tiendas'
        self.destino.mkdir(parents=True, exist_ok=True)
        self.resumen = {}
        for calle, salida, puntos, regreso, vuelta in (
            (29, 35, ((300, 170), (380, 150), (380, 120)), 46, ((200, 155), (210, 145))),
            (30, 38, ((420, 170), (300, 170), (115, 150), (108, 127)), 48, ((120, 170), (85, 145))),
            (30, 39, ((420, 170), (300, 155), (270, 120)), 49, ((160, 175),)),
        ):
            origen = self.raiz / 'trabajo/comparacion_calles' / f'calle_{calle}'
            d = bytearray(origen.with_suffix('.ds.bin').read_bytes())
            self.semilla_menu[:] = json.loads(origen.with_suffix('.semilla.json').read_text())
            sem = self.semilla_menu
            self.retomar_recursos(d)
            encontrada = None
            for x, y in puntos:
                encontrada = self.ir(d, sem, x, y)
                if encontrada is not None:
                    break
            if encontrada != salida:
                raise ErrorAnalisis(
                    f'Entrada {salida}: salida {encontrada}; posición '
                    f'{palabra(d, 0x2C2D)},{palabra(d, 0x2C2F)}, cámara {palabra(d, 0x3591)}.'
                )
            self.cambiar(d, sem, salida)
            self.andar(d, sem, 65535, 128)
            self.hito_tienda(d, f'tienda_{salida}')
            if salida == 35:
                self.variantes_benn(d)
            if salida == 38:
                for x, y in ((120, 178), (200, 178)):
                    self.ir(d, sem, x, y)
                self.menu(d, (4, 0))
                self.andar(d, sem, 65535, 96)
                self.hito_tienda(d, 'adivina_saludo')
                self.elegir_objeto(d, (2, 6), 76)
                self.elegir_objeto(d, (), 81)
                self.andar(d, sem, 65535, 192)
                self.hito_tienda(d, 'adivina_respuesta')
            for x, y in vuelta:
                encontrada = self.ir(d, sem, x, y)
                if encontrada is not None:
                    break
            if encontrada != regreso:
                raise ErrorAnalisis(f'Regreso de {salida}: {encontrada}, esperado {regreso}.')
            self.cambiar(d, sem, regreso)
            self.andar(d, sem, 65535, 128)
            self.hito_tienda(d, f'regreso_tienda_{salida}')
        informe = {
            'resultado': 'coinciden',
            'casos_y_pasos': sum(self.resumen.values()),
            'grupos': self.resumen,
            'estados_auditados': self.estados_auditados,
            'capturas': self.capturas,
        }
        (self.destino / 'resultado.json').write_text(
            json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8'
        )
        print(
            f'Tiendas: {informe["casos_y_pasos"]} casos/pasos y {self.capturas} composiciones coincidentes.'
        )


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Compara los interiores laterales de la población.'
    )
    parser.add_argument('--biblioteca', type=Path, required=True)
    parser.add_argument('--muestra', action='store_true')
    opciones = parser.parse_args()
    try:
        ComparacionTiendas(
            Path(__file__).resolve().parents[1], opciones.biblioteca, opciones.muestra
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        parser.exit(1, f'Error: {error}\n')
