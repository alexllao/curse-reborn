"""Contrasta la llegada al palacio, errores y secuencia de cuatro carámbanos hasta83."""

import argparse
import ctypes
import json
from pathlib import Path

from comparar_lago_helado import ComparacionLagoHelado, ErrorAnalisis
from comparar_objetos import guardar, palabra
from comparar_calles import ComparacionCalles


class ComparacionEntradaPalacio(ComparacionLagoHelado):
    # CF34 recicla cuatro grupos; los botones conservan los bits ajenos y canales no escritos.
    def variantes_palacio(self, base):
        funcion = self.nativa.procesar_rutina_nieve
        funcion.argtypes = [ctypes.POINTER(ctypes.c_ubyte)]
        funcion.restype = ctypes.c_int
        for x in (0, 599, 600, 601, 32767, 32768, 65535):
            d = bytearray(base)
            guardar(d, 0x2D98, x)
            for i in range(4):
                for p in (0x2D45, 0x2D4A, 0x2D4F):
                    d[p + i * 33] = 0x7B
            self.disponer(d, self.geometria, self.semilla_menu)
            self.ejecutar_original(0xCF34)
            esperado = bytes(self.maquina.mem_read(self.segmento, 65536))
            if not funcion((ctypes.c_ubyte * 65536).from_buffer(d)) or bytes(d) != esperado:
                raise ErrorAnalisis(f'CF34 distinto para{x}.')
        self.resumen['rutina_palacio'] = 7
        for bandera in (*range(16), 127, 255):
            for id in range(136, 140):
                cambios = [(0x35CA, bandera, True)] + [
                    (p, 0xCAFE, False) for p in (0xA0, 0xA2, 0xA4, 0xAA, 0xAC, 0xAE, 0x0AE1)
                ]
                self.contrato(base, id, 65535, 2, cambios)
        self.retomar_recursos(base)

    # Ejecuta una pulsación errónea desde la partida real y restaura su rama sin modificar el recorrido.
    def rama_error(self, base):
        d = bytearray(base)
        semilla = list(self.semilla_menu)
        self.retomar_recursos(d)
        self.ruta_sin_salida(d, 234, 145)
        self.elegir_objeto(d, (2, 2), 139)
        self.esperar(d, 192)
        if d[0x35CA] & 6 or d[0x2C6D] == 0:
            raise ErrorAnalisis('El error no conserva la puerta cerrada y el estado cero.')
        self.hito_borde(d, 'palacio_error_139')
        self.auditar(d, 'despues de carambano incorrecto')
        self.semilla_menu[:] = semilla
        self.retomar_recursos(base)

    # Usa la llegada82 real, no una partida preparada delante de una puerta ya abierta.
    def ejecutar(self):
        origen = self.raiz / 'apps/juego/trabajo/comparacion_lago_helado/lago_solicitud82'
        d = bytearray(origen.with_suffix('.ds.bin').read_bytes())
        self.semilla_menu[:] = json.loads(
            origen.with_suffix('.semilla.json').read_text(encoding='utf-8')
        )
        self.destino = self.raiz / 'apps/juego/trabajo/comparacion_entrada_palacio'
        self.destino.mkdir(parents=True, exist_ok=True)
        self.resumen = {}
        self.retomar_recursos(d)
        self.cambiar(d, self.semilla_menu, 82)
        self.hito_borde(d, 'palacio_entrada')
        self.esperar(d, 256)
        self.hito_borde(d, 'palacio_control')
        self.auditar(d, 'entrada del palacio')
        self.variantes_palacio(d)
        self.ruta_sin_salida(d, 240, 145)
        self.hito_borde(d, 'palacio_ante_botones')
        self.auditar(d, 'ante carambanos')
        self.rama_error(d)
        for id in range(136, 140):
            entidad = 0x2CAF + (id - 136) * 33
            self.ruta_sin_salida(d, palabra(d, entidad + 2) + 7, 145)
            self.elegir_objeto(d, (2, 2), id)
            self.esperar(d, 192)
            self.hito_borde(d, f'palacio_boton_{id}')
            self.auditar(d, f'despues de carambano{id}')
        if d[0x2C6D] != 0 or (d[0x35CA] & 6) != 6:
            raise ErrorAnalisis('La secuencia no abre la puerta original.')
        salida = ComparacionCalles.ir(self, d, self.semilla_menu, 145, 100)
        if salida != 83:
            raise ErrorAnalisis(f'La puerta abierta no permite83: {salida}.')
        (self.destino / 'palacio_solicitud83.ds.bin').write_bytes(d)
        (self.destino / 'palacio_solicitud83.semilla.json').write_text(
            json.dumps(list(self.semilla_menu)), encoding='utf-8'
        )
        informe = {
            'resultado': 'coinciden',
            'casos_y_pasos': sum(self.resumen.values()),
            'grupos': self.resumen,
            'capturas': self.capturas,
            'estados_auditados': self.estados_auditados,
            'alcance': 'Llegada82, error y secuencia136/137/138/139, apertura y solicitud83. Interior ySDL pendientes.',
        }
        (self.destino / 'resultado.json').write_text(
            json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8'
        )
        print(
            f'Entrada palacio: {informe["casos_y_pasos"]} casos/pasos y {self.capturas} composiciones coincidentes.'
        )


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Contrasta la puerta del palacio de hielo.')
    parser.add_argument('--biblioteca', type=Path, required=True)
    parser.add_argument('--muestra', action='store_true')
    opciones = parser.parse_args()
    try:
        ComparacionEntradaPalacio(
            Path(__file__).resolve().parents[3], opciones.biblioteca, opciones.muestra
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        parser.exit(1, f'Error: {error}\n')
