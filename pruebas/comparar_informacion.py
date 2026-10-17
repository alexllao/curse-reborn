"""Contrasta las cuatro páginas de información con E160, D623, D87A y FONT.SPR."""

import argparse
import ctypes
import json
from pathlib import Path
import struct

from comparar_composicion import ComparacionComposicion, ErrorAnalisis
from comparar_objetos import guardar, palabra


class ComparacionInformacion(ComparacionComposicion):
    # Prepara el banco tipográfico y el puente sin sustituir las rutinas originales de dibujo.
    def __init__(self, raiz, biblioteca):
        super().__init__(raiz, biblioteca, True)
        memoria = ctypes.POINTER(ctypes.c_ubyte)
        self.nativa.componer_informacion.argtypes = [
            memoria,
            memoria,
            ctypes.c_uint,
            ctypes.c_uint,
            memoria,
            ctypes.POINTER(ctypes.c_uint),
        ]
        self.nativa.componer_informacion.restype = ctypes.c_int
        self.fuente = (self.carpeta / 'FONT.SPR').read_bytes()
        self.banco_fuente = (ctypes.c_ubyte * len(self.fuente)).from_buffer_copy(self.fuente)
        self.recoger_paginas = False
        self.relleno_vga = 0
        self.color_relleno_vga = 0
        self.paginas = []
        self.maquina.hook_add(self.unicorn.UC_HOOK_CODE, self.capturar_pagina)

    # 7714 activa los registros set/reset de VGA para rellenar la placa de negro.
    def aplicar_puerto(self, puerto, valor):
        if puerto == 0x3CF and self.indice_grafico in (0, 1):
            if self.indice_grafico == 0:
                self.color_relleno_vga = valor
            else:
                self.relleno_vga = valor
        else:
            super().aplicar_puerto(puerto, valor)

    # Conserva la máscara de planos y aplica el color constante solo donde 7714 lo habilita.
    def escribir_video(self, maquina, acceso, direccion, tamano, valor, contexto):
        if not self.relleno_vga:
            return super().escribir_video(maquina, acceso, direccion, tamano, valor, contexto)
        for desplazamiento in range(tamano):
            posicion = direccion - 0xA0000 + desplazamiento
            for plano in range(4):
                if self.mascara & (1 << plano):
                    self.planos[plano * 65536 + posicion] = (
                        (255 if self.color_relleno_vga & (1 << plano) else 0)
                        if self.relleno_vga & (1 << plano)
                        else (valor >> (8 * desplazamiento)) & 255
                    )

    # E160 conserva el flujo de cuatro fichas; se omite únicamente su espera de teclado y ratón.
    def capturar_pagina(self, maquina, direccion, tamano, contexto):
        if not self.recoger_paginas or direccion != 0xD623:
            return
        self.paginas.append(maquina.reg_read(self.r.UC_X86_REG_SI))
        pila = maquina.reg_read(self.r.UC_X86_REG_SP)
        retorno = struct.unpack('<H', maquina.mem_read(0x80000 + pila, 2))[0]
        maquina.reg_write(self.r.UC_X86_REG_SP, pila + 2)
        maquina.reg_write(self.r.UC_X86_REG_IP, retorno)
        maquina.reg_write(self.r.UC_X86_REG_EFLAGS, maquina.reg_read(self.r.UC_X86_REG_EFLAGS) & ~1)

    # Contrasta el formato decimal original y cada píxel; exige DS intacto y centinelas del destino.
    def comprobar(self):
        m, r = self.maquina, self.r
        casos = 0
        for puntos in (0, 1, 4, 9, 10, 99, 100, 239, 477, 478):
            datos = bytearray(self.base)
            guardar(datos, 0x35C7, puntos)
            m.mem_write(self.segmento, bytes(datos))
            self.paginas = []
            self.recoger_paginas = True
            self.ejecutar_original(0xE160)
            self.recoger_paginas = False
            formateados = bytes(m.mem_read(self.segmento, 65536))
            if len(self.paginas) != 4:
                raise ErrorAnalisis('E160 no recorrió las cuatro páginas de información.')
            for pagina, inicio in enumerate(self.paginas):
                self.reiniciar_video()
                inicial = self.imagen_original(112)
                m.mem_write(self.segmento, formateados)
                m.mem_write(8, struct.pack('<H', 0xA586))
                m.mem_write(0xD57C, struct.pack('<H', 0x5000))
                m.mem_write(0xD586, struct.pack('<H', 0x6000))
                m.mem_write(0x50000, self.fuente)
                m.mem_write(0x6BFC, struct.pack('<200H', *(i * 112 for i in range(200))))
                for registro, valor in (
                    (r.UC_X86_REG_CS, 0),
                    (r.UC_X86_REG_DS, 0xF31),
                    (r.UC_X86_REG_ES, 0xA000),
                    (r.UC_X86_REG_SS, 0x8000),
                    (r.UC_X86_REG_SP, 0xFF00),
                    (r.UC_X86_REG_SI, inicio),
                    (r.UC_X86_REG_EFLAGS, 2),
                ):
                    m.reg_write(registro, valor)
                m.emu_start(0xD623, 0xD6AA, count=3_000_000)
                if m.reg_read(r.UC_X86_REG_IP) != 0xD6AA:
                    raise ErrorAnalisis('D623 no terminó el dibujo previo a la espera.')
                destino = (ctypes.c_ubyte * 64016)(*inicial, *([0xCD] * 16))
                entrada = (ctypes.c_ubyte * 65536).from_buffer_copy(datos)
                rectangulo = (ctypes.c_uint * 4)()
                if not self.nativa.componer_informacion(
                    entrada,
                    self.banco_fuente,
                    len(self.fuente),
                    pagina,
                    destino,
                    rectangulo,
                ):
                    raise ErrorAnalisis('La ficha C++ rechazó los datos originales.')
                esperado = self.imagen_original(112)
                if bytes(destino[:64000]) != esperado:
                    distintos = [i for i in range(64000) if destino[i] != esperado[i]]
                    raise ErrorAnalisis(
                        f'Ficha {pagina + 1}, puntos {puntos}: {len(distintos)} píxeles distintos; primero {distintos[0]}.'
                    )
                ancho, alto = palabra(datos, inicio), palabra(datos, inicio + 2)
                if list(rectangulo) != [(320 - ancho) // 2, (200 - alto) // 2, ancho, alto]:
                    raise ErrorAnalisis('El encuadre no coincide con D623.')
                if bytes(entrada) != datos or bytes(destino[64000:]) != bytes([0xCD] * 16):
                    raise ErrorAnalisis('Información modifica DS o excede su destino.')
                casos += 1
        salida = self.raiz / 'trabajo' / 'comparacion_informacion'
        salida.mkdir(parents=True, exist_ok=True)
        (salida / 'resultado.json').write_text(
            json.dumps(
                {
                    'casos': casos,
                    'paginas': 4,
                    'pixeles_por_caso': 64000,
                    'resultado': 'coinciden',
                    'estado_conservado': True,
                },
                ensure_ascii=False,
                indent=2,
            )
            + '\n',
            encoding='utf-8',
        )
        print(f'Información: {casos} casos coincidentes; cuatro páginas y 64.000 píxeles por caso.')


if __name__ == '__main__':
    argumentos = argparse.ArgumentParser(
        description='Compara las fichas originales de información.'
    )
    argumentos.add_argument('--biblioteca', type=Path, required=True)
    argumentos.add_argument('--muestra', action='store_true')
    opciones = argumentos.parse_args()
    ComparacionInformacion(Path(__file__).resolve().parents[1], opciones.biblioteca).comprobar()
