"""Compara cada paleta del fundido con las escrituras DAC de 77E4."""

import argparse
import ctypes
import json
from pathlib import Path
import struct

from comparar_video import ComparacionVideo, ErrorAnalisis


class ComparacionFundidos(ComparacionVideo):
    # Añade únicamente el servicio de memoria DOS que necesita el fundido original.
    def __init__(self, raiz, biblioteca, muestra):
        super().__init__(raiz, biblioteca, muestra)
        memoria = ctypes.POINTER(ctypes.c_ubyte)
        self.nativa.calcular_fundido.argtypes = [
            memoria,
            ctypes.c_ushort,
            ctypes.c_ushort,
            ctypes.c_ushort,
            memoria,
        ]
        self.nativa.calcular_fundido.restype = ctypes.c_int
        self.maquina.hook_add(self.unicorn.UC_HOOK_INTR, self.interrupcion)
        self.fase_retrazo = 0

    # Alterna la señal de retrazo para permitir avanzar las esperas sin medir tiempo real.
    def leer_puerto(self, maquina, puerto, tamano, contexto):
        if puerto == 0x3DA and tamano == 1:
            self.fase_retrazo ^= 8
            return self.fase_retrazo
        return super().leer_puerto(maquina, puerto, tamano, contexto)

    # Sustituye reserva/liberación DOS por un segmento fijo solo durante la comparación.
    def interrupcion(self, maquina, numero, contexto):
        r = self.r
        funcion = maquina.reg_read(r.UC_X86_REG_AX) >> 8
        if numero != 0x21 or funcion not in (0x48, 0x49):
            raise ErrorAnalisis('Servicio DOS inesperado en el fundido.')
        if funcion == 0x48:
            maquina.reg_write(r.UC_X86_REG_AX, 0x6000)
        maquina.reg_write(r.UC_X86_REG_EFLAGS, maquina.reg_read(r.UC_X86_REG_EFLAGS) & ~1)

    # Contrasta todos los pasos visibles y verifica que la paleta de partida no se modifica.
    def caso(self, paleta, pasos):
        m, r = self.maquina, self.r
        self.traza = []
        m.mem_write(0x40000, paleta)
        for registro, valor in (
            (r.UC_X86_REG_CS, 0),
            (r.UC_X86_REG_DS, 0x4000),
            (r.UC_X86_REG_ES, 0x5000),
            (r.UC_X86_REG_SS, 0x8000),
            (r.UC_X86_REG_SP, 0xFF00),
            (r.UC_X86_REG_SI, 0),
            (r.UC_X86_REG_AX, pasos),
            (r.UC_X86_REG_BX, 256),
            (r.UC_X86_REG_EFLAGS, 2),
        ):
            m.reg_write(registro, valor)
        m.mem_write(0x8FF00, struct.pack('<H', 0x7000))
        m.emu_start(0x77E4, 0x7000, count=5_000_000)
        if m.reg_read(r.UC_X86_REG_IP) != 0x7000 or m.reg_read(r.UC_X86_REG_SP) != 0xFF02:
            raise ErrorAnalisis('El fundido original no terminó con la pila equilibrada.')
        originales = bytes(valor for _, puerto, valor, _ in self.traza if puerto == 0x3C9)
        if len(originales) != pasos * 768 or bytes(m.mem_read(0x40000, 768)) != paleta:
            raise ErrorAnalisis('Cantidad de pasos o paleta original modificada.')
        entrada = (ctypes.c_ubyte * 768).from_buffer_copy(paleta)
        for paso in range(1, pasos + 1):
            salida = (ctypes.c_ubyte * 768)()
            if (
                not self.nativa.calcular_fundido(entrada, 256, pasos, paso, salida)
                or bytes(salida) != originales[(paso - 1) * 768 : paso * 768]
            ):
                raise ErrorAnalisis(f'Paleta distinta en el paso {paso}/{pasos}.')
            self.resumen['pasos_fundido'] = self.resumen.get('pasos_fundido', 0) + 1

    # Comprueba las paletas del arranque; la muestra usa sus 32 pasos originales.
    def ejecutar(self):
        for nombre in ('CORE.PAL', 'TITLE.PAL'):
            paleta = (self.raiz / 'datos_originales/CURSE/DATA' / nombre).read_bytes()
            for pasos in (32,) if self.muestra else (1, 3, 7, 32):
                self.caso(paleta, pasos)
        informe = {
            'sha256_ejecutable': self.cabecera['sha256'],
            'casos_por_grupo': self.resumen,
            'casos_totales': sum(self.resumen.values()),
            'resultado': 'coinciden',
            'muestra': self.muestra,
            'alcance': 'Paletas RGB de cada paso frente al DAC original. La señal de retrazo se simula; '
            'no certifica duración de presentación ni sincronización de un monitor real.',
        }
        salida = self.raiz / 'trabajo/comparacion_fundidos'
        salida.mkdir(parents=True, exist_ok=True)
        (salida / ('muestra.json' if self.muestra else 'resultado.json')).write_text(
            json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8'
        )
        print(
            f'Fundidos: coinciden las {informe["casos_totales"]} paletas comparadas con el original.'
        )


if __name__ == '__main__':
    argumentos = argparse.ArgumentParser(description='Compara los fundidos originales de paleta.')
    argumentos.add_argument('--biblioteca', type=Path, required=True)
    argumentos.add_argument('--muestra', action='store_true')
    opciones = argumentos.parse_args()
    try:
        ComparacionFundidos(
            Path(__file__).resolve().parents[1], opciones.biblioteca, opciones.muestra
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        argumentos.exit(1, f'Error: {error}\n')
