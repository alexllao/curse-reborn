"""Contrasta PCM de mazmorra contra los paquetes emitidos por el ensamblador original."""

import ctypes
import json
from pathlib import Path
import struct
import sys
import wave

from comparar_musica_rs import ComparacionMusicaRS


class SintetizadorReferencia:
    # Usa el mismo sintetizador y ROM, pero recibe MIDI reconstruido desde los puertos del oráculo.
    def __init__(self, raiz):
        nombre = (
            'mt32emu.dll'
            if sys.platform == 'win32'
            else 'libmt32emu.dylib'
            if sys.platform == 'darwin'
            else 'libmt32emu.so'
        )
        self.api = ctypes.CDLL(str(raiz / 'trabajo/aplicacion' / nombre))
        self.contexto = None
        puntero = ctypes.c_void_p
        for nombre, argumentos, retorno in (
            ('create_context', [puntero, puntero], puntero),
            ('add_rom_file', [puntero, ctypes.c_char_p], ctypes.c_int),
            ('get_rom_info', [puntero, puntero], None),
            ('set_analog_output_mode', [puntero, ctypes.c_int], None),
            ('set_stereo_output_samplerate', [puntero, ctypes.c_double], None),
            ('open_synth', [puntero], ctypes.c_int),
            ('play_msg', [puntero, ctypes.c_uint], ctypes.c_int),
            ('render_bit16s', [puntero, ctypes.POINTER(ctypes.c_short), ctypes.c_uint], None),
            ('free_context', [puntero], None),
        ):
            funcion = getattr(self.api, 'mt32emu_' + nombre)
            funcion.argtypes, funcion.restype = argumentos, retorno
        self.contexto = self.api.mt32emu_create_context(None, None)
        if not self.contexto:
            raise ValueError('No se ha creado el sintetizador de referencia.')
        for nombre in ('MT32_CONTROL.ROM', 'MT32_PCM.ROM'):
            ruta = str(raiz / 'datos_originales/ROMS' / nombre).encode('utf-8')
            if self.api.mt32emu_add_rom_file(self.contexto, ruta) < 0:
                self.cerrar()
                raise ValueError('Las ROM de referencia no se han reconocido.')
        self.api.mt32emu_set_analog_output_mode(self.contexto, 2)
        self.api.mt32emu_set_stereo_output_samplerate(self.contexto, 48000)
        if self.api.mt32emu_open_synth(self.contexto) != 0:
            self.cerrar()
            raise ValueError('No se ha abierto el sintetizador de referencia.')

    # Libera las ROM y el sintetizador incluso si falla la comparación.
    def cerrar(self):
        if self.contexto:
            self.api.mt32emu_free_context(self.contexto)
            self.contexto = None

    # Sustituye la parada del transporte por All Notes Off, igual que la plataforma actual.
    def parar(self):
        for canal in range(16):
            self.enviar(0xB0 | canal | 123 << 8)

    # Aplica mensajes completos obtenidos del puerto, sin usar EventoMusicaRS.mensaje.
    def enviar(self, mensaje):
        if self.api.mt32emu_play_msg(self.contexto, mensaje) != 0:
            raise ValueError('La cola MIDI de referencia no ha aceptado una orden.')

    # Conserva la conversión y ganancia de la plataforma para comparar también sus muestras.
    def generar(self, cantidad):
        estereo = (ctypes.c_short * (cantidad * 2))()
        self.api.mt32emu_render_bit16s(self.contexto, estereo, cantidad)
        return [max(-28000, min(28000, int(estereo[i]) * 2)) for i in range(cantidad * 2)]


# 7DF9–7EA1 genera los bytes de puerto; este decodificador separado restituye el estado MIDI.
def paquetes_originales(comparador, indice):
    banco = (comparador.raiz / f'datos_originales/CURSE/DATA/CURSE.RS{indice}').read_bytes()
    comparador.maquina.mem_write(0x60000, banco)
    datos = bytearray(comparador.base)
    datos[0x9955] = 0
    struct.pack_into('<HH', datos, 0x9926, 0, 0x6000)
    estado = ctypes.c_ubyte(0)
    estado_midi = 0
    for _ in range(20000):
        comparador.paquete(datos, banco, estado)
        tiempo, *mensaje = comparador.bytes
        if tiempo >= 0xF0:
            yield 240, None
            continue
        if mensaje[0] >= 0xF0:
            yield tiempo, None
            continue
        if mensaje[0] & 128:
            estado_midi = mensaje.pop(0)
        cantidad = 1 if estado_midi & 0xF0 in (0xC0, 0xD0) else 2
        if not estado_midi or len(mensaje) != cantidad or any(b >= 128 for b in mensaje):
            raise ValueError('Mensaje MIDI de referencia inválido.')
        completo = estado_midi | mensaje[0] << 8
        if len(mensaje) == 2:
            completo |= mensaje[1] << 16
        yield tiempo, completo
    raise ValueError('El recorrido musical de referencia superó su límite.')


