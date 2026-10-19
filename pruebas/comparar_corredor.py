"""Prolonga el puzle real hasta el corredor, desplaza su cámara y regresa con estado persistente."""

import argparse
import ctypes
import struct
from pathlib import Path

from comparar_puzle import ComparacionPuzle, ErrorAnalisis
from comparar_objetos import ComparacionObjetos, guardar, palabra

BANCOS_ESCENA = {
    43: 'SNOWAST4',
    44: 'ICELAKE',
    45: 'ICEPAL',
    46: 'ICEROOM',
    42: 'SNOWAST3',
    41: 'SNOWAST2',
    40: 'SNOWAST1',
    36: 'HOLD',
    37: 'GHETTO',
    38: 'CLOUD',
    39: 'ELECTRO',
    0: 'DUNGEON',
    1: 'CORRIDOR',
    2: 'DROP',
    3: 'LAKEBED',
    4: 'CAVECOR',
    5: 'CAVECOR',
    6: 'CAVECOR',
    7: 'CAVECOR',
    8: 'POOL',
    9: 'WELL',
    10: 'CAVERBMI',
    11: 'SEESAW',
    12: 'SHAFT',
    13: 'CAVE',
    14: 'CAVE',
    15: 'COUNTRY1',
    16: 'COUNTRY2',
    17: 'XROADS',
    18: 'SORCERY',
    19: 'ROGUE',
    20: 'GYPSY',
    21: 'DRAGON',
    22: 'MAGE',
    23: 'DERELICT',
    24: 'BENN',
    25: 'CHANGE',
    26: 'SALLY',
    27: 'DERELICT',
    28: 'INNSIDE',
    29: 'FRANKS',
    30: 'EDGE',
    31: 'WISE',
    35: 'POST',
    32: 'DINO',
    33: 'CELESTE',
    34: 'REMOTE',
}


