"""Contrasta la llegada81, las cenizas y la criatura del lago helado hasta solicitar82."""

import argparse
import ctypes
import json
from pathlib import Path

from comparar_morsa import ComparacionMorsa, ErrorAnalisis
from comparar_objetos import guardar


class ComparacionLagoHelado(ComparacionMorsa):
    # CF26: solo la imagen34 de la cuarta entidad cambia358F, aunque su actividad sea cero.
    def variantes_lago(self, base):
        funcion = self.nativa.procesar_rutina_nieve
        funcion.argtypes = [ctypes.POINTER(ctypes.c_ubyte)]
        funcion.restype = ctypes.c_int
        for imagen in (0, 33, 34, 35, 255):
            for limite in (0, 223, 65535):
                d = bytearray(base)
                d[0x2CD1] = imagen
                guardar(d, 0x358F, limite)
                self.disponer(d, self.geometria, self.semilla_menu)
                self.ejecutar_original(0xCF26)
                esperado = bytes(self.maquina.mem_read(self.segmento, 65536))
                if not funcion((ctypes.c_ubyte * 65536).from_buffer(d)) or bytes(d) != esperado:
                    raise ErrorAnalisis('CF26 altera el limite del lago de forma distinta.')
        self.resumen['rutina_lago'] = 15
        for presentes in (False, True):
            for puntos in (0, 65535):
                cambios = [(0x35C7, puntos, False)]
                cambios += [
                    (0x5225 + i * 2, 131 if presentes and i == 0 else 65535, False)
                    for i in range(10)
                ]
                cambios += [(p, 0xCAFE, False) for p in (0xAC, 0xC0, 0xCC, 0xCE)]
                self.contrato(base, 131, 65535, 5, cambios)
                self.variante(base, 'salto135', 0x3143, 135, cambios=cambios)
        self.retomar_recursos(base)

    # B579: el salto alternativo devuelve el control y conserva las cenizas para la solución.
    def rama_salto_lago(self, base):
        semilla = list(self.semilla_menu)
        d = bytearray(base)
        self.retomar_recursos(d)
        puntos = int.from_bytes(d[0x35C7:0x35C9], 'little')
        self.elegir_objeto(d, (6,), 135)
        self.esperar(d, 512)
        self.exigir_objeto(d, 131)
        if not d[0x3594] or int.from_bytes(d[0x35C7:0x35C9], 'little') != (puntos + 2) & 65535:
            raise ErrorAnalisis('El salto no recupera control con los dos puntos originales.')
        self.hito_borde(d, 'lago_salto135')
        self.semilla_menu[:] = semilla
        self.retomar_recursos(base)

    # Continúa desde la petición81 contrastada; no concede131 ni simula el rescate previo.
    def ejecutar(self):
        origen = self.raiz / 'apps/juego/trabajo/comparacion_morsa/rescate_solicitud81'
        if not origen.with_suffix('.ds.bin').exists():
            raise ErrorAnalisis('Falta la solicitud81 real: ejecuta comparar:morsa.')
        d = bytearray(origen.with_suffix('.ds.bin').read_bytes())
        self.semilla_menu[:] = json.loads(
            origen.with_suffix('.semilla.json').read_text(encoding='utf-8')
        )
        self.destino = self.raiz / 'apps/juego/trabajo/comparacion_lago_helado'
        self.destino.mkdir(parents=True, exist_ok=True)
        self.resumen = {}
        self.variantes = {}
        self.retomar_recursos(d)
        self.cambiar(d, self.semilla_menu, 81)
        self.hito_borde(d, 'lago_entrada')
        self.esperar(d, 256)
        self.hito_borde(d, 'lago_control')
        self.auditar(d, 'entrada al lago helado')
        self.variantes_lago(d)
        self.ruta_sin_salida(d, 110, 180)
        self.hito_borde(d, 'lago_ante_cenizas')
        self.auditar(d, 'ante el lago')
        self.rama_salto_lago(d)
        self.elegir_objeto(d, (2, 5), 131)
        self.hito_borde(d, 'lago_cenizas')
        salida = None
        for paso in range(1, 1537):
            salida = self.andar(d, self.semilla_menu, 65535, 1)
            if salida is not None:
                break
            if paso in (64, 256, 512, 1024):
                self.hito_borde(d, f'lago_criatura_{paso}')
        if salida != 82:
            self.hito_borde(d, 'lago_espera_final')
            raise ErrorAnalisis(f'La criatura no solicita82: {salida}.')
        (self.destino / 'lago_solicitud82.ds.bin').write_bytes(d)
        (self.destino / 'lago_solicitud82.semilla.json').write_text(
            json.dumps(list(self.semilla_menu)), encoding='utf-8'
        )
        self.conservar_contexto(self.destino, 'lago_solicitud82')
        informe = {
            'resultado': 'coinciden',
            'casos_y_pasos': sum(self.resumen.values()),
            'grupos': self.resumen,
            'capturas': self.capturas,
            'estados_auditados': self.estados_auditados,
            'alcance': 'Llegada81 real, cenizas, criatura y solicitud82. Palacio ySDL pendientes.',
        }
        (self.destino / 'resultado.json').write_text(
            json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8'
        )
        print(
            f'Lago helado: {informe["casos_y_pasos"]} casos/pasos y {self.capturas} composiciones coincidentes.'
        )


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Contrasta el lago helado desde el rescate real.')
    parser.add_argument('--biblioteca', type=Path, required=True)
    parser.add_argument('--muestra', action='store_true')
    opciones = parser.parse_args()
    try:
        ComparacionLagoHelado(
            Path(__file__).resolve().parents[4], opciones.biblioteca, opciones.muestra
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        parser.exit(1, f'Error: {error}\n')
