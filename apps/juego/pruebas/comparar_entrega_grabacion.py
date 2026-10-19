"""Contrasta entrega de106, obtención del pase99 y petición de salida66; CLOUD todavía pendiente."""

import argparse
import json
from pathlib import Path

from comparar_grabacion_completa import ComparacionGrabacionCompleta, ErrorAnalisis
from comparar_objetos import palabra


class ComparacionEntregaGrabacion(ComparacionGrabacionCompleta):
    # Contrasta A8AE/A8DF: cinta vacía/grabada, fotograma4, capacidad, puntos y canales ajenos.
    def variantes_entrega(self, base):
        for previo in (105, 106):
            for fotograma in (0, 3, 4, 10):
                for activo in (0, 1, 3):
                    for objetos in ((), (105,), (106,), (99, 106), (3,) * 10, (3,) * 9 + (106,)):
                        for camara in (0, 256, 640):
                            for puntos in (0, 65535):
                                cambios = [
                                    (0x2C4C, activo, True),
                                    (0x2C4D, fotograma, True),
                                    (0x3591, camara, False),
                                    (0x35C7, puntos, False),
                                    (0xA2, 0xCAFE, False),
                                    (0xA4, 0xBEEF, False),
                                ]
                                cambios += [
                                    (
                                        0x5225 + i * 2,
                                        objetos[i] if i < len(objetos) else 65535,
                                        False,
                                    )
                                    for i in range(10)
                                ]
                                self.contrato(base, 98, previo, 6, cambios)
        self.retomar_recursos(base)

    # AC5E no retira el pase ni suma puntos; solo escribe A0 del actor diferido.
    def variantes_puerta(self, base):
        for id, previo in ((116, 99), (99, 116)):
            for objetos in ((), (99,), (3,) * 10, (3,) * 9 + (99,)):
                for puntos in (0, 65535):
                    for camara in (0, 256, 640):
                        for activo in (0, 1, 3):
                            cambios = [
                                (0x2CD0, activo, True),
                                (0x3591, camara, False),
                                (0x35C7, puntos, False),
                                (0xA2, 0xCAFE, False),
                                (0xA4, 0xBEEF, False),
                            ]
                            cambios += [
                                (0x5225 + i * 2, objetos[i] if i < len(objetos) else 65535, False)
                                for i in range(10)
                            ]
                            self.contrato(base, id, previo, 1, cambios)
        self.retomar_recursos(base)

    # Continúa la misma grabación/correo: entrega106, recibe99 y usa el pase hasta solicitar66.
    def ejecutar(self):
        origen = (
            self.raiz
            / 'apps/juego/trabajo/comparacion_grabacion_completa/grabacion_y_correo_enviado'
        )
        if not origen.with_suffix('.ds.bin').exists():
            ComparacionGrabacionCompleta(self.raiz, self.biblioteca, True).ejecutar()
        d = bytearray(origen.with_suffix('.ds.bin').read_bytes())
        self.semilla_menu[:] = json.loads(
            origen.with_suffix('.semilla.json').read_text(encoding='utf-8')
        )
        self.destino = self.raiz / 'apps/juego/trabajo/comparacion_entrega_grabacion'
        self.destino.mkdir(parents=True, exist_ok=True)
        self.resumen = {}
        self.retomar_recursos(d)
        self.ruta(d, ((470, 145), (550, 140), (620, 145), (704, 145), (850, 190), (850, 199)), 57)
        if d[0x2C4D] != 4:
            raise ErrorAnalisis('El correo enviado no conserva el fotograma4 del destinatario.')
        self.hito_borde(d, 'destinatario_con_correo')
        self.ruta_sin_salida(d, 150, 165)
        self.auditar(d, 'destinatario antes de entregar106')
        self.variantes_entrega(d)
        puntos = palabra(d, 0x35C7)
        self.elegir_objeto(d, (2, 6), 106)
        self.elegir_objeto(d, (), 98)
        self.esperar(d, 512)
        inventario = [palabra(d, 0x5225 + i * 2) for i in range(10)]
        if 99 not in inventario or 106 in inventario or d[0x2C4C] or d[0x44F9]:
            raise ErrorAnalisis('La entrega no retira106 y libera el paso con el pase99 real.')
        if palabra(d, 0x35C7) != (puntos + 5) & 65535:
            raise ErrorAnalisis('La puntuación de la entrega difiere del contrato original.')
        self.hito_borde(d, 'pase_recibido')
        self.auditar(d, 'DINO después de recibir el pase')
        self.ruta(d, ((250, 199),), 58)
        for punto in ((850, 145), (925, 140)):
            self.ruta_sin_salida(d, *punto)
        self.hito_borde(d, 'puerta_antes_del_pase')
        self.auditar(d, 'POST ante la puerta del pase')
        self.variantes_puerta(d)
        puntos = palabra(d, 0x35C7)
        self.elegir_objeto(d, (2, 1), 99)
        self.elegir_objeto(d, (), 116)
        salida = self.andar(d, self.semilla_menu, 65535, 256)
        if salida != 66 or d[0x2CD0] != 1 or palabra(d, 0x35C7) != puntos:
            raise ErrorAnalisis('El pase no abre la puerta y solicita66 sin añadir puntos.')
        if 99 not in [palabra(d, 0x5225 + i * 2) for i in range(10)]:
            raise ErrorAnalisis('El pase se consume, a diferencia del original.')
        self.hito_borde(d, 'puerta_solicitud66')
        informe = {
            'resultado': 'coinciden',
            'casos_y_pasos': sum(self.resumen.values()),
            'grupos': self.resumen,
            'capturas': self.capturas,
            'estados_auditados': self.estados_auditados,
            'variantes_entrega': 864,
            'variantes_puerta': 144,
            'alcance': 'Entrega real106 tras grabación/correo, pase99 y aperturaAC5E hasta solicitud66. Contratos aislados105/106 con y sin correo/capacidad. No carga CLOUD38 ni acredita su puzle o SDL del valle.',
        }
        (self.destino / 'resultado.json').write_text(
            json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8'
        )
        print(
            f'Entrega de grabación: {informe["casos_y_pasos"]} casos/pasos y {self.capturas} composiciones coincidentes.'
        )


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Contrasta la entrega de la grabación y el pase originales.'
    )
    parser.add_argument('--biblioteca', type=Path, required=True)
    parser.add_argument('--muestra', action='store_true')
    opciones = parser.parse_args()
    try:
        ComparacionEntregaGrabacion(
            Path(__file__).resolve().parents[3], opciones.biblioteca, opciones.muestra
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        parser.exit(1, f'Error: {error}\n')
