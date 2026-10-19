"""Contrasta DINO y el pelo del valle; conserva como pendientes las rutas posteriores."""

import argparse
import ctypes
import json
from pathlib import Path

from comparar_ramas_valle import ComparacionRamasValle, ErrorAnalisis
from comparar_objetos import guardar, palabra


class ComparacionDinosaurio(ComparacionRamasValle):
    # Contrasta C5E0 con inventarios y límites firmados, sin conceder objetos a la partida.
    def variantes_dinosaurio(self, base):
        funcion = self.nativa.procesar_ramas_valle
        funcion.argtypes = [ctypes.POINTER(ctypes.c_ubyte)]
        funcion.restype = ctypes.c_int
        numero = 0
        for mascara in range(16):
            for cantidad in (8, 9, 128, 255):
                for activo in (0, 1, 3):
                    for global_x in (249, 250, 251, 32767, 32768, 65535):
                        d = bytearray(base)
                        guardar(d, 0x8082, 32)
                        guardar(d, 0x3591, 256 if numero % 2 else 0)
                        guardar(d, 0x2C2D, global_x - palabra(d, 0x3591))
                        guardar(d, 0x2C2F, (0, 100, 65535, 32910, 32911)[numero % 5])
                        d[0x2C4C] = numero % 2
                        d[0x2C6D] = activo
                        d[0x2C8E] = (numero // 2) % 2
                        d[0x3597] = cantidad
                        guardar(d, 0x2C6F, (0, 65535)[numero % 2])
                        guardar(d, 0x2C71, (0, 100, 65535, 32910, 32911)[numero % 5])
                        guardar(d, 0x2C73, 31 if numero % 2 else 32)
                        guardar(d, 0x2C75, 48)
                        for i in range(10):
                            guardar(
                                d,
                                0x5225 + i * 2,
                                (121, 119, 120, 115)[i] if i < 4 and mascara & (1 << i) else 65535,
                            )
                        self.disponer(d, self.geometria, self.semilla_menu)
                        self.ejecutar_original(0xC5E0)
                        esperado = bytes(self.maquina.mem_read(self.segmento, 65536))
                        if (
                            not funcion((ctypes.c_ubyte * 65536).from_buffer(d))
                            or bytes(d) != esperado
                        ):
                            diferencias = [f'{i:04X}' for i in range(65536) if d[i] != esperado[i]][
                                :8
                            ]
                            raise ErrorAnalisis(f'C5E0 variante{numero} distinta: {diferencias}.')
                        numero += 1
        self.resumen['rutina_dinosaurio'] = numero
        self.retomar_recursos(base)

    # Compara A81A en ambos órdenes, posiciones límite y puntuación envolvente.
    def variantes_estornudo(self, base):
        for horizontal in (-1, 0, 1, 40):
            for vertical in (-1, 0, 1, 40):
                for camara in (0, 256):
                    for puntos in (0, 65535):
                        for id, previo in ((102, 110), (110, 102)):
                            d = bytearray(base)
                            guardar(
                                d, 0x2C2D, palabra(d, 0x2C90) - palabra(d, 0x2C31) + 6 + horizontal
                            )
                            guardar(
                                d,
                                0x2C2F,
                                palabra(d, 0x2C92)
                                + palabra(d, 0x2C96)
                                - palabra(d, 0x2C33)
                                + vertical,
                            )
                            guardar(d, 0x3591, camara)
                            guardar(d, 0x35C7, puntos)
                            guardar(d, 0x5225, 110)
                            guardar(d, 0xB60C, 0x3B53)
                            guardar(d, 0xB620, id + 26)
                            guardar(d, 0xB634, 1)
                            guardar(d, 0xB638, previo)
                            self.seleccionar(d, 0)
        # Las comparaciones tras SUB conservan OF: se varían operandos, sin un recorrido enorme de Brad.
        for x_nariz in (0, 32767, 32768, 65535):
            for altura in (0, 32767, 32768, 65535):
                d = bytearray(base)
                guardar(d, 0x2C90, x_nariz)
                guardar(d, 0x2C96, altura)
                guardar(d, 0x5225, 110)
                guardar(d, 0xB60C, 0x3B53)
                guardar(d, 0xB620, 128)
                guardar(d, 0xB634, 1)
                guardar(d, 0xB638, 110)
                self.seleccionar(d, 0)
        self.retomar_recursos(base)

    # Contrasta el saludo372A con actor inactivo y distancia firmada a ambos lados del límite64.
    def variantes_saludo(self, base):
        for activo in (0, 1, 3):
            for distancia in (-32768, -65, -64, -1, 0, 1, 64, 65, 32767):
                for camara in (0, 256):
                    self.variante(
                        base,
                        'saludo',
                        0x32DE,
                        1,
                        cambios=(
                            (0x2C4C, activo, True),
                            (0x2C4E, palabra(base, 0x2C2D) + distancia, False),
                            (0x3591, camara, False),
                        ),
                    )
        self.retomar_recursos(base)

    # Usa el mismo clic global contrastado de las calles, sin forzar posiciones para pasar el terreno.
    def ruta_sin_salida(self, d, x, y):
        from comparar_calles import ComparacionCalles

        if ComparacionCalles.ir(self, d, self.semilla_menu, x, y) is not None:
            raise ErrorAnalisis('La aproximación al monstruo abandona DINO.')

    # Retoma el regreso POST real y recoge110 antes de volver a REMOTE y CELESTE.
    def ejecutar(self):
        origen = self.raiz / 'apps/juego/trabajo/comparacion_ramas_valle/post_regreso_rama'
        if not origen.with_suffix('.ds.bin').exists():
            ComparacionRamasValle(self.raiz, self.biblioteca, True).ejecutar()
        d = bytearray(origen.with_suffix('.ds.bin').read_bytes())
        self.semilla_menu[:] = json.loads(
            origen.with_suffix('.semilla.json').read_text(encoding='utf-8')
        )
        self.destino = self.raiz / 'apps/juego/trabajo/comparacion_dinosaurio'
        self.destino.mkdir(parents=True, exist_ok=True)
        self.resumen = {}
        self.variantes = {}
        self.retomar_recursos(d)
        self.variantes_objetos(d, (110,))
        for x, y in ((550, 100), (550, 140)):
            self.clic(d, x - palabra(d, 0x3591), y)
            self.esperar(d, 128)
        self.elegir_objeto(d, (1,), 110)
        self.esperar(d, 128)
        if 110 not in [palabra(d, 0x5225 + i * 2) for i in range(10)]:
            raise ErrorAnalisis('El pelo110 no llega al inventario.')
        self.hito_borde(d, 'pelo_recogido')
        self.auditar(d, 'POST tras recoger el pelo')
        self.ruta(d, ((550, 140), (620, 145), (704, 145), (850, 190), (850, 199)), 57)
        self.hito_borde(d, 'dinosaurio_desde_post')
        for x, y in ((250, 170), (340, 140)):
            self.ruta_sin_salida(d, x, y)
        self.auditar(d, 'DINO antes del estornudo')
        self.variantes_dinosaurio(d)
        self.variantes_saludo(d)
        self.variantes_estornudo(d)
        self.variantes_objetos(d, (99, 100, 101, 109))
        self.elegir_objeto(d, (2, 1), 110)
        self.elegir_objeto(d, (), 102)
        self.esperar(d, 256)
        if 110 in [palabra(d, 0x5225 + i * 2) for i in range(10)] or d[0x2C8F] != 29:
            raise ErrorAnalisis('El estornudo no retira el pelo y conserva su fotograma final.')
        self.hito_borde(d, 'dinosaurio_estornudo')
        self.auditar(d, 'DINO después del estornudo')
        for id in (109, 101):
            self.elegir_objeto(d, (1,), id)
            self.esperar(d, 128)
            if id not in [palabra(d, 0x5225 + i * 2) for i in range(10)]:
                raise ErrorAnalisis(f'La recogida{id} no llega al inventario.')
        self.hito_borde(d, 'dinosaurio_sello_boligrafo')
        self.auditar(d, 'DINO con sello y bolígrafo')
        self.ruta(d, ((400, 185), (250, 197), (250, 199)), 58)
        self.hito_borde(d, 'dinosaurio_regreso_post')
        self.auditar(d, 'POST con sello y bolígrafo')
        # El otro acceso mantiene su origen CELESTE real; rama independiente sin el inventario anterior.
        origen_celeste = self.raiz / 'apps/juego/trabajo/comparacion_ramas_valle/celeste_entrada'
        d = bytearray(origen_celeste.with_suffix('.ds.bin').read_bytes())
        self.semilla_menu[:] = json.loads(
            origen_celeste.with_suffix('.semilla.json').read_text(encoding='utf-8')
        )
        self.retomar_recursos(d)
        self.ruta(d, ((200, 134),), 54)
        self.hito_borde(d, 'dinosaurio_desde_celeste')
        self.auditar(d, 'DINO desde CELESTE')
        informe = {
            'resultado': 'coinciden',
            'casos_y_pasos': sum(self.resumen.values()),
            'grupos': self.resumen,
            'variantes_aisladas': self.variantes,
            'capturas': self.capturas,
            'estados_auditados': self.estados_auditados,
            'alcance': 'Ruta real pelo110, entrada57 aDINO, estornudo, sello109 y bolígrafo101, regreso58. Rama independiente54 desdeCELESTE. Variantes aisladas C5E0/A81A y recogidas99/100; demás rutas, puzles y SDL pendientes.',
        }
        (self.destino / 'resultado.json').write_text(
            json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8'
        )
        print(
            f'Dinosaurio: {informe["casos_y_pasos"]} casos/pasos y {self.capturas} composiciones coincidentes.'
        )


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Contrasta el acceso nativo a DINO y el pelo.')
    parser.add_argument('--biblioteca', type=Path, required=True)
    parser.add_argument('--muestra', action='store_true')
    opciones = parser.parse_args()
    try:
        ComparacionDinosaurio(
            Path(__file__).resolve().parents[3], opciones.biblioteca, opciones.muestra
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        parser.exit(1, f'Error: {error}\n')
