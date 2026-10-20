"""Contrasta CE35–CF25 y su repetición; no habilita todavía SNOWAST4 en SDL."""

import argparse
import ctypes
from itertools import product
import json
from pathlib import Path
import struct

from comparar_avance import Comparacion, ErrorAnalisis
from comparar_objetos import guardar


class ComparacionRutinaNieve(Comparacion):
    # Ejecuta CE35 con retorno centinela, sin sustituir ninguna escritura de la rutina original.
    def original(self, datos):
        m, r = self.maquina, self.r
        m.mem_write(self.segmento, bytes(datos))
        for registro, valor in (
            (r.UC_X86_REG_CS, 0),
            (r.UC_X86_REG_DS, 0x0F31),
            (r.UC_X86_REG_ES, 0x0F31),
            (r.UC_X86_REG_SS, 0x8000),
            (r.UC_X86_REG_SP, 0xFF00),
            (r.UC_X86_REG_EFLAGS, 2),
        ):
            m.reg_write(registro, valor)
        m.mem_write(0x8FF00, struct.pack('<H', 0x7000))
        m.emu_start(0xCE35, 0x7000, count=200)
        if m.reg_read(r.UC_X86_REG_IP) != 0x7000:
            raise ErrorAnalisis('CE35 no llega al retorno esperado.')
        return bytes(m.mem_read(self.segmento, 65536))

    # Compara todo DS, incluidos canales y campos ajenos, y repite tras modificar sus fases.
    def ejecutar(self):
        funcion = self.nativa.procesar_rutina_nieve
        funcion.argtypes = [ctypes.POINTER(ctypes.c_ubyte)]
        funcion.restype = ctypes.c_int
        numero = 0
        for x, ancho, bandera, activo, imagen in product(
            (0, 1, 288, 289, 319, 320, 321, 32767, 32768, 65535),
            (0, 1, 32, 320, 32768, 65535),
            (0, 1, 2, 254, 255),
            (0, 1, 3),
            (0, 31, 32, 33),
        ):
            datos = self.datos(numero)
            guardar(datos, 0x8082, 43)
            guardar(datos, 0x2C4E, x)
            guardar(datos, 0x2C52, ancho)
            guardar(datos, 0x3591, (0, 480, 65535)[numero % 3])
            guardar(datos, 0x2C50, (0, 190, 65535)[numero % 3])
            guardar(datos, 0x2C54, (0, 48, 65535)[numero % 3])
            datos[0x35CA] = bandera
            datos[0x2C4C] = activo
            datos[0x2C4D] = imagen
            for repeticion in range(2):
                esperado = self.original(datos)
                if (
                    funcion((ctypes.c_ubyte * 65536).from_buffer(datos)) != 1
                    or bytes(datos) != esperado
                ):
                    diferencias = [f'{i:04X}' for i in range(65536) if datos[i] != esperado[i]][:8]
                    raise ErrorAnalisis(f'CE35 caso{numero}/{repeticion}: {diferencias}.')
                # Una segunda llamada debe respetar la animación ya iniciada, sin reiniciar fases.
                datos[0x2C5E] = 7
                datos[0x2CA0] = 9
            numero += 1
        destino = self.raiz / 'apps/juego/trabajo/comparacion_rutina_nieve'
        destino.mkdir(parents=True, exist_ok=True)
        informe = {
            'resultado': 'coinciden',
            'variantes': numero,
            'llamadas': numero * 2,
            'alcance': 'CE35–CF25: visibilidad firmada con envoltura, bit persistente, seis canales, proximidad y repetición. No acredita carga, acciones, salidas ni SDL de la nieve.',
        }
        (destino / 'resultado.json').write_text(
            json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8'
        )
        print(
            f'Rutina SNOWAST4: {numero} variantes y {numero * 2} llamadas coincidentes en DS completo.'
        )


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Contrasta únicamente la rutina CE35 de la nieve.')
    parser.add_argument('--biblioteca', type=Path, required=True)
    parser.add_argument('--muestra', action='store_true')
    opciones = parser.parse_args()
    try:
        ComparacionRutinaNieve(
            Path(__file__).resolve().parents[4], opciones.biblioteca, opciones.muestra
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        parser.exit(1, f'Error: {error}\n')
