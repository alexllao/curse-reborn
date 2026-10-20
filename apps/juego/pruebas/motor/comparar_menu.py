"""Compara navegación, primeras acciones y dibujo del menú con las rutinas del EXE."""

import argparse
import ctypes
import json
from pathlib import Path
import struct

from comparar_composicion import ComparacionComposicion, ErrorAnalisis
from comparar_objetos import ComparacionObjetos, guardar, palabra
from generar_titulo import guardar_png


class ComparacionMenu(ComparacionComposicion):
    # Reutiliza los bancos y el modelo VGA contrastados y añade el puente del menú.
    def __init__(self, raiz, biblioteca, muestra):
        super().__init__(raiz, biblioteca, muestra)
        self.referencia_modelo = 0x5400
        memoria = ctypes.POINTER(ctypes.c_ubyte)
        self.nativa.procesar_menu.argtypes = [
            ctypes.c_uint,
            memoria,
            ctypes.c_ushort,
            ctypes.c_ushort,
        ]
        self.nativa.procesar_menu.restype = ctypes.c_int
        self.nativa.dibujar_menu.argtypes = [memoria, memoria, memoria, ctypes.c_uint, memoria]
        self.nativa.dibujar_menu.restype = ctypes.c_int
        self.nativa.componer_menu_escena.argtypes = [
            memoria,
            memoria,
            memoria,
            ctypes.c_uint,
            memoria,
            ctypes.c_uint,
            memoria,
            ctypes.c_uint,
        ]
        self.nativa.componer_menu_escena.restype = ctypes.c_int
        self.nativa.componer_menu_escena_bancos.argtypes = (
            self.nativa.componer_menu_escena.argtypes + [ctypes.c_ushort]
        )
        self.nativa.componer_menu_escena_bancos.restype = ctypes.c_int
        self.estandar = (self.carpeta / 'STD.BRD').read_bytes()
        self.raton = (self.carpeta / 'MOUSE.SPR').read_bytes()
        self.tipo_dimensiones = ctypes.CFUNCTYPE(
            ctypes.c_bool,
            ctypes.c_void_p,
            ctypes.c_ushort,
            ctypes.c_ushort,
            ctypes.POINTER(ctypes.c_ushort),
            ctypes.POINTER(ctypes.c_ushort),
        )
        self.tipo_modelo = ctypes.CFUNCTYPE(
            ctypes.c_bool, ctypes.c_void_p, memoria, ctypes.c_ushort, ctypes.c_ushort
        )
        self.callback_dimensiones = self.tipo_dimensiones(self.dimensiones_recurso)
        self.callback_modelo = self.tipo_modelo(self.leer_modelo)
        self.nativa.avanzar_menu.argtypes = [
            memoria,
            ctypes.POINTER(ctypes.c_ushort),
            self.tipo_dimensiones,
            self.tipo_modelo,
        ]
        self.nativa.avanzar_menu.restype = ctypes.c_int
        self.maquina.hook_add(self.unicorn.UC_HOOK_INTR, self.servicio_modelo)
        self.logica = True
        self.maquina.hook_add(self.unicorn.UC_HOOK_CODE, self.instruccion_menu)
        self.iconos = (self.carpeta / 'ICONS.SPR').read_bytes()
        comprimido = (self.carpeta / 'MENU.DAT').read_bytes()
        entrada = (ctypes.c_ubyte * len(comprimido)).from_buffer_copy(comprimido)
        self.fondo_menu = (ctypes.c_ubyte * 10240)()
        escritos = ctypes.c_uint()
        self.nativa.descomprimir_rnc1.argtypes = [
            memoria,
            ctypes.c_uint,
            memoria,
            ctypes.c_uint,
            ctypes.POINTER(ctypes.c_uint),
        ]
        if (
            self.nativa.descomprimir_rnc1(
                entrada, len(comprimido), self.fondo_menu, 10240, ctypes.byref(escritos)
            )
            != 0
            or escritos.value != 10240
        ):
            raise ErrorAnalisis('No se pudo preparar el fondo del menú.')
        self.maquina.mem_write(0xD57A, struct.pack('<H', 0x4000))
        self.maquina.mem_write(0xD57E, struct.pack('<H', 0xA000))
        self.maquina.mem_write(0xD580, struct.pack('<H', 0x7600))
        self.maquina.mem_write(0x40000, self.iconos)
        self.maquina.mem_write(0x76000, bytes(self.fondo_menu))
        self.maquina.mem_write(0x6BFC, struct.pack('<200H', *(i * 112 for i in range(200))))

    # En comparaciones de memoria omite solo dibujo; conserva DF15 y los retornos y bloqueos.
    def instruccion_menu(self, maquina, direccion, tamano, contexto):
        if self.logica and direccion == 0xDF4E:
            maquina.reg_write(self.r.UC_X86_REG_IP, 0xDFCF)
        elif self.logica and direccion in (0x7636, 0x6D8C):
            pila = maquina.reg_read(self.r.UC_X86_REG_SP)
            retorno = struct.unpack('<H', maquina.mem_read(0x80000 + pila, 2))[0]
            maquina.reg_write(self.r.UC_X86_REG_SP, pila + 2)
            maquina.reg_write(self.r.UC_X86_REG_IP, retorno)

    # Compara una selección completa contra su manejador original y los 65.536 bytes de DS.
    def seleccionar(self, datos, casilla):
        m, r = self.maquina, self.r
        manejador = palabra(datos, 0xB60C + casilla * 2)
        m.mem_write(self.segmento, bytes(datos))
        m.reg_write(r.UC_X86_REG_BX, casilla)
        self.ejecutar_original(manejador)
        esperado = bytes(m.mem_read(self.segmento, 65536))
        resultado = self.nativa.procesar_menu(
            1, (ctypes.c_ubyte * 65536).from_buffer(datos), casilla, 0
        )
        if resultado not in (0, 1) or bytes(datos) != esperado:
            diferencias = [f'{i:04X}' for i in range(65536) if datos[i] != esperado[i]][:8]
            raise ErrorAnalisis(
                f'Manejador {manejador:04X} distinto: {diferencias}, resultado={resultado}.'
            )
        if (resultado == 1) != bool(m.reg_read(r.UC_X86_REG_EFLAGS) & 1):
            raise ErrorAnalisis('El retorno de selección no coincide con el acarreo original.')
        self.resumen['selecciones'] = self.resumen.get('selecciones', 0) + 1

    # Contrasta los diez iconos y las máscaras contra DEFB y DE96 completos con dibujo original.
    def dibujar_panel(self, datos, principal=False):
        self.logica = False
        self.maquina.mem_write(0x40000, self.iconos)
        self.maquina.mem_write(0x76000, bytes(self.fondo_menu))
        self.reiniciar_video()
        self.maquina.mem_write(self.segmento, bytes(datos))
        self.ejecutar_original(0xDE96 if principal else 0xDEFB)
        esperado = self.imagen_original(112)[:10240]
        if principal:
            self.nativa.procesar_menu(0, (ctypes.c_ubyte * 65536).from_buffer(datos), 152, 92)
        salida = (ctypes.c_ubyte * 64016)(*([0] * 64000), *([0xCD] * 16))
        iconos = (ctypes.c_ubyte * len(self.iconos)).from_buffer_copy(self.iconos)
        if not self.nativa.dibujar_menu(
            (ctypes.c_ubyte * 65536).from_buffer(datos),
            self.fondo_menu,
            iconos,
            len(self.iconos),
            salida,
        ):
            raise ErrorAnalisis('El dibujo del menú rechazó sus recursos reales.')
        if bytes(salida[:10240]) != esperado or bytes(salida[64000:]) != bytes([0xCD] * 16):
            raise ErrorAnalisis('Dibujo de iconos distinto o fuera de límites.')
        self.logica = True
        self.resumen['paneles'] = self.resumen.get('paneles', 0) + 1
        return bytes(salida[:64000])

    # Resuelve dimensiones de los tres bancos, incluidas las sustituciones que ordena F8.
    def dimensiones_recurso(self, contexto, referencia, indice, ancho, alto):
        banco = {
            0x4000: self.escena,
            self.referencia_modelo: self.modelo,
            0x6400: self.estandar,
            0x7400: self.raton,
        }.get(referencia)
        if banco is None or indice * 2 + 2 > len(banco):
            return False
        posicion = struct.unpack_from('<H', banco, indice * 2)[0] * 16
        if posicion + 10 > len(banco):
            return False
        ancho[0], alto[0] = struct.unpack_from('<2H', banco, posicion)
        return True

    # Atiende la carga nativa conservando el archivo elegido por las tablas originales del EXE.
    def leer_modelo(self, contexto, datos, nombre, capacidad):
        archivo = bytes(datos[nombre : nombre + 13]).split(b'\0')[0].decode('ascii').upper()
        if '/' in archivo or '\\' in archivo or not archivo.endswith('.BRD'):
            return False
        self.modelo = (self.carpeta / archivo).read_bytes()
        return len(self.modelo) <= capacidad

    # Sustituye exclusivamente los servicios DOS de lectura del banco solicitado por F8.
    def servicio_modelo(self, maquina, numero, contexto):
        r = self.r
        funcion = maquina.reg_read(r.UC_X86_REG_AX) >> 8
        destino = maquina.reg_read(r.UC_X86_REG_DS) * 16 + maquina.reg_read(r.UC_X86_REG_DX)
        if numero != 0x21:
            raise ErrorAnalisis('Interrupción inesperada en los guiones.')
        if funcion == 0x3D:
            archivo = bytes(maquina.mem_read(destino, 13)).split(b'\0')[0].decode('ascii').upper()
            self.archivo_abierto = (self.carpeta / archivo).read_bytes()
            maquina.reg_write(r.UC_X86_REG_AX, 1)
        elif funcion == 0x3F:
            maquina.mem_write(destino, self.archivo_abierto)
            maquina.reg_write(r.UC_X86_REG_AX, len(self.archivo_abierto))
        elif funcion != 0x3E:
            raise ErrorAnalisis('Servicio inesperado al cambiar de modelo.')
        maquina.reg_write(r.UC_X86_REG_EFLAGS, maquina.reg_read(r.UC_X86_REG_EFLAGS) & ~1)

    # Mantiene bancos separados: DUNGEON.SPR supera 64 KiB y su final no cabe antes de 50000h.
    def preparar_bancos_originales(self):
        bancos = (
            ('DUNGEON.SPR', 0x40000, self.escena),
            ('modelo', self.referencia_modelo * 16, self.modelo),
            ('STD.BRD', 0x64000, self.estandar),
            ('MOUSE.SPR', 0x74000, self.raton),
            ('MENU.DAT', 0x76000, bytes(self.fondo_menu)),
        )
        for anterior, siguiente in zip(bancos, bancos[1:]):
            if anterior[1] + len(anterior[2]) > siguiente[1]:
                raise ErrorAnalisis(
                    f'Bancos de referencia solapados: {anterior[0]} y {siguiente[0]}.'
                )
        self.maquina.mem_write(0x0010, struct.pack('<H', self.referencia_modelo))
        self.maquina.mem_write(0x000E, struct.pack('<H', 0x6400))
        for nombre, direccion, datos in bancos:
            self.maquina.mem_write(direccion, datos)
        for nombre, direccion, datos in bancos:
            if bytes(self.maquina.mem_read(direccion, len(datos))) != bytes(datos):
                raise ErrorAnalisis(
                    f'El banco de referencia {nombre} no conserva sus bytes originales.'
                )

    # Contrasta movimiento de acciones, guiones, comportamientos y modelo con estado persistente.
    def avanzar(self, datos, semilla):
        m = self.maquina
        self.preparar_bancos_originales()
        m.mem_write(0x000E, struct.pack('<H', 0x6400))
        m.mem_write(self.segmento, bytes(datos))
        m.mem_write(0x78DE, struct.pack('<4H', *semilla))
        self.ejecutar_hasta(0x096D, 0x0A59)
        self.ejecutar_original(0x83DE)
        self.ejecutar_original(0x10EC)
        esperado = bytes(m.mem_read(self.segmento, 65536))
        azar = bytes(m.mem_read(0x78DE, 8))
        resultado = self.nativa.avanzar_menu(
            (ctypes.c_ubyte * 65536).from_buffer(datos),
            semilla,
            self.callback_dimensiones,
            self.callback_modelo,
        )
        if resultado != 1:
            raise ErrorAnalisis(
                f'El avance nativo quedó pendiente de un servicio: {resultado}, paso={self.resumen.get("pasos_interaccion", 0)}.'
            )
        if bytes(datos) != esperado or bytes(semilla) != azar:
            diferencias = [f'{i:04X}' for i in range(65536) if datos[i] != esperado[i]][:8]
            detalle = ', '.join(
                f'{i}: C++={datos[int(i, 16)]:02X}, original={esperado[int(i, 16)]:02X}'
                for i in diferencias
            )
            raise ErrorAnalisis(
                f'Avance de interacción distinto en el paso {self.resumen.get("pasos_interaccion", 0) + 1}: {detalle}.'
            )
        self.resumen['pasos_interaccion'] = self.resumen.get('pasos_interaccion', 0) + 1

    # Recorre los menús compartidos y arranca la secuencia particular mediante el tema original cero.
    def ejecutar(self):
        preparacion = ComparacionObjetos(self.raiz, self.biblioteca, True)
        datos = preparacion.caso('entrada', 4, bytearray(self.base), 'MANA.BRD')
        guardar(datos, 0x2C4A, 0x5400)
        preparacion.caso('carga', 0, datos)
        guardar(datos, 0x2C29, 0x7400)
        # Una iteración consume FC/7C y habilita el menú como hace el bucle inicial.
        semilla = (ctypes.c_ushort * 4)(*self.semilla_original)
        self.avanzar(datos, semilla)
        self.resumen = {}
        base = bytes(datos)
        for ocupado in (0, 0x8197):
            for casilla in range(10):
                if casilla == 9:
                    continue  # Información y guardado se mantienen pendientes.
                prueba = bytearray(base)
                guardar(prueba, 0x2C3B, ocupado)
                self.nativa.procesar_menu(0, (ctypes.c_ubyte * 65536).from_buffer(prueba), 152, 92)
                self.seleccionar(prueba, casilla)
                if casilla == 2:
                    for subaccion in range(8):
                        sub = bytearray(prueba)
                        self.seleccionar(sub, subaccion)
        salida = self.raiz / 'apps/juego/trabajo/comparacion_menu'
        salida.mkdir(parents=True, exist_ok=True)
        prueba = bytearray(base)
        self.nativa.procesar_menu(0, (ctypes.c_ubyte * 65536).from_buffer(prueba), 152, 92)
        imagen = self.dibujar_panel(prueba)
        for ocupado in (0x8197, 0):
            estado = bytearray(prueba)
            guardar(estado, 0x2C3B, ocupado)
            self.dibujar_panel(estado, True)
        for objeto in (1, 3):
            estado = bytearray(prueba)
            guardar(estado, 0xB620, 0x1A + objeto)
            guardar(estado, 0xB60C, 0x563A)
            self.seleccionar(estado, 0)
        guardar_png(salida / 'menu_cpp.png', imagen, (self.carpeta / 'SPRITES.PAL').read_bytes())
        escena_inicial, paleta = self.componer_resultado(prueba, salida)
        panel = self.dibujar_panel(prueba)
        imagen_menu = bytearray(escena_inicial)
        imagen_menu[:10240] = panel[:10240]
        for i in range(26):
            for x, y in ((131 + i, 3), (131 + i, 28), (131, 3 + i), (156, 3 + i)):
                imagen_menu[y * 320 + x] = 255
        paleta = bytearray(paleta)
        paleta[765:768] = prueba[0xB63E:0xB641]
        guardar_png(salida / 'menu_escena_cpp.png', imagen_menu, paleta)
        self.seleccionar(prueba, 4)
        self.dibujar_panel(prueba)
        self.seleccionar(prueba, 1)  # Tema cero: su posición real sigue al tema uno.
        for paso in range(32 if self.muestra else 192):
            self.avanzar(prueba, semilla)
            if paso == 31:
                self.componer_resultado(prueba, salida)
        for invertido in (0, 0x80):
            for modelo in (0, 1):
                estado = bytearray(base)
                estado[0x2C2C] = invertido
                estado[0x358C] = modelo
                self.maquina.mem_write(self.segmento, bytes(estado))
                self.ejecutar_original(0x989E)
                esperado = bytes(self.maquina.mem_read(self.segmento, 65536))
                if (
                    self.nativa.procesar_menu(
                        2, (ctypes.c_ubyte * 65536).from_buffer(estado), 0x989E, 0
                    )
                    != 1
                    or bytes(estado) != esperado
                ):
                    raise ErrorAnalisis('La combinación inicial no coincide.')
                self.resumen['combinaciones'] = self.resumen.get('combinaciones', 0) + 1
        # Contrasta la activación de una acción al alcanzar un nodo, con canales programados.
        for caso in range(8 if self.muestra else 64):
            estado = bytearray(base)
            guardar(estado, 0x0DFA, 0x1A36)
            guardar(estado, 0x1A36, 0)
            guardar(estado, 0x1A38, 125 + caso)
            guardar(estado, 0x1A3A, 157)
            guardar(estado, 0x0AD7, 0x972C)
            guardar(estado, 0x009C, 0x2C4C)
            guardar(estado, 0x009E, 0x0201)
            guardar(estado, 0x00A0, 0x8197)
            self.maquina.mem_write(self.segmento, bytes(estado))
            self.ejecutar_hasta(0x096D, 0x0A59)
            esperado = bytes(self.maquina.mem_read(self.segmento, 65536))
            if (
                self.nativa.procesar_menu(3, (ctypes.c_ubyte * 65536).from_buffer(estado), 0, 0)
                != 0
                or bytes(estado) != esperado
            ):
                raise ErrorAnalisis('Activación de canales pendiente distinta.')
            self.resumen['activaciones'] = self.resumen.get('activaciones', 0) + 1
        ruta = salida / ('muestra.json' if self.muestra else 'resultado.json')
        ruta.write_text(
            json.dumps(
                {
                    'resultado': 'coinciden',
                    'casos_por_grupo': self.resumen,
                    'casos_totales': sum(self.resumen.values()),
                    'muestra': self.muestra,
                },
                ensure_ascii=False,
                indent=2,
            )
            + '\n',
            encoding='utf-8',
        )
        for grupo, cantidad in self.resumen.items():
            print(f'{grupo}: {cantidad} casos coincidentes.')
        print(f'Total: {sum(self.resumen.values())} casos/pasos. Coinciden memoria y composición.')
        print(f'Informe local: {ruta}')

    # Contrasta la imagen posterior a una acción con EBBD usando los bancos que dejó el guion.
    def componer_resultado(self, datos, salida):
        self.logica = False
        fondo = self.fondo(
            getattr(self, 'nombre_mapa', 'DUNGEON.MAP'), getattr(self, 'seleccion_fondo', range(10))
        )
        self.maquina.mem_write(0x0008, struct.pack('<H', 0xA000))
        self.preparar_bancos_originales()
        self.maquina.mem_write(self.segmento, bytes(datos))
        self.maquina.mem_write(0x0BF7, b'\0')
        self.reiniciar_video()
        for y in range(200):
            for x in range(320):
                self.planos[(x & 3) * 65536 + y * 112 + x // 4] = fondo.indices[y * 320 + x]
        self.logica = False
        self.ejecutar_original(0xEBBD)
        esperado = self.imagen_original(112)
        imagen = (ctypes.c_ubyte * 64000).from_buffer_copy(bytes(fondo.indices))
        bancos = [
            (ctypes.c_ubyte * len(b)).from_buffer_copy(b)
            for b in (self.escena, self.modelo, self.estandar)
        ]
        if (
            not self.nativa.componer_menu_escena_bancos(
                (ctypes.c_ubyte * 65536).from_buffer(datos),
                imagen,
                bancos[0],
                len(self.escena),
                bancos[1],
                len(self.modelo),
                bancos[2],
                len(self.estandar),
                self.referencia_modelo,
            )
            or bytes(imagen) != esperado
        ):
            raise ErrorAnalisis('La composición de la interacción no coincide.')
        guardar_png(salida / 'interaccion_32_cpp.png', bytes(imagen), fondo.paleta)
        # La composición del fondo utiliza CS:0010; se restablece para el siguiente F8.
        self.maquina.mem_write(0x0010, struct.pack('<H', self.referencia_modelo))
        self.logica = True
        self.resumen['composicion_interaccion'] = self.resumen.get('composicion_interaccion', 0) + 1
        return bytes(imagen), fondo.paleta

    # Ejecuta el fragmento del bucle hasta la siguiente llamada a secuencias, sin entrar en ella.
    def ejecutar_hasta(self, entrada, final):
        m, r = self.maquina, self.r
        for registro, valor in (
            (r.UC_X86_REG_CS, 0),
            (r.UC_X86_REG_DS, 0x0F31),
            (r.UC_X86_REG_ES, 0x0F31),
            (r.UC_X86_REG_SS, 0x8000),
            (r.UC_X86_REG_SP, 0xFF00),
            (r.UC_X86_REG_EFLAGS, 2),
        ):
            m.reg_write(registro, valor)
        m.emu_start(entrada, final, count=20000)
        if m.reg_read(r.UC_X86_REG_IP) != final:
            raise ErrorAnalisis('El bloque de activación no terminó.')


if __name__ == '__main__':
    argumentos = argparse.ArgumentParser(description='Compara menú y primeras interacciones.')
    argumentos.add_argument('--biblioteca', type=Path, required=True)
    argumentos.add_argument('--muestra', action='store_true')
    opciones = argumentos.parse_args()
    try:
        ComparacionMenu(
            Path(__file__).resolve().parents[4], opciones.biblioteca, opciones.muestra
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        argumentos.exit(1, f'Error: {error}\n')
