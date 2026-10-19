"""Contrasta calcetín100, combinación104, guardia y llegada aHOLD; su interior sigue pendiente."""

import argparse
import ctypes
import json
from pathlib import Path

from comparar_cueva_valle import ComparacionCuevaValle, ErrorAnalisis
from comparar_objetos import guardar, palabra


class ComparacionGuardiaBarco(ComparacionCuevaValle):
    # A952 no exige presencia100 ni éxito al insertar104; conserva107 y el tercer guion.
    def variantes_moneda(self, base):
        self.variantes = {}
        for id, previo in ((100, 107), (107, 100)):
            for objetos in ((), (100,), (104,), (100, 104), (3,) * 10, (3,) * 9 + (100,)):
                for camara in (0, 320):
                    for puntos in (0, 65535):
                        cambios = [
                            (0x3591, camara, False),
                            (0x35C7, puntos, False),
                            (0x0AE1, 0xCAFE, False),
                        ]
                        cambios += [
                            (0x5225 + i * 2, objetos[i] if i < len(objetos) else 65535, False)
                            for i in range(10)
                        ]
                        self.contrato(base, id, previo, 7, cambios)
        self.retomar_recursos(base)

    # A982 conserva la resta/negación de16 bits, umbral40 y campos diferidos ajenos.
    def variantes_guardia(self, base):
        for distancia in (-32768, -41, -40, -1, 0, 40, 41):
            for objetos in ((), (104,)):
                for puntos in (0, 65535):
                    cambios = [
                        (0x2C2D, (palabra(base, 0x2C4E) + distancia) & 65535, False),
                        (0x35C7, puntos, False),
                    ]
                    cambios += [
                        (0x5225 + i * 2, objetos[i] if i < len(objetos) else 65535, False)
                        for i in range(10)
                    ]
                    cambios += [(i, 0xCAFE, False) for i in (0xAC, 0xB6, 0xC0, 0xCA, 0xCC)]
                    self.variante(base, 'guardia', 0x2FCA, 104, cambios=cambios)
        self.retomar_recursos(base)

    # C8B1 aislada: verifica ambos tablones y ausencia de escritura cuando ninguno está en cuadro2.
    def variantes_proximidad_barco(self, base):
        funcion = self.nativa.procesar_ramas_valle
        funcion.argtypes = [ctypes.POINTER(ctypes.c_ubyte)]
        funcion.restype = ctypes.c_int
        for primero in (0, 1, 2, 255):
            for segundo in (0, 1, 2, 255):
                d = bytearray(base)
                guardar(d, 0x8082, 36)
                d[0x2C4D], d[0x2C6E] = primero, segundo
                for campo in (0x18F5, 0x18F7, 0x18F9, 0x18FB):
                    guardar(d, campo, 0xBEEF)
                self.disponer(d, self.geometria, self.semilla_menu)
                self.ejecutar_original(0xC8B1)
                esperado = bytes(self.maquina.mem_read(self.segmento, 65536))
                if not funcion((ctypes.c_ubyte * 65536).from_buffer(d)) or bytes(d) != esperado:
                    raise ErrorAnalisis('C8B1 no conserva las ventanas de los tablones.')
        self.resumen['proximidad_HOLD_aislada'] = 16
        self.retomar_recursos(base)

    # Sigue la partida con detergente y canicas, sin añadir el calcetín ni el arma al inventario.
    def ejecutar(self):
        origen = self.raiz / 'apps/juego/trabajo/comparacion_cueva_valle/cueva_regreso_dino'
        if not origen.with_suffix('.ds.bin').exists():
            ComparacionCuevaValle(self.raiz, self.biblioteca, True).ejecutar()
        d = bytearray(origen.with_suffix('.ds.bin').read_bytes())
        self.semilla_menu[:] = json.loads(
            origen.with_suffix('.semilla.json').read_text(encoding='utf-8')
        )
        self.destino = self.raiz / 'apps/juego/trabajo/comparacion_guardia_barco'
        self.destino.mkdir(parents=True, exist_ok=True)
        self.resumen = {}
        self.retomar_recursos(d)
        for punto in ((250, 170), (400, 170), (450, 115), (500, 115)):
            self.ruta_sin_salida(d, *punto)
        self.elegir_objeto(d, (1,), 100)
        self.esperar(d, 192)
        if 100 not in [palabra(d, 0x5225 + i * 2) for i in range(10)]:
            raise ErrorAnalisis('El calcetín100 no se ha recogido realmente.')
        self.hito_borde(d, 'calcetin_recogido')
        self.ruta(d, ((550, 150), (620, 120), (639, 110)), 53)
        self.hito_borde(d, 'barco_exterior')
        for punto in ((330, 170), (500, 180), (552, 163)):
            self.ruta_sin_salida(d, *punto)
        self.auditar(d, 'CELESTE junto al montón de monedas')
        self.variantes_moneda(d)
        puntos = palabra(d, 0x35C7)
        self.elegir_objeto(d, (2, 7), 100)
        self.elegir_objeto(d, (), 107)
        self.esperar(d, 256)
        inventario = [palabra(d, 0x5225 + i * 2) for i in range(10)]
        if 104 not in inventario or 100 in inventario or palabra(d, 0x35C7) != (puntos + 5) & 65535:
            raise ErrorAnalisis('La combinación real no obtiene104 y suma5 puntos.')
        self.hito_borde(d, 'calcetin_con_moneda')
        for punto in ((500, 180), (360, 170)):
            self.ruta_sin_salida(d, *punto)
        self.auditar(d, 'CELESTE antes de golpear al guardia')
        self.variantes_guardia(d)
        puntos = palabra(d, 0x35C7)
        self.elegir_objeto(d, (5,), 104)
        self.esperar(d, 768)
        if (
            d[0x2C4D] != 19
            or 104 in [palabra(d, 0x5225 + i * 2) for i in range(10)]
            or palabra(d, 0x35C7) != (puntos + 5) & 65535
        ):
            raise ErrorAnalisis('El golpe no consume104 y deja el guardia en cuadro19.')
        self.hito_borde(d, 'guardia_apartado')
        self.auditar(d, 'CELESTE con la entrada del barco libre')
        self.variantes_proximidad_barco(d)
        self.ruta(d, ((340, 140),), 62)
        if palabra(d, 0x8082) != 36:
            raise ErrorAnalisis('La salida62 no cargaHOLD36.')
        self.hito_borde(d, 'barco_interior_entrada')
        informe = {
            'resultado': 'coinciden',
            'casos_y_pasos': sum(self.resumen.values()),
            'grupos': self.resumen,
            'variantes_moneda': 48,
            'variantes_guardia': 28,
            'capturas': self.capturas,
            'estados_auditados': self.estados_auditados,
            'alcance': 'Calcetín real, combinación con107, golpe al guardia y llegada62 aHOLD. No acredita puzle de tablones/trapo, otras rutas del valle ni SDL.',
        }
        (self.destino / 'resultado.json').write_text(
            json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8'
        )
        print(
            f'Guardia del barco: {informe["casos_y_pasos"]} casos/pasos y {self.capturas} composiciones coincidentes.'
        )


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Contrasta calcetín, moneda y guardia del barco.')
    parser.add_argument('--biblioteca', type=Path, required=True)
    parser.add_argument('--muestra', action='store_true')
    opciones = parser.parse_args()
    try:
        ComparacionGuardiaBarco(
            Path(__file__).resolve().parents[3], opciones.biblioteca, opciones.muestra
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        parser.exit(1, f'Error: {error}\n')
