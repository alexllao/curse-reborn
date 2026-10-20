"""Contrasta PCM y reloj con las escrituras de las rutinas DOS ejecutadas por Unicorn."""

import argparse
import ctypes
import hashlib
import json
from pathlib import Path
import struct
import wave

from comparar_altavoz import ComparacionAltavoz, ErrorAnalisis


class ComparacionAudio(ComparacionAltavoz):
    # Reutiliza el ejecutable verificado y los ganchos reales de IN/OUT del comparador del altavoz.
    def __init__(self, raiz, biblioteca, muestra):
        super().__init__(raiz, biblioteca, muestra)
        self.nativa.procesar_pcm.argtypes = [
            ctypes.c_uint,
            ctypes.POINTER(ctypes.c_ubyte),
            ctypes.c_ushort,
            ctypes.c_ushort,
            ctypes.POINTER(ctypes.c_short),
            ctypes.c_uint,
        ]
        self.nativa.procesar_pcm.restype = ctypes.c_int

    # Invoca un fragmento DOS y comprueba el retorno, sin usar su traducción C++ como oráculo.
    def original(self, entrada, datos, identificador=0, entidad=0x2C2B):
        self.traza = []
        self.maquina.mem_write(self.segmento, bytes(datos))
        for registro, valor in {
            self.r.UC_X86_REG_CS: 0,
            self.r.UC_X86_REG_DS: 0x0F31,
            self.r.UC_X86_REG_SS: 0x8000,
            self.r.UC_X86_REG_SP: 0xFF00,
            self.r.UC_X86_REG_SI: entidad,
            self.r.UC_X86_REG_AX: identificador,
            self.r.UC_X86_REG_EFLAGS: 2,
        }.items():
            self.maquina.reg_write(registro, valor)
        self.maquina.mem_write(0x8FF00, struct.pack('<H', 0x7000))
        self.maquina.emu_start(entrada, 0x7000, count=20000)
        if self.maquina.reg_read(self.r.UC_X86_REG_IP) != 0x7000:
            raise ErrorAnalisis('El fragmento original de audio no terminó.')
        return bytearray(self.maquina.mem_read(self.segmento, 65536))

    # Mantiene una única instancia nativa entre bloques para detectar reinicios de fase o reloj.
    def nativo(self, operacion, datos, primero=0, segundo=0, cantidad=0):
        memoria = (ctypes.c_ubyte * 65536).from_buffer(datos)
        salida = (ctypes.c_short * cantidad)()
        if not self.nativa.procesar_pcm(operacion, memoria, primero, segundo, salida, cantidad):
            raise ErrorAnalisis(f'Operación PCM rechazada: {operacion}.')
        return list(salida)

    # Suma analíticamente los tramos altos y bajos de una onda, sin copiar el bucle C++ por ciclo.
    @staticmethod
    def integral(tiempo, divisor):
        vueltas, resto = divmod(tiempo, divisor)
        altos = vueltas * ((divisor + 1) // 2) + min(resto, (divisor + 1) // 2)
        return 2 * altos - tiempo

    # Aplica las escrituras DOS en sus tiempos de IRQ y evalúa cada intervalo de muestra exacto.
    def referencia_pcm(self, eventos, cantidad):
        divisor, origen, control, bajo, indice, reloj = 65536, 0, 0, None, 0, 0
        salida = []
        for muestra in range(cantidad):
            fin = (muestra + 1) * 14318180 // 576000
            inicio, suma = reloj, 0
            while reloj < fin:
                while indice < len(eventos) and eventos[indice][0] <= reloj:
                    instante, puerto, valor = eventos[indice]
                    indice += 1
                    if puerto == 0x43:
                        bajo = None
                    elif puerto == 0x42:
                        if bajo is None:
                            bajo = valor
                        else:
                            divisor = (bajo | valor << 8) or 65536
                            origen, bajo = instante, None
                    elif puerto == 0x61:
                        if valor & 1 and not control & 1:
                            origen = instante
                        control = valor
                limite = min(fin, eventos[indice][0]) if indice < len(eventos) else fin
                if control & 3 == 3:
                    suma += self.integral(limite - origen, divisor) - self.integral(
                        reloj - origen, divisor
                    )
                reloj = limite
            salida.append((1 if suma >= 0 else -1) * (abs(suma) * 4096 // (fin - inicio)))
        return salida

    # Recorre los efectos reales de las escenas integradas y compara memoria, señal y particiones.
    def ejecutar(self):
        cantidad = 24000 if self.muestra else 96000
        comprobados, audibles, ejemplo = 0, 0, []
        for escena in range(4):
            for identificador in (0, 1, 15, 17) if self.muestra else range(18):
                datos = bytearray(self.base)
                datos[0x0AD4], datos[0x9924] = 1, 0
                struct.pack_into('<H', datos, 0x8082, escena)
                struct.pack_into('<4H', datos, 0x2C2D, 120, 80, 32, 48)
                struct.pack_into('<H', datos, 0x9920, 0)
                self.control = 0
                self.maquina.mem_write(0x5306, b'\0')
                self.nativo(0, datos)
                esperado = self.original(0x5288, datos, identificador)
                self.nativo(1, datos, 0x2C2B, identificador)
                if datos != esperado:
                    raise ErrorAnalisis('Selección original y selección PCM distintas.')
                inicio = bytes(datos)
                eventos = []
                ciclos = cantidad * 14318180 // 576000
                for instante in range(0x4250, ciclos + 1, 0x4250):
                    esperado = self.original(0x7AC6, esperado)
                    eventos.extend(
                        (instante, puerto, valor) for tipo, puerto, valor in self.traza if tipo
                    )
                pcm = self.nativo(2, datos, cantidad=cantidad)
                if datos != esperado or pcm != self.referencia_pcm(eventos, cantidad):
                    raise ErrorAnalisis(
                        f'Audio distinto en escena {escena}, efecto {identificador}.'
                    )
                final = bytes(datos)
                datos[:] = inicio
                self.nativo(0, datos)
                # Restaurar también la prioridad mediante la selección real inicial.
                datos[0x9924] = 0
                self.nativo(1, datos, 0x2C2B, identificador)
                bloques = []
                restantes = cantidad
                for tamano in (1, 17, 683, 1024, 4096):
                    tomar = min(tamano, restantes)
                    bloques.extend(self.nativo(2, datos, cantidad=tomar))
                    restantes -= tomar
                bloques.extend(self.nativo(2, datos, cantidad=restantes))
                if pcm != bloques or bytes(datos) != final:
                    raise ErrorAnalisis('La partición de bloques altera el sonido o DS.')
                comprobados += 1
                if any(pcm):
                    audibles += 1
                    if not ejemplo:
                        ejemplo = pcm
        # 3545 y 37FF: respuestas del menú con y sin dispositivo, visibles y fuera de pantalla.
        respuestas = 0
        for modalidad in (0, 1):
            for identificador in (0, 1):
                for x in (120, 400):
                    datos = bytearray(self.base)
                    datos[0x0AD4] = modalidad
                    datos[0x9924] = datos[0x3607] = 0
                    struct.pack_into('<H', datos, 0x8082, 0)
                    struct.pack_into('<H', datos, 0x9920, 0)
                    struct.pack_into('<4H', datos, 0x2C2D, x, 80, 32, 48)
                    self.nativo(0, datos)
                    self.maquina.mem_write(0x5306, b'\0')
                    esperado = self.original(0x3545 if identificador == 0 else 0x37FF, datos)
                    self.nativo(5, datos, 0x32DE, identificador)
                    if datos != esperado:
                        raise ErrorAnalisis('La respuesta de menú no conserva el efecto original.')
                    respuestas += 1
        # Contadores extremos, divisor cero, periodos impares y compuerta apagada.
        fronteras = 0
        for divisor in (0, 1, 2, 3, 1193, 65535):
            for control in (0, 1, 2, 3):
                datos = bytearray(65536)
                self.nativo(0, datos)
                puertos = [
                    (0x43, 0xB6),
                    (0x42, divisor & 255),
                    (0x42, divisor >> 8),
                    (0x61, control),
                ]
                for puerto, valor in puertos:
                    self.nativo(3, datos, puerto, valor)
                pcm = self.nativo(2, datos, cantidad=4096)
                if pcm != self.referencia_pcm([(0, p, v) for p, v in puertos], 4096):
                    raise ErrorAnalisis('Señal incorrecta en un límite del contador o compuerta.')
                self.nativo(4, datos)
                if any(self.nativo(2, datos, cantidad=512)):
                    raise ErrorAnalisis('La parada del altavoz deja muestras audibles.')
                fronteras += 1
        if not audibles:
            raise ErrorAnalisis('Ningún efecto real produjo una señal audible.')
        carpeta = self.raiz / 'apps/juego/trabajo/comparacion_audio'
        carpeta.mkdir(parents=True, exist_ok=True)
        with wave.open(str(carpeta / 'efecto_original.wav'), 'wb') as archivo:
            archivo.setparams((1, 2, 48000, 0, 'NONE', 'sin compresión'))
            archivo.writeframes(struct.pack(f'<{len(ejemplo)}h', *ejemplo))
        informe = {
            'sha256_ejecutable': self.cabecera['sha256'],
            'sha256_biblioteca': hashlib.sha256(self.biblioteca.read_bytes()).hexdigest(),
            'efectos_comparados': comprobados,
            'efectos_con_senal': audibles,
            'fronteras': fronteras,
            'respuestas_menu': respuestas,
            'muestras_por_efecto': cantidad,
            'resultado': 'coinciden memoria, PCM e independencia de bloques',
            'limites': 'Señal digital del PIT; no valida acústica del altavoz físico, música ni otros dispositivos.',
        }
        (carpeta / ('muestra.json' if self.muestra else 'resultado.json')).write_text(
            json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8'
        )
        print(f'PCM: {comprobados} efectos, {fronteras} fronteras; memoria y señal coincidentes.')
        print(f'Efectos con señal: {audibles}. WAV local: {carpeta / "efecto_original.wav"}')


if __name__ == '__main__':
    argumentos = argparse.ArgumentParser(description='Contrasta la salida digital del PC Speaker.')
    argumentos.add_argument('--biblioteca', type=Path, required=True)
    argumentos.add_argument('--muestra', action='store_true')
    opciones = argumentos.parse_args()
    try:
        ComparacionAudio(
            Path(__file__).resolve().parents[4], opciones.biblioteca, opciones.muestra
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        argumentos.exit(1, f'Error: {error}\n')
