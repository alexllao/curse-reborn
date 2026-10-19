"""Contrasta paquetes reales y su retorno circular con el alimentador MPU del EXE."""

import argparse
import ctypes
import json
from pathlib import Path
import struct

from comparar_avance import Comparacion, ErrorAnalisis


class ComparacionMusicaRS(Comparacion):
    # El oráculo ejecuta 7DF9 con una petición F0 y recoge los bytes enviados al puerto 330h.
    def __init__(self, raiz, biblioteca, muestra):
        super().__init__(raiz, biblioteca, muestra)
        memoria = ctypes.POINTER(ctypes.c_ubyte)
        palabras = ctypes.POINTER(ctypes.c_ushort)
        self.nativa.procesar_musica_rs.argtypes = [
            memoria,
            memoria,
            ctypes.c_uint,
            memoria,
            palabras,
        ]
        self.nativa.procesar_musica_rs.restype = ctypes.c_int
        self.nativa.seleccionar_musica_rs.argtypes = [memoria, ctypes.c_ushort, palabras]
        self.nativa.seleccionar_musica_rs.restype = ctypes.c_int
        self.bytes = []

        # El MPU está listo para recibir; no se sustituye la lectura ni interpretación del recurso.
        def leer(maquina, puerto, tamano, contexto):
            if puerto != 0x331 or tamano != 1:
                raise ErrorAnalisis('Lectura MPU inesperada.')
            return 0

        # Conserva todos los bytes, incluidos tiempos, marcas y mensajes con estado implícito.
        def escribir(maquina, puerto, tamano, valor, contexto):
            if puerto != 0x330 or tamano != 1:
                raise ErrorAnalisis('Escritura MPU inesperada.')
            self.bytes.append(valor)

        self.maquina.hook_add(self.unicorn.UC_HOOK_INSN, leer, None, 1, 0, self.r.UC_X86_INS_IN)
        self.maquina.hook_add(
            self.unicorn.UC_HOOK_INSN, escribir, None, 1, 0, self.r.UC_X86_INS_OUT
        )

    # Compara DS, estado MIDI en CS, pila y todos los bytes de una petición real.
    def paquete(self, datos, banco, estado):
        self.bytes = []
        self.maquina.mem_write(self.segmento, bytes(datos))
        self.maquina.mem_write(0x7EED, bytes((estado.value,)))
        self.maquina.mem_write(0x8FF00, struct.pack('<H', 0x7000))
        for r, v in {
            self.r.UC_X86_REG_CS: 0,
            self.r.UC_X86_REG_DS: 0x0F31,
            self.r.UC_X86_REG_SS: 0x8000,
            self.r.UC_X86_REG_SP: 0xFF00,
            self.r.UC_X86_REG_AX: 0xF0,
            self.r.UC_X86_REG_EFLAGS: 2,
        }.items():
            self.maquina.reg_write(r, v)
        self.maquina.emu_start(0x7DF9, 0x7000, count=10000)
        if self.maquina.reg_read(self.r.UC_X86_REG_IP) != 0x7000:
            raise ErrorAnalisis('El alimentador original no retornó.')
        salida = (ctypes.c_ushort * 5)()
        if not self.nativa.procesar_musica_rs(
            (ctypes.c_ubyte * 65536).from_buffer(datos),
            (ctypes.c_ubyte * len(banco)).from_buffer_copy(banco),
            len(banco),
            ctypes.byref(estado),
            salida,
        ):
            raise ErrorAnalisis('Paquete RS rechazado.')
        if datos != bytes(self.maquina.mem_read(self.segmento, 65536)):
            raise ErrorAnalisis('DS distinto tras leer un paquete musical.')
        if estado.value != self.maquina.mem_read(0x7EED, 1)[0]:
            raise ErrorAnalisis('Estado MIDI implícito distinto.')
        if list(salida[1 : 1 + salida[0]]) != self.bytes:
            raise ErrorAnalisis('Bytes MPU distintos.')
        if self.maquina.reg_read(self.r.UC_X86_REG_SP) != 0xFF02:
            raise ErrorAnalisis('Pila MPU desequilibrada.')

    # Recorre los cinco bancos; la comparación amplia alcanza su enlace circular final.
    def ejecutar(self):
        casos = 0
        for indice in range(5):
            banco = (
                self.raiz / f'apps/juego/datos_originales/CURSE/DATA/CURSE.RS{indice + 1}'
            ).read_bytes()
            self.maquina.mem_write(0x60000, banco)
            datos = bytearray(self.base)
            datos[0x9955] = 0
            struct.pack_into('<HH', datos, 0x9926, 0, 0x6000)
            estado = ctypes.c_ubyte(0)
            for _ in range(512 if self.muestra else 20000):
                anterior = struct.unpack_from('<H', datos, 0x9926)[0]
                self.paquete(datos, banco, estado)
                casos += 1
                siguiente = struct.unpack_from('<H', datos, 0x9926)[0]
                if siguiente < anterior:
                    break
            struct.pack_into('<H', datos, 0x9926, len(banco) - 3)
            self.paquete(datos, banco, estado)
            casos += 1
        # 4DE3 usa el byte de 80CD; el descriptor posterior aporta tempo y nombre.
        for escena in range(78):
            salida = (ctypes.c_ushort * 3)()
            datos = (ctypes.c_ubyte * 65536).from_buffer_copy(self.base)
            if not self.nativa.seleccionar_musica_rs(datos, escena, salida):
                raise ErrorAnalisis('Escena musical no aceptada.')
            indice = self.base[0x80CD + escena]
            posicion = 0x80B9 + indice * 4
            if tuple(salida) != (
                indice,
                self.base[posicion],
                struct.unpack_from('<H', self.base, posicion + 1)[0],
            ):
                raise ErrorAnalisis('La referencia musical no corresponde a la tabla original.')
        carpeta = self.raiz / 'apps/juego/trabajo/comparacion_musica_rs'
        carpeta.mkdir(parents=True, exist_ok=True)
        informe = {
            'paquetes': casos,
            'escenas': 78,
            'bancos': 5,
            'muestra': self.muestra,
            'resultado': 'coinciden DS, pila, estado MIDI y bytes MPU',
            'sha256_ejecutable': self.cabecera['sha256'],
        }
        (carpeta / ('muestra.json' if self.muestra else 'resultado.json')).write_text(
            json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8'
        )
        print(
            f'Música RS: {casos} paquetes originales coincidentes; 78 referencias de escena y cinco bancos.'
        )


if __name__ == '__main__':
    argumentos = argparse.ArgumentParser(
        description='Contrasta los recursos musicales RS originales.'
    )
    argumentos.add_argument('--biblioteca', type=Path, required=True)
    argumentos.add_argument('--muestra', action='store_true')
    opciones = argumentos.parse_args()
    try:
        ComparacionMusicaRS(
            Path(__file__).resolve().parents[3], opciones.biblioteca, opciones.muestra
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        argumentos.exit(1, f'Error: {error}\n')
