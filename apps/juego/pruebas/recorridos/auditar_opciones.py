"""Explora los menús ofrecidos en las instantáneas contrastadas de la aventura."""

import argparse
import ctypes
import hashlib
import json
from pathlib import Path

from comparar_final_cuevas import ComparacionFinalCuevas, ErrorAnalisis
from comparar_objetos import palabra


# Explora las listas reales de un estado, sin ejecutar los paneles de plataforma.
def auditar_estado(comparacion, datos, nombre, consecuencia=None):
    pendientes = []
    comparacion.nativa.procesar_menu(0, (ctypes.c_ubyte * 65536).from_buffer(datos), 152, 92)
    semilla_inicial = list(comparacion.semilla_menu)
    cola = [(datos, (), semilla_inicial)]
    vistos = set()
    while cola:
        base, ruta, semilla = cola.pop()
        clave = bytes(base[0xB60C:0xB63A])
        if clave in vistos or len(ruta) > 6:
            continue
        vistos.add(clave)
        for casilla in range(10):
            if palabra(base, 0xB620 + casilla * 2) == 65535:
                continue
            manejador = palabra(base, 0xB60C + casilla * 2)
            if manejador in (0xE19F, 0xE325, 0xE793, 0xE160):
                continue
            copia = bytearray(base)
            comparacion.semilla_menu[:] = semilla
            comparacion.retomar_recursos(base)
            try:
                comparacion.seleccionar(copia, casilla)
            except ErrorAnalisis as error:
                pendientes.append(
                    {'estado': nombre, 'casillas': ruta + (casilla,), 'error': str(error)}
                )
                comparacion.retomar_recursos(base)
                continue
            termina = bool(comparacion.maquina.reg_read(comparacion.r.UC_X86_REG_EFLAGS) & 1)
            if termina and consecuencia is not None:
                consecuencia(comparacion, copia, nombre, ruta + (casilla,), manejador, base)
            if not termina:
                cola.append((copia, ruta + (casilla,), list(comparacion.semilla_menu)))
    comparacion.semilla_menu[:] = semilla_inicial
    comparacion.retomar_recursos(datos)
    return pendientes


# Recorre verbos, inventarios y destinos por las listas reales; las ventanas de plataforma se prueban aparte.
def auditar(raiz, biblioteca, todas=False):
    comparacion = ComparacionFinalCuevas(raiz, biblioteca, True)
    comparacion.resumen = {}
    comparacion.recursos_menu = True
    # El modelo separado admite también los recursos grandes del cementerio y el desenlace.
    comparacion.referencia_modelo = 0x9000
    carpeta = raiz / 'apps/juego/trabajo/comparacion_final_cuevas'
    instantaneas = sorted(carpeta.glob('*.ds.bin'))
    if todas:
        instantaneas = sorted((raiz / 'apps/juego/trabajo').glob('comparacion*/*.ds.bin'))
    if not instantaneas:
        raise ErrorAnalisis('Faltan referencias: ejecuta pnpm comparar:final-cuevas --muestra.')
    pendientes = []
    estados = 0
    escenas = {}
    vistos = set()
    descartados = 0
    for fichero in instantaneas:
        datos = bytearray(fichero.read_bytes())
        if len(datos) != 65536:
            raise ErrorAnalisis(f'Instantánea incompleta: {fichero}.')
        escena = palabra(datos, 0x8082)
        if escena not in (*range(74), 77) or (not todas and escena > 14) or not datos[0x3595]:
            descartados += 1
            continue
        semilla = fichero.with_name(fichero.name.replace('.ds.bin', '.semilla.json'))
        azar = (
            json.loads(semilla.read_text())
            if semilla.is_file()
            else list(comparacion.semilla_original)
        )
        clave = (hashlib.sha256(datos).digest(), tuple(azar))
        if clave in vistos:
            continue
        vistos.add(clave)
        comparacion.semilla_menu[:] = azar
        estados += 1
        nombre = str(fichero.relative_to(raiz / 'apps/juego/trabajo'))
        comparacion.retomar_recursos(datos)
        errores = auditar_estado(comparacion, datos, nombre)
        pendientes.extend(errores)
        detalle = escenas.setdefault(escena, {'estados': 0, 'pendientes': 0, 'semillas_reales': 0})
        detalle['estados'] += 1
        detalle['pendientes'] += len(errores)
        detalle['semillas_reales'] += int(semilla.is_file())
        if todas:
            print(f'Escena {escena}: {nombre}, {len(errores)} diferencias.', flush=True)
    informe = {
        'resultado': 'pendientes' if pendientes else 'coinciden',
        'estados': estados,
        'selecciones': comparacion.resumen.get('selecciones', 0),
        'pendientes': pendientes,
        'escenas': escenas,
        'escenas_sin_estado_auditable': sorted(set((*range(74), 77)) - set(escenas))
        if todas
        else [],
        'descartados_sin_menu_o_fuera_del_catalogo': descartados,
        'limite': 'Opciones ofrecidas en estas posiciones y estados; no es una exploración exhaustiva de todos los estados posibles.',
    }
    destino = raiz / 'apps/juego/trabajo/auditoria_opciones'
    destino.mkdir(exist_ok=True)
    (destino / 'resultado.json').write_text(
        json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8'
    )
    if pendientes:
        raise ErrorAnalisis(
            f'Quedan {len(pendientes)} selecciones distintas; consulta apps/juego/trabajo/auditoria_opciones/resultado.json.'
        )
    print(
        f'Opciones: {informe["selecciones"]} selecciones coincidentes en {estados} estados; sin manejadores pendientes en las listas recorridas.'
    )


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Audita las opciones reales de las instantáneas disponibles.'
    )
    parser.add_argument('--biblioteca', type=Path, required=True)
    parser.add_argument('--muestra', action='store_true')
    parser.add_argument(
        '--todas',
        action='store_true',
        help='Recorre todas las carpetas de comparación, sin limitar escenas.',
    )
    opciones = parser.parse_args()
    try:
        auditar(Path(__file__).resolve().parents[4], opciones.biblioteca, opciones.todas)
    except (OSError, ErrorAnalisis) as error:
        parser.exit(1, f'Error: {error}\n')
