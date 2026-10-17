"""Contrasta carga, orden y selección de objetos con las rutinas originales."""

import argparse
import ctypes
from itertools import product
import json
from pathlib import Path
import struct

from comparar_avance import Comparacion, ErrorAnalisis


# Lee una palabra del estado usado por el comparador, con la envoltura del segmento DOS.
def palabra(datos, posicion):
    return datos[posicion & 65535] | (datos[(posicion + 1) & 65535] << 8)


# Prepara campos concretos de un caso sin copiar datos comerciales al repositorio.
def guardar(datos, posicion, valor):
    struct.pack_into('<H', datos, posicion, valor & 65535)


class ComparacionObjetos(Comparacion):
    # Prepara el puente nativo y un servicio DOS limitado a la lectura ya dispuesta del modelo.
    def __init__(self, raiz, biblioteca, muestra):
        super().__init__(raiz, biblioteca, muestra)
        memoria = ctypes.POINTER(ctypes.c_ubyte)
        palabras = ctypes.POINTER(ctypes.c_ushort)
        self.nativa.procesar_objetos.argtypes = [
            ctypes.c_uint,
            memoria,
            palabras,
            memoria,
            ctypes.c_uint,
            ctypes.c_ushort,
            ctypes.c_ushort,
            palabras,
        ]
        self.nativa.procesar_objetos.restype = ctypes.c_int
        self.nativa.leer_datos_iniciales.argtypes = [memoria, ctypes.c_uint, memoria]
        self.nativa.leer_datos_iniciales.restype = ctypes.c_int
        self.maquina.hook_add(self.unicorn.UC_HOOK_INTR, self.servicio_dos)
        self.carpeta = raiz / 'datos_originales/CURSE/DATA'

    # Sustituye solo apertura, lectura y cierre del modelo, cuyo contenido ya está en memoria.
    def servicio_dos(self, maquina, numero, contexto):
        r = self.r
        funcion = maquina.reg_read(r.UC_X86_REG_AX) >> 8
        if numero != 0x21 or funcion not in (0x3D, 0x3F, 0x3E):
            raise ErrorAnalisis('Servicio inesperado al preparar el personaje.')
        maquina.reg_write(r.UC_X86_REG_AX, 1 if funcion == 0x3D else 0)
        maquina.reg_write(r.UC_X86_REG_EFLAGS, maquina.reg_read(r.UC_X86_REG_EFLAGS) & ~1)

    # Ejecuta un bloque original y compara DS completo, generador y resultado de selección.
    def caso(self, grupo, operacion, datos, nombre='DUNGEON.SPR', mascara=255, capacidad=68):
        m, r = self.maquina, self.r
        banco = (self.carpeta / nombre).read_bytes()
        m.mem_write(self.segmento, bytes(datos))
        m.mem_write(0x40000, banco)
        m.mem_write(0x4AEE, struct.pack('<H', 0x4000))
        m.mem_write(0x0010, struct.pack('<H', 0x4000))
        m.mem_write(0x78DE, struct.pack('<4H', *self.semilla_original))
        for registro, valor in (
            (r.UC_X86_REG_CS, 0),
            (r.UC_X86_REG_DS, 0x0F31),
            (r.UC_X86_REG_ES, 0x0F31),
            (r.UC_X86_REG_SS, 0x8000),
            (r.UC_X86_REG_SP, 0xFF00),
            (r.UC_X86_REG_EFLAGS, 2),
            (r.UC_X86_REG_CX, capacidad),
            (r.UC_X86_REG_DX, mascara),
            (r.UC_X86_REG_DI, 0xA800),
            (r.UC_X86_REG_BX, 0xFFFF),
        ):
            m.reg_write(registro, valor)
        m.mem_write(0x8FF00, struct.pack('<H', 0x7000))
        if operacion == 4:
            # Se recorren los tres fragmentos que el adaptador reúne, sin ejecutar el cargador DOS completo.
            m.reg_write(r.UC_X86_REG_SI, 0x859E)
            m.reg_write(r.UC_X86_REG_DI, 0x2C2B)
            m.mem_write(self.segmento + 0x3598, b'\xff\xff')
            m.mem_write(self.segmento + 0x2C0B, b'\0')
            m.mem_write(self.segmento + 0x2C1A, b'\0\0')
            m.emu_start(0x4C43, 0x4D2B, count=20000)
            m.emu_start(0x4D2B, 0x4D40, count=20000)
            m.emu_start(0x5145, 0x5182, count=20000)
        else:
            m.emu_start((0x8AA6, 0x183F, 0xF29A, 0x891E)[operacion], 0x7000, count=2_000_000)
            if m.reg_read(r.UC_X86_REG_IP) != 0x7000 or m.reg_read(r.UC_X86_REG_SP) != 0xFF02:
                raise ErrorAnalisis(f'{grupo}: retorno o pila distintos.')
        originales = bytes(m.mem_read(self.segmento, 65536))
        esperada = struct.unpack('<4H', m.mem_read(0x78DE, 8))
        semilla = (ctypes.c_ushort * 4)(*self.semilla_original)
        salida = ctypes.c_ushort()
        memoria = (ctypes.c_ubyte * 65536).from_buffer(datos)
        if not self.nativa.procesar_objetos(
            operacion,
            memoria,
            semilla,
            (ctypes.c_ubyte * len(banco)).from_buffer_copy(banco),
            len(banco),
            mascara,
            capacidad,
            ctypes.byref(salida),
        ):
            raise ErrorAnalisis(f'{grupo}: caso válido rechazado por C++.')
        if datos != originales:
            posicion = next(i for i, (a, b) in enumerate(zip(datos, originales)) if a != b)
            raise ErrorAnalisis(
                f'{grupo}: diferencia en DS:{posicion:04X}: C++={datos[posicion]}, original={originales[posicion]}.'
            )
        if tuple(semilla) != esperada:
            raise ErrorAnalisis(f'{grupo}: generador diferente.')
        if operacion == 3 and salida.value != sum(
            palabra(originales, 0xA800 + i * 2) != 0xFFFF for i in range(capacidad)
        ):
            raise ErrorAnalisis('Cantidad de objetos seleccionados diferente.')
        self.resumen[grupo] = self.resumen.get(grupo, 0) + 1
        return datos

    # Combina listas reales con condiciones de escena, claves de orden y fronteras de proximidad.
    def ejecutar(self):
        ejecutable = (self.raiz / 'datos_originales/CURSE/CURSE.EXE').read_bytes()
        datos = (ctypes.c_ubyte * 65536)()
        if (
            not self.nativa.leer_datos_iniciales(
                (ctypes.c_ubyte * len(ejecutable)).from_buffer_copy(ejecutable),
                len(ejecutable),
                datos,
            )
            or bytes(datos) != self.base
        ):
            raise ErrorAnalisis(
                'La copia del segmento de datos no coincide con el lector MZ de referencia.'
            )
        self.resumen['lectura_datos'] = 1
        for paquete in (
            ejecutable[:-1],
            b'XX' + ejecutable[2:],
            ejecutable[:8] + b'\x21\0' + ejecutable[10:],
        ):
            datos = (ctypes.c_ubyte * 65536)(*([0xCD] * 65536))
            if self.nativa.leer_datos_iniciales(
                (ctypes.c_ubyte * len(paquete)).from_buffer_copy(paquete), len(paquete), datos
            ) or bytes(datos) != bytes([0xCD] * 65536):
                raise ErrorAnalisis('Un MZ incompatible se aceptó o modificó el destino.')
            self.resumen['lectura_datos'] += 1
        self.caso('entrada_personaje', 4, bytearray(self.base), 'MANA.BRD')
        for escena, nombre in ((0, 'DUNGEON.SPR'), (1, 'CORRIDOR.SPR'), (2, 'DROP.SPR')):
            for opciones, camara in product((0, 1), (0,) if self.muestra else (0, 2, 17)):
                datos = bytearray(self.base)
                guardar(datos, 0x8082, escena)
                datos[0x0AD6] = opciones
                datos[0x3597] = camara
                self.caso('listas_reales', 0, datos, nombre)
        for banderas, opciones in product((2, 4, 6, 0x81), (0, 1)):
            datos = bytearray(self.base)
            guardar(datos, 0x8082, 0)
            guardar(datos, 0x53B5, 0xA000)
            datos[0x0AD6] = opciones
            datos[0xA000:0xA009] = struct.pack('<BBBHHH', banderas, 1, 2, 100, 120, 0xA300)
            datos[0xA009:0xA012] = struct.pack('<BBBHHH', 0, 1, 1, 20, 170, 0)
            datos[0xA012] = 255
            datos[0xA300:0xA312] = struct.pack(
                '<4H2B2H2B2B', 0, 0, 0xFFFF, 0xA350, 3, 2, 0, 0x0101, 0, 0, 1, 1
            )
            datos[0xA350:0xA359] = bytes(range(8)) + b'\x7f'
            self.caso('banderas_carga', 0, datos)
        for estado, secuencia, control, condicion in product((0, 1), (0, 1), (0, 1), (0, 1)):
            datos = bytearray(self.base)
            datos[0x2CF1], datos[0x3594], datos[0x2D54] = estado, control, condicion
            guardar(datos, 0x2D01, secuencia)
            guardar(datos, 0x2D14, 0xFFF0)
            guardar(datos, 0x2D16, 0x7FF0)
            self.caso('rutina_mazmorra', 1, datos)
        for caso in range(8 if self.muestra else 128):
            datos = bytearray(self.base)
            for i in range(69):
                entidad = 0x2C2B + i * 33
                for campo in (4, 8, 10):
                    guardar(
                        datos,
                        entidad + campo,
                        self.azar.choice((0, 1, 0xFFFF, 0x7FFF, self.azar.randrange(65536))),
                    )
            self.caso('orden_dibujo', 2, datos)
        for alcance, mascara, desplazamiento in product(
            (0, 1),
            (1, 2, 4, 8, 16, 255),
            (-6, 0, 6) if self.muestra else (-100, -6, -1, 0, 1, 6, 100),
        ):
            datos = bytearray(self.base)
            guardar(datos, 0x2C2D, 100)
            guardar(datos, 0x2C2F, 80)
            guardar(datos, 0x2C31, 32)
            guardar(datos, 0x2C33, 48)
            datos[0x3593] = alcance
            for i in range(12):
                entidad = 0x2C4C + i * 33
                datos[entidad] = (0, 1, 2, 3, 4, 5, 7)[i % 7]
                guardar(datos, entidad + 2, 100 + desplazamiento + i * 4)
                guardar(datos, entidad + 4, 120 + desplazamiento)
                guardar(datos, entidad + 6, 16)
                guardar(datos, entidad + 8, 12)
                guardar(datos, entidad + 12, 4)
                guardar(datos, entidad + 14, (0, 1, 2, 3, 4, 5, 0x400, 0xFFFF)[i % 8])
            self.caso('objetos_cercanos', 3, datos, mascara=mascara)
        informe = {
            'sha256_ejecutable': self.cabecera['sha256'],
            'casos_por_grupo': self.resumen,
            'casos_totales': sum(self.resumen.values()),
            'resultado': 'coinciden',
            'muestra': self.muestra,
            'alcance': 'DS completo y estado aleatorio para carga, rutina de escena, orden y selección. '
            'No verifica todavía el menú de acciones ni la resolución de puzles.',
        }
        salida = self.raiz / 'trabajo/comparacion_objetos'
        salida.mkdir(parents=True, exist_ok=True)
        (salida / ('muestra.json' if self.muestra else 'resultado.json')).write_text(
            json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8'
        )
        for grupo, cantidad in self.resumen.items():
            print(f'{grupo}: {cantidad} casos coincidentes.')
        print(f'Total: {informe["casos_totales"]} casos. Coinciden memoria y estado aleatorio.')


if __name__ == '__main__':
    argumentos = argparse.ArgumentParser(
        description='Compara carga, orden y proximidad de objetos.'
    )
    argumentos.add_argument('--biblioteca', type=Path, required=True)
    argumentos.add_argument('--muestra', action='store_true')
    opciones = argumentos.parse_args()
    try:
        ComparacionObjetos(
            Path(__file__).resolve().parents[1], opciones.biblioteca, opciones.muestra
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        argumentos.exit(1, f'Error: {error}\n')
