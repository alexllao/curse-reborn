"""Compara una captura MIDI real de DOSBox con el flujo reconstruido de la mazmorra."""

import argparse
import json
from pathlib import Path
import struct

from comparar_avance import ErrorAnalisis
from comparar_musica_rs import ComparacionMusicaRS
from comparar_sintesis_mt32 import paquetes_originales


# Lee el entero de longitud variable de un archivo MIDI sin aceptar datos truncados.
def leer_variable(datos, posicion):
    valor = 0
    for _ in range(4):
        if posicion >= len(datos):
            raise ValueError('Captura MIDI truncada.')
        byte = datos[posicion]
        posicion += 1
        valor = valor * 128 + (byte & 127)
        if byte < 128:
            return valor, posicion
    raise ValueError('Tiempo MIDI fuera de formato.')


# DOSBox guarda una pista SMF cero; conserva cambios de tempo y estado MIDI implícito.
def leer_captura(ruta):
    datos = ruta.read_bytes()
    if len(datos) < 22 or datos[:8] != b'MThd\0\0\0\6' or datos[14:18] != b'MTrk':
        raise ValueError('La referencia no es una captura MIDI de una pista.')
    formato, pistas, division = struct.unpack_from('>HHH', datos, 8)
    if formato != 0 or pistas != 1 or not 0 < division < 32768:
        raise ValueError('La captura debe usar formato cero y pulsos por negra.')
    longitud = struct.unpack_from('>I', datos, 18)[0]
    if longitud != len(datos) - 22:
        raise ValueError('Longitud MIDI inesperada.')
    posicion, estado, tempo, tiempo = 22, 0, 500000, 0
    eventos = []
    while posicion < len(datos):
        delta, posicion = leer_variable(datos, posicion)
        tiempo += delta * tempo / division
        if posicion >= len(datos):
            raise ValueError('Falta el mensaje MIDI.')
        primero = datos[posicion]
        if primero >= 128:
            posicion += 1
        else:
            primero = estado
        if primero == 255:
            tipo = datos[posicion]
            posicion += 1
            cantidad, posicion = leer_variable(datos, posicion)
            valor = datos[posicion : posicion + cantidad]
            posicion += cantidad
            if tipo == 81:
                if cantidad != 3:
                    raise ValueError('Tempo MIDI inválido.')
                tempo = int.from_bytes(valor, 'big')
            continue
        if primero in (240, 247):
            cantidad, posicion = leer_variable(datos, posicion)
            posicion += cantidad
            estado = 0
            continue
        if not 128 <= primero < 240:
            raise ValueError('Estado MIDI no reconocido.')
        estado = primero
        cantidad = 1 if primero & 240 in (192, 208) else 2
        valor = datos[posicion : posicion + cantidad]
        if len(valor) != cantidad or any(v >= 128 for v in valor):
            raise ValueError('Datos MIDI incompletos o inválidos.')
        posicion += cantidad
        eventos.append((tiempo / 1000, (primero, *valor)))
    return eventos


# Alinea la cabecera de RS1 y compara los mensajes y sus tiempos con una captura independiente.
def ejecutar(biblioteca, captura):
    raiz = Path(__file__).resolve().parents[3]
    comparador = ComparacionMusicaRS(raiz, biblioteca, True)
    reales = leer_captura(captura)
    inicio = next((i for i, (_, m) in enumerate(reales) if m == (0xC1, 0x24)), None)
    if inicio is None:
        raise ValueError('La captura no contiene el comienzo de CURSE.RS1.')
    reales = reales[inicio:]
    if len(reales) < 128:
        raise ValueError('La captura necesita al menos 128 mensajes desde el inicio de RS1.')
    pulsos, indice, desfase = 0, 0, 0
    for delta, mensaje in paquetes_originales(comparador, 1):
        pulsos += delta
        if not mensaje:
            continue
        datos = (mensaje & 255, mensaje >> 8 & 255)
        if mensaje & 240 not in (192, 208):
            datos += (mensaje >> 16 & 255,)
        instante, recibido = reales[indice]
        if recibido != datos:
            raise ValueError(f'Mensaje {indice} distinto: {recibido} frente a {datos}.')
        diferencia = abs(instante - reales[0][0] - pulsos * 60000 / (140 * 120))
        desfase = max(desfase, diferencia)
        if diferencia > 3:
            raise ValueError(f'Tiempo MIDI distinto en el mensaje {indice}: {diferencia:.3f} ms.')
        indice += 1
        if indice == len(reales):
            break
    informe = {
        'mensajes': indice,
        'desfase_maximo_ms': desfase,
        'tolerancia_ms': 3,
        'captura': str(captura),
        'resultado': 'coinciden mensajes y tiempos',
        'limite': 'No compara timbres, PCM ni efectos; requiere una captura local de DOSBox.',
    }
    salida = raiz / 'apps/juego/trabajo/comparacion_musica_rs/dosbox_midi.json'
    salida.parent.mkdir(parents=True, exist_ok=True)
    salida.write_text(json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
    print(f'DOSBox real: {indice} mensajes MIDI coincidentes; desfase máximo {desfase:.3f} ms.')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Contrasta una captura MIDI real del original.')
    parser.add_argument('--biblioteca', type=Path, required=True)
    parser.add_argument(
        '--captura',
        type=Path,
        default=Path(__file__).resolve().parents[3]
        / 'apps/juego/trabajo/referencia_dosbox/capturas/curse_000.mid',
    )
    argumentos = parser.parse_args()
    try:
        ejecutar(argumentos.biblioteca, argumentos.captura)
    except (OSError, ValueError, ErrorAnalisis) as error:
        parser.exit(1, f'Error: {error}\n')
