"""Contrasta los puzles del trayecto de Paso del Dragón hasta el transporte del mago."""

import argparse
import ctypes
import json
import os
from pathlib import Path
from comparar_asaltante import ComparacionAsaltante, ErrorAnalisis
from comparar_objetos import guardar, palabra


class ComparacionPoblacion(ComparacionAsaltante):
    # Reutiliza la ordenación y los servicios contrastados de la partida anterior.
    def __init__(self, raiz, biblioteca, muestra):
        super().__init__(raiz, biblioteca, muestra)
        self.carpeta_resultados = 'comparacion_poblacion'

    # Recorre el mago en todas sus ramas, incluida puntuación envolvente e inventario lleno.
    def variantes(self, base, mago=False):
        for progreso in range(8) if mago else (0,):
            for ocupadas in (0, 9, 10):
                for puntos in (0, 65535):
                    for visitado in (0, 1) if mago else (0,):
                        d = bytearray(base)
                        d[0x35C9] = progreso
                        d[0x402E] = visitado
                        guardar(d, 0x35C7, puntos)
                        for i in range(10):
                            guardar(d, 0x5225 + i * 2, 84 if i < ocupadas else 65535)
                        guardar(d, 0xB60C, 0x3B53)
                        guardar(d, 0xB620, (77 if mago else 83) + 26)
                        guardar(d, 0xB634, 6)
                        guardar(d, 0xB638, 76)
                        self.seleccionar(d, 0)
        self.retomar_recursos(base)
        self.maquina.ctl_flush_tb()

    # Contrasta las orientaciones de comer, el rechazo del modelo pequeño y la respuesta del ID81.
    def variantes_adicionales(self, base):
        for imagen in (0, 17, 18, 26, 27, 127):
            for modelo in (255, 20):
                d = bytearray(base)
                d[0x2C2C] = imagen
                d[0x358C] = modelo
                guardar(d, 0xB60C, 0x3B53)
                guardar(d, 0xB620, 84 + 26)
                guardar(d, 0xB634, 3)
                guardar(d, 0xB638, 65535)
                self.seleccionar(d, 0)
        for progreso in range(8):
            d = bytearray(base)
            d[0x35C9] = progreso
            guardar(d, 0xB60C, 0x3B53)
            guardar(d, 0xB620, 81 + 26)
            guardar(d, 0xB634, 6)
            guardar(d, 0xB638, 76)
            self.seleccionar(d, 0)
        self.retomar_recursos(base)
        self.maquina.ctl_flush_tb()

    # 5C8F selecciona otra entidad para el mismo dinero al entrar directamente en DRAGON.
    def variantes_dinero(self, base):
        for ocupadas in (0, 9, 10):
            for modelo in (255, 20):
                d = bytearray(base)
                d[0x358C] = modelo
                for i in range(10):
                    guardar(d, 0x5225 + i * 2, 3 if i < ocupadas else 65535)
                guardar(d, 0xB60C, 0x563A)
                guardar(d, 0xB620, 76 + 26)
                self.seleccionar(d, 0)
        self.retomar_recursos(base)
        self.maquina.ctl_flush_tb()

    # Recorre en una copia la reacción del portero sin adelantar la ruta pendiente de la posada.
    def comprobar_portero(self, base):
        semilla = list(self.semilla_menu)
        d = bytearray(base)
        try:
            for x, y in ((320, 185), (192, 175), (192, 150)):
                salida = self.caminar_hasta(d, self.semilla_menu, x, y)
                if salida is not None:
                    break
            if d[0x2C4D] != 19:
                raise ErrorAnalisis('No se ha alcanzado la reacción del portero original.')
        finally:
            self.semilla_menu[:] = semilla
            self.retomar_recursos(base)
            self.maquina.ctl_flush_tb()

    # Conserva cada hito y revisa las opciones ofrecidas en su posición real.
    def hito(self, d, nombre):
        self.captura(d, self.destino, 'poblacion_' + nombre)
        self.auditar(d, nombre)

    # Camina con coordenadas globales y comprueba el identificador de salida antes de cargar.
    def tramo(self, d, sem, puntos, salida=None, espera=128):
        encontrada = None
        for x, y in puntos:
            encontrada = self.caminar_hasta(d, sem, x, y)
            if encontrada is not None:
                break
        if encontrada != salida:
            raise ErrorAnalisis(f'Salida distinta en población: {encontrada}, esperada {salida}.')
        if encontrada is not None:
            self.cambiar(d, sem, encontrada)
            if espera:
                self.andar(d, sem, 65535, espera)

    # Continúa desde el dinero ganado, sin fabricar objetos ni banderas para resolver los puzles.
    def ejecutar(self):
        origen = self.raiz / 'trabajo/comparacion_asaltante'
        if not (origen / 'asaltante_salida.ds.bin').exists():
            ComparacionAsaltante(self.raiz, self.biblioteca, True).ejecutar()
        d = bytearray((origen / 'asaltante_salida.ds.bin').read_bytes())
        if len(d) != 65536 or palabra(d, 0x8082) != 21 or palabra(d, 0x3598) != 26:
            raise ErrorAnalisis('La referencia no corresponde a la llegada real a DRAGON.')
        sem = (ctypes.c_ushort * 4)(
            *json.loads((origen / 'asaltante_salida.semilla.json').read_text(encoding='utf-8'))
        )
        self.semilla_menu = sem
        self.recursos_menu = True
        self.retomar_recursos(d)
        referencia = os.environ.get('CURSE_REBORN_ORDEN_ENTRADA')
        if referencia:
            entrada = Path(referencia).read_bytes()
            if (
                len(entrada) != 65536
                or palabra(entrada, 0x8082) != 21
                or palabra(entrada, 0x3598) != 26
            ):
                raise ErrorAnalisis('La entrada gráfica común no corresponde a DRAGON.')
            if sorted(palabra(entrada, 0xBDD4 + i * 4) for i in range(69)) != [
                0x2C2B + i * 33 for i in range(69)
            ]:
                raise ErrorAnalisis('La permutación inicial no es válida.')
            d[0xBDD2:0xBEE6] = entrada[0xBDD2:0xBEE6]
        self.destino = self.raiz / 'trabajo/comparacion_poblacion'
        self.destino.mkdir(parents=True, exist_ok=True)
        self.resumen = {}
        self.hito(d, 'entrada')
        self.andar(d, sem, 65535, 160)
        self.hito(d, 'guardias')
        self.variantes_dinero(d)
        self.comprobar_portero(d)
        self.tramo(d, sem, ((320, 185), (250, 170), (245, 150)), 42)
        self.hito(d, 'frank')
        for x, y in ((150, 140), (150, 125)):
            self.clic(d, x, y)
            self.andar(d, sem, 65535, 128)
        self.menu(d, (4, 0))
        self.andar(d, sem, 65535, 96)
        self.hito(d, 'saludo')
        self.variantes(d)
        self.elegir_objeto(d, (2, 6), 76)
        self.elegir_objeto(d, (), 83)
        self.andar(d, sem, 65535, 192)
        if 84 not in [palabra(d, 0x5225 + i * 2) for i in range(10)]:
            raise ErrorAnalisis('Frank no entrega comida.')
        self.hito(d, 'comida')
        self.variantes_adicionales(d)
        self.elegir_objeto(d, (2, 3), 84)
        self.hito(d, 'comer')
        self.andar(d, sem, 65535, 192)
        if 84 in [palabra(d, 0x5225 + i * 2) for i in range(10)]:
            raise ErrorAnalisis('La comida no se consume.')
        self.hito(d, 'consumida')
        self.tramo(d, sem, ((160, 180),), 51)
        self.hito(d, 'regreso')
        self.tramo(d, sem, ((140, 185), (110, 160), (105, 140)), 43)
        self.hito(d, 'fuente')
        self.tramo(d, sem, ((220, 180), (235, 120), (225, 70), (170, 50), (170, 5)), 28)
        self.hito(d, 'maravillas')
        self.tramo(d, sem, ((115, 170), (110, 130)), 32)
        self.hito(d, 'mago')
        self.variantes(d, True)
        self.elegir_objeto(d, (2, 6), 76)
        self.elegir_objeto(d, (), 77)
        self.hito(d, 'transporte')
        for _ in range(768):
            salida = self.andar(d, sem, 65535, 1)
            if salida is not None:
                break
        else:
            raise ErrorAnalisis('El transporte del mago no termina.')
        if salida != 47:
            raise ErrorAnalisis(f'El mago solicita una salida distinta: {salida}.')
        # El contrato de entrada se contrasta antes de cargar EDGE; no afirma fidelidad de su dibujo.
        self.entrada(d, salida)
        (self.destino / 'poblacion_entrada_borde.ds.bin').write_bytes(d)
        informe = {
            'resultado': 'coinciden',
            'casos_y_pasos': sum(self.resumen.values()),
            'grupos': self.resumen,
            'capturas': self.capturas,
            'estados_auditados': self.estados_auditados,
            'orden_inicial_compartido_con_sdl': bool(referencia),
            'alcance': 'DRAGON, FRANKS, XROADS, SORCERY y MAGE por el trayecto de la guía. Entrada en EDGE sin avanzar su lógica. Las ramas laterales de la ciudad y los puzles de EDGE siguen pendientes.',
        }
        (self.destino / 'resultado.json').write_text(
            json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8'
        )
        print(
            f'Población: {informe["casos_y_pasos"]} casos/pasos, {self.capturas} composiciones y {self.estados_auditados} estados de menú coincidentes.'
        )


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Compara las compras, la comida y el transporte del mago.'
    )
    parser.add_argument('--biblioteca', type=Path, required=True)
    parser.add_argument('--muestra', action='store_true')
    opciones = parser.parse_args()
    try:
        ComparacionPoblacion(
            Path(__file__).resolve().parents[1], opciones.biblioteca, opciones.muestra
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        parser.exit(1, f'Error: {error}\n')
