"""Contrasta la continuación de las opciones ofrecidas en los primeros capítulos."""

import argparse
import ctypes
import hashlib
import json
from pathlib import Path

from auditar_opciones import auditar_estado
from comparar_final_cuevas import ComparacionFinalCuevas, ErrorAnalisis
from comparar_objetos import palabra


# No cambia objetos ni posiciones: cada rama parte del menú de una instantánea real y su semilla.
def auditar(raiz, biblioteca, estado=None, escenas=None, incluir_clc=False, solo_clc=False):
    incluir_clc = incluir_clc or solo_clc
    solicitadas = escenas or (1, *range(3, 15))
    comparacion = ComparacionFinalCuevas(raiz, biblioteca, True)
    comparacion.recursos_menu = True
    comparacion.resumen = {}
    vistos, acciones = set(), set()
    informe = {
        'resultado': 'en curso',
        'estados': 0,
        'ramas': [],
        'observacion_minima': 128,
        'escenas_solicitadas': solicitadas,
        'incluye_clc': incluir_clc,
        'incluye_stc': not solo_clc,
        'limite': 'Selecciones de los tipos indicados por incluye_stc/incluye_clc; los verbos CLC se observan tras cerrar con clic derecho; referencias con semilla, al menos128 pasos salvo salida y hasta512 sin reposo. No carga destinos, no observa el menú abierto ni agota todos los estados.',
    }
    carpeta = raiz / 'apps/juego/trabajo/auditoria_consecuencias'
    carpeta.mkdir(exist_ok=True)
    sufijo = ('_' + '_'.join(map(str, escenas))) if escenas else ''
    sufijo += '_filtrado' if estado else ''
    sufijo += '_solo_clc' if solo_clc else '_clc' if incluir_clc else ''
    destino_informe = carpeta / f'resultado{sufijo}.json'

    # Compara cada paso contra el EXE y conserva las ramas que no terminan en el plazo observado.
    def continuar(c, base, nombre, ruta, manejador, antes, cierre='STC'):
        clave = (hashlib.sha256(base).digest(), tuple(c.semilla_menu), cierre)
        if clave in acciones:
            return
        acciones.add(clave)
        d = bytearray(base)
        semilla = (ctypes.c_ushort * 4)(*c.semilla_menu)
        rama = {
            'estado': nombre,
            'escena': palabra(base, 0x8082),
            'casillas': ruta,
            'manejador': f'{manejador:04X}',
            'cierre_menu': cierre,
        }
        # Una respuesta del cursor no sustituye el canal de caminar previo de Brad.
        # Esta clasificación nunca excluye el cursor de las comparaciones contra el EXE.
        sin_secuencia_nueva = (
            base[:0x2C0A] == antes[:0x2C0A]
            and base[0x2C2B:0xB600] == antes[0x2C2B:0xB600]
            and base[0xB640:] == antes[0xB640:]
        )
        informe['ramas'].append(rama)
        for paso in range(1, 513):
            salida = c.andar(d, semilla, 0xFFFF, 1)
            rama['pasos'] = paso
            if salida is not None:
                rama.update(resultado='petición de salida coincidente', salida=salida)
                break
            # El cubo y las ofertas del pozo admiten acciones con el movimiento bloqueado.
            if paso < informe['observacion_minima']:
                continue
            if d[0x3595] and palabra(d, 0x359A) == 0 and palabra(d, 0x2C3B) == 0:
                rama['resultado'] = 'reposo con menú habilitado'
                break
            if sin_secuencia_nueva and palabra(base, 0x2C3B) != 0:
                rama['resultado'] = 'sin secuencia nueva; canal previo observado'
                break
        else:
            rama['resultado'] = 'plazo agotado; requiere revisión'
        c.retomar_recursos(base)

    # Cerrar con clic derecho solo cambia menu_visible en SDL, sin tocar DS ni el azar.
    # Observa los verbos CLC después de ese cierre, no los simples cambios de submenú.
    def continuar_clc(c, base, nombre, ruta, manejador, antes):
        if manejador in (0x32DE, 0x563A, 0x2FCA, 0x3143, 0x384A, 0x3B53):
            continuar(c, base, nombre, ruta, manejador, antes, 'CLC seguido de clic derecho')

    try:
        for fichero in sorted((raiz / 'apps/juego/trabajo').glob('comparacion*/*.ds.bin')):
            if estado and fichero.resolve() != (raiz / 'apps/juego/trabajo' / estado).resolve():
                continue
            d = bytearray(fichero.read_bytes())
            if len(d) != 65536:
                raise ErrorAnalisis(f'Instantánea incompleta: {fichero}.')
            escena = palabra(d, 0x8082)
            semilla = fichero.with_name(fichero.name.replace('.ds.bin', '.semilla.json'))
            if escena not in solicitadas or not d[0x3595] or not semilla.is_file():
                continue
            azar = json.loads(semilla.read_text(encoding='utf-8'))
            clave = (hashlib.sha256(d).digest(), tuple(azar))
            if clave in vistos:
                continue
            vistos.add(clave)
            comparacion.semilla_menu[:] = azar
            comparacion.retomar_recursos(d)
            nombre = str(fichero.relative_to(raiz / 'apps/juego/trabajo'))
            informe['estados'] += 1
            errores = auditar_estado(
                comparacion,
                d,
                nombre,
                None if solo_clc else continuar,
                continuar_clc if incluir_clc else None,
            )
            if errores:
                raise ErrorAnalisis(f'Despacho distinto en {nombre}: {errores[:3]}')
            print(f'Escena {escena}: {nombre}; {len(acciones)} ramas acumuladas.', flush=True)
        if not vistos:
            raise ErrorAnalisis(
                'Faltan instantáneas con semilla y menú de las escenas solicitadas.'
            )
        informe['resultado'] = (
            'requiere revisión'
            if any(r['resultado'].startswith('plazo agotado') for r in informe['ramas'])
            else 'coinciden'
        )
    except ErrorAnalisis as error:
        informe['resultado'] = 'diferencia'
        informe['error'] = str(error)
        raise
    finally:
        informe['selecciones'] = comparacion.resumen.get('selecciones', 0)
        informe['pasos_contrastados'] = comparacion.resumen.get('pasos_jugables', 0)
        destino_informe.write_text(
            json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8'
        )
    print(
        f'Consecuencias: {len(acciones)} ramas, {informe["pasos_contrastados"]} pasos; {informe["resultado"]}.'
    )
    if informe['resultado'] != 'coinciden':
        raise ErrorAnalisis(f'Hay ramas cuyo final requiere revisión; consultar {destino_informe}.')


