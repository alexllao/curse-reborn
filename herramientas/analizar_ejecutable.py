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
    (
        _,
        ultimo,
        paginas,
        cantidad,
        parrafos,
        minimo,
        maximo,
        ss,
        sp,
        _,
        ip,
        cs,
        tabla,
        superposicion,
    ) = campos
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
        reubicaciones.append(
            {
                'segmento': segmento,
                'desplazamiento': desplazamiento,
                'posicion_archivo': cabecera + posicion,
                'valor_sin_reubicar': struct.unpack_from('<H', imagen, posicion)[0],
            }
        )
    return {
        'sha256': hashlib.sha256(contenido).hexdigest(),
        'bytes_archivo': len(contenido),
        'bytes_declarados': tamano,
        'bytes_cabecera': cabecera,
        'bytes_imagen': len(imagen),
        'bytes_adicionales': len(contenido) - tamano,
        'entrada_cs': cs,
        'entrada_ip': ip,
        'pila_ss': ss,
        'pila_sp': sp,
        'memoria_minima_parrafos': minimo,
        'memoria_maxima_parrafos': maximo,
        'superposicion': superposicion,
        'reubicaciones': reubicaciones,
    }, imagen


def direccion(texto):
    try:
        segmento, desplazamiento = (int(parte, 16) for parte in texto.split(':'))
        if not (0 <= segmento <= 65535 and 0 <= desplazamiento <= 65535):
            raise ValueError()
        return segmento, desplazamiento
    except ValueError as error:
        raise argparse.ArgumentTypeError(
            'Usa segmento:desplazamiento hexadecimal, por ejemplo 0000:0238.'
        ) from error


def etiqueta(segmento, desplazamiento):
    return f'{segmento:04X}:{desplazamiento:04X}'


def leer_mapa(mapa, metadatos, imagen):
    """Aplica anotaciones revisadas únicamente a la copia exacta del ejecutable."""
    if mapa.get('sha256') != metadatos['sha256']:
        raise ErrorAnalisis('El mapa de código no corresponde al SHA-256 de este ejecutable.')
    raices = [direccion(texto) for texto in mapa.get('raices', [])]
    tablas = []
    resoluciones = {}
    for tabla in mapa.get('tablas', []):
        origen = direccion(tabla['origen'])
        segmento, desplazamiento = direccion(tabla['ubicacion'])
        cantidad = tabla['cantidad']
        cs_destino = tabla['segmento_destino']
        if (
            type(cantidad) is not int
            or cantidad < 1
            or desplazamiento + cantidad * 2 > 65536
            or type(cs_destino) is not int
            or not 0 <= cs_destino <= 65535
        ):
            raise ErrorAnalisis('El tamaño o segmento de una tabla de saltos no es válido.')
        posicion = segmento * 16 + desplazamiento
        if posicion + cantidad * 2 > len(imagen):
            raise ErrorAnalisis('Una tabla de saltos queda fuera de la imagen.')
        if origen in resoluciones:
            raise ErrorAnalisis('Hay dos tablas para la misma transferencia indirecta.')
        entradas = []
        for indice in range(cantidad):
            ip = struct.unpack_from('<H', imagen, posicion + indice * 2)[0]
            if ip == 0 and tabla.get('omitir_cero', False):
                entradas.append({'indice': indice, 'destino': None})
                continue
            if cs_destino * 16 + ip >= len(imagen):
                raise ErrorAnalisis('Un destino de tabla queda fuera de la imagen.')
            entradas.append(
                {
                    'indice': indice,
                    'destino': etiqueta(cs_destino, ip),
                    'tabla_dinamica': tabla.get('derivada_en_memoria', False),
                }
            )
        for adicional in tabla.get('destinos_adicionales', []):
            cs, ip = direccion(adicional['destino'])
            if cs * 16 + ip >= len(imagen):
                raise ErrorAnalisis('Un destino adicional queda fuera de la imagen.')
            entradas.append(
                {
                    'indice': None,
                    'destino': etiqueta(cs, ip),
                    'tabla_dinamica': True,
                    'evidencia': adicional['evidencia'],
                }
            )
        resoluciones[origen] = entradas
        tablas.append({**tabla, 'entradas': entradas})
    return raices, tablas, resoluciones


