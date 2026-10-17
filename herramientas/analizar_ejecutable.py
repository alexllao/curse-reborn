"""Inspección MZ y desensamblado por flujo; no ejecuta el programa analizado."""

import argparse
from collections import deque
import hashlib
import json
from pathlib import Path
import struct


class ErrorAnalisis(ValueError):
    """El ejecutable no tiene una estructura válida para este análisis."""


def leer_mz(contenido):
    if len(contenido) < 28 or contenido[:2] != b'MZ':
        raise ErrorAnalisis('Se necesita una cabecera DOS MZ de al menos 28 bytes.')
    campos = struct.unpack_from('<14H', contenido)
    _, ultimo, paginas, cantidad, parrafos, minimo, maximo, ss, sp, _, ip, cs, tabla, superposicion = campos
    if not paginas or ultimo > 511:
        raise ErrorAnalisis('El tamaño declarado en la cabecera no es válido.')
    tamano = (paginas - 1) * 512 + (ultimo or 512)
    cabecera = parrafos * 16
    if not 28 <= cabecera <= tamano <= len(contenido):
        raise ErrorAnalisis('La imagen está truncada o la cabecera tiene un tamaño inválido.')
    if tabla < 28 or tabla + cantidad * 4 > cabecera:
        raise ErrorAnalisis('La tabla de reubicaciones queda fuera de la cabecera.')
    imagen = contenido[cabecera:tamano]
    if cs * 16 + ip >= len(imagen):
        raise ErrorAnalisis('El punto de entrada queda fuera de la imagen del archivo.')
    reubicaciones = []
    for indice in range(cantidad):
        desplazamiento, segmento = struct.unpack_from('<HH', contenido, tabla + indice * 4)
        posicion = segmento * 16 + desplazamiento
        if posicion + 2 > len(imagen):
            raise ErrorAnalisis('Una reubicación queda fuera de la imagen del archivo.')
        reubicaciones.append({
            'segmento': segmento, 'desplazamiento': desplazamiento,
            'posicion_archivo': cabecera + posicion,
            'valor_sin_reubicar': struct.unpack_from('<H', imagen, posicion)[0],
        })
    return {
        'sha256': hashlib.sha256(contenido).hexdigest(),
        'bytes_archivo': len(contenido), 'bytes_declarados': tamano,
        'bytes_cabecera': cabecera, 'bytes_imagen': len(imagen),
        'bytes_adicionales': len(contenido) - tamano,
        'entrada_cs': cs, 'entrada_ip': ip, 'pila_ss': ss, 'pila_sp': sp,
        'memoria_minima_parrafos': minimo, 'memoria_maxima_parrafos': maximo,
        'superposicion': superposicion, 'reubicaciones': reubicaciones,
    }, imagen


def direccion(texto):
    try:
        segmento, desplazamiento = (int(parte, 16) for parte in texto.split(':'))
        if not (0 <= segmento <= 65535 and 0 <= desplazamiento <= 65535):
            raise ValueError()
        return segmento, desplazamiento
    except ValueError as error:
        raise argparse.ArgumentTypeError('Usa segmento:desplazamiento hexadecimal, por ejemplo 0000:0238.') from error


def etiqueta(segmento, desplazamiento):
    return f'{segmento:04X}:{desplazamiento:04X}'