# Sitúa eventos en muestras absolutas y conserva las fronteras de pulso del MPU de 120 PPQN.
def producir_banco(comparador, sintetizador, banco, tempo, cantidad):
    muestras, pulsos = [], 0
    incremento = tempo * 120
    for tiempo, mensaje in paquetes_originales(comparador, banco):
        pulsos += tiempo
        objetivo = min(cantidad, (pulsos * 2880000 + incremento - 1) // incremento)
        while len(muestras) // 2 < objetivo:
            siguiente_pulso = (len(muestras) // 2) * incremento // 2880000 + 1
            frontera = (siguiente_pulso * 2880000 + incremento - 1) // incremento
            bloque = min(objetivo - len(muestras) // 2, 512, frontera - len(muestras) // 2)
            muestras.extend(sintetizador.generar(bloque))
        if len(muestras) == cantidad * 2:
            return muestras
        if mensaje:
            sintetizador.enviar(mensaje)
    raise ValueError('La referencia musical terminó antes de tiempo.')


# Cambia de banco con voces todavía activas para detectar pérdidas de órdenes durante su liberación.
def producir_referencia(comparador, sintetizador, cantidad):
    sintetizador.parar()
    producir_banco(comparador, sintetizador, 5, 110, 48000 * 8)
    sintetizador.parar()
    return producir_banco(comparador, sintetizador, 1, 140, cantidad)


# Contrasta todo el WAV y guarda referencia e informe sin afirmar equivalencia con hardware físico.
def ejecutar():
    raiz = Path(__file__).resolve().parents[1]
    extension = 'dll' if sys.platform == 'win32' else 'so'
    comparador = ComparacionMusicaRS(
        raiz, raiz / f'trabajo/comparacion_acciones/tabla_acciones.{extension}', True
    )
    carpeta = raiz / 'trabajo/comparacion_musica_rs'
    with wave.open(str(carpeta / 'mazmorra_mt32.wav'), 'rb') as archivo:
        if (archivo.getnchannels(), archivo.getsampwidth(), archivo.getframerate()) != (
            2,
            2,
            48000,
        ):
            raise ValueError('Formato de salida MT-32 inesperado.')
        cantidad = archivo.getnframes()
        if cantidad < 48000 * 32:
            raise ValueError('Falta generar la muestra actual de 32 segundos.')
        original = struct.unpack('<' + 'h' * (cantidad * 2), archivo.readframes(cantidad))
    sintetizador = SintetizadorReferencia(raiz)
    try:
        referencia = producir_referencia(comparador, sintetizador, cantidad)
    finally:
        sintetizador.cerrar()
    distintas = sum(a != b for a, b in zip(original, referencia))
    with wave.open(str(carpeta / 'referencia_ensamblador_mt32.wav'), 'wb') as archivo:
        archivo.setparams((2, 2, 48000, 0, 'NONE', 'sin compresión'))
        archivo.writeframes(struct.pack('<' + 'h' * (cantidad * 2), *referencia))
    informe = {
        'fotogramas': cantidad,
        'muestras': cantidad * 2,
        'canales': 2,
        'distintas': distintas,
        'frecuencia': 48000,
        'limite': 'Misma biblioteca Munt y ROM; no ejecuta un MPU completo ni verifica hardware físico.',
    }
    (carpeta / 'sintesis.json').write_text(
        json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8'
    )
    if distintas:
        raise ValueError(f'Hay {distintas} muestras diferentes respecto al oráculo de puertos.')
    print(
        f'Síntesis MT-32: {cantidad * 2} muestras estéreo idénticas al oráculo de puertos del ensamblador.'
    )


if __name__ == '__main__':
    try:
        ejecutar()
    except (OSError, ValueError) as error:
        sys.exit(f'Error: {error}')
