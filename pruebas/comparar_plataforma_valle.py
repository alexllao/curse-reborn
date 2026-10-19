"""Contrasta bandeja115 y ELECTRO39 hasta volver al campo con el ventilador124."""

import argparse
import ctypes
import json
from pathlib import Path

from comparar_puente_barco import ComparacionPuenteBarco, ErrorAnalisis
from comparar_objetos import guardar, palabra


class ComparacionPlataformaValle(ComparacionPuenteBarco):
    # CAE1–CBCF: barrera, SAR negativo, lados inclinados, caída y límite final del ventilador.
    def variantes_rutina_electro(self, base):
        funcion = self.nativa.procesar_ramas_valle
        funcion.argtypes = [ctypes.POINTER(ctypes.c_ubyte)]
        funcion.restype = ctypes.c_int
        numero = 0
        for x in (0, 71, 72, 105, 159, 160, 177, 230, 32767, 32768):
            for y in (0, 101, 102, 126, 127, 199):
                for barrera in (-71, -70, 0):
                    for control in (0, 1):
                        for ventilador in (0, 1):
                            d = bytearray(base)
                            guardar(d, 0x2C2D, x)
                            guardar(d, 0x2C2F, y)
                            guardar(d, 0x2C71, barrera)
                            guardar(d, 0x18F7, 0xBEEF)
                            d[0x3594] = control
                            d[0x2D33] = ventilador
                            self.disponer(d, self.geometria, self.semilla_menu)
                            self.ejecutar_original(0xCAE1)
                            esperado = bytes(self.maquina.mem_read(self.segmento, 65536))
                            if (
                                not funcion((ctypes.c_ubyte * 65536).from_buffer(d))
                                or bytes(d) != esperado
                            ):
                                diferencias = [
                                    f'{i:04X}' for i in range(65536) if d[i] != esperado[i]
                                ][:8]
                                raise ErrorAnalisis(f'CAE1 variante{numero}: {diferencias}.')
                            numero += 1
        self.resumen['rutina_ELECTRO_aislada'] = numero
        self.retomar_recursos(base)

    # AFBE–B16D: presencia/ausencia de objetos, actores por fase, barrera, puntos y campos ajenos.
    def variantes_acciones_electro(self, base):
        self.variantes = {}
        for id, verbo in ((120, 5), (119, 2), (122, 2), (121, 5), (115, 5)):
            for actor in (0, 1, 3):
                for barrera in (0, 1, 65456):
                    for presente in (False, True):
                        for puntos in (0, 65535):
                            cambios = [
                                (0x2CD0, actor, True),
                                (0x2CF1, actor, True),
                                (0x2C71, barrera, False),
                                (0x35C7, puntos, False),
                            ]
                            cambios += [
                                (p, 0xCAFE, False)
                                for p in (0xA0, 0xA2, 0xA4, 0xAA, 0xAC, 0xAE, 0x0AE1)
                            ]
                            cambios += [
                                (0x5225 + i * 2, id if presente and i == 0 else 65535, False)
                                for i in range(10)
                            ]
                            self.variante(
                                base, f'accion{id}', 0x3B53, id, verbo=verbo, cambios=cambios
                            )
        for id, previo in ((123, 3), (3, 123)):
            for objetos in ((), (3, 99), (3,) * 10):
                for banderas in (0, 2, 255):
                    for puntos in (0, 65535):
                        cambios = [(0x35C9, banderas, True), (0x35C7, puntos, False)]
                        cambios += [
                            (0x5225 + i * 2, objetos[i] if i < len(objetos) else 65535, False)
                            for i in range(10)
                        ]
                        self.contrato(base, id, previo, 0, cambios)
        self.retomar_recursos(base)

    # 563A: contrasta capacidad, iconos y puntos para115/124 sobre copias del estado real adecuado.
    def variantes_recogida_plataforma(self, base, id):
        for ocupadas in (0, 9, 10):
            for icono in (0, 0x1F, 0x22):
                for puntos in (0, 65535):
                    cambios = [(0x524E + id * 2, icono, True), (0x35C7, puntos, False)]
                    cambios += [
                        (0x5225 + i * 2, 3 if i < ocupadas else 65535, False) for i in range(10)
                    ]
                    self.variante(base, f'recoger{id}', 0x563A, id, cambios=cambios)
        self.retomar_recursos(base)

    # Verifica una adquisición/consumo de inventario sin tocar la partida.
    def exigir_objeto(self, d, id, presente=True):
        if (id in [palabra(d, 0x5225 + i * 2) for i in range(10)]) != presente:
            raise ErrorAnalisis(f'Inventario no conserva el contrato de{id}.')

    # CAE1/B2E4: cae con clic real, usa FALL6 y recupera control sin recargar ni consumir objetos.
    def rama_caida(self, base, x, nombre):
        semilla = list(self.semilla_menu)
        d = bytearray(base)
        self.retomar_recursos(d)
        self.clic(d, x, 160)
        caida = False
        for _ in range(192):
            self.esperar(d, 1)
            caida = caida or d[0x358C] == 6
        self.esperar(d, 128)
        if (
            not caida
            or d[0x358C] != 255
            or not d[0x3594]
            or palabra(d, 0x8082) != 39
            or d[0x5225:0x5239] != base[0x5225:0x5239]
            or palabra(d, 0x35C7) != palabra(base, 0x35C7)
        ):
            raise ErrorAnalisis('La caída no recupera a Brad sin cambiar inventario y puntos.')
        self.hito_borde(d, nombre)
        self.resumen['caidas_recuperadas'] = self.resumen.get('caidas_recuperadas', 0) + 1
        self.semilla_menu[:] = semilla
        self.retomar_recursos(base)

    # Continúa trapo→bandeja→puerta68→limpieza→botón→canicas/bandeja→ventilador→salida69.
    def ejecutar(self):
        origen = self.raiz / 'trabajo/comparacion_puente_barco/barco_regreso_celeste'
        if not origen.with_suffix('.ds.bin').exists():
            ComparacionPuenteBarco(self.raiz, self.biblioteca, True).ejecutar()
        d = bytearray(origen.with_suffix('.ds.bin').read_bytes())
        self.semilla_menu[:] = json.loads(
            origen.with_suffix('.semilla.json').read_text(encoding='utf-8')
        )
        self.destino = self.raiz / 'trabajo/comparacion_plataforma_valle'
        self.destino.mkdir(parents=True, exist_ok=True)
        self.resumen, self.variantes = {}, {}
        self.retomar_recursos(d)
        self.ruta(d, ((330, 170), (100, 198)), 61)
        for punto in ((550, 140), (450, 145), (300, 160), (140, 175), (94, 165)):
            self.ruta_sin_salida(d, *punto)
        self.variantes_recogida_plataforma(d, 115)
        self.elegir_objeto(d, (1,), 115)
        self.esperar(d, 192)
        self.exigir_objeto(d, 115)
        self.hito_borde(d, 'plataforma_bandeja_recogida')
        self.ruta(
            d,
            (
                (140, 175),
                (300, 160),
                (450, 145),
                (550, 140),
                (620, 145),
                (704, 145),
                (850, 190),
                (850, 199),
            ),
            57,
        )
        if d[0x2C6D] != 1:
            raise ErrorAnalisis('Los cuatro objetos no abren la puerta original deDINO.')
        self.hito_borde(d, 'plataforma_puerta_dino')
        self.ruta(d, ((250, 170), (340, 170), (320, 140), (295, 130)), 68)
        self.hito_borde(d, 'plataforma_entrada')
        self.variantes_rutina_electro(d)
        self.variantes_acciones_electro(d)
        self.auditar(d, 'ELECTRO antes de limpiar')
        puntos = palabra(d, 0x35C7)
        self.elegir_objeto(d, (2, 5), 120)
        self.esperar(d, 256)
        self.exigir_objeto(d, 120, False)
        if d[0x2CD0] != 1:
            raise ErrorAnalisis('El detergente no prepara la pared original.')
        self.hito_borde(d, 'plataforma_detergente')
        self.menu(d, (2, 2))
        self.seleccionar(d, 0)
        self.elegir_objeto(d, (), 119)
        self.esperar(d, 256)
        self.exigir_objeto(d, 119, False)
        if d[0x2CAF] != 1:
            raise ErrorAnalisis('El trapo no revela el botón122.')
        self.hito_borde(d, 'plataforma_boton_descubierto')
        self.auditar(d, 'ELECTRO con botón descubierto')
        self.elegir_objeto(d, (2, 2), 122)
        self.esperar(d, 512)
        self.hito_borde(d, 'plataforma_barrera_abierta')
        self.rama_caida(d, 100, 'plataforma_caida_izquierda')
        self.elegir_objeto(d, (2, 5), 121)
        self.esperar(d, 256)
        self.exigir_objeto(d, 121, False)
        if d[0x2CF1] != 1:
            raise ErrorAnalisis('Las canicas no preparan el cruce.')
        self.hito_borde(d, 'plataforma_canicas')
        self.elegir_objeto(d, (2, 5), 115)
        self.esperar(d, 1024)
        self.exigir_objeto(d, 115, False)
        self.hito_borde(d, 'plataforma_cruzada')
        self.rama_caida(d, 180, 'plataforma_caida_derecha')
        for punto in ((245, 180), (264, 176)):
            self.ruta_sin_salida(d, *punto)
        self.auditar(d, 'ELECTRO junto al ventilador')
        self.variantes_recogida_plataforma(d, 124)
        self.elegir_objeto(d, (1,), 124)
        self.esperar(d, 192)
        self.exigir_objeto(d, 124)
        self.hito_borde(d, 'plataforma_ventilador')
        self.menu(d, (2, 0))
        self.seleccionar(d, 0)
        self.elegir_objeto(d, (), 3)
        self.elegir_objeto(d, (), 123)
        salida = self.andar(d, self.semilla_menu, 65535, 256)
        if salida != 69 or not d[0x35C9] & 2 or palabra(d, 0x35C7) != (puntos + 30) & 65535:
            raise ErrorAnalisis(
                'La salida final no conserva69, bandera2 y seis acciones de cinco puntos.'
            )
        self.exigir_objeto(d, 3, False)
        self.exigir_objeto(d, 99, False)
        self.cambiar(d, self.semilla_menu, 69)
        self.esperar(d, 128)
        if palabra(d, 0x8082) != 16:
            raise ErrorAnalisis('La salida69 no vuelve al campo.')
        self.exigir_objeto(d, 124)
        self.hito_borde(d, 'plataforma_regreso_campo')
        informe = {
            'resultado': 'coinciden',
            'casos_y_pasos': sum(self.resumen.values()),
            'grupos': self.resumen,
            'variantes': self.variantes,
            'variantes_puerta': 36,
            'capturas': self.capturas,
            'estados_auditados': self.estados_auditados,
            'alcance': 'Bandeja115, entrada68, seis acciones deELECTRO, ventilador124 y regreso69 al campo en la misma partida. Incluye caídas reales a ambos lados y recuperación con FALL6 sin consumir inventario. Otros estados y menús posteriores del campo pendientes.',
            'limites': 'F6 con cabecera fuera del banco se normaliza a0x0 en C++ porque no se dibuja antes de cargar; coincide con la memoria cero de esta referencia DOS, no emula lecturas arbitrarias de RAM de otra asignación.',
        }
        (self.destino / 'resultado.json').write_text(
            json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8'
        )
        print(
            f'Plataforma del valle: {informe["casos_y_pasos"]} casos/pasos y {self.capturas} composiciones coincidentes.'
        )


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Contrasta la bandeja y el recorrido deELECTRO39.')
    parser.add_argument('--biblioteca', type=Path, required=True)
    parser.add_argument('--muestra', action='store_true')
    opciones = parser.parse_args()
    try:
        ComparacionPlataformaValle(
            Path(__file__).resolve().parents[1], opciones.biblioteca, opciones.muestra
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        parser.exit(1, f'Error: {error}\n')
