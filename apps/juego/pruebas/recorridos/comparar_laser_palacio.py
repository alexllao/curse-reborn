"""Contrasta la entrada90, los reflectores y la reja de la sala56 del palacio."""

import argparse
import ctypes
import itertools
import json
from pathlib import Path

from comparar_resolucion_palacio import ComparacionResolucionPalacio, ErrorAnalisis
from comparar_objetos import palabra, guardar


class ComparacionLaserPalacio(ComparacionResolucionPalacio):
    # CFD2: contrasta todos los cortes del circuito y la explosión con entidades libres o agotadas.
    def variantes_rayo(self, base):
        self.nativa.procesar_laser.argtypes = [
            ctypes.POINTER(ctypes.c_ubyte),
            ctypes.POINTER(ctypes.c_ushort),
            self.tipo_dimensiones,
        ]
        self.nativa.procesar_laser.restype = ctypes.c_int
        casos = []
        for estados in itertools.product(((0, 30), (1, 0), (1, 30), (3, 30)), repeat=4):
            d = bytearray(base)
            for i, (actividad, imagen) in enumerate(estados):
                entidad = 0x2CD0 + i * 33
                d[entidad] = actividad
                d[entidad + 1] = (30, 27, 34, 14)[i] if imagen == 30 else imagen
            casos.append(d)
        for emisor, final, libres in itertools.product((0, 1), (0, 1), (0, 1, 6)):
            d = bytearray(base)
            d[0x2D75], d[0x2E1A] = emisor, final
            primera = palabra(d, 0x3588)
            for i in range(primera, 70):
                d[0x2C0A + i * 33] = 0 if i - primera < libres else 1
                guardar(d, 0x2C0A + i * 33 + 14, 65535)
            casos.append(d)
        for numero, d in enumerate(casos):
            semilla = (ctypes.c_ushort * 4)(*((v + numero * 17) & 65535 for v in self.semilla_menu))
            self.disponer(d, self.geometria, semilla)
            self.ejecutar_original(0x10EC)
            esperado = bytes(self.maquina.mem_read(self.segmento, 65536))
            azar = bytes(self.maquina.mem_read(0x78DE, 8))
            resultado = self.nativa.procesar_laser(
                (ctypes.c_ubyte * 65536).from_buffer(d), semilla, self.callback_dimensiones
            )
            if resultado != 1 or bytes(d) != esperado or bytes(semilla) != azar:
                diferencias = [
                    f'{i:04X}:{d[i]:02X}/{esperado[i]:02X}'
                    for i in range(65536)
                    if d[i] != esperado[i]
                ][:8]
                raise ErrorAnalisis(
                    f'Rutina del láser distinta en variante{numero}: {diferencias}.'
                )
        self.variantes['rutina_rayo'] = len(casos)
        self.retomar_recursos(base)

    # BB8D–BCDC: soportes ocupados/libres, ambos órdenes, objetos ausentes y reja protegida.
    def variantes_soportes(self, base):
        numero = 0
        for soporte in range(155, 159):
            entidad = 0x2CD0 + (soporte - 155) * 33
            for pieza in (150, 153, 154, 145):
                for activo in (0, 1, 3):
                    for presente in (False, True):
                        cambios = [(entidad, activo, True)]
                        cambios += [
                            (0x5225 + i * 2, pieza if presente and i == 0 else 65535, False)
                            for i in range(10)
                        ]
                        for id, previo in ((soporte, pieza), (pieza, soporte)):
                            self.contrato(base, id, previo, 7, cambios)
                            numero += 1
        for proteccion in (0, 1, 3):
            for presente in (False, True):
                for puntos in (0, 65535):
                    cambios = [(0x2D96, proteccion, True), (0x35C7, puntos, False)]
                    cambios += [
                        (0x5225 + i * 2, 149 if presente and i == 0 else 65535, False)
                        for i in range(10)
                    ]
                    for id, previo in ((159, 149), (149, 159)):
                        self.contrato(base, id, previo, 1, cambios)
                        numero += 1
        self.variantes['colocaciones_y_reja'] = numero
        self.retomar_recursos(base)

    # 5FFD–6060/61A7: se pueden recuperar las piezas de cualquiera de los cuatro soportes.
    def variantes_retirada(self, base):
        numero = 0
        for entidad in (0x2CD0, 0x2CF1, 0x2D12, 0x2D33):
            for pieza, imagen in ((150, 30), (153, 27), (154, 34), (145, 14)):
                for capacidad in (0, 9, 10):
                    for modelo in (20, 255):
                        cambios = [
                            (entidad, 1, True),
                            (entidad + 1, imagen, True),
                            (entidad + 14, pieza, False),
                            (0x358C, modelo, True),
                        ]
                        cambios += [
                            (0x5225 + i * 2, 76 if i < capacidad else 65535, False)
                            for i in range(10)
                        ]
                        self.variante(base, 'retirada', 0x563A, pieza, cambios=cambios)
                        numero += 1
        self.variantes['retiradas_reflectores'] = numero
        self.retomar_recursos(base)

    # Prolonga el salto contrastado desde la ventana, sin añadir útiles ni abrir la reja a mano.
    def ejecutar(self):
        origen = self.raiz / 'apps/juego/trabajo/comparacion_resolucion_palacio/ventana_solicitud90'
        d = bytearray(origen.with_suffix('.ds.bin').read_bytes())
        self.semilla_menu[:] = json.loads(
            origen.with_suffix('.semilla.json').read_text(encoding='utf-8')
        )
        self.destino = self.raiz / 'apps/juego/trabajo/comparacion_laser_palacio'
        self.destino.mkdir(parents=True, exist_ok=True)
        self.resumen = {}
        self.variantes = {}
        self.retomar_recursos(d)
        self.cambiar(d, self.semilla_menu, 90)
        self.hito_borde(d, 'laser_entrada')
        self.esperar(d, 128)
        self.hito_borde(d, 'laser_control')
        self.variantes_soportes(d)
        self.variantes_retirada(d)
        self.variantes_rayo(d)
        self.auditar(d, 'sala del láser sin reflectores')
        self.ruta_sin_salida(d, 258, 166)
        self.elegir_objeto(d, (2, 7), 153)
        self.elegir_objeto(d, (), 155)
        self.esperar(d, 192)
        if d[0x2DB7] != 3:
            raise ErrorAnalisis('Un reflector incorrecto ha desviado el primer tramo.')
        self.hito_borde(d, 'laser_pieza_erronea')
        self.auditar(d, 'cubito en soporte de lupa')
        self.ruta_sin_salida(d, palabra(d, 0x2CD2) + 5, palabra(d, 0x2CD4) + palabra(d, 0x2CD8) + 1)
        self.elegir_objeto(d, (1,), 153)
        self.esperar(d, 192)
        self.exigir_objeto(d, 153)
        self.hito_borde(d, 'laser_pieza_recuperada')
        self.auditar(d, 'reflector recuperado y soporte libre')
        for pieza, soporte, nombre, x, y in (
            (150, 155, 'lupa', 258, 166),
            (153, 156, 'cubito', 74, 156),
            (154, 157, 'carambano', 40, 192),
            (145, 158, 'cristal', 275, 192),
        ):
            self.ruta_sin_salida(d, x, y)
            self.elegir_objeto(d, (2, 7), pieza)
            self.elegir_objeto(d, (), soporte)
            self.esperar(d, 192)
            self.hito_borde(d, 'laser_' + nombre)
            self.auditar(d, 'láser tras ' + nombre)
        if d[0x2D75] != 0 or d[0x2D96] != 0:
            raise ErrorAnalisis('El circuito no destruye el emisor y su protector.')
        self.ruta_sin_salida(d, 275, 153)
        self.elegir_objeto(d, (2, 1), 149)
        self.elegir_objeto(d, (), 159)
        for paso in range(384):
            salida = self.andar(d, self.semilla_menu, 65535, 1)
            if salida is not None:
                print('Salida automática de la reja:', salida, 'paso', paso)
                if salida != 103:
                    raise ErrorAnalisis('La reja solicita una salida distinta de103.')
                (self.destino / 'laser_solicitud103.ds.bin').write_bytes(d)
                (self.destino / 'laser_solicitud103.semilla.json').write_text(
                    json.dumps(list(self.semilla_menu)), encoding='utf-8'
                )
                break
            if paso in (31, 63):
                self.hito_borde(d, 'laser_reja_' + str(paso + 1))
        else:
            raise ErrorAnalisis('El gato no entrega la salida103 de la reja.')
        self.conservar_contexto(self.destino, 'laser_solicitud103')
        informe = dict(
            resultado='coinciden',
            casos_y_pasos=sum(self.resumen.values()) + self.variantes['rutina_rayo'],
            grupos=self.resumen,
            variantes=self.variantes,
            capturas=self.capturas,
            estados_auditados=self.estados_auditados,
            limite='DS completo y azar sin normalización de actores inactivos. Destino de la salida103 pendiente.',
        )
        (self.destino / 'resultado.json').write_text(
            json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8'
        )
        print(
            f'Láser: {informe["casos_y_pasos"]} casos/pasos y {self.capturas} imágenes contrastados.'
        )


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--biblioteca', type=Path, required=True)
    parser.add_argument('--muestra', action='store_true')
    opciones = parser.parse_args()
    try:
        ComparacionLaserPalacio(
            Path(__file__).resolve().parents[4], opciones.biblioteca, opciones.muestra
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        parser.exit(1, f'Error: {error}\n')
