"""Contrasta CURSE.SAV con las lecturas y escrituras DOS de E19F, E325 y E793."""

import argparse
import ctypes
import json
import struct
from pathlib import Path

from comparar_menu import ComparacionMenu, ErrorAnalisis
from comparar_objetos import guardar

TAMANO_ESTADO = 10151
TAMANO_REGISTRO = TAMANO_ESTADO + 13
TAMANO_ARCHIVO = 1 + 8 * TAMANO_REGISTRO


class ComparacionPartidas(ComparacionMenu):
    # Reutiliza la máquina original y facilita un archivo DOS en memoria, sin tocar partidas locales.
    def __init__(self, raiz, biblioteca):
        super().__init__(raiz, biblioteca, True)
        memoria = ctypes.POINTER(ctypes.c_ubyte)
        self.nativa.procesar_partidas.argtypes = [
            ctypes.c_uint,
            memoria,
            ctypes.c_uint,
            ctypes.c_uint,
            ctypes.c_char_p,
            memoria,
        ]
        self.nativa.procesar_partidas.restype = ctypes.c_int
        self.archivo_dos = bytearray()
        self.cursor_dos = 0

    # Sustituye solamente INT 21h; las instrucciones originales deciden desplazamientos y tamaños.
    def servicio_modelo(self, maquina, numero, contexto):
        if numero != 0x21:
            raise ErrorAnalisis('Interrupción inesperada en el archivo de partidas.')
        r = self.r
        ax = maquina.reg_read(r.UC_X86_REG_AX)
        funcion = ax >> 8
        cantidad = maquina.reg_read(r.UC_X86_REG_CX)
        dx = maquina.reg_read(r.UC_X86_REG_DX)
        puntero = maquina.reg_read(r.UC_X86_REG_DS) * 16 + dx
        if funcion == 0x40:
            fin = self.cursor_dos + cantidad
            if fin > len(self.archivo_dos):
                self.archivo_dos.extend(bytes(fin - len(self.archivo_dos)))
            self.archivo_dos[self.cursor_dos : fin] = bytes(maquina.mem_read(puntero, cantidad))
            self.cursor_dos = fin
            maquina.reg_write(r.UC_X86_REG_AX, cantidad)
        elif funcion == 0x3F:
            leido = self.archivo_dos[self.cursor_dos : self.cursor_dos + cantidad]
            maquina.mem_write(puntero, bytes(leido))
            self.cursor_dos += len(leido)
            maquina.reg_write(r.UC_X86_REG_AX, len(leido))
        elif funcion == 0x42:
            desplazamiento = (cantidad << 16) | dx
            if desplazamiento & 0x80000000:
                desplazamiento -= 1 << 32
            self.cursor_dos = (
                desplazamiento if (ax & 255) == 0 else self.cursor_dos + desplazamiento
            )
            maquina.reg_write(r.UC_X86_REG_AX, self.cursor_dos & 65535)
            maquina.reg_write(r.UC_X86_REG_DX, self.cursor_dos >> 16)
        elif funcion != 0x3E:
            raise ErrorAnalisis(f'Servicio de partidas inesperado: {funcion:02X}.')
        maquina.reg_write(r.UC_X86_REG_EFLAGS, maquina.reg_read(r.UC_X86_REG_EFLAGS) & ~1)

    # Copia entradas independientes y exige que ninguna operación escriba fuera del archivo o DS.
    def nativa_archivo(self, operacion, archivo, datos, ranura, nombre=b'Alex 286'):
        buffer = (ctypes.c_ubyte * (len(archivo) + 16))(*archivo, *([0xCD] * 16))
        destino = (ctypes.c_ubyte * (65536 + 16))(*datos, *([0xCD] * 16))
        resultado = self.nativa.procesar_partidas(
            operacion, buffer, len(archivo), ranura, nombre, destino
        )
        if bytes(buffer[len(archivo) :]) != bytes([0xCD] * 16) or bytes(destino[65536:]) != bytes(
            [0xCD] * 16
        ):
            raise ErrorAnalisis('La operación de partidas sobrepasa su destino.')
        return resultado, bytes(buffer[: len(archivo)]), bytes(destino[:65536])

    # E718, E253 y E824 se contrastan para ocho ranuras, nombres de trece bytes y máscaras distintas.
    def ejecutar(self):
        total = 0
        for ranura in range(8):
            for mascara in (0, 1, 0x55, 0x7F, 0x80, 0xFF):
                for nombre in (b'Alex 286', b'Trece letras!'):
                    datos = bytearray(self.base)
                    datos[0x2C0A:0x53B1] = self.azar.randbytes(TAMANO_ESTADO)
                    guardar(datos, 0x53B1, TAMANO_ESTADO)
                    archivo = bytearray(self.azar.randbytes(TAMANO_ARCHIVO))
                    archivo[0] = mascara
                    self.archivo_dos = bytearray(archivo)
                    self.cursor_dos = 1 + ranura * TAMANO_REGISTRO
                    datos[0xB602] = mascara | (1 << ranura)
                    datos[0xBADA : 0xBADA + 13] = nombre.ljust(13, b' ')
                    self.maquina.mem_write(self.segmento, bytes(datos))
                    self.maquina.reg_write(self.r.UC_X86_REG_BP, 0xFF80)
                    self.maquina.mem_write(0x8FF7E, struct.pack('<H', 1))
                    self.ejecutar_hasta(0xE718, 0xE74B)
                    hecho, salida, _ = self.nativa_archivo(1, archivo, datos, ranura, nombre)
                    if not hecho or salida != bytes(self.archivo_dos):
                        raise ErrorAnalisis(f'Guardado distinto en ranura {ranura + 1}.')
                    total += 1
                    self.archivo_dos = bytearray(salida)
                    destino = bytearray(self.base)
                    guardar(destino, 0x53B1, TAMANO_ESTADO)
                    self.maquina.mem_write(self.segmento, bytes(destino))
                    self.maquina.reg_write(self.r.UC_X86_REG_BP, 0xFF80)
                    self.maquina.mem_write(0x8FF7C, struct.pack('<H', 1))
                    self.maquina.reg_write(self.r.UC_X86_REG_DX, ranura)
                    self.ejecutar_hasta(0xE253, 0xE27C)
                    esperado = bytes(self.maquina.mem_read(self.segmento, 65536))
                    hecho, _, restaurado = self.nativa_archivo(2, salida, destino, ranura)
                    if not hecho or restaurado != esperado:
                        raise ErrorAnalisis(f'Lectura distinta en ranura {ranura + 1}.')
                    total += 1
                    self.archivo_dos = bytearray(salida)
                    destino[0xB602] = salida[0]
                    self.maquina.mem_write(self.segmento, bytes(destino))
                    self.maquina.reg_write(self.r.UC_X86_REG_AX, ranura)
                    self.maquina.reg_write(self.r.UC_X86_REG_BP, 0xFF80)
                    self.maquina.mem_write(0x8FF7E, struct.pack('<H', 1))
                    self.ejecutar_hasta(0xE824, 0xE855)
                    hecho, borrado, _ = self.nativa_archivo(3, salida, destino, ranura)
                    if not hecho or borrado != bytes(self.archivo_dos):
                        raise ErrorAnalisis(f'Borrado distinto en ranura {ranura + 1}.')
                    total += 1
        for archivo, ranura in (
            (b'', 0),
            (b'\x01', 0),
            (b'\x40' + bytes(10), 6),
            (bytes(TAMANO_ARCHIVO + 1), 0),
            (bytes(TAMANO_ARCHIVO), 8),
        ):
            for operacion in (1, 2, 3):
                resultado, salida, datos = self.nativa_archivo(
                    operacion, archivo, bytearray(self.base), ranura
                )
                if resultado or salida != archivo or datos != bytes(self.base):
                    raise ErrorAnalisis('Entrada inválida aceptada o modificada.')
                total += 1
        carpeta = self.raiz / 'apps/juego/trabajo/comparacion_partidas'
        carpeta.mkdir(exist_ok=True)
        # Genera una partida con las escrituras originales y un estado real contrastado.
        referencia = self.raiz / 'apps/juego/trabajo/comparacion_final_cuevas/final_barro.ds.bin'
        if referencia.exists():
            datos = bytearray(referencia.read_bytes())
            datos[0xBADA : 0xBADA + 13] = b'DOS ORIGINAL '.ljust(13, b' ')
            datos[0xB602] = 1
            self.archivo_dos = bytearray(TAMANO_ARCHIVO)
            self.cursor_dos = 1
            self.maquina.mem_write(self.segmento, bytes(datos))
            self.maquina.reg_write(self.r.UC_X86_REG_BP, 0xFF80)
            self.maquina.mem_write(0x8FF7E, struct.pack('<H', 1))
            self.ejecutar_hasta(0xE718, 0xE74B)
            (carpeta / 'original_dos.sav').write_bytes(self.archivo_dos)
        (carpeta / 'resultado.json').write_text(
            json.dumps(
                {
                    'resultado': 'coinciden',
                    'casos': total,
                    'ranuras': 8,
                    'bytes_estado': TAMANO_ESTADO,
                },
                indent=2,
            )
            + '\n',
            encoding='utf-8',
        )
        print(f'Partidas DOS: {total} casos coincidentes; ocho ranuras y límites comprobados.')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Compara el formato CURSE.SAV con DOS.')
    parser.add_argument('--biblioteca', type=Path, required=True)
    parser.add_argument('--muestra', action='store_true')
    opciones = parser.parse_args()
    try:
        ComparacionPartidas(Path(__file__).resolve().parents[3], opciones.biblioteca).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        parser.exit(1, f'Error: {error}\n')
