"""Compara canales, reloj y escrituras AdLib con el reproductor musical del EXE."""

import argparse
import ctypes
import hashlib
import json
from pathlib import Path
import struct

from comparar_avance import Comparacion, ErrorAnalisis


class ComparacionMusica(Comparacion):
    # Conserva la imagen CS separada de DS, pues el reproductor modifica sus propios datos CS.
    def __init__(self, raiz, biblioteca, muestra):
        super().__init__(raiz, biblioteca, muestra)
        self.codigo = (raiz / 'datos_originales/CURSE/CURSE.EXE').read_bytes()[512:66048]
        self.nativa.procesar_musica.argtypes = [
            ctypes.c_uint,
            ctypes.POINTER(ctypes.c_ubyte),
            ctypes.c_ushort,
            ctypes.POINTER(ctypes.c_ushort),
        ]
        self.nativa.procesar_musica.restype = ctypes.c_int
        self.traza, self.registro = [], 0

        # Registra las dos escrituras de puerto como una pareja de registro y valor.
        def escribir(maquina, puerto, tamano, valor, contexto):
            if tamano != 1 or puerto not in (0x388, 0x389):
                raise ErrorAnalisis('Puerto inesperado del reproductor AdLib.')
            if puerto == 0x388:
                self.registro = valor
            else:
                self.traza.append((self.registro, valor))

        self.maquina.hook_add(
            self.unicorn.UC_HOOK_INSN, escribir, None, 1, 0, self.r.UC_X86_INS_OUT
        )

    # Evita calibrar un procesador DOS: las esperas de bus no cambian el orden de los registros.
    def preparar(self):
        datos = bytearray(self.codigo)
        struct.pack_into('<HHH', datos, 0x8FF6, 0, 1, 1)
        return datos

    # Ejecuta el fragmento original y su traslado; compara todo CS y cada escritura del chip.
    def contrastar(self, grupo, datos, operacion, parametro=0):
        self.traza = []
        self.maquina.mem_write(0, bytes(datos))
        for registro, valor in {
            self.r.UC_X86_REG_CS: 0,
            self.r.UC_X86_REG_DS: 0,
            self.r.UC_X86_REG_SS: 0x8000,
            self.r.UC_X86_REG_SP: 0xFF00,
            self.r.UC_X86_REG_AX: parametro,
            self.r.UC_X86_REG_DI: parametro,
            self.r.UC_X86_REG_SI: 0x9749,
            self.r.UC_X86_REG_EFLAGS: 2,
        }.items():
            self.maquina.reg_write(registro, valor)
        self.maquina.mem_write(0x8FF00, struct.pack('<H', 0x7000))
        self.maquina.emu_start((0x8DB0, 0x8DFE, 0x8E5F, 0x8FFC)[operacion], 0x7000, count=300000)
        if self.maquina.reg_read(self.r.UC_X86_REG_IP) != 0x7000:
            raise ErrorAnalisis(f'{grupo}: el fragmento original no retornó.')
        salida = (ctypes.c_ushort * 2049)()
        if not self.nativa.procesar_musica(
            operacion, (ctypes.c_ubyte * 65536).from_buffer(datos), parametro, salida
        ):
            raise ErrorAnalisis(f'{grupo}: el traslado rechazó el caso.')
        esperado = bytes(self.maquina.mem_read(0, 65536))
        if esperado != datos:
            diferencia = next(i for i, (a, b) in enumerate(zip(esperado, datos)) if a != b)
            raise ErrorAnalisis(f'{grupo}: diferencia en CS:{diferencia:04X}.')
        registros = [tuple(salida[1 + 2 * i : 3 + 2 * i]) for i in range(salida[0])]
        if registros != self.traza:
            raise ErrorAnalisis(f'{grupo}: escrituras OPL distintas: {registros} / {self.traza}.')
        if self.maquina.reg_read(self.r.UC_X86_REG_SP) != 0xFF02:
            raise ErrorAnalisis(f'{grupo}: pila desequilibrada.')
        self.resumen[grupo] = self.resumen.get(grupo, 0) + 1
        return registros

    # 4DD5–4DDC omite la selección RS con AdLib: cambiar de escena no toca CS ni DS.
    def contrastar_transiciones_adlib(self):
        codigo = self.preparar()
        for escena in range(78):
            self.maquina.mem_write(0, bytes(codigo))
            datos = bytearray(self.base)
            datos[0x0AD3] = 2
            struct.pack_into('<H', datos, 0x8082, escena)
            self.maquina.mem_write(self.segmento, bytes(datos))
            antes = bytes(self.maquina.mem_read(0, 65536))
            for registro, valor in {
                self.r.UC_X86_REG_CS: 0,
                self.r.UC_X86_REG_DS: 0x0F31,
                self.r.UC_X86_REG_SS: 0x8000,
                self.r.UC_X86_REG_SP: 0xFF00,
                self.r.UC_X86_REG_EFLAGS: 2,
            }.items():
                self.maquina.reg_write(registro, valor)
            self.traza = []
            self.maquina.emu_start(0x4DD5, 0x4E98, count=20)
            if (
                self.maquina.reg_read(self.r.UC_X86_REG_IP) != 0x4E98
                or bytes(self.maquina.mem_read(0, 65536)) != antes
                or bytes(self.maquina.mem_read(self.segmento, 65536)) != datos
                or self.maquina.reg_read(self.r.UC_X86_REG_SP) != 0xFF00
                or self.traza
            ):
                raise ErrorAnalisis('La transición original alteró la música AdLib.')
        self.resumen['transiciones_adlib'] = 78

    # Recorre los temas reales y prepara casos de borde que ejercitan cada orden del intérprete.
    def ejecutar(self):
        pulsos = 1024 if self.muestra else 8192
        registros_totales = 0
        for tema in range(2):
            datos = self.preparar()
            self.contrastar('inicializacion_chip', datos, 3)
            self.contrastar('inicio_tema', datos, 0, tema)
            for _ in range(pulsos):
                registros_totales += len(self.contrastar(f'tema_{tema}', datos, 1))
        for contador in (0, 1, 2, 255, 65535):
            for cadencia in (0, 1, 7, 65535):
                datos = self.preparar()
                self.contrastar('preparar_reloj', datos, 0, 1)
                struct.pack_into('<HH', datos, 0x8E5B, contador, cadencia)
                self.contrastar('fronteras_reloj', datos, 1)
        patrones = [
            bytes((0,)),
            bytes((0xFE, 0, 0x81, 25)),
            bytes((0xFD, 12, 0xFE, 1, 0x80, 25)),
            bytes((0xFC, 0xFD, 255, 0xFE, 2, 1)),
            bytes((0xFF,)),
        ]
        for canal in range(9):
            for patron in patrones:
                for repeticion in (0, 1, 128, 255):
                    datos = self.preparar()
                    self.contrastar('preparar_canal', datos, 0, 0)
                    posicion = 0x8CE2 + 17 * canal
                    struct.pack_into(
                        '<HHHHHH', datos, posicion + 1, 0, 0, 0xA100, 0xA120, 0x958F, 0xA000
                    )
                    datos[posicion + 13] = repeticion
                    datos[posicion + 16] = 0x2F
                    struct.pack_into(
                        '<HHHHHH', datos, 0xA000, 0xFFFE, 2, 0xFFFD, 0xA010, 0xFFFF, 0xFFFF
                    )
                    struct.pack_into('<HH', datos, 0xA010, 0xA120, 0xFFFF)
                    datos[0xA100 : 0xA100 + len(patron)] = patron
                    datos[0xA120:0xA124] = bytes((0xFE, 3, 0x82, 30))
                    self.contrastar('ordenes_canal', datos, 2, posicion)
        self.contrastar_transiciones_adlib()
        if not registros_totales:
            raise ErrorAnalisis('Los temas no emitieron notas.')
        informe = {
            'sha256_ejecutable': self.cabecera['sha256'],
            'sha256_biblioteca': hashlib.sha256(self.biblioteca.read_bytes()).hexdigest(),
            'casos_por_grupo': self.resumen,
            'casos_totales': sum(self.resumen.values()),
            'registros_en_temas': registros_totales,
            'pulsos_por_tema': pulsos,
            'resultado': 'coinciden CS completo, pila y escrituras OPL',
            'limites': 'No valida acústica ni calibración DOS de las esperas de puerto. La síntesis se comprueba aparte.',
        }
        carpeta = self.raiz / 'apps/juego/trabajo/comparacion_musica'
        carpeta.mkdir(parents=True, exist_ok=True)
        salida = carpeta / ('muestra.json' if self.muestra else 'resultado.json')
        salida.write_text(
            json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8'
        )
        for grupo, cantidad in self.resumen.items():
            print(f'{grupo}: {cantidad} casos coincidentes.')
        print(
            f'Total: {informe["casos_totales"]}; {registros_totales} escrituras OPL en los temas.'
        )
        print(f'Informe local: {salida}')


if __name__ == '__main__':
    argumentos = argparse.ArgumentParser(
        description='Contrasta el reproductor musical AdLib original.'
    )
    argumentos.add_argument('--biblioteca', type=Path, required=True)
    argumentos.add_argument('--muestra', action='store_true')
    opciones = argumentos.parse_args()
    try:
        ComparacionMusica(
            Path(__file__).resolve().parents[3], opciones.biblioteca, opciones.muestra
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        argumentos.exit(1, f'Error: {error}\n')
