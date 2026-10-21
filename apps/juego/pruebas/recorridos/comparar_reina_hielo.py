"""Contrasta el acceso103 y el enfrentamiento original D114 ante la reina del hielo."""

import argparse
import ctypes
import itertools
import json
from pathlib import Path

from comparar_laser_palacio import ComparacionLaserPalacio, ErrorAnalisis
from comparar_objetos import palabra, guardar


class ComparacionReinaHielo(ComparacionLaserPalacio):
    # D114: variantes aisladas de umbrales, impactos, salto, recuperación y entrada al trono.
    def variantes_reina(self, base):
        rutina = self.nativa.procesar_rutina_reina
        rutina.argtypes = [ctypes.POINTER(ctypes.c_ubyte)]
        rutina.restype = ctypes.c_int
        numero = 0
        for x, y, control, modelo, imagen in itertools.product(
            (39, 40, 41, 100, 196, 197),
            (124, 125, 130),
            (0, 1),
            (9, 24, 255),
            (47, 129, 130, 131, 132),
        ):
            d = bytearray(base)
            guardar(d, 0x2C2D, x)
            guardar(d, 0x2C2F, y)
            d[0x3594] = control
            d[0x358C] = modelo
            d[0x2C2C] = imagen
            d[0x2CAF] = 1
            guardar(d, 0x2CB1, x + 10)
            guardar(d, 0x2CB5, 12)
            guardar(d, 0x2CBF, 0)
            self.disponer(d, self.geometria, self.semilla_menu)
            self.ejecutar_original(0xD114)
            esperado = bytes(self.maquina.mem_read(self.segmento, 65536))
            if rutina((ctypes.c_ubyte * 65536).from_buffer(d)) != 1 or bytes(d) != esperado:
                diferencias = [
                    f'{i:04X}:{d[i]:02X}/{esperado[i]:02X}'
                    for i in range(65536)
                    if d[i] != esperado[i]
                ][:8]
                raise ErrorAnalisis(f'Reina D114 variante{numero} distinta: {diferencias}')
            numero += 1
        self.variantes['rutina_reina'] = numero
        self.retomar_recursos(base)

    # Parte de la reja abierta por la partida comprobada, sin cambiar inventario ni posiciones.
    def ejecutar(self):
        origen = self.raiz / 'apps/juego/trabajo/comparacion_laser_palacio/laser_solicitud103'
        d = bytearray(origen.with_suffix('.ds.bin').read_bytes())
        self.semilla_menu[:] = json.loads(
            origen.with_suffix('.semilla.json').read_text(encoding='utf-8')
        )
        self.destino = self.raiz / 'apps/juego/trabajo/comparacion_reina_hielo'
        self.destino.mkdir(parents=True, exist_ok=True)
        self.resumen = {}
        self.variantes = {}
        self.retomar_recursos(d)
        self.cambiar(d, self.semilla_menu, 103)
        self.hito_borde(d, 'reina_entrada')
        self.variantes_reina(d)
        for i in range(192):
            self.esperar(d, 1)
            if d[0x3594]:
                break
        print(
            'Entrada:',
            i,
            'posición',
            palabra(d, 0x2C2D),
            palabra(d, 0x2C2F),
            'modelo',
            d[0x358C],
            flush=True,
        )
        self.hito_borde(d, 'reina_control')
        self.auditar(d, 'reina: control inicial')
        for x, modelo, cuadro in itertools.product(
            (40, 54, 196, 197), (9, 24, 255), (47, 130, 131)
        ):
            self.variante(
                d,
                'salto160',
                0x3143,
                160,
                cambios=((0x2C2D, x, False), (0x358C, modelo, True), (0x2C2C, cuadro, True)),
            )
        self.retomar_recursos(d)
        # Un salto demasiado pronto recibe un golpe: se conserva el retroceso y la recuperación.
        self.elegir_objeto(d, (6,), 160)
        self.esperar(d, 8, 4)
        self.hito_borde(d, 'reina_salto_temprano')
        self.esperar(d, 10, 4)
        if d[0x358C] != 24 or d[0x2C2C] != 47:
            raise ErrorAnalisis('El proyectil no produce el retroceso original del salto temprano.')
        self.hito_borde(d, 'reina_golpe')
        self.esperar(d, 20, 4)
        if not d[0x3594] or palabra(d, 0x2C2D) != 40:
            raise ErrorAnalisis('El golpe no devuelve el control junto al extremo izquierdo.')
        self.hito_borde(d, 'reina_recuperada')
        self.auditar(d, 'reina: recuperada tras golpe')
        self.esperar(d, 8, 4)
        self.hito_borde(d, 'reina_preparada')
        self.elegir_objeto(d, (6,), 160)
        self.esperar(d, 8, 4)
        self.hito_borde(d, 'reina_salto_primero')
        self.esperar(d, 17, 4)
        self.hito_borde(d, 'reina_entre_saltos')
        self.auditar(d, 'reina: primer proyectil esquivado')
        if not d[0x3594]:
            raise ErrorAnalisis('El primer salto correcto no devuelve el control.')
        self.elegir_objeto(d, (6,), 160)
        self.esperar(d, 8, 4)
        self.hito_borde(d, 'reina_salto_segundo')
        for paso in range(384):
            salida = self.andar(d, self.semilla_menu, 4, 1)
            if salida is not None:
                if salida != 104:
                    raise ErrorAnalisis(f'El trono solicita una salida inesperada: {salida}.')
                (self.destino / 'reina_solicitud104.ds.bin').write_bytes(d)
                (self.destino / 'reina_solicitud104.semilla.json').write_text(
                    json.dumps(list(self.semilla_menu)), encoding='utf-8'
                )
                break
            if paso == 19:
                self.hito_borde(d, 'reina_trono')
            if paso == 35:
                self.hito_borde(d, 'reina_expulsion')
        else:
            raise ErrorAnalisis('El trono no expulsa al personaje hacia104.')
        self.conservar_contexto(self.destino, 'reina_solicitud104')
        informe = dict(
            resultado='coinciden',
            casos_y_pasos=sum(self.resumen.values()) + self.variantes['rutina_reina'],
            grupos=self.resumen,
            variantes=self.variantes,
            capturas=self.capturas,
            estados_auditados=self.estados_auditados,
            limite='Salida104 solicitada por el trono; destino58 pendiente. DS completo y azar sin normalización.',
        )
        (self.destino / 'resultado.json').write_text(
            json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8'
        )
        print(
            'Reina:',
            informe['casos_y_pasos'],
            'casos/pasos;',
            self.capturas,
            'imágenes; salida104 comprobada.',
            flush=True,
        )


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--biblioteca', type=Path, required=True)
    parser.add_argument('--muestra', action='store_true')
    args = parser.parse_args()
    try:
        ComparacionReinaHielo(
            Path(__file__).resolve().parents[4], args.biblioteca, args.muestra
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        parser.exit(1, f'Error: {error}\n')
