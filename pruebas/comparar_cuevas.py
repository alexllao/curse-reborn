"""Continúa la partida de POOL hasta obtener la cuerda del picapedrero con doce piedras."""

import argparse
from pathlib import Path

from comparar_poza import ComparacionPoza, ErrorAnalisis
from comparar_objetos import guardar, palabra


class ComparacionCuevas(ComparacionPoza):
    # Mantiene todos los pasos previos y guarda las referencias de esta ampliación por separado.
    def __init__(self, raiz, biblioteca, muestra):
        super().__init__(raiz, biblioteca, muestra)
        self.carpeta_resultados = 'comparacion_cuevas'

    # Llega por clic y flechas a una posición global, respetando cámara, obstáculos y salidas.
    def caminar_hasta(self, datos, semilla, x, y):
        local = x - palabra(datos, 0x3591)
        if not 30 <= local <= 290:
            self.clic(datos, palabra(datos, 0x2C2D) + 16, 170)
            if self.andar(datos, semilla, 0xFFFF, 96) is not None:
                raise ErrorAnalisis(
                    'Una salida interrumpe la aproximación al camino de las cuevas.'
                )
        for _ in range(1500):
            local = x - palabra(datos, 0x3591)
            if 30 <= local <= 290:
                break
            if self.andar(datos, semilla, 8 if local < 30 else 4, 1) is not None:
                raise ErrorAnalisis('Una salida interrumpe el recorrido de las cuevas.')
        else:
            raise ErrorAnalisis(f'No se alcanza el destino global {x}, {y}.')
        self.clic(datos, local, y)
        return self.andar(datos, semilla, 0xFFFF, 192)

    # Selecciona la piedra real de la escena y verifica su incorporación al inventario.
    def recoger_piedra(self, datos, semilla, identificador):
        for i in range(68):
            entidad = 0x2C4C + 33 * i
            if datos[entidad] and palabra(datos, entidad + 14) == identificador:
                x = (palabra(datos, entidad + 2) + palabra(datos, 0x3591) + 5) & 65535
                y = palabra(datos, entidad + 4) + palabra(datos, entidad + 8) - 2
                break
        else:
            raise ErrorAnalisis(f'No existe la piedra {identificador} en la escena.')
        if self.caminar_hasta(datos, semilla, x, y) is not None:
            raise ErrorAnalisis('Una salida interrumpe la recogida de una piedra.')
        self.elegir_objeto(datos, (1,), identificador)
        self.andar(datos, semilla, 0xFFFF, 96)
        if identificador not in [palabra(datos, 0x5225 + i * 2) for i in range(10)]:
            raise ErrorAnalisis(f'La piedra {identificador} no se conserva en el inventario.')
        grupo = (
            'piedras_recogidas'
            if identificador in (*range(27, 36), *range(38, 47))
            else 'objetos_recogidos'
        )
        self.resumen[grupo] = self.resumen.get(grupo, 0) + 1

    # Cruza el umbral real y consume la entrada original de la habitación siguiente.
    def cruzar_cueva(self, datos, semilla, x, y, salida):
        detectada = self.caminar_hasta(datos, semilla, x, y)
        if detectada != salida:
            raise ErrorAnalisis(f'Salida de las cuevas distinta: {detectada}, esperada {salida}.')
        self.cambiar(datos, semilla, salida)
        self.andar(datos, semilla, 0xFFFF, 96)

    # Usa Dar, la casilla de inventario y el picapedrero, sin llamar directamente a su guion.
    def entregar_piedras(self, datos, semilla, piedras):
        if self.caminar_hasta(datos, semilla, 66, 174) is not None:
            raise ErrorAnalisis('La aproximación al picapedrero abandona la habitación.')
        for piedra in piedras:
            self.elegir_objeto(datos, (2, 6), piedra)
            self.elegir_objeto(datos, (), 54)
            self.andar(datos, semilla, 0xFFFF, 96)
            if piedra in [palabra(datos, 0x5225 + i * 2) for i in range(10)]:
                raise ErrorAnalisis(f'El picapedrero no recibe la piedra {piedra}.')
            self.resumen['piedras_entregadas'] = self.resumen.get('piedras_entregadas', 0) + 1

    # Contrasta capacidad y puntuación en los tres corredores que aportan las doce piedras.
    def variantes_recogida(self, base, piedras):
        for piedra in piedras:
            for ocupadas in (0, 9, 10):
                for puntos in (0, 65535):
                    datos = bytearray(base)
                    for i in range(10):
                        guardar(datos, 0x5225 + i * 2, 3 if i < ocupadas else 0xFFFF)
                    guardar(datos, 0x35C7, puntos)
                    guardar(datos, 0xB60C, 0x563A)
                    guardar(datos, 0xB620, piedra + 0x1A)
                    self.seleccionar(datos, 0)

    # 9F39–A06B: compara tipos, fin del cartel, inventario y DEC/JGE con desbordamiento de signo.
    def variantes_entrega(self, base):
        grupos = ((27, 34, 39, 41, 43, 46), (28, 30, 32, 35, 40, 45), (29, 31, 33, 38, 42, 44))
        for grupo, piedras in enumerate(grupos):
            for piedra in piedras:
                for contador in (0, 1, 4, 127, 128, 255):
                    for otras in (0, 1):
                        datos = bytearray(base)
                        datos[0x35CB:0x35CE] = bytes([otras] * 3)
                        datos[0x35CB + grupo] = contador
                        for i in range(10):
                            guardar(datos, 0x5225 + i * 2, piedra if i == 0 else 0xFFFF)
                        guardar(datos, 0x35C7, 65535 if otras else 0)
                        guardar(datos, 0xB60C, 0x3B53)
                        guardar(datos, 0xB620, 54 + 0x1A)
                        guardar(datos, 0xB634, 6)
                        guardar(datos, 0xB638, piedra)
                        self.seleccionar(datos, 0)

    # Prolonga la partida inicial hasta la cuerda, conservando las piedras retiradas al regresar.
    def despues_poza(self, datos, semilla, carpeta):
        self.variantes_recogida(datos, (27, 28, 29, 30))
        for piedra in (27, 28, 29):
            self.recoger_piedra(datos, semilla, piedra)
        self.captura(datos, carpeta, 'cuevas_piedras_uno')
        self.cruzar_cueva(datos, semilla, 160, 199, 5)
        self.variantes_recogida(datos, (31, 32, 33, 34, 35))
        for piedra in (31, 32, 34):
            self.recoger_piedra(datos, semilla, piedra)
        self.cruzar_cueva(datos, semilla, 1048, 90, 9)
        if datos[0x35CB:0x35CE] != bytes((4, 4, 4)):
            raise ErrorAnalisis('El cartel inicial no solicita cuatro piedras de cada tamaño.')
        self.variantes_entrega(datos)
        self.entregar_piedras(datos, semilla, (27, 28, 29, 31, 32, 34))
        if datos[0x35CB:0x35CE] != bytes((2, 2, 2)):
            raise ErrorAnalisis('La primera entrega no deja dos piedras de cada tamaño.')
        self.captura(datos, carpeta, 'cuevas_primera_entrega')
        self.cruzar_cueva(datos, semilla, 150, 199, 19)
        for piedra in (33, 35):
            self.recoger_piedra(datos, semilla, piedra)
        self.cruzar_cueva(datos, semilla, 148, 90, 7)
        self.variantes_recogida(datos, (38, 39, 40, 41, 42))
        for piedra in (38, 39, 40, 41):
            self.recoger_piedra(datos, semilla, piedra)
        self.captura(datos, carpeta, 'cuevas_piedras_tres')
        self.cruzar_cueva(datos, semilla, 160, 199, 10)
        self.cruzar_cueva(datos, semilla, 1048, 90, 9)
        self.entregar_piedras(datos, semilla, (33, 35, 38, 39, 40, 41))
        inventario = [palabra(datos, 0x5225 + i * 2) for i in range(10)]
        if datos[0x35CB:0x35CE] != bytes((0, 0, 0)) or 55 not in inventario:
            raise ErrorAnalisis('La entrega de las doce piedras no proporciona la cuerda.')
        self.captura(datos, carpeta, 'cuevas_cuerda')
        self.despues_cuevas(datos, semilla, carpeta)

    # Permite continuar la misma partida sin fabricar objetos ni resolver puzles por adelantado.
    def despues_cuevas(self, datos, semilla, carpeta):
        pass


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Compara el recorrido de piedras y cuerda con DOS.'
    )
    parser.add_argument('--biblioteca', type=Path, required=True)
    parser.add_argument('--muestra', action='store_true')
    opciones = parser.parse_args()
    try:
        ComparacionCuevas(
            Path(__file__).resolve().parents[1], opciones.biblioteca, opciones.muestra
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        parser.exit(1, f'Error: {error}\n')
