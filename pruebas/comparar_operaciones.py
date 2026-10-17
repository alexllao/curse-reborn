"""Contrasta selección de sonido y servicios externos de las secuencias."""

import argparse
import ctypes
import hashlib
from itertools import product
import json
from pathlib import Path
import struct

from comparar_avance import Comparacion, ErrorAnalisis, escribir


class ComparacionOperaciones(Comparacion):
    def __init__(self, raiz, biblioteca, muestra):
        super().__init__(raiz, biblioteca, muestra)
        memoria = ctypes.POINTER(ctypes.c_ubyte)
        palabras = ctypes.POINTER(ctypes.c_ushort)
        self.nativa.seleccionar_sonido.argtypes = [memoria, palabras, ctypes.c_ushort, ctypes.c_ubyte, palabras]
        self.nativa.seleccionar_sonido.restype = ctypes.c_int
        self.nativa.ejecutar_operaciones_secuencia.argtypes = [memoria, palabras, memoria, memoria,
                                                             ctypes.c_uint, palabras, ctypes.c_ushort]
        self.nativa.ejecutar_operaciones_secuencia.restype = ctypes.c_int
        self.destino = None
        def detener(emulador, direccion, tamano, contexto):
            if self.seleccionando and direccion in (0x7A46, 0x7FB3, 0x7D6A):
                self.destino = direccion
                emulador.emu_stop()
        self.seleccionando = False
        self.maquina.hook_add(self.unicorn.UC_HOOK_CODE, detener)
        self.maquina.hook_add(self.unicorn.UC_HOOK_INTR, self.interrupcion)
        for recurso in (0x5000, 0x5100):
            for i in range(256):
                self.maquina.mem_write(recurso * 16 + i * 2, struct.pack('<H', 0x3000 + i * 4))
                self.maquina.mem_write(recurso * 16 + 0x3000 + i * 4,
                    struct.pack('<H', 1000 + i * 4 + (7 if recurso == 0x5100 else 0)))
        self.maquina.mem_write(0x10, struct.pack('<H', 0x4200))
        self.archivo = b''
        self.llamadas_dos = []

    def interrupcion(self, maquina, numero, contexto):
        r = self.r
        if numero != 0x21: raise ErrorAnalisis('Interrupción no prevista en la carga de modelo.')
        ax = maquina.reg_read(r.UC_X86_REG_AX)
        funcion = ax >> 8
        ds = maquina.reg_read(r.UC_X86_REG_DS)
        dx = maquina.reg_read(r.UC_X86_REG_DX)
        if funcion == 0x3D:
            nombre = bytes(maquina.mem_read(ds * 16 + dx, 32)).split(b'\0')[0]
            self.llamadas_dos.append(('abrir', nombre.decode('ascii')))
            salida = 7
        elif funcion == 0x3F:
            cantidad = maquina.reg_read(r.UC_X86_REG_CX)
            self.llamadas_dos.append(('leer', maquina.reg_read(r.UC_X86_REG_BX), ds, dx, cantidad))
            contenido = self.archivo[:cantidad]
            if contenido: maquina.mem_write(ds * 16 + dx, contenido)
            salida = len(contenido)
        elif funcion == 0x3E:
            self.llamadas_dos.append(('cerrar', maquina.reg_read(r.UC_X86_REG_BX)))
            salida = 0
        else: raise ErrorAnalisis(f'Servicio DOS no previsto: {funcion:02X}.')
        maquina.reg_write(r.UC_X86_REG_AX, salida)
        maquina.reg_write(r.UC_X86_REG_EFLAGS, maquina.reg_read(r.UC_X86_REG_EFLAGS) & ~1)

    def preparar_sonido(self, modalidad, variante):
        datos = self.datos(variante)
        datos[0x0AD4], datos[0x8082], datos[0x9048 + 11] = modalidad, 11, 1
        for campo, valor in ((2, 150), (4, 100), (6, 32), (8, 48)):
            escribir(datos, 0x2C4C + campo, valor)
        for tabla, base in ((0x8E41, 0xA000), (0x901E, 0xA800), (0x92FE, 0xB000)):
            escribir(datos, tabla + 2, base)
        for tabla, cantidad in ((0x8E14, 15), (0xA000, 240)):
            for i in range(cantidad): struct.pack_into('<BH', datos, tabla + i * 3, 10, 0x9800 if variante == 0 else 0)
        for tabla, cantidad in ((0x8FD3, 15), (0xA800, 240)):
            for i in range(cantidad): struct.pack_into('<BBHB', datos, tabla + i * 5, 10, i % 8, i * 17, i % 256)
        for tabla, cantidad in ((0x92EF, 15), (0xB000, 240)):
            for i in range(cantidad): datos[tabla + i] = 0xFF if variante else i % 128
        return datos

    def comparar_sonido(self, datos, identificador, prioridad, activo, variante, grupo='seleccion_sonido'):
        r, m = self.r, self.maquina
        estado = (ctypes.c_ushort * 6)(prioridad, prioridad, activo, 0x5000,
                                      0 if variante else 0x5100, 0x0F31)
        self.destino, self.seleccionando = None, True
        m.mem_write(self.segmento, bytes(datos))
        for direccion, contenido in ((0x5306, bytes((prioridad,))), (0x5375, bytes((prioridad,))),
                                     (0x8351, bytes((activo,))), (0x5284, struct.pack('<H', estado[3])),
                                     (0x5286, struct.pack('<H', estado[4]))):
            m.mem_write(direccion, contenido)
        for registro, valor in ((r.UC_X86_REG_CS, 0), (r.UC_X86_REG_DS, 0x0F31),
            (r.UC_X86_REG_SS, 0x8000), (r.UC_X86_REG_SP, 0xFF00), (r.UC_X86_REG_SI, 0x2C4C),
            (r.UC_X86_REG_AX, identificador), (r.UC_X86_REG_BX, 0x1234), (r.UC_X86_REG_EFLAGS, 2)):
            m.reg_write(registro, valor)
        datos[0x9924] = activo
        m.mem_write(self.segmento + 0x9924, bytes((activo,)))
        m.mem_write(0x8FF00, struct.pack('<H', 0x7000))
        m.emu_start(0x5288, 0x7000, count=20000)
        self.seleccionando = False
        destino = self.destino or 0
        pila = 0xFF02 if destino == 0 else 0xFEF6 if destino == 0x7FB3 else 0xFEFA
        if m.reg_read(r.UC_X86_REG_SP) != pila or (destino == 0 and m.reg_read(r.UC_X86_REG_IP) != 0x7000):
            raise ErrorAnalisis('Selección de sonido: frontera o pila original inesperada.')
        salida = (ctypes.c_ushort * 5)()
        memoria = (ctypes.c_ubyte * 65536).from_buffer(datos)
        if self.nativa.seleccionar_sonido(memoria, estado, 0x2C4C, identificador, salida) != 1:
            raise ErrorAnalisis('El selector C++ rechazó una entrada preparada.')
        if bytes(m.mem_read(self.segmento, 65536)) != datos:
            raise ErrorAnalisis(f'{grupo}: escrituras en DS diferentes.')
        esperado = [destino, m.reg_read(r.UC_X86_REG_DS), m.reg_read(r.UC_X86_REG_SI),
                    m.reg_read(r.UC_X86_REG_AX), m.reg_read(r.UC_X86_REG_BX) & 0xFF]
        campos = (0,) if destino == 0 else (0, 1, 2, 3, 4) if destino == 0x7FB3 else (0, 1, 2)
        if any(salida[i] != esperado[i] for i in campos):
            raise ErrorAnalisis(f'{grupo}: orden de dispositivo diferente: {list(salida)} frente a {esperado}.')
        if list(estado)[:2] != [m.mem_read(0x5306, 1)[0], m.mem_read(0x5375, 1)[0]]:
            raise ErrorAnalisis('Prioridades de sonido diferentes.')
        self.resumen[grupo] = self.resumen.get(grupo, 0) + 1

    def contenido_modelo(self, variante):
        datos = bytearray(((i * 13 + variante * 7) & 255) for i in range(65536))
        for i in range(128):
            escribir(datos, i * 2, 0x200 + i)
            escribir(datos, (0x200 + i) * 16, 16 + i + variante)
            escribir(datos, (0x200 + i) * 16 + 2, 20 + i + variante)
        return datos

    def ejecutar(self):
        for modalidad, identificador, prioridad, activo, variante in product(range(5),
            (0, 14, 15, 16, 254, 255), (9, 10) if self.muestra else (0, 9, 10, 11, 255),
            range(2), range(1 if self.muestra else 2)):
            self.comparar_sonido(self.preparar_sonido(modalidad, variante), identificador, prioridad, activo, variante)
        for modalidad, geometria in product(range(5), ((-33, 100), (-32, 100), (-31, 100), (319, 100),
            (320, 100), (150, -49), (150, -48), (150, -47), (150, 200))):
            datos = self.preparar_sonido(modalidad, 0)
            escribir(datos, 0x2C4E, geometria[0]); escribir(datos, 0x2C50, geometria[1])
            self.comparar_sonido(datos, 3, 0, 0, 0, 'visibilidad')
        fuente = self.contenido_modelo(1)
        for modalidad, longitud, cache, numero in product((0, 1, 2, 3, 4), (0, 128, 64000, 65000),
                                                         range(2), range(1 if self.muestra else 8)):
            datos = self.preparar_sonido(modalidad, numero % 2)
            datos[0x2C4C] = 1
            for campo in (0x10, 0x15, 0x1A): escribir(datos, 0x2C4C + campo, 0)
            escribir(datos, 0x2C5C, 0x9000)
            datos[0x2C60] = 2
            datos[0x9000:0x9007] = bytes((0xF8, 4, 0xFB, 3, 7, 0x7F, 1))
            datos[0x9700:0x970B] = b'MODELO.SPR\0'
            escribir(datos, 0x0920, 0x9700)
            datos[0x9974] = 4 if cache else 3
            if modalidad in (1, 3, 4): escribir(datos, 0x2C4E, 320)
            modelo = self.contenido_modelo(0)
            self.maquina.mem_write(0x42000, bytes(modelo))
            self.archivo = bytes(fuente[:longitud])
            estado = self.semilla_original
            for paso in range(2):
                datos[0x2C5E:0x2C60] = bytes((0, 1))
                bitacora = (ctypes.c_ushort * 3)()
                self.llamadas_dos = []
                estado = self.caso('secuencia_con_servicios', datos, 0x83EA,
                    lambda m, ax, e: int(self.nativa.ejecutar_operaciones_secuencia(m, e,
                        (ctypes.c_ubyte * 65536).from_buffer(modelo),
                        (ctypes.c_ubyte * len(fuente)).from_buffer(fuente), longitud, bitacora, 0x2C4C) == 1),
                    {self.r.UC_X86_REG_SI: 0x2C4C}, semilla=estado, destinos={0x87C7: 1})
                if bytes(self.maquina.mem_read(0x42000, 65536)) != modelo:
                    raise ErrorAnalisis('Contenido del modelo diferente, incluida la cola de la lectura corta.')
                cargar = paso == 0 and not cache
                esperado = [('abrir', 'MODELO.SPR'), ('leer', 7, 0x4200, 0, 64000), ('cerrar', 7)] if cargar else []
                if self.llamadas_dos != esperado or list(bitacora) != ([1, 0x9700, 64000] if cargar else [0, 0, 0]):
                    raise ErrorAnalisis('Apertura, lectura, cierre o caché de modelo diferentes.')
        informe = {'sha256_ejecutable': self.cabecera['sha256'],
            'sha256_biblioteca_comparada': hashlib.sha256(self.biblioteca.read_bytes()).hexdigest(),
            'version_unicorn': self.unicorn.__version__, 'casos_por_grupo': self.resumen,
            'casos_totales': sum(self.resumen.values()), 'resultado': 'coinciden', 'muestra': self.muestra,
            'alcance': 'Selección de sonido hasta entrada al dispositivo, prioridades y DS completo. '
                'Secuencias con carga sintética y sonido sin emisión; comprueba petición de 64000 bytes y caché. '
                'No ejecuta los dispositivos de audio ni lee archivos comerciales.'}
        salida = self.raiz / 'trabajo/comparacion_operaciones' / ('muestra.json' if self.muestra else 'resultado.json')
        salida.parent.mkdir(parents=True, exist_ok=True)
        salida.write_text(json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
        for grupo, cantidad in self.resumen.items(): print(f'{grupo}: {cantidad} casos coincidentes.')
        print(f'Total: {informe["casos_totales"]} casos. Coinciden selección, memoria y operaciones comprobadas.')
        print(f'Informe local: {salida}')


if __name__ == '__main__':
    argumentos = argparse.ArgumentParser(description='Compara las operaciones externas de las secuencias.')
    argumentos.add_argument('--biblioteca', type=Path, required=True)
    argumentos.add_argument('--muestra', action='store_true')
    opciones = argumentos.parse_args()
    try:
        ComparacionOperaciones(Path(__file__).resolve().parents[1], opciones.biblioteca, opciones.muestra).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        argumentos.exit(1, f'Error: {error}\n')
