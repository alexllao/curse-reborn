"""Contrasta selección real, programación DSP/DMA y PCM digital con el ejecutable DOS."""

import argparse
import ctypes
import hashlib
import json
from pathlib import Path
import struct
import wave

from comparar_avance import Comparacion, ErrorAnalisis


class ComparacionDigital(Comparacion):
    # Utiliza Unicorn como oráculo de 5288 y 7FB3, con los bancos comerciales locales sin modificar.
    def __init__(self, raiz, biblioteca, muestra):
        super().__init__(raiz, biblioteca, muestra)
        memoria = ctypes.POINTER(ctypes.c_ubyte)
        self.nativa.procesar_digital.argtypes = [
            ctypes.c_uint,
            memoria,
            memoria,
            ctypes.c_uint,
            memoria,
            ctypes.c_uint,
            ctypes.c_ushort,
            ctypes.c_ubyte,
            ctypes.POINTER(ctypes.c_short),
            ctypes.c_uint,
        ]
        self.nativa.procesar_digital.restype = ctypes.c_int
        self.nativa.preparar_dma_digital.argtypes = [
            ctypes.c_ushort,
            ctypes.c_ushort,
            ctypes.c_uint,
            ctypes.POINTER(ctypes.c_uint),
        ]
        self.nativa.nombre_sfx.argtypes = [memoria, ctypes.c_ushort, ctypes.c_char_p]
        self.maquina.hook_add(self.unicorn.UC_HOOK_CODE, self.instruccion)
        self.maquina.hook_add(
            self.unicorn.UC_HOOK_INSN, self.entrada, None, 1, 0, self.r.UC_X86_INS_IN
        )
        self.maquina.hook_add(
            self.unicorn.UC_HOOK_INSN, self.salida, None, 1, 0, self.r.UC_X86_INS_OUT
        )
        self.seleccionando = False
        self.orden = None
        self.puertos = []
        self.dsp = []
        self.general = (raiz / 'datos_originales/CURSE/DATA/STD.SFX').read_bytes()
        self.bancos = []

    # El estado preparado siempre presenta el DSP listo; no se simula la temporización de su bus ISA.
    def entrada(self, maquina, puerto, tamano, contexto):
        return 0

    # Registra las escrituras DMA de la rutina original para comprobar dirección, página y cuenta.
    def salida(self, maquina, puerto, tamano, valor, contexto):
        self.puertos.append((puerto, valor))

    # Intercepta únicamente la frontera de selección y la función de envío al DSP 839F.
    def instruccion(self, maquina, direccion, tamano, contexto):
        r = self.r
        if self.seleccionando and direccion == 0x7FB3:
            self.orden = tuple(
                maquina.reg_read(registro)
                for registro in (r.UC_X86_REG_DS, r.UC_X86_REG_SI, r.UC_X86_REG_AX, r.UC_X86_REG_BX)
            )
            maquina.emu_stop()
        elif direccion == 0x839F:
            self.dsp.append(maquina.reg_read(r.UC_X86_REG_AX) & 255)
            sp = maquina.reg_read(r.UC_X86_REG_SP)
            ss = maquina.reg_read(r.UC_X86_REG_SS)
            retorno = struct.unpack('<H', maquina.mem_read(ss * 16 + sp, 2))[0]
            maquina.reg_write(r.UC_X86_REG_SP, sp + 2)
            maquina.reg_write(r.UC_X86_REG_IP, retorno)

    # Prepara una llamada cerca, sin alterar el código de las rutinas que se están comprobando.
    def ejecutar_original(self, direccion, registros):
        r, maquina = self.r, self.maquina
        for registro, valor in {
            r.UC_X86_REG_CS: 0,
            r.UC_X86_REG_SS: 0x8000,
            r.UC_X86_REG_SP: 0xFF00,
            r.UC_X86_REG_EFLAGS: 2,
            **registros,
        }.items():
            maquina.reg_write(registro, valor)
        maquina.mem_write(0x8FF00, struct.pack('<H', 0x7000))
        maquina.emu_start(direccion, 0x7000, count=20000)
        if not self.orden and maquina.reg_read(r.UC_X86_REG_IP) != 0x7000:
            raise ErrorAnalisis('La rutina digital original no terminó.')

    # Conserva los búferes ctypes durante toda la sesión porque el motor referencia sus bancos.
    def conectar(self, escena, banco):
        self.datos_actuales = bytearray(self.base)
        self.datos_actuales[0x0AD4] = 3
        self.datos_actuales[0x8082] = escena
        for campo, valor in ((2, 100), (4, 100), (6, 32), (8, 48)):
            struct.pack_into('<H', self.datos_actuales, 0x2C4C + campo, valor)
        self.memoria = (ctypes.c_ubyte * 65536).from_buffer(self.datos_actuales)
        self.bancos = [(ctypes.c_ubyte * len(b)).from_buffer_copy(b) for b in (self.general, banco)]
        self.operar(0)
        self.maquina.mem_write(0x50000, self.general)
        self.maquina.mem_write(0x60000, banco)

    # Invoca el servicio de efectos real y devuelve las muestras sin interpretar su contenido.
    def operar(self, operacion, identificador=0, cantidad=0):
        salida = (ctypes.c_short * cantidad)()
        resultado = self.nativa.procesar_digital(
            operacion,
            self.memoria,
            self.bancos[0],
            len(self.bancos[0]),
            self.bancos[1],
            len(self.bancos[1]),
            0x2C4C,
            identificador,
            salida,
            cantidad,
        )
        if operacion != 3 and resultado != 1:
            raise ErrorAnalisis(
                f'Operación digital rechazada: {operacion}, efecto {identificador}.'
            )
        return resultado if operacion == 3 else list(salida)

    # Obtiene posición, longitud y constante temporal directamente de los registros originales.
    def seleccionar_original(self, identificador, prioridad=0, activa=0):
        self.maquina.mem_write(self.segmento, bytes(self.datos_actuales))
        for direccion, contenido in (
            (0x5284, struct.pack('<H', 0x5000)),
            (0x5286, struct.pack('<H', 0x6000)),
            (0x5375, bytes((prioridad,))),
            (0x8351, bytes((activa,))),
        ):
            self.maquina.mem_write(direccion, contenido)
        self.orden, self.seleccionando = None, True
        self.ejecutar_original(
            0x5288,
            {
                self.r.UC_X86_REG_DS: 0x0F31,
                self.r.UC_X86_REG_SI: 0x2C4C,
                self.r.UC_X86_REG_AX: identificador,
            },
        )
        self.seleccionando = False
        return self.orden

    # Contrasta dirección física, frontera de 64 KiB y los seis bytes DSP emitidos por 7FB3.
    def comparar_dma(self):
        casos = 0
        for segmento in (0, 0x0F31, 0x5000, 0x5123, 0xFFFF):
            for posicion in (0, 1, 0xFFF0, 0xFFFF):
                for longitud in (0, 1, 2, 17, 65535, 65536, 131073):
                    self.orden, self.dsp, self.puertos = None, [], []
                    self.ejecutar_original(
                        0x7FB3,
                        {
                            self.r.UC_X86_REG_DS: segmento,
                            self.r.UC_X86_REG_SI: posicion,
                            self.r.UC_X86_REG_AX: longitud & 65535,
                            self.r.UC_X86_REG_DX: longitud >> 16,
                            self.r.UC_X86_REG_BX: 0x83,
                        },
                    )
                    direccion, cuenta = struct.unpack('<HH', self.maquina.mem_read(0x806F, 4))
                    pagina = self.maquina.mem_read(0x8352, 1)[0]
                    restantes = struct.unpack('<I', self.maquina.mem_read(0x8353, 4))[0]
                    salida = (ctypes.c_uint * 4)()
                    self.nativa.preparar_dma_digital(segmento, posicion, longitud, salida)
                    esperado = [direccion, cuenta, pagina, restantes]
                    puertos = [
                        (10, 5),
                        (12, 0),
                        (11, 0x49),
                        (2, direccion & 255),
                        (2, direccion >> 8),
                        (0x83, pagina),
                        (3, cuenta & 255),
                        (3, cuenta >> 8),
                        (10, 1),
                    ]
                    if (
                        list(salida) != esperado
                        or self.puertos != puertos
                        or self.dsp != [0xD0, 0x40, 0x83, 0x14, cuenta & 255, cuenta >> 8]
                    ):
                        raise ErrorAnalisis(
                            f'Programación DSP/DMA distinta: {segmento:04X}:{posicion:04X}.'
                        )
                    casos += 1
        return casos

    # Compara señal completa y fragmentada con muestras elegidas por el EXE, además de prioridades.
    def ejecutar(self):
        dma = self.comparar_dma()
        escenas, grupos = [], set()
        for escena in range(78):
            referencia = struct.unpack_from('<H', self.base, 0x784E + escena * 8 + 4)[0]
            nombre = self.base[referencia:].split(b'\0')[0].decode('ascii') if referencia else ''
            salida = ctypes.create_string_buffer(13)
            memoria = (ctypes.c_ubyte * 65536).from_buffer_copy(self.base)
            if (
                not self.nativa.nombre_sfx(memoria, escena, salida)
                or salida.value.decode() != nombre
            ):
                raise ErrorAnalisis('Referencia SFX de escena diferente.')
            if nombre:
                banco = (self.raiz / 'datos_originales/CURSE/DATA' / nombre.upper()).read_bytes()
                grupo = self.base[0x9048 + escena]
                if (grupo, nombre) not in grupos:
                    grupos.add((grupo, nombre))
                    escenas.append((escena, grupo, banco))
        if self.muestra:
            escenas = [e for e in escenas if e[0] in (0, 3)]
        tablas = sorted(
            {struct.unpack_from('<H', self.base, 0x901E + i * 2)[0] for i in range(21)} | {0x92E9}
        )
        efectos, total_muestras, voces = 0, 0, []
        for escena, grupo, banco in escenas:
            tabla = struct.unpack_from('<H', self.base, 0x901E + grupo * 2)[0]
            siguientes = [t for t in tablas if t > tabla]
            cantidad = (min(siguientes) - tabla) // 5
            identificadores = list(range(15, 15 + cantidad))
            if escena == 0:
                identificadores = list(range(15)) + identificadores
            for identificador in identificadores:
                self.conectar(escena, banco)
                orden = self.seleccionar_original(identificador)
                if not orden:
                    raise ErrorAnalisis('El original omitió un efecto previsto por su tabla.')
                segmento, posicion, longitud, bx = orden
                fuente = self.general if segmento == 0x5000 else banco
                if posicion + longitud > len(fuente) or not longitud:
                    raise ErrorAnalisis('Muestra original fuera del banco.')
                periodo = 48000 * (256 - (bx & 255))
                duracion = (longitud * periodo + 999999) // 1000000
                esperado = [
                    (fuente[posicion + i * 1000000 // periodo] - 128) * 256 for i in range(duracion)
                ] + [0] * 31
                self.operar(1, identificador)
                if self.operar(2, cantidad=len(esperado)) != esperado or self.operar(3):
                    raise ErrorAnalisis(
                        f'PCM o final de muestra distinto: escena {escena}, efecto {identificador}.'
                    )
                self.conectar(escena, banco)
                self.operar(1, identificador)
                obtenido, paso = [], 0
                while len(obtenido) < len(esperado):
                    longitud_bloque = min(
                        (1, 17, 512, 1023, 4096)[paso % 5], len(esperado) - len(obtenido)
                    )
                    obtenido.extend(self.operar(2, cantidad=longitud_bloque))
                    paso += 1
                if obtenido != esperado or bytes(self.datos_actuales) != self.base_modificada(
                    escena
                ):
                    raise ErrorAnalisis('La partición de bloques modifica PCM o DS.')
                if escena == 0 and identificador < 3:
                    voces.extend(esperado + [0] * 12000)
                total_muestras += len(esperado)
                efectos += 1
        # La voz general cero tiene prioridad FF: un efecto ambiental no puede interrumpirla.
        banco = (self.raiz / 'datos_originales/CURSE/DATA/DUNGEON.SFX').read_bytes()
        self.conectar(0, banco)
        self.operar(1, 0)
        if self.seleccionar_original(15, 255, 1) is not None:
            raise ErrorAnalisis('El original no respeta la prioridad FF esperada.')
        antes = self.operar(2, cantidad=32)
        self.operar(1, 15)
        despues = self.operar(2, cantidad=64)
        self.conectar(0, banco)
        self.operar(1, 0)
        if antes + despues != self.operar(2, cantidad=96):
            raise ErrorAnalisis('Una orden inferior interrumpe una voz digital.')
        self.operar(4)
        if self.operar(3) or any(self.operar(2, cantidad=64)):
            raise ErrorAnalisis('La parada deja una muestra digital activa.')
        destino = self.raiz / 'apps/juego/trabajo/comparacion_digital'
        destino.mkdir(parents=True, exist_ok=True)
        with wave.open(str(destino / 'efectos_digitales.wav'), 'wb') as archivo:
            archivo.setparams((1, 2, 48000, 0, 'NONE', 'sin compresión'))
            archivo.writeframes(struct.pack(f'<{len(voces)}h', *voces))
        informe = {
            'sha256_ejecutable': self.cabecera['sha256'],
            'sha256_biblioteca': hashlib.sha256(self.biblioteca.read_bytes()).hexdigest(),
            'escenas_referenciadas': 78,
            'combinaciones_grupo_banco': len(escenas),
            'programaciones_dma': dma,
            'efectos_comparados': efectos,
            'muestras_comparadas': total_muestras,
            'resultado': 'coinciden órdenes originales, PCM, prioridades y particiones',
            'limites': 'No compara respuesta analógica ni filtros de una tarjeta física; no añade acciones pendientes del juego.',
        }
        (destino / ('muestra.json' if self.muestra else 'resultado.json')).write_text(
            json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8'
        )
        print(
            f'Digital: {dma} programaciones DSP/DMA, {efectos} efectos, {total_muestras} muestras coincidentes.'
        )
        print(
            f'Bancos: 78 referencias comprobadas; {len(escenas)} combinaciones reproducidas. Prioridad y parada correctas.'
        )
        print(f'Informe y muestra local: {destino}')

    # Segmento esperado tras preparar una entidad visible; el audio digital no escribe en DS.
    def base_modificada(self, escena):
        datos = bytearray(self.base)
        datos[0x0AD4], datos[0x8082] = 3, escena
        for campo, valor in ((2, 100), (4, 100), (6, 32), (8, 48)):
            struct.pack_into('<H', datos, 0x2C4C + campo, valor)
        return bytes(datos)


if __name__ == '__main__':
    argumentos = argparse.ArgumentParser(
        description='Contrasta los efectos digitales Sound Blaster.'
    )
    argumentos.add_argument('--biblioteca', type=Path, required=True)
    argumentos.add_argument('--muestra', action='store_true')
    opciones = argumentos.parse_args()
    try:
        ComparacionDigital(
            Path(__file__).resolve().parents[3], opciones.biblioteca, opciones.muestra
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        argumentos.exit(1, f'Error: {error}\n')
