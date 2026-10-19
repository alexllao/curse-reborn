"""Contrasta el regreso real del valle a Benn, vestido y probador hasta la solicitud74."""

import argparse
import ctypes
import json
from pathlib import Path

from comparar_plataforma_valle import ComparacionPlataformaValle, ErrorAnalisis
from comparar_calles import ComparacionCalles
from comparar_objetos import palabra


class ComparacionRegresoBenn(ComparacionPlataformaValle):
    # Aporta explícitamente el byte externo del probador y el banco independiente5800.
    def __init__(self, raiz, biblioteca, muestra):
        super().__init__(raiz, biblioteca, muestra)
        self.fase = ctypes.c_ubyte(0)
        self.nativa.avanzar_probador_bancos.argtypes = (
            self.nativa.avanzar_jugabilidad_bancos.argtypes + [ctypes.POINTER(ctypes.c_ubyte)]
        )
        self.nativa.avanzar_probador_bancos.restype = ctypes.c_int
        self.nativa.avanzar_jugabilidad = self.avanzar_con_fase

    # Usa la misma ABI del bucle común y conserva la fase en propiedad de esta comparación.
    def avanzar_con_fase(self, *argumentos):
        return self.nativa.avanzar_probador_bancos(
            *argumentos, self.referencia_modelo, ctypes.byref(self.fase)
        )

    # CS:1BA2 no pertenece al bloque DS y también debe coincidir después de cada paso.
    def paso(self, datos, semilla, direccion=65535):
        self.maquina.mem_write(0x1BA2, bytes([self.fase.value]))
        self.maquina.ctl_flush_tb()
        salida = super().paso(datos, semilla, direccion)
        if self.maquina.mem_read(0x1BA2, 1)[0] != self.fase.value:
            raise ErrorAnalisis('El regreso al probador altera su fase externa de forma distinta.')
        if getattr(self, 'rastrear_vestido', False):
            self.fotogramas_vestido.add((datos[0x358C], datos[0x2C2C], self.fase.value))
        return salida

    # Continúa la partida real con124, sin asignar79 ni los avances argumentales del vestido.
    def ejecutar(self):
        origen = (
            self.raiz / 'apps/juego/trabajo/comparacion_plataforma_valle/plataforma_regreso_campo'
        )
        if not origen.with_suffix('.ds.bin').exists():
            ComparacionPlataformaValle(self.raiz, self.biblioteca, True).ejecutar()
        d = bytearray(origen.with_suffix('.ds.bin').read_bytes())
        self.semilla_menu[:] = json.loads(
            origen.with_suffix('.semilla.json').read_text(encoding='utf-8')
        )
        self.destino = self.raiz / 'apps/juego/trabajo/comparacion_regreso_benn'
        self.destino.mkdir(parents=True, exist_ok=True)
        self.resumen = {}
        self.retomar_recursos(d)
        if palabra(d, 0x8082) != 16 or not d[0x35C9] & 2:
            raise ErrorAnalisis('La continuación no parte del regreso real al campo.')
        self.exigir_objeto(d, 124)
        self.auditar(d, 'campo después del valle')
        self.ruta(d, ((65, 175), (65, 140), (95, 130), (110, 110), (175, 94), (220, 60)), 26)
        self.auditar(d, 'entrada de la población con ventilador')
        self.ruta(d, ((140, 185), (110, 160), (105, 140)), 43)
        self.auditar(d, 'fuente después del valle')
        self.ruta(d, ((260, 180), (310, 100), (319, 70)), 29)
        self.ruta(d, ((300, 170), (380, 150), (380, 120)), 35)
        self.hito_borde(d, 'benn_regreso')
        self.auditar(d, 'Benn después del valle')
        self.ruta_sin_salida(d, 128, 145)
        self.hito_borde(d, 'benn_frente_vendedor')
        self.auditar(d, 'Benn ante el vendedor')
        self.elegir_objeto(d, (2, 6), 76)
        self.elegir_objeto(d, (), 78)
        self.esperar(d, 256)
        self.hito_borde(d, 'benn_pago_real')
        self.auditar(d, 'Benn después del pago real')
        self.elegir_objeto(d, (1,), 79)
        self.esperar(d, 192)
        self.exigir_objeto(d, 79)
        self.hito_borde(d, 'benn_vestido_real')
        self.auditar(d, 'Benn con vestido recogido')
        self.ruta(d, ((140, 160), (100, 155), (77, 135)), 36)
        self.hito_borde(d, 'probador_regreso_valle')
        self.fotogramas_vestido = set()
        self.rastrear_vestido = True
        self.elegir_objeto(d, (2, 4), 79)
        self.esperar(d, 512)
        self.rastrear_vestido = False
        if not any(modelo == 19 for modelo, _, _ in self.fotogramas_vestido) or not d[0x3594]:
            raise ErrorAnalisis('El vestido real no pasa por CHANGE19 y devuelve el control.')
        self.exigir_objeto(d, 79, False)
        self.exigir_objeto(d, 124)
        self.hito_borde(d, 'probador_vestido_real')
        self.auditar(d, 'probador con puerta posterior')
        encontrada = None
        for x, y in ((160, 160), (120, 142)):
            encontrada = ComparacionCalles.ir(self, d, self.semilla_menu, x, y)
            if encontrada is not None:
                break
        if encontrada != 74:
            raise ErrorAnalisis(f'El vestido real no alcanza la puerta74: {encontrada}.')
        self.hito_borde(d, 'probador_solicitud74')
        informe = {
            'resultado': 'coinciden',
            'casos_y_pasos': sum(self.resumen.values()),
            'grupos': self.resumen,
            'capturas': self.capturas,
            'estados_auditados': self.estados_auditados,
            'fotogramas_vestido': sorted(self.fotogramas_vestido),
            'alcance': 'Regreso real después del ventilador, pago aBenn, recogida79 y vestido enCHANGE hasta solicitar74. No cargaSNOWAST4/43 ni acredita sus puzles o SDL de esta continuación.',
        }
        (self.destino / 'resultado.json').write_text(
            json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8'
        )
        print(
            f'Regreso aBenn: {informe["casos_y_pasos"]} casos/pasos y {self.capturas} composiciones coincidentes.'
        )


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Contrasta la vuelta real aBenn tras el valle.')
    parser.add_argument('--biblioteca', type=Path, required=True)
    parser.add_argument('--muestra', action='store_true')
    opciones = parser.parse_args()
    try:
        ComparacionRegresoBenn(
            Path(__file__).resolve().parents[3], opciones.biblioteca, opciones.muestra
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        parser.exit(1, f'Error: {error}\n')