# Limita esta primera extensión a capítulos sin el byte CS mutable del probador25.
def leer_escenas(texto):
    try:
        escenas = tuple(sorted(set(int(valor) for valor in texto.split(','))))
    except ValueError as error:
        raise argparse.ArgumentTypeError('Indica índices separados por comas.') from error
    if not escenas or any(i not in range(25) or i == 2 for i in escenas):
        raise argparse.ArgumentTypeError('Ámbito admitido:0–24, salvo la secuencia automática2.')
    return escenas


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Contrasta las consecuencias de menús en los primeros capítulos.'
    )
    parser.add_argument('--biblioteca', type=Path, required=True)
    grupo_clc = parser.add_mutually_exclusive_group()
    grupo_clc.add_argument(
        '--incluir-clc',
        action='store_true',
        help='Observa también verbos CLC tras cerrar el menú con clic derecho.',
    )
    grupo_clc.add_argument(
        '--solo-clc',
        action='store_true',
        help='Observa solo verbos CLC para no repetir las animaciones STC ya contrastadas.',
    )
    parser.add_argument(
        '--escenas', type=leer_escenas, help='Índices0–24 separados por comas; por defecto1,3–14.'
    )
    parser.add_argument(
        '--estado', help='Referencia relativa a trabajo/ para repetir un caso sin rehacer la tanda.'
    )
    opciones = parser.parse_args()
    try:
        auditar(
            Path(__file__).resolve().parents[4],
            opciones.biblioteca,
            opciones.estado,
            opciones.escenas,
            opciones.incluir_clc,
            opciones.solo_clc,
        )
    except (OSError, ErrorAnalisis) as error:
        parser.exit(1, f'Error: {error}\n')
