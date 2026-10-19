"""Contrasta E997, E89D y E46D con la composición de los paneles y el teclado originales."""

import argparse
import ctypes
import json
import struct
from pathlib import Path
from comparar_informacion import ComparacionInformacion, ErrorAnalisis
from generar_titulo import guardar_png


class ComparacionPanelPartidas(ComparacionInformacion):
    # Prepara el archivo DOS simulado y conserva el dibujante VGA de las fichas de información.
    def __init__(self, raiz, biblioteca):
        super().__init__(raiz, biblioteca)
        memoria = ctypes.POINTER(ctypes.c_ubyte)
        self.nativa.componer_panel_partidas.argtypes = [
            memoria,
            memoria,
            ctypes.c_uint,
            memoria,
            ctypes.c_uint,
            ctypes.c_uint,
            ctypes.c_uint,
            memoria,
            ctypes.c_uint,
            memoria,
        ]
        self.nativa.componer_panel_partidas.restype = ctypes.c_int
        self.archivo_dos = bytes()
        self.cursor_dos = 1
        self.maquina.hook_add(self.unicorn.UC_HOOK_INTR, self.servicio_archivo)

    # Sustituye solo lecturas y búsquedas INT21; E997 decide qué descripciones leer o sustituir.
    def servicio_archivo(self, maquina, numero, contexto):
        r = self.r
        ax = maquina.reg_read(r.UC_X86_REG_AX)
        cx = maquina.reg_read(r.UC_X86_REG_CX)
        dx = maquina.reg_read(r.UC_X86_REG_DX)
        if numero != 0x21:
            raise ErrorAnalisis('Interrupción inesperada al componer partidas.')
        if ax >> 8 == 0x3F:
            leido = self.archivo_dos[self.cursor_dos : self.cursor_dos + cx]
            maquina.mem_write(maquina.reg_read(r.UC_X86_REG_DS) * 16 + dx, leido)
            self.cursor_dos += len(leido)
            maquina.reg_write(r.UC_X86_REG_AX, len(leido))
        elif ax >> 8 == 0x42:
            offset = (cx << 16) | dx
            if offset & 0x80000000:
                offset -= 1 << 32
            self.cursor_dos = offset if ax & 255 == 0 else self.cursor_dos + offset
        else:
            raise ErrorAnalisis('El panel solicitó una operación DOS no prevista.')
        maquina.reg_write(r.UC_X86_REG_EFLAGS, maquina.reg_read(r.UC_X86_REG_EFLAGS) & ~1)

    # Compara cada píxel con un fragmento original sin ejecutar su espera de interacción.
    def caso(self, modo, mascara, nombre=None, compatible=False):
        m, r = self.maquina, self.r
        datos = bytearray(self.base)
        archivo = bytearray(1 + 8 * 10164)
        archivo[0] = mascara
        for i in range(8):
            texto = f'PARTIDA {i + 1}'.encode().ljust(13, b' ')
            archivo[1 + i * 10164 : 14 + i * 10164] = texto
        if compatible:
            archivo[1:14] = b'ANTIGUA 286'.ljust(13, b' ')
        archivo_port = bytearray(archivo)
        if compatible:
            archivo_port[1:14] = b'Antigua 286'.ljust(13, b' ')
        self.archivo_dos = bytes(archivo)
        self.cursor_dos = 1
        self.reiniciar_video()
        inicial = self.imagen_original(112)
        m.mem_write(self.segmento, bytes(datos))
        m.mem_write(8, struct.pack('<H', 0xA586))
        m.mem_write(0xD57C, struct.pack('<H', 0x5000))
        m.mem_write(0xD582, struct.pack('<H', 0xA000))
        m.mem_write(0x50000, self.fuente)
        m.mem_write(0x6BFC, struct.pack('<200H', *(i * 112 for i in range(200))))
        for registro, valor in (
            (r.UC_X86_REG_CS, 0),
            (r.UC_X86_REG_DS, 0xF31),
            (r.UC_X86_REG_ES, 0xF31),
            (r.UC_X86_REG_SS, 0x8000),
            (r.UC_X86_REG_SP, 0xFF00),
            (r.UC_X86_REG_BP, 0xFF80),
            (r.UC_X86_REG_EFLAGS, 2),
        ):
            m.reg_write(registro, valor)
        m.mem_write(self.segmento + 0xB602, bytes([mascara]))
        m.mem_write(0x8FF7C, struct.pack('<2H', 1, 1))
        if nombre is None:
            inicio, fin = ((0xE201, 0xE222), (0xE382, 0xE3A3), (0xE7E5, 0xE806))[modo - 1]
            m.emu_start(inicio, fin, count=100000)
            if m.reg_read(r.UC_X86_REG_IP) != fin:
                raise ErrorAnalisis('E997 no terminó la lectura de nombres.')
            m.emu_start(0xE89D, 0xE8C2, count=1000000)
        else:
            m.mem_write(self.segmento + 0xBADA, nombre + b'-' * (13 - len(nombre)))
            m.emu_start(0xE46D, 0xE499, count=1000000)
        esperado = self.imagen_original(112)
        destino = (ctypes.c_ubyte * 64016)(*inicial, *([0xCD] * 16))
        entrada = (ctypes.c_ubyte * 65536).from_buffer_copy(datos)
        if not self.nativa.componer_panel_partidas(
            entrada,
            self.banco_fuente,
            len(self.fuente),
            (ctypes.c_ubyte * len(archivo_port)).from_buffer_copy(archivo_port),
            len(archivo),
            modo,
            nombre is not None,
            (ctypes.c_ubyte * len(nombre or b'')).from_buffer_copy(
                nombre.lower() if compatible and nombre is not None else (nombre or b'')
            ),
            len(nombre or b''),
            destino,
        ):
            raise ErrorAnalisis('El panel portátil rechazó el caso original.')
        if bytes(destino[:64000]) != esperado:
            distintos = [i for i in range(64000) if destino[i] != esperado[i]]
            raise ErrorAnalisis(
                f'Panel{modo} máscara{mascara}: {len(distintos)} píxeles distintos, primero{distintos[0]}.'
            )
        if bytes(entrada) != datos or bytes(destino[64000:]) != bytes([0xCD] * 16):
            raise ErrorAnalisis('El panel modificó DS o excedió el destino.')
        return bytes(destino[:64000])

    # Incluye ocho filas, todos los bits de ocupación y el teclado vacío, parcial y completo.
    def comprobar(self):
        destino = self.raiz / 'apps/juego/trabajo/comparacion_panel_partidas'
        destino.mkdir(parents=True, exist_ok=True)
        casos = 0
        paleta = (self.carpeta / 'DUNGEON.MAP').read_bytes()[:576] + bytes([63] * 192)
        for modo in (1, 2, 3):
            for mascara in (0, 1, 0x55, 0x7F, 0x80, 0xFF):
                imagen = self.caso(modo, mascara)
                casos += 1
                if mascara == 0x55:
                    guardar_png(destino / f'panel_{modo}.png', imagen, paleta)
        for i, nombre in enumerate((b'', b'A', b'BRAD 286', b'TRECE LETRAS!', b'&+? 012345678')):
            imagen = self.caso(2, 0xFF, nombre)
            casos += 1
            guardar_png(destino / f'teclado_{i}.png', imagen, paleta)
        for modo in (1, 2, 3):
            self.caso(modo, 0xFF, compatible=True)
            casos += 1
        self.caso(2, 0xFF, b'ANTIGUA 286', compatible=True)
        casos += 1
        (destino / 'resultado.json').write_text(
            json.dumps(
                {
                    'resultado': 'coinciden',
                    'casos': casos,
                    'composiciones_originales': 23,
                    'compatibilidad_nombres_anteriores': 4,
                    'pixeles_por_caso': 64000,
                    'ranuras': 8,
                    'editor_original': True,
                },
                indent=2,
            )
            + '\n',
            encoding='utf-8',
        )
        print(
            'Paneles originales: 23 composiciones coincidentes de 64.000 píxeles y cuatro casos de nombres anteriores; ocho ranuras y teclado.'
        )


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Contrasta los paneles originales de partidas.')
    parser.add_argument('--biblioteca', type=Path, required=True)
    parser.add_argument('--muestra', action='store_true')
    opciones = parser.parse_args()
    try:
        ComparacionPanelPartidas(
            Path(__file__).resolve().parents[3], opciones.biblioteca
        ).comprobar()
    except (OSError, ErrorAnalisis) as error:
        parser.exit(1, f'Error: {error}\n')
