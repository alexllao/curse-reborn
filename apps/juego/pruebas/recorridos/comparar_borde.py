"""Contrasta C093 aisladamente; no presenta EDGE como una habitación jugable completa."""

import argparse
import ctypes
import json
import struct
from pathlib import Path

from comparar_poblacion import ComparacionPoblacion, ErrorAnalisis
from comparar_objetos import ComparacionObjetos, guardar, palabra


class ComparacionBorde(ComparacionPoblacion):
    # Limita la comparación a la rutina de escena y a sus recursos reales de creación.
    def __init__(self, raiz, biblioteca, muestra):
        super().__init__(raiz, biblioteca, muestra)
        self.nativa.procesar_borde.argtypes = [
            ctypes.POINTER(ctypes.c_ubyte),
            ctypes.POINTER(ctypes.c_ushort),
            self.tipo_dimensiones,
            ctypes.POINTER(ctypes.c_uint),
        ]
        self.nativa.procesar_borde.restype = ctypes.c_int
        self.sonidos_originales = 0
        self.maquina.hook_add(self.unicorn.UC_HOOK_CODE, self.registrar_sonido)

    # Cuenta las llamadas originales de 5288 sin sustituir sus instrucciones ni el selector común.
    def registrar_sonido(self, maquina, direccion, tamano, contexto):
        if direccion == 0x5288:
            if (
                maquina.reg_read(self.r.UC_X86_REG_SI) != 0x2C2B
                or maquina.reg_read(self.r.UC_X86_REG_AX) & 255 != 6
            ):
                raise ErrorAnalisis('Petición sonora inesperada en C093.')
            self.sonidos_originales += 1

    # Solo C093/8B6F usa EDGE.SPR: no dispone SMALL ni bancos de dibujo que se solapen.
    def preparar_bancos_originales(self):
        if getattr(self, 'rutina_aislada', False):
            self.maquina.mem_write(0x40000, self.escena)
            self.maquina.mem_write(0x4AEE, struct.pack('<H', 0x4000))
        else:
            super().preparar_bancos_originales()

    # Compara DS completo, las cuatro palabras aleatorias y el número de órdenes de audio.
    def rutina(self, datos, grupo, numero):
        sem = (ctypes.c_ushort * 4)(*((v + numero * 17) & 65535 for v in self.semilla_original))
        self.disponer(datos, self.geometria, sem)
        self.sonidos_originales = 0
        self.ejecutar_original(0xC093)
        esperado = bytes(self.maquina.mem_read(self.segmento, 65536))
        azar = bytes(self.maquina.mem_read(0x78DE, 8))
        sonidos = ctypes.c_uint()
        valido = self.nativa.procesar_borde(
            (ctypes.c_ubyte * 65536).from_buffer(datos),
            sem,
            self.callback_dimensiones,
            ctypes.byref(sonidos),
        )
        if (
            not valido
            or datos != esperado
            or bytes(sem) != azar
            or sonidos.value != self.sonidos_originales
        ):
            diferencias = [
                f'{i:04X}:{datos[i]:02X}/{esperado[i]:02X}'
                for i in range(65536)
                if datos[i] != esperado[i]
            ][:8]
            raise ErrorAnalisis(
                f'C093 {grupo}/{numero}: {diferencias}, válido={valido}, sonidos={sonidos.value}/{self.sonidos_originales}.'
            )
        self.resumen[grupo] = self.resumen.get(grupo, 0) + 1
        self.sonidos += sonidos.value

    # Prepara variantes neutras de la rutina sin afirmar que sean una partida avanzada.
    def neutro(self, base):
        d = bytearray(base)
        d[0x3594] = 0
        d[0x2DB7] |= 2
        d[0x2DD8] = d[0x2DF9] = d[0x2EE0] = d[0x2C4C] = 0
        d[0xADCE] = 180
        guardar(d, 0x2CB1, palabra(d, 0x2C2D) - 20)
        return d

    # Cubre límites, contactos, rescate y consumos aleatorios sobre descriptores originales.
    def ejecutar(self):
        origen = (
            self.raiz / 'apps/juego/trabajo/comparacion_poblacion/poblacion_entrada_borde.ds.bin'
        )
        if not origen.exists():
            ComparacionPoblacion(self.raiz, self.biblioteca, True).ejecutar()
        base = bytearray(origen.read_bytes())
        self.resumen = {}
        self.sonidos = 0
        self.escena = (self.carpeta / 'EDGE.SPR').read_bytes()
        if 0x40000 + len(self.escena) > 0x74000:
            raise ErrorAnalisis('EDGE rebasa el área reservada a la rutina aislada.')
        self.rutina_aislada = True
        ComparacionObjetos.caso(self, 'carga_entidades', 0, base, 'EDGE.SPR')
        self.rutina(bytearray(base), 'entrada', 0)
        numero = 0
        for control in (0, 1, 3):
            for diferencia in (-1, 0, 10, 11):
                for modelo in (255, 20):
                    for fotograma in (89, 90, 91):
                        for canal in (0, 0xB0ED):
                            d = self.neutro(base)
                            d[0x3594], d[0x358C], d[0x2C2C] = control, modelo, fotograma
                            guardar(d, 0x2CB1, palabra(d, 0x2C2D) + diferencia)
                            guardar(d, 0x2C5C, canal)
                            self.rutina(d, 'activacion_y_transicion', numero)
                            numero += 1
        for x in (0, 1, 174, 175, 176, 32767, 32768, 65535):
            for ancho in (1, 65535):
                for y in (49, 50, 51):
                    for imagen in (22, 23, 24, 25):
                        d = self.neutro(base)
                        d[0x2DB7] &= 253
                        d[0x2DB8] = imagen
                        guardar(d, 0x2DB9, x)
                        guardar(d, 0x2DBD, ancho)
                        guardar(d, 0x2DBB, y)
                        self.rutina(d, 'rescate', numero)
                        numero += 1
        for registro, margen in ((0x2DD8, 16), (0x2EE0, 14)):
            for canal in (0, 0xB0A8):
                for dx in (-1, 0, margen, margen + 1):
                    for dy in (-1, 0, 48, 49):
                        for control in (0, 1):
                            d = self.neutro(base)
                            d[registro], d[0x3594] = 1, control
                            guardar(d, registro + 16, canal)
                            guardar(d, registro + 2, palabra(d, 0x2C2D) + dx)
                            guardar(d, registro + 4, palabra(d, 0x2C2F) + dy)
                            self.rutina(d, 'obstaculos', numero)
                            numero += 1
        for temporizador in (0, 1, 2, 49, 50, 51, 179, 180, 181, 255):
            for ventana in (20, 21, 40, 41):
                for lleno in (False, True):
                    for semilla in range(8):
                        d = self.neutro(base)
                        if lleno:
                            for p in range(0x2C4C, 0x3510, 33):
                                d[p] = 1
                            d[0x3594] = 0
                            d[0x2DB7] |= 2
                            guardar(d, 0x2E09, 0xB104)
                        d[0xADCE], d[0x3597] = temporizador, ventana
                        guardar(d, 0x3591, ventana * 32)
                        self.rutina(d, 'temporizador_y_azar', semilla)
        for distancia in (-32768, -4, -3, 0, 3, 4):
            for ocupadas in (0, 9, 10):
                for oferta in (0, 1):
                    d = self.neutro(base)
                    d[0x2DF9], d[0x2C8E] = 1, oferta
                    guardar(d, 0x2C9C, 65535)
                    guardar(d, 0x2E09, 0)
                    guardar(d, 0x2DFB, palabra(d, 0x2C2D) + 4 + distancia)
                    for i in range(10):
                        guardar(d, 0x5225 + i * 2, 3 if i < ocupadas else 65535)
                    self.rutina(d, 'piedra_y_inventario', numero)
                    numero += 1
        for bandera in (0, 2):
            for limite in (0, 1):
                for postura in (56, 57, 58):
                    d = self.neutro(base)
                    d[0x2D12] = bandera
                    guardar(d, 0x18F5, limite)
                    guardar(d, 0x2CD4, postura)
                    self.rutina(d, 'proximidad', numero)
                    numero += 1
        for canal in (0, 0xB104, 0xB11C):
            for activo in (0, 1):
                d = self.neutro(base)
                d[0x2DF9] = activo
                guardar(d, 0x2E09, canal)
                guardar(d, 0x2DFB, palabra(d, 0x2C2D) + 4)
                self.rutina(d, 'canales_piedra', numero)
                numero += 1
        for registro in (0x2DD8, 0x2EE0):
            for x in (-32768, -32767, -1, 0, 1, 32766, 32767):
                for diferencia in (-1, 0, 10, 14, 16, 17):
                    d = self.neutro(base)
                    d[registro], d[0x3594] = 1, 1
                    guardar(d, 0x2C2D, x)
                    guardar(d, 0x2CB1, x - 20)
                    guardar(d, registro + 2, x + diferencia)
                    guardar(d, registro + 4, palabra(d, 0x2C2F))
                    guardar(d, registro + 16, 0)
                    self.rutina(d, 'fronteras_con_signo', numero)
                    numero += 1
        carpeta = self.raiz / 'apps/juego/trabajo/comparacion_borde'
        carpeta.mkdir(parents=True, exist_ok=True)
        informe = {
            'resultado': 'coinciden',
            'casos': sum(self.resumen.values()),
            'grupos': self.resumen,
            'ordenes_sonoras': self.sonidos,
            'alcance': 'Rutina C093 aislada, carga de entidades, DS completo, azar y peticiones sonoras. No comprueba movimiento, composición, SDL ni puzles completos de EDGE.',
        }
        (carpeta / 'resultado.json').write_text(
            json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8'
        )
        print(
            f'Borde: {informe["casos"]} casos coincidentes de la rutina aislada; {self.sonidos} órdenes sonoras.'
        )


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Contrasta aisladamente la rutina C093 del borde del mundo.'
    )
    parser.add_argument('--biblioteca', type=Path, required=True)
    parser.add_argument('--muestra', action='store_true')
    opciones = parser.parse_args()
    try:
        ComparacionBorde(
            Path(__file__).resolve().parents[4], opciones.biblioteca, opciones.muestra
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        parser.exit(1, f'Error: {error}\n')