def seguir_flujo(imagen, raices, limite=100000, resoluciones=None):
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
    resoluciones = resoluciones or {}
    while pendientes:
        cs, ip = pendientes.popleft()
        while (cs, ip) not in instrucciones:
            origen = etiqueta(cs, ip)
            posicion = cs * 16 + ip
            if not 0 <= posicion < len(imagen):
                incidencias.append(
                    {'origen': origen, 'motivo': 'Destino fuera de la imagen almacenada.'}
                )
                break
            if posicion in ocupados:
                incidencias.append(
                    {
                        'origen': origen,
                        'motivo': 'Destino dentro de otra instrucción o alias de segmento; revisar manualmente.',
                    }
                )
                break
            if len(instrucciones) >= limite:
                raise ErrorAnalisis(
                    'Se ha alcanzado el límite de instrucciones; no se genera un informe parcial.'
                )
            bloque = imagen[posicion : min(posicion + 15, len(imagen), cs * 16 + 65536)]
            instruccion = next(motor.disasm(bloque, ip, count=1), None)
            if instruccion is None:
                incidencias.append({'origen': origen, 'motivo': 'Instrucción no decodificable.'})
                break
            if any(p in ocupados for p in range(posicion, posicion + instruccion.size)):
                incidencias.append(
                    {'origen': origen, 'motivo': 'Instrucciones solapadas; revisar manualmente.'}
                )
                break
            llamada = instruccion.group(capstone.CS_GRP_CALL)
            salto = instruccion.group(capstone.CS_GRP_JUMP) or instruccion.mnemonic in (
                'loop',
                'loope',
                'loopne',
            )
            retorno = instruccion.group(capstone.CS_GRP_RET) or instruccion.group(
                capstone.CS_GRP_IRET
            )
            registro = {
                'segmento': cs,
                'desplazamiento': ip,
                'posicion_imagen': posicion,
                'tamano': instruccion.size,
                'bytes': instruccion.bytes.hex(' '),
                # Capstone 5 etiqueta estos opcodes sin prefijo como CWDE/CDQ incluso
                # en este listado de 16 bits. Conservamos la nomenclatura CBW/CWD.
                'operacion': {b'\x98': 'cbw', b'\x99': 'cwd'}.get(
                    bytes(instruccion.bytes), instruccion.mnemonic
                ),
                'operandos': instruccion.op_str,
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
                enlaces.append(
                    {
                        'origen': origen,
                        'tipo': 'llamada' if llamada else 'salto',
                        'destino': etiqueta(*destino) if destino else None,
                    }
                )
                if destino:
                    registro['destino'] = etiqueta(*destino)
                    pendientes.append(destino)
                    if llamada:
                        destinos_llamada.add(destino)
                elif (cs, ip) in resoluciones:
                    registro['tabla_anotada'] = True
                    for entrada in resoluciones[cs, ip]:
                        if entrada['destino'] is not None:
                            pendientes.append(direccion(entrada['destino']))
                            dinamica = entrada.get('tabla_dinamica', False)
                            enlaces.append(
                                {
                                    'origen': origen,
                                    'tipo': 'candidato_de_tabla_dinamica'
                                    if dinamica
                                    else 'destino_por_tabla_anotada',
                                    'indice_fuente' if dinamica else 'indice': entrada['indice'],
                                    'destino': entrada['destino'],
                                }
                            )
                else:
                    incidencias.append(
                        {
                            'origen': origen,
                            'motivo': 'Destino indirecto: requiere registros o memoria en ejecución.',
                        }
                    )
                if salto and instruccion.mnemonic in ('jmp', 'ljmp'):
                    break
            # No se sigue a través de retornos ni de una salida DOS inequívoca.
            anterior = instrucciones.get((cs, ip - 3))
            anterior_corto = instrucciones.get((cs, ip - 2))
            salida_dos = (
                instruccion.mnemonic == 'int'
                and instruccion.op_str == '0x21'
                and (
                    (anterior is not None and anterior['bytes'] == 'b8 00 4c')
                    or (anterior_corto is not None and anterior_corto['bytes'] == 'b4 4c')
                )
            )
            if retorno or salida_dos or instruccion.mnemonic in ('hlt', 'ud2'):
                break
            ip = siguiente
    return {
        'version_capstone': capstone.__version__,
        'raices': [etiqueta(*r) for r in raices],
        'instrucciones': sorted(
            instrucciones.values(), key=lambda r: (r['segmento'], r['desplazamiento'])
        ),
        'destinos_llamada': sorted(etiqueta(*d) for d in destinos_llamada),
        'enlaces': enlaces,
        'incidencias': incidencias,
    }


def crear_listado(metadatos, flujo, anotaciones=()):
    comentarios = {}
    for anotacion in anotaciones:
        clave = direccion(anotacion['direccion'])
        if clave in comentarios:
            raise ErrorAnalisis('Hay dos anotaciones para la misma dirección.')
        comentarios[clave] = anotacion
    lineas = [
        '; Listado reconstruido del ejecutable, con anotaciones propias en castellano.',
        '; Conserva direcciones y bytes como referencia para el traslado a C/C++.',
        '; No es el fuente original ni un archivo listo para ensamblar.',
        '; Direcciones relativas a la imagen MZ sin reubicar; no son direcciones de ejecución DOS.',
        '; El recorrido estático no demuestra cobertura completa ni ejecución del juego.',
    ]
    for r in flujo['instrucciones']:
        anotacion = comentarios.pop((r['segmento'], r['desplazamiento']), None)
        if anotacion:
            lineas.extend(['', '; ' + anotacion['nombre']])
            lineas.extend('; ' + parte for parte in anotacion['comentario'].splitlines())
            if anotacion.get('traslado'):
                lineas.append('; Traslado C++: ' + anotacion['traslado'])
        archivo = metadatos['bytes_cabecera'] + r['posicion_imagen']
        destino = ' ; destino ' + r['destino'] if 'destino' in r else ''
        if r.get('tabla_anotada'):
            destino += ' ; candidatos según tabla anotada en analisis.json'
        lineas.append(
            f'{etiqueta(r["segmento"], r["desplazamiento"])} archivo={archivo:06X} '
            f'{r["bytes"]:<44} {r["operacion"]:<9} {r["operandos"]}{destino}'
        )
    for clave, anotacion in comentarios.items():
        lineas.append(
            f'; Anotación no alcanzada en esta pasada: {etiqueta(*clave)} — {anotacion["nombre"]}'
        )
    return '\n'.join(lineas) + '\n'


def analizar(ruta, salida, raices_adicionales=(), mapa=None):
    metadatos, imagen = leer_mz(ruta.read_bytes())
    raices = [(metadatos['entrada_cs'], metadatos['entrada_ip']), *raices_adicionales]
    tablas, resoluciones = [], {}
    if mapa is not None:
        entradas, tablas, resoluciones = leer_mapa(mapa, metadatos, imagen)
        raices.extend(entradas)
    flujo = seguir_flujo(imagen, list(dict.fromkeys(raices)), resoluciones=resoluciones)
    anotaciones = mapa.get('anotaciones', []) if mapa else []
    listado = crear_listado(metadatos, flujo, anotaciones)
    informe = {
        'cabecera': metadatos,
        'flujo': flujo,
        'tablas_anotadas': tablas,
        'anotaciones': anotaciones,
    }
    salida.mkdir(parents=True, exist_ok=True)
    (salida / 'analisis.json').write_text(
        json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8'
    )
    (salida / 'desensamblado.asm').write_text(listado, encoding='utf-8')
    print(f'Ejecutable: {ruta}')
    print(f'SHA-256: {metadatos["sha256"]}')
    print(f'Instrucciones alcanzadas: {len(flujo["instrucciones"])}')
    print(f'Destinos de llamada directa: {len(flujo["destinos_llamada"])}')
    print(f'Tablas y fuentes de destinos anotadas: {len(tablas)}')
    print(f'Comentarios de código conservados: {len(anotaciones)}')
    print(f'Incidencias pendientes de revisión: {len(flujo["incidencias"])}')
    print(f'Resultados locales: {salida}')
    return informe


def principal():
    raiz = Path(__file__).resolve().parents[1]
    argumentos = argparse.ArgumentParser(
        description='Analiza un ejecutable DOS MZ siguiendo llamadas y saltos directos.'
    )
    argumentos.add_argument(
        '--ejecutable', type=Path, default=raiz / 'datos_originales' / 'CURSE' / 'CURSE.EXE'
    )
    argumentos.add_argument('--salida', type=Path, default=raiz / 'trabajo' / 'analisis_ejecutable')
    argumentos.add_argument(
        '--raiz',
        type=direccion,
        action='append',
        default=[],
        help='Entrada adicional confirmada, en hexadecimal.',
    )
    argumentos.add_argument(
        '--mapa',
        type=Path,
        help='Mapa revisado de raíces y tablas, vinculado al SHA-256 del ejecutable.',
    )
    opciones = argumentos.parse_args()
    if opciones.salida.resolve().is_relative_to((raiz / 'datos_originales').resolve()):
        argumentos.error('La salida no puede escribirse sobre la carpeta de originales.')
    try:
        mapa = json.loads(opciones.mapa.read_text(encoding='utf-8')) if opciones.mapa else None
        analizar(opciones.ejecutable, opciones.salida, opciones.raiz, mapa)
    except (OSError, ValueError, KeyError, TypeError, argparse.ArgumentTypeError) as error:
        argumentos.exit(1, f'Error: {error}\n')


if __name__ == '__main__':
    principal()