class ComparacionCorredor(ComparacionPuzle):
    # Reutiliza la partida completa y el modelo de memoria sin crear una partida sintética liberada.
    def __init__(self, raiz, biblioteca, muestra):
        super().__init__(raiz, biblioteca, muestra)
        self.capturar_crtc = False
        self.retrazo = 0
        self.indice_crtc = 0
        self.origen_crtc = 0
        self.fotogramas_crtc = []
        self.nativa.componer_camara.argtypes = [
            ctypes.POINTER(ctypes.c_ubyte),
            ctypes.POINTER(ctypes.c_ubyte),
            ctypes.c_int,
            ctypes.c_uint,
            ctypes.POINTER(ctypes.c_ubyte),
        ]
        self.nativa.componer_camara.restype = ctypes.c_int
        self.carpeta_resultados = 'comparacion_corredor'
        self.nativa.preparar_transicion.argtypes = [ctypes.POINTER(ctypes.c_ubyte)] + [
            ctypes.c_ushort
        ] * 5
        self.nativa.preparar_transicion.restype = ctypes.c_int
        self.nativa.procesar_camara.argtypes = [
            ctypes.c_uint,
            ctypes.POINTER(ctypes.c_ubyte),
            ctypes.c_int,
        ]
        self.nativa.procesar_camara.restype = ctypes.c_int
        preparacion = ComparacionObjetos(raiz, biblioteca, True)
        self.nativa.procesar_objetos.argtypes = preparacion.nativa.procesar_objetos.argtypes

    # Ejecuta 4C43 con la lectura DOS del modelo original cuando el descriptor solicita PLUNGE.
    def entrada(self, datos, salida):
        if salida in (0, 2):
            return super().entrada(datos, salida)
        if salida == 3:
            self.modelo = (self.carpeta / 'PLUNGE.BRD').read_bytes()
        elif salida == 4:
            self.modelo = (self.carpeta / 'EMERGE.BRD').read_bytes()
        elif salida == 47:
            self.modelo = (self.carpeta / 'SMALL.BRD').read_bytes()
        elif datos[0x85AC + salida * 14 + 13] != 0xFF:
            p = palabra(datos, 0x0918 + datos[0x85AC + salida * 14 + 13] * 2)
            nombre = bytes(datos[p : p + 13]).split(b'\0')[0].decode('ascii').upper()
            self.modelo = (self.carpeta / nombre).read_bytes()
        self.preparar_bancos_originales()
        m, r = self.maquina, self.r
        m.mem_write(self.segmento, bytes(datos))
        m.reg_write(r.UC_X86_REG_BX, salida)
        self.ejecutar_hasta(0x4B05, 0x4B14)
        m.reg_write(r.UC_X86_REG_SI, 0x85AC + salida * 14)
        m.reg_write(r.UC_X86_REG_DI, 0x2C2B)
        self.ejecutar_hasta(0x4C43, 0x4D2B)
        m.reg_write(r.UC_X86_REG_BX, salida)
        self.ejecutar_hasta(0x4D2B, 0x4D40)
        self.ejecutar_hasta(0x5145, 0x5182)
        esperado = bytes(m.mem_read(self.segmento, 65536))
        ancho, alto = 32, 48
        if datos[0x85AC + salida * 14 + 13] != 0xFF:
            imagen_entrada = datos[0x85AC + salida * 14 + 1] & 0x7F
            p = struct.unpack_from('<H', self.modelo, imagen_entrada * 2)[0] * 16
            ancho, alto = struct.unpack_from('<2H', self.modelo, p)
        if (
            not self.nativa.preparar_transicion(
                (ctypes.c_ubyte * 65536).from_buffer(datos),
                salida,
                0x6400,
                self.referencia_modelo,
                ancho,
                alto,
            )
            or bytes(datos) != esperado
        ):
            diferencias = [
                f'{i:04X}:{datos[i]:02X}/{esperado[i]:02X}'
                for i in range(65536)
                if datos[i] != esperado[i]
            ][:8]
            raise ErrorAnalisis(f'Entrada de caída {salida} distinta: {diferencias}.')
        self.resumen['entradas'] = self.resumen.get('entradas', 0) + 1

    # Alterna el retrazo VGA únicamente para permitir las esperas originales de la animación.
    def leer_puerto(self, maquina, puerto, tamano, contexto):
        if puerto == 0x3DA and self.capturar_crtc:
            self.retrazo ^= 8
            return self.retrazo
        return super().leer_puerto(maquina, puerto, tamano, contexto)

    # Recupera la ventana que muestra cada escritura del origen CRTC, desde los cuatro planos.
    def aplicar_puerto(self, puerto, valor):
        if self.capturar_crtc and puerto in (0x3D4, 0x3D5):
            if puerto == 0x3D4:
                self.indice_crtc = valor
            elif self.indice_crtc == 12:
                self.origen_crtc = (self.origen_crtc & 255) | (valor << 8)
            elif self.indice_crtc == 13:
                self.origen_crtc = (self.origen_crtc & 0xFF00) | valor
                self.fotogramas_crtc.append(
                    bytes(
                        self.planos[
                            (x & 3) * 65536 + ((self.origen_crtc + y * 112 + x // 4) & 65535)
                        ]
                        for y in range(200)
                        for x in range(320)
                    )
                )
            return
        super().aplicar_puerto(puerto, valor)

    # Ejecuta la rutina VGA completa de cámara y contrasta sus nueve posiciones visibles con C++.
    def camara_visual(self, datos, direccion, carpeta):
        anterior = bytearray(datos)
        p = palabra(anterior, 0x0AED) + anterior[0x3597]
        self.seleccion_fondo = anterior[p : p + 10]
        imagen_anterior, paleta = self.componer_resultado(anterior, carpeta)
        self.logica = False
        fondo_anterior = bytes(self.fondo(self.nombre_mapa, self.seleccion_fondo).indices)
        self.logica = True
        destino = bytearray(anterior)
        self.camara(destino, direccion)
        p = palabra(destino, 0x0AED) + destino[0x3597]
        self.seleccion_fondo = destino[p : p + 10]
        imagen_destino, _ = self.componer_resultado(destino, carpeta)
        mapa = (self.carpeta / self.nombre_mapa).read_bytes()[578:]
        if len(mapa) > 0x20000:
            raise ErrorAnalisis('Mapa demasiado grande para la reserva del comparador de cámara.')
        self.preparar_bancos_originales()
        m = self.maquina
        m.mem_write(self.segmento, bytes(anterior))
        for a, valor in ((0x0008, 0xA586), (0x000A, 0xA000), (0x0012, 0x3000), (0x0016, 0xB000)):
            m.mem_write(a, struct.pack('<H', valor))
        m.mem_write(0xB0000, mapa)
        m.mem_write(0x0BF7, b'\0')
        self.reiniciar_video()
        for y in range(200):
            for x in range(320):
                self.planos[(x & 3) * 65536 + y * 80 + x // 4] = fondo_anterior[y * 320 + x]
                self.planos[(x & 3) * 65536 + 0x5860 + y * 112 + x // 4] = imagen_anterior[
                    y * 320 + x
                ]
        self.capturar_crtc = True
        self.fotogramas_crtc = []
        self.logica = False
        try:
            self.ejecutar_original(0xEC8A if direccion > 0 else 0xEF7F)
        finally:
            self.capturar_crtc = False
            self.logica = True
        if len(self.fotogramas_crtc) != 9:
            raise ErrorAnalisis(
                f'Cantidad de posiciones CRTC inesperada: {len(self.fotogramas_crtc)}.'
            )
        a = (ctypes.c_ubyte * 64000).from_buffer_copy(imagen_anterior)
        b = (ctypes.c_ubyte * 64000).from_buffer_copy(imagen_destino)
        salida = (ctypes.c_ubyte * 64000)()
        for paso, esperado in enumerate(self.fotogramas_crtc):
            if (
                not self.nativa.componer_camara(a, b, direccion, paso, salida)
                or bytes(salida) != esperado
            ):
                diferencias = sum(x != y for x, y in zip(bytes(salida), esperado))
                raise ErrorAnalisis(
                    f'Cámara visual {direccion}, paso {paso}: {diferencias} píxeles distintos.'
                )
            self.resumen['fotogramas_camara'] = self.resumen.get('fotogramas_camara', 0) + 1

    # Contrasta DS y azar completos y detiene el original antes de entrar en el cargador DOS.
    def paso(self, datos, semilla, direccion=0xFFFF):
        self.disponer(datos, self.geometria, semilla)
        guardar(datos, 0x0AE6, palabra(datos, 0x2C2D))
        guardar(datos, 0x0AE8, palabra(datos, 0x2C2F))
        m, r = self.maquina, self.r
        m.mem_write(self.segmento, bytes(datos))
        if direccion != 0xFFFF and datos[0x3594]:
            m.mem_write(self.segmento + 0x0DFA, b'\0\0')
            m.reg_write(r.UC_X86_REG_SI, direccion)
            self.ejecutar_original(0x0BF8)
        salida = []
        marcadores_f6 = []
        self.retorno_salida = 2

        # 0AE5 se alcanza tanto desde BOXDET como desde una petición F6 del guion.
        def detener(maquina, posicion, tamano, contexto):
            if posicion == 0x85DB:
                marcadores_f6.append(
                    (maquina.reg_read(r.UC_X86_REG_SI), maquina.reg_read(r.UC_X86_REG_AX) & 255)
                )
            if posicion == 0x0A6C:
                self.retorno_salida = 16
            if posicion == 0x0AE5:
                salida.append(maquina.reg_read(r.UC_X86_REG_BX) & 255)
                maquina.emu_stop()

        gancho = m.hook_add(self.unicorn.UC_HOOK_CODE, detener)
        try:
            try:
                self.ejecutar_hasta(0x096D, 0x0BC4)
            except ErrorAnalisis:
                if not salida:
                    raise
        finally:
            m.hook_del(gancho)
        esperado = bytes(m.mem_read(self.segmento, 65536))
        # F6 no dibuja su imagen transitoria. Si su cabecera excede el archivo, DOS lee RAM
        # ajena (incluso con envoltura de ES); la API acotada documenta ancho/alto cero.
        # Solo se normalizan esos cuatro bytes al solicitar efectivamente el cambio de sala.
        if salida and self.retorno_salida == 16:
            esperado_acotado = bytearray(esperado)
            for entidad, imagen in marcadores_f6:
                ancho, alto = ctypes.c_ushort(), ctypes.c_ushort()
                if not self.dimensiones_recurso(
                    None,
                    palabra(esperado, entidad + 31),
                    imagen & 127,
                    ctypes.pointer(ancho),
                    ctypes.pointer(alto),
                ):
                    esperado_acotado[entidad + 6 : entidad + 10] = bytes(4)
                    self.resumen['cabeceras_F6_acotadas'] = (
                        self.resumen.get('cabeceras_F6_acotadas', 0) + 1
                    )
            esperado = bytes(esperado_acotado)
        azar = bytes(m.mem_read(0x78DE, 8))
        resultado = self.nativa.avanzar_jugabilidad(
            (ctypes.c_ubyte * 65536).from_buffer(datos),
            semilla,
            self.callback_dimensiones,
            self.callback_modelo,
            self.buffer_geometria,
            len(self.geometria),
            direccion,
        )
        if (
            resultado != (self.retorno_salida if salida else 0)
            or bytes(datos) != esperado
            or bytes(semilla) != azar
        ):
            diferencias = [
                f'{i:04X}:{datos[i]:02X}/{esperado[i]:02X}'
                for i in range(65536)
                if datos[i] != esperado[i]
            ][:8]
            raise ErrorAnalisis(
                f'Paso de corredor distinto: {diferencias}, retorno={resultado}, salida={salida}.'
            )
        self.resumen['pasos_jugables'] = self.resumen.get('pasos_jugables', 0) + 1
        return salida[0] if salida else None

    # Contrasta los umbrales EAFE y el acarreo de las dos rutinas sin ejecutar sus copias gráficas.
    def decisiones_camara(self, base):
        m, r = self.maquina, self.r
        for ventana in (0, 1, 27, 28):
            for centro in (48, 64, 65, 222, 223, 224):
                d = bytearray(base)
                d[0x3597] = ventana
                guardar(d, 0x2C2D, centro - (palabra(d, 0x2C31) >> 1))
                m.mem_write(self.segmento, bytes(d))
                elegida = []

                # Omite la llamada gráfica después de haber ejecutado sus comparaciones de posición.
                def detener(maquina, posicion, tamano, contexto):
                    if posicion in (0xEB0D, 0xEB1A):
                        elegida.append(-1 if posicion == 0xEB0D else 1)
                        maquina.reg_write(r.UC_X86_REG_IP, 0xEB1F)

                gancho = m.hook_add(self.unicorn.UC_HOOK_CODE, detener)
                try:
                    self.ejecutar_hasta(0xEAFE, 0xEB1F)
                finally:
                    m.hook_del(gancho)
                esperado = elegida[0] if elegida else 0
                if esperado:
                    # Mantiene la comprobación de capacidad y el retorno con acarreo originales.
                    def omitir(maquina, posicion, tamano, contexto):
                        if posicion == 0xEC99:
                            maquina.reg_write(r.UC_X86_REG_IP, 0xEF79)
                        elif posicion == 0xEF8A:
                            maquina.reg_write(r.UC_X86_REG_IP, 0xF294)

                    gancho = m.hook_add(self.unicorn.UC_HOOK_CODE, omitir)
                    try:
                        self.ejecutar_original(0xEC8A if esperado > 0 else 0xEF7F)
                    finally:
                        m.hook_del(gancho)
                    if m.reg_read(r.UC_X86_REG_EFLAGS) & 1:
                        esperado = 0
                actual = self.nativa.procesar_camara(0, (ctypes.c_ubyte * 65536).from_buffer(d), 0)
                if actual != esperado or bytes(m.mem_read(self.segmento, 65536)) != bytes(d):
                    raise ErrorAnalisis(
                        f'Decisión de cámara distinta: centro {centro}, ventana {ventana}.'
                    )
                self.resumen['umbrales_camara'] = self.resumen.get('umbrales_camara', 0) + 1

    # Compara los fragmentos de estado de cámara y el ajuste de cursor de 6B46.
    def camara(self, datos, direccion):
        m = self.maquina
        m.mem_write(self.segmento, bytes(datos))
        self.ejecutar_hasta(
            0xEC99 if direccion > 0 else 0xEF8A, 0xECA3 if direccion > 0 else 0xEF94
        )
        self.ejecutar_hasta(
            0xED41 if direccion > 0 else 0xF05C, 0xED95 if direccion > 0 else 0xF0B0
        )
        esperado = bytes(m.mem_read(self.segmento, 65536))
        if (
            not self.nativa.procesar_camara(
                1, (ctypes.c_ubyte * 65536).from_buffer(datos), direccion
            )
            or bytes(datos) != esperado
        ):
            diferencias = [f'{i:04X}' for i in range(65536) if datos[i] != esperado[i]][:8]
            raise ErrorAnalisis(f'Cámara {direccion} distinta: {diferencias}.')
        self.resumen['cambios_camara'] = self.resumen.get('cambios_camara', 0) + 1

    # Carga la misma lista de la escena y mantiene la semilla que dejó la habitación anterior.
    def cambiar(self, datos, semilla, salida):
        self.entrada(datos, salida)
        self.maquina.mem_write(self.segmento, bytes(datos))
        self.ejecutar_hasta(0x4E98, 0x4EC8)
        esperado = bytes(self.maquina.mem_read(self.segmento, 65536))
        if (
            not self.nativa.procesar_camara(2, (ctypes.c_ubyte * 65536).from_buffer(datos), 0)
            or bytes(datos) != esperado
        ):
            diferencias = [
                f'{i:04X}:{datos[i]:02X}/{esperado[i]:02X}'
                for i in range(65536)
                if datos[i] != esperado[i]
            ][:8]
            raise ErrorAnalisis(f'Tabla de tiras distinta al entrar en la escena: {diferencias}.')
        escena = palabra(datos, 0x8082)
        self.escena = (
            self.carpeta / (('SNOWAST3' if escena == 40 else BANCOS_ESCENA[escena]) + '.SPR')
        ).read_bytes()
        self.nombre_mapa = BANCOS_ESCENA[escena] + '.MAP'
        mapa = (self.carpeta / self.nombre_mapa).read_bytes()
        datos[0x0AEF : 0x0AEF + 576] = mapa[:576]
        archivo = (self.carpeta / 'BOXDET.DAT').read_bytes()
        inicio = struct.unpack_from('<H', archivo, escena * 2)[0]
        self.cantidad = struct.unpack_from('<H', archivo, inicio)[0]
        self.geometria = archivo[inicio + 2 : inicio + 2 + self.cantidad * 10]
        self.buffer_geometria = (ctypes.c_ubyte * len(self.geometria)).from_buffer_copy(
            self.geometria
        )
        guardar(datos, 0x0DF0, self.cantidad)
        guardar(datos, 0x0DF2, 0)
        self.disponer(datos, self.geometria, semilla)
        self.maquina.mem_write(0x4AEE, struct.pack('<H', 0x4000))
        self.ejecutar_original(0x8AA6)
        esperado = bytes(self.maquina.mem_read(self.segmento, 65536))
        azar = bytes(self.maquina.mem_read(0x78DE, 8))
        if (
            not self.nativa.procesar_objetos(
                0,
                (ctypes.c_ubyte * 65536).from_buffer(datos),
                semilla,
                (ctypes.c_ubyte * len(self.escena)).from_buffer_copy(self.escena),
                len(self.escena),
                255,
                68,
                ctypes.byref(ctypes.c_ushort()),
            )
            or bytes(datos) != esperado
            or bytes(semilla) != azar
        ):
            raise ErrorAnalisis('Carga de entidades persistentes distinta.')
        guardar(datos, 0x0358, 0)
        guardar(datos, 0x0616, 0)
        self.resumen['cargas_persistentes'] = self.resumen.get('cargas_persistentes', 0) + 1

    # Usa la tabla real de la escena para contrastar el fondo visible en cada ventana seleccionada.
    def captura(self, datos, carpeta, nombre):
        p = palabra(datos, 0x0AED) + datos[0x3597]
        self.seleccion_fondo = datos[p : p + 10]
        self.componer_resultado(datos, carpeta)
        (carpeta / f'{nombre}_cpp.png').write_bytes(
            (carpeta / 'interaccion_32_cpp.png').read_bytes()
        )

    # Recorre flechas reales y consume por separado cada cambio de cámara, como EAD5 antes del paso.
    def andar(self, datos, semilla, direccion, pasos):
        for _ in range(pasos):
            camara = self.nativa.procesar_camara(0, (ctypes.c_ubyte * 65536).from_buffer(datos), 0)
            if camara:
                self.camara(datos, camara)
            salida = self.paso(datos, semilla, direccion)
            if salida is not None:
                return salida
        return None

    # Prepara el clic con el planificador original sin consumir todavía los pasos que pueden cambiar de escena.
    def clic(self, datos, x, y):
        guardar(datos, 0x2C0C, x)
        guardar(datos, 0x2C0E, y)
        self.bloque('clics_transicion', 1, 0x89DA, datos, self.geometria)
        self.maquina.mem_write(self.segmento, bytes(datos))
        self.maquina.reg_write(self.r.UC_X86_REG_SI, palabra(datos, 0x0DFA))
        self.ejecutar_original(0x0F5F)
        esperado = bytes(self.maquina.mem_read(self.segmento, 65536))
        if (
            not self.nativa.planificar_movimiento(
                (ctypes.c_ubyte * 65536).from_buffer(datos),
                palabra(datos, 0x0DFA),
                ctypes.byref(ctypes.c_ushort()),
            )
            or bytes(datos) != esperado
        ):
            raise ErrorAnalisis('Clic de transición distinto.')

    # Recupera solo los archivos de una instantánea de partida; nunca modifica sus banderas o entidades.
    def retomar_recursos(self, datos):
        # La instantánea recupera bancos y segmentos; descarta también sus traducciones anteriores.
        self.maquina.ctl_flush_tb()
        escena = palabra(datos, 0x8082)
        nombre = BANCOS_ESCENA[escena]
        self.escena = (
            self.carpeta / (('SNOWAST3' if escena == 40 else nombre) + '.SPR')
        ).read_bytes()
        self.nombre_mapa = nombre + '.MAP'
        modelo = datos[0x9974]
        if modelo < 0xFE:
            p = palabra(datos, 0x0918 + modelo * 2)
            nombre_modelo = bytes(datos[p : p + 13]).split(b'\0')[0].decode('ascii').upper()
            self.modelo = (self.carpeta / nombre_modelo).read_bytes()
        archivo = (self.carpeta / 'BOXDET.DAT').read_bytes()
        p = struct.unpack_from('<H', archivo, escena * 2)[0]
        self.cantidad = struct.unpack_from('<H', archivo, p)[0]
        self.geometria = archivo[p + 2 : p + 2 + self.cantidad * 10]
        self.buffer_geometria = (ctypes.c_ubyte * len(self.geometria)).from_buffer_copy(
            self.geometria
        )
        self.maquina.mem_write(0x4AEE, struct.pack('<H', 0x4000))

    # Contrasta todas las recogidas y sus límites de inventario, puntuación y fase en copias del estado real.
    def interacciones_corredor(self, base):
        for identificador in (6, 9, 10, 11, 12, 13, 14):
            for ocupadas in (2, 9, 10) if self.muestra else (0, 2, 8, 9, 10):
                for puntuacion in (0, 0xFFFF):
                    d = bytearray(base)
                    for i in range(10):
                        guardar(d, 0x5225 + i * 2, 40 + i if i < ocupadas else 0xFFFF)
                    guardar(d, 0x35C7, puntuacion)
                    guardar(d, 0xB60C, 0x563A)
                    guardar(d, 0xB620, identificador + 0x1A)
                    self.seleccionar(d, 0)
                    self.resumen['recogidas_corredor'] = (
                        self.resumen.get('recogidas_corredor', 0) + 1
                    )
        for manejador in (0x2FCA, 0x3143, 0x384A):
            for identificador in (1, 3, *range(6, 15)):
                d = bytearray(base)
                guardar(d, 0xB60C, manejador)
                guardar(d, 0xB620, identificador + 0x1A)
                self.seleccionar(d, 0)
        for accion in range(8):
            for previo in (0xFFFF, 1, 3, 6, 7):
                for objeto in (1, 3, *range(6, 15)):
                    d = bytearray(base)
                    guardar(d, 0xB60C, 0x3B53)
                    guardar(d, 0xB620, objeto + 0x1A)
                    guardar(d, 0xB634, accion)
                    guardar(d, 0xB638, previo)
                    self.seleccionar(d, 0)

    # Recorre la solución aportada por Àlex desde una entrada real, sin resolver puzles mediante atajos.
    def recorrido_completo(self, datos, semilla, carpeta):
        self.clic(datos, 267, 149)
        self.andar(datos, semilla, 0xFFFF, 96)
        self.menu(datos, (1,))
        objetos = [palabra(datos, 0xB620 + i * 2) for i in range(10)]
        if 0x20 not in objetos:
            raise ErrorAnalisis('La pecera no aparece en recoger al aproximarse a su soporte.')
        self.seleccionar(datos, objetos.index(0x20))
        self.esperar(datos, semilla, 96)
        inventario = [palabra(datos, 0x5225 + i * 2) for i in range(10)]
        if 6 not in inventario or 7 not in inventario or datos[0x366C] or datos[0x3675]:
            raise ErrorAnalisis(
                'La pecera no se conserva con su agua o sus descriptores siguen activos.'
            )
        self.captura(datos, carpeta, 'corredor_pecera')
        golpes = 0
        for paso in range(800):
            anterior = palabra(datos, 0x2C3B)
            salida = self.andar(datos, semilla, 8, 1)
            if palabra(datos, 0x2C3B) == 0x0F94 and anterior != 0x0F94:
                golpes += 1
                if golpes == 1:
                    self.captura(datos, carpeta, 'corredor_golpe')
            if salida is not None:
                break
        if salida != 1 or datos[0x3597] != 0 or golpes == 0:
            raise ErrorAnalisis(
                f'El corredor no termina por su salida uno: {salida}; golpes {golpes}.'
            )
        self.captura(datos, carpeta, 'corredor_salida')
        self.resumen['golpes_estatuas'] = self.resumen.get('golpes_estatuas', 0) + golpes
        self.cambiar(datos, semilla, 1)
        for paso in range(1, 257):
            salida = self.paso(datos, semilla)
            if paso == 16:
                self.captura(datos, carpeta, 'corredor_caida')
            if salida is not None:
                break
        if salida != 3:
            raise ErrorAnalisis(
                'La partida del corredor no enlaza con la salida automática de DROP.'
            )
        self.cambiar(datos, semilla, 3)
        self.esperar(datos, semilla, 24)
        self.captura(datos, carpeta, 'corredor_agua')
        inventario = [palabra(datos, 0x5225 + i * 2) for i in range(10)]
        if (
            palabra(datos, 0x8082) != 3
            or 6 not in inventario
            or 7 not in inventario
            or 1 in inventario
        ):
            raise ErrorAnalisis(
                'La llegada al agua altera la pecera o conserva la llave que retira BEA8.'
            )
        self.despues_corredor(datos, semilla, carpeta)
        self.resumen['recorridos_completos'] = self.resumen.get('recorridos_completos', 0) + 1

    # Permite prolongar la llegada real al lago sin inventar inventario ni saltar las escenas previas.
    def despues_corredor(self, datos, semilla, carpeta):
        pass

    # Cruza la salida original, prueba las dos direcciones de cámara y vuelve por la salida dos.
    def despues_puerta(self, datos, semilla, carpeta):
        inventario = bytes(datos[0x5225:0x5239])
        self.clic(datos, 45, 152)
        salida = self.andar(datos, semilla, 0xFFFF, 96)
        if salida != 0:
            raise ErrorAnalisis(
                f'No se alcanzó la salida cero: {salida}; x/y/ancho/alto={[palabra(datos, p) for p in (0x2C2D, 0x2C2F, 0x2C31, 0x2C33)]}; controles={list(datos[0x3594:0x3597])}; imagen={datos[0x2CB0]}.'
            )
        self.cambiar(datos, semilla, salida)
        self.esperar(datos, semilla, 48)
        self.captura(datos, carpeta, 'corredor_entrada')
        copia = bytearray(datos)
        copia_semilla = list(semilla)
        self.interacciones_corredor(datos)
        self.recorrido_completo(datos, semilla, carpeta)
        datos[:] = copia
        semilla[:] = copia_semilla
        self.retomar_recursos(datos)
        self.clic(datos, 210, 185)
        self.andar(datos, semilla, 0xFFFF, 96)
        self.clic(datos, 210, 160)
        self.andar(datos, semilla, 0xFFFF, 96)
        self.menu(datos, (3,))
        destinos = [palabra(datos, 0xB620 + i * 2) for i in range(10)]
        if 0x22 not in destinos:
            raise ErrorAnalisis(f'Mecanismo fuera del menú: {destinos}.')
        self.seleccionar(datos, destinos.index(0x22))
        self.esperar(datos, semilla, 192)
        self.captura(datos, carpeta, 'corredor_mecanismo')
        self.clic(datos, 210, 185)
        self.andar(datos, semilla, 0xFFFF, 96)
        if self.andar(datos, semilla, 8, 80) is not None:
            raise ErrorAnalisis('Se abandonó el corredor durante el desplazamiento de cámara.')
        self.captura(datos, carpeta, 'corredor_izquierda')
        # Los estados de cámara se prueban en copias: la entidad del corredor sigue bloqueando el paso real.
        self.decisiones_camara(datos)
        self.camara_visual(datos, -1, carpeta)
        prueba_derecha = bytearray(datos)
        self.camara(prueba_derecha, -1)
        self.camara_visual(prueba_derecha, 1, carpeta)
        prueba = bytearray(datos)
        for direccion in (-1, 1):
            for _ in range(28):
                self.camara(prueba, direccion)
        if bytes(prueba[0x5225:0x5239]) != inventario:
            raise ErrorAnalisis('La cámara alteró el inventario en sus estados sintéticos.')
        salida = self.andar(datos, semilla, 4, 160)
        if salida != 2:
            raise ErrorAnalisis(f'No se alcanzó el regreso por la salida dos: {salida}.')
        self.cambiar(datos, semilla, salida)
        self.esperar(datos, semilla, 48)
        if (
            palabra(datos, 0x8082) != 0
            or datos[0x2CB0] != 0x10
            or bytes(datos[0x5225:0x5239]) != inventario
        ):
            raise ErrorAnalisis('El regreso no conservó inventario o puerta abierta.')
        self.captura(datos, carpeta, 'mazmorra_regreso')


if __name__ == '__main__':
    argumentos = argparse.ArgumentParser(
        description='Compara corredor, cámara y regreso persistente.'
    )
    argumentos.add_argument('--biblioteca', type=Path, required=True)
    argumentos.add_argument('--muestra', action='store_true')
    opciones = argumentos.parse_args()
    try:
        ComparacionCorredor(
            Path(__file__).resolve().parents[1], opciones.biblioteca, opciones.muestra
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        argumentos.exit(1, f'Error: {error}\n')