def seguir_flujo(imagen, raices, limite=100000):
    try:
        import capstone
        from capstone.x86 import X86_OP_IMM
    except ImportError as error:
        raise ErrorAnalisis('Falta Capstone. Instala herramientas/requisitos.txt.') from error
    motor = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_16)
    motor.detail = True
    pendientes = deque(raices)
    instrucciones = {}
    ocupados = {}
    enlaces = []
    incidencias = []
    destinos_llamada = set()
    while pendientes:
        cs, ip = pendientes.popleft()
        while (cs, ip) not in instrucciones:
            origen = etiqueta(cs, ip)
            posicion = cs * 16 + ip
            if not 0 <= posicion < len(imagen):
                incidencias.append({'origen': origen, 'motivo': 'Destino fuera de la imagen almacenada.'})
                break
            if posicion in ocupados:
                incidencias.append({'origen': origen, 'motivo': 'Destino dentro de otra instrucción o alias de segmento; revisar manualmente.'})
                break
            if len(instrucciones) >= limite:
                raise ErrorAnalisis('Se ha alcanzado el límite de instrucciones; no se genera un informe parcial.')
            bloque = imagen[posicion:min(posicion + 15, len(imagen), cs * 16 + 65536)]
            instruccion = next(motor.disasm(bloque, ip, count=1), None)
            if instruccion is None:
                incidencias.append({'origen': origen, 'motivo': 'Instrucción no decodificable.'})
                break
            if any(p in ocupados for p in range(posicion, posicion + instruccion.size)):
                incidencias.append({'origen': origen, 'motivo': 'Instrucciones solapadas; revisar manualmente.'})
                break
            llamada = instruccion.group(capstone.CS_GRP_CALL)
            salto = (instruccion.group(capstone.CS_GRP_JUMP) or
                     instruccion.mnemonic in ('loop', 'loope', 'loopne'))
            retorno = instruccion.group(capstone.CS_GRP_RET) or instruccion.group(capstone.CS_GRP_IRET)
            registro = {
                'segmento': cs, 'desplazamiento': ip, 'posicion_imagen': posicion,
                'tamano': instruccion.size, 'bytes': instruccion.bytes.hex(' '),
                'operacion': instruccion.mnemonic, 'operandos': instruccion.op_str,
            }
            instrucciones[cs, ip] = registro
            for p in range(posicion, posicion + instruccion.size):
                ocupados[p] = origen
            siguiente = (ip + instruccion.size) & 65535
            if llamada or salto:
                operandos = instruccion.operands
                destino = None
                if len(operandos) == 1 and operandos[0].type == X86_OP_IMM:
                    # El destino de los saltos relativos se envuelve a IP de 16 bits.
                    destino = cs, operandos[0].imm & 65535
                elif len(operandos) == 2 and all(o.type == X86_OP_IMM for o in operandos):
                    destino = operandos[0].imm & 65535, operandos[1].imm & 65535
                enlaces.append({'origen': origen, 'tipo': 'llamada' if llamada else 'salto',
                                'destino': etiqueta(*destino) if destino else None})
                if destino:
                    registro['destino'] = etiqueta(*destino)
                    pendientes.append(destino)
                    if llamada:
                        destinos_llamada.add(destino)
                else:
                    incidencias.append({'origen': origen, 'motivo': 'Destino indirecto: requiere registros o memoria en ejecución.'})
                if salto and instruccion.mnemonic in ('jmp', 'ljmp'):
                    break
            # No se sigue a través de retornos ni de una salida DOS inequívoca.
            anterior = instrucciones.get((cs, ip - 3))
            anterior_corto = instrucciones.get((cs, ip - 2))
            salida_dos = (instruccion.mnemonic == 'int' and instruccion.op_str == '0x21' and (
                (anterior is not None and anterior['bytes'] == 'b8 00 4c') or
                (anterior_corto is not None and anterior_corto['bytes'] == 'b4 4c')))
            if retorno or salida_dos or instruccion.mnemonic in ('hlt', 'ud2'):
                break
            ip = siguiente
    return {
        'version_capstone': capstone.__version__,
        'raices': [etiqueta(*r) for r in raices],
        'instrucciones': sorted(instrucciones.values(), key=lambda r: (r['segmento'], r['desplazamiento'])),
        'destinos_llamada': sorted(etiqueta(*d) for d in destinos_llamada),
        'enlaces': enlaces, 'incidencias': incidencias,
    }


def analizar(ruta, salida, raices_adicionales=()):
    metadatos, imagen = leer_mz(ruta.read_bytes())
    raices = [(metadatos['entrada_cs'], metadatos['entrada_ip']), *raices_adicionales]
    flujo = seguir_flujo(imagen, list(dict.fromkeys(raices)))
    informe = {'cabecera': metadatos, 'flujo': flujo}
    salida.mkdir(parents=True, exist_ok=True)
    (salida / 'analisis.json').write_text(json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
    lineas = [
        '; Desensamblado estático provisional. Los destinos indirectos requieren revisión.',
        '; Direcciones relativas a la imagen MZ sin reubicar; no son direcciones de ejecución DOS.',
        '; No implica cobertura completa ni verificación del comportamiento.',
    ]
    for r in flujo['instrucciones']:
        archivo = metadatos['bytes_cabecera'] + r['posicion_imagen']
        destino = ' ; destino ' + r['destino'] if 'destino' in r else ''
        lineas.append(f"{etiqueta(r['segmento'], r['desplazamiento'])} archivo={archivo:06X} "
                      f"{r['bytes']:<44} {r['operacion']:<9} {r['operandos']}{destino}")
    (salida / 'desensamblado.asm').write_text('\n'.join(lineas) + '\n', encoding='utf-8')
    print(f"Ejecutable: {ruta}")
    print(f"SHA-256: {metadatos['sha256']}")
    print(f"Instrucciones alcanzadas: {len(flujo['instrucciones'])}")
    print(f"Destinos de llamada directa: {len(flujo['destinos_llamada'])}")
    print(f"Incidencias pendientes de revisión: {len(flujo['incidencias'])}")
    print(f'Resultados locales: {salida}')
    return informe


def principal():
    raiz = Path(__file__).resolve().parents[1]
    argumentos = argparse.ArgumentParser(description='Analiza un ejecutable DOS MZ siguiendo llamadas y saltos directos.')
    argumentos.add_argument('--ejecutable', type=Path, default=raiz / 'datos_originales' / 'CURSE.EXE')
    argumentos.add_argument('--salida', type=Path, default=raiz / 'trabajo' / 'analisis_ejecutable')
    argumentos.add_argument('--raiz', type=direccion, action='append', default=[], help='Entrada adicional confirmada, en hexadecimal.')
    opciones = argumentos.parse_args()
    if opciones.salida.resolve().is_relative_to((raiz / 'datos_originales').resolve()):
        argumentos.error('La salida no puede escribirse sobre la carpeta de originales.')
    try:
        analizar(opciones.ejecutable, opciones.salida, opciones.raiz)
    except (OSError, ErrorAnalisis) as error:
        argumentos.exit(1, f'Error: {error}\n')


if __name__ == '__main__':
    principal()
