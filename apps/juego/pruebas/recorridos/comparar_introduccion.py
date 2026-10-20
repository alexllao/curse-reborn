"""Contrasta el recorrido de la introducción y sus dieciséis imágenes con el ejecutable."""

import argparse
import ctypes
import json
from pathlib import Path
import struct

from comparar_video import ComparacionVideo, ErrorAnalisis
from generar_titulo import guardar_png


class ComparacionIntroduccion(ComparacionVideo):
    # Prepara el puente de eventos y el modelo VGA usado como referencia de 2F50.
    def __init__(self, raiz, biblioteca, muestra):
        super().__init__(raiz, biblioteca, muestra)
        memoria = ctypes.POINTER(ctypes.c_ubyte)
        self.nativa.evento_introduccion.argtypes = [ctypes.c_uint, ctypes.POINTER(ctypes.c_ushort)]
        self.nativa.evento_introduccion.restype = ctypes.c_int
        self.nativa.pagina_introduccion.argtypes = [
            memoria,
            ctypes.c_uint,
            ctypes.c_ushort,
            memoria,
            memoria,
            memoria,
        ]
        self.nativa.pagina_introduccion.restype = ctypes.c_int
        self.carpeta = raiz / 'apps/juego/datos_originales/CURSE/DATA'
        self.imagenes = (
            [f'CAULDRN{i}.DAT' for i in range(1, 5)]
            + [f'BALCONY{i}.DAT' for i in range(1, 6)]
            + [f'BASEBAL{i}.DAT' for i in range(1, 7)]
            + ['BASEBAT.DAT']
        )
        self.paletas = ['CAULDRN.PAL', 'BALCONY.PAL', 'BASEBALL.PAL']
        self.codigo = bytes(self.maquina.mem_read(0, 0xF310))
        self.esperas = {
            i for i in range(0x28B2, 0x2F18) if self.codigo[i : i + 5] == b'\x50\x52\xba\xda\x03'
        }
        self.fase = 0
        self.modo = 'recorrido'
        self.maquina.hook_add(self.unicorn.UC_HOOK_CODE, self.instruccion)
        self.maquina.hook_add(self.unicorn.UC_HOOK_INTR, self.servicio)

    # Simula únicamente la alternancia de retrazo; la comparación cuenta esperas explícitas.
    def leer_puerto(self, maquina, puerto, tamano, contexto):
        if puerto == 0x3DA:
            self.fase ^= 8
            return self.fase
        return super().leer_puerto(maquina, puerto, tamano, contexto)

    # Registra las selecciones de página del CRTC; el resto del dibujo usa el modelo VGA previo.
    def escribir_puerto(self, maquina, puerto, tamano, valor, contexto):
        if puerto == 0x3D4:
            if self.modo == 'recorrido' and valor & 255 == 12:
                self.eventos.append((3, (valor >> 8) // 64, 0))
            return
        super().escribir_puerto(maquina, puerto, tamano, valor, contexto)

    # Obtiene un nombre DOS del segmento del llamador sin interpretar su contenido como código.
    def nombre(self, posicion):
        return bytes(self.maquina.mem_read(posicion, 32)).split(b'\0')[0].decode('ascii').upper()

    # Abre, lee y cierra solo recursos conocidos; las reservas DOS se sustituyen por memoria fija.
    def servicio(self, maquina, numero, contexto):
        r = self.r
        funcion = maquina.reg_read(r.UC_X86_REG_AX) >> 8
        if numero != 0x21:
            raise ErrorAnalisis('Interrupción inesperada en la introducción.')
        destino = maquina.reg_read(r.UC_X86_REG_DS) * 16 + maquina.reg_read(r.UC_X86_REG_DX)
        if funcion == 0x48:
            maquina.reg_write(r.UC_X86_REG_AX, 0x6000)
        elif funcion == 0x3D:
            self.abierto = self.nombre(destino)
            if self.abierto not in self.imagenes + self.paletas:
                raise ErrorAnalisis('Recurso inesperado en la introducción.')
            maquina.reg_write(r.UC_X86_REG_AX, 1)
        elif funcion == 0x3F:
            datos = (self.carpeta / self.abierto).read_bytes()[: maquina.reg_read(r.UC_X86_REG_CX)]
            maquina.mem_write(destino, datos)
            maquina.reg_write(r.UC_X86_REG_AX, len(datos))
        elif funcion not in (0x3E, 0x49):
            raise ErrorAnalisis('Servicio DOS inesperado en la introducción.')
        maquina.reg_write(r.UC_X86_REG_EFLAGS, maquina.reg_read(r.UC_X86_REG_EFLAGS) & ~1)

    # Retorna de un servicio sustituido conservando la pila de llamadas del programa original.
    def retornar(self):
        m, r = self.maquina, self.r
        pila = m.reg_read(r.UC_X86_REG_SP)
        direccion = m.reg_read(r.UC_X86_REG_SS) * 16 + pila
        retorno = struct.unpack('<H', m.mem_read(direccion, 2))[0]
        m.reg_write(r.UC_X86_REG_SP, pila + 2)
        m.reg_write(r.UC_X86_REG_IP, retorno)

    # Sigue el flujo original, abrevia las esperas y sustituye solo cargas/dibujo en la prueba lógica.
    def instruccion(self, maquina, direccion, tamano, contexto):
        if self.modo != 'recorrido':
            return
        r = self.r
        if direccion in self.esperas:
            self.eventos.append((5, 1, 0))
            maquina.reg_write(r.UC_X86_REG_IP, direccion + 22)
        elif direccion == 0x2F8E:
            self.eventos.append((0, 0, 0))
        elif direccion == 0x77E4:
            self.eventos.append((1, maquina.reg_read(r.UC_X86_REG_AX), 0))
            self.retornar()
        elif direccion == 0x2F50:
            nombre = self.nombre(self.segmento + maquina.reg_read(r.UC_X86_REG_DX))
            self.eventos.append(
                (2, self.imagenes.index(nombre), maquina.reg_read(r.UC_X86_REG_DI) // 0x4000)
            )
            self.retornar()
        elif direccion == 0x787E:
            self.eventos.append((4, self.paletas.index(self.abierto), 0))
            self.retornar()
        elif direccion == 0x2F29:
            self.eventos.append((6, 0, 0))
            maquina.reg_write(r.UC_X86_REG_IP, 0x2F3E)

    # Dispone segmentos y retorno para cada ejecución independiente de la referencia.
    def preparar(self):
        m, r = self.maquina, self.r
        m.mem_write(self.segmento, self.base)
        m.mem_write(self.segmento + 0x0AD5, b'\0')
        for registro, valor in (
            (r.UC_X86_REG_CS, 0),
            (r.UC_X86_REG_DS, 0x0F31),
            (r.UC_X86_REG_ES, 0x6000),
            (r.UC_X86_REG_SS, 0x8000),
            (r.UC_X86_REG_SP, 0xFF00),
            (r.UC_X86_REG_EFLAGS, 2),
        ):
            m.reg_write(registro, valor)
        m.mem_write(0x8FF00, struct.pack('<H', 0x7000))

    # Une únicamente esperas contiguas para contrastar los bloques compactados en C++.
    def compactar(self, eventos):
        salida = []
        for evento in eventos:
            if evento[0] == 5 and salida and salida[-1][0] == 5:
                salida[-1] = (5, salida[-1][1] + evento[1], 0)
            else:
                salida.append(evento)
        return salida

    # Comprueba el orden completo de cargas, páginas, controles, fundidos y retrazos.
    def recorrido(self):
        self.preparar()
        self.eventos = []
        self.maquina.emu_start(0x28B2, 0x7000, count=200000)
        if self.maquina.reg_read(self.r.UC_X86_REG_IP) != 0x7000:
            raise ErrorAnalisis('El recorrido original no terminó.')
        if self.maquina.reg_read(self.r.UC_X86_REG_SP) != 0xFF02:
            raise ErrorAnalisis('Pila desequilibrada en la introducción.')
        self.eventos.append((7, 0, 0))
        nativos = []
        for indice in range(1000):
            salida = (ctypes.c_ushort * 3)()
            if not self.nativa.evento_introduccion(indice, salida):
                break
            nativos.append(tuple(salida))
        original, nativo = self.compactar(self.eventos), self.compactar(nativos)
        if original != nativo:
            for i, (a, b) in enumerate(zip(original, nativo)):
                if a != b:
                    raise ErrorAnalisis(f'Evento {i} distinto: original={a}, C++={b}.')
            raise ErrorAnalisis(f'Cantidad de eventos distinta: {len(original)}/{len(nativo)}.')
        self.resumen['eventos_recorrido'] = len(nativo)

    # Ejecuta la carga original completa 2F50, incluido RNC2 y 7636, y contrasta planos e índices.
    def paginas(self):
        self.modo = 'paginas'
        for i, nombre in enumerate(self.imagenes):
            for pagina in (i % 4,) if self.muestra else range(4):
                self.preparar()
                m, r = self.maquina, self.r
                self.planos = bytearray(self.planos_base)
                self.control, self.mascara, self.plano_lectura = 0, 15, 0
                self.indice_secuenciador, self.indice_grafico = 0, 0
                m.mem_write(0x16, struct.pack('<H', 0x6000))
                m.mem_write(self.segmento + 0xA800, nombre.lower().encode() + b'\0')
                m.reg_write(r.UC_X86_REG_DX, 0xA800)
                m.reg_write(r.UC_X86_REG_DI, pagina * 0x4000)
                m.emu_start(0x2F50, 0x7000, count=4000000)
                if m.reg_read(r.UC_X86_REG_IP) != 0x7000 or m.reg_read(r.UC_X86_REG_SP) != 0xFF02:
                    raise ErrorAnalisis('La carga original no terminó correctamente.')
                datos = (self.carpeta / nombre).read_bytes()
                entrada = (ctypes.c_ubyte * len(datos)).from_buffer_copy(datos)
                planos = (ctypes.c_ubyte * 262144).from_buffer_copy(self.planos_base)
                temporal, imagen = (ctypes.c_ubyte * 64000)(), (ctypes.c_ubyte * 64000)()
                if not self.nativa.pagina_introduccion(
                    entrada, len(datos), pagina, planos, temporal, imagen
                ):
                    raise ErrorAnalisis(f'La carga C++ rechazó {nombre}.')
                if bytes(planos) != bytes(self.planos):
                    raise ErrorAnalisis(f'Planos distintos para {nombre}, página {pagina}.')
                referencia = bytes(
                    self.planos[(x % 4) * 65536 + pagina * 0x4000 + y * 80 + x // 4]
                    for y in range(200)
                    for x in range(320)
                )
                if bytes(imagen) != referencia:
                    raise ErrorAnalisis(f'Recuperación distinta para {nombre}.')
                self.resumen['paginas_reales'] = self.resumen.get('paginas_reales', 0) + 1
                paleta = (
                    self.carpeta / self.paletas[(0 if i < 4 else 1 if i < 9 else 2)]
                ).read_bytes()
                guardar_png(
                    self.raiz
                    / 'apps/juego/trabajo/comparacion_introduccion'
                    / f'{nombre[:-4].lower()}.png',
                    referencia,
                    paleta,
                )

    # Agrupa las evidencias nuevas; la señal VGA se simula y no certifica tiempos físicos de monitor.
    def ejecutar(self):
        carpeta = self.raiz / 'apps/juego/trabajo/comparacion_introduccion'
        carpeta.mkdir(parents=True, exist_ok=True)
        self.recorrido()
        self.paginas()
        informe = {
            'casos_por_grupo': self.resumen,
            'resultado': 'coinciden',
            'muestra': self.muestra,
            'sha256_ejecutable': self.cabecera['sha256'],
        }
        ruta = carpeta / ('muestra.json' if self.muestra else 'resultado.json')
        ruta.write_text(json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
        for grupo, cantidad in self.resumen.items():
            print(f'{grupo}: {cantidad} casos coincidentes.')
        print(f'Informe local: {ruta}')


if __name__ == '__main__':
    argumentos = argparse.ArgumentParser(description='Contrasta la introducción de la bruja.')
    argumentos.add_argument('--biblioteca', type=Path, required=True)
    argumentos.add_argument('--muestra', action='store_true')
    opciones = argumentos.parse_args()
    try:
        ComparacionIntroduccion(
            Path(__file__).resolve().parents[4], opciones.biblioteca, opciones.muestra
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        argumentos.exit(1, f'Error: {error}\n')
