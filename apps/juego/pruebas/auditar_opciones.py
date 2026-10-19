"""Explora los menús disponibles de las instantáneas contrastadas de las primeras quince escenas."""

import argparse
import ctypes
import json
from pathlib import Path

from comparar_final_cuevas import ComparacionFinalCuevas, ErrorAnalisis
from comparar_objetos import palabra


# Explora las listas reales de un estado, sin ejecutar los paneles de plataforma.
def auditar_estado(comparacion, datos, nombre):
    pendientes = []
    comparacion.nativa.procesar_menu(0, (ctypes.c_ubyte * 65536).from_buffer(datos), 152, 92)
    cola = [(datos, ())]
    vistos = set()
    while cola:
        base, ruta = cola.pop()
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
            try:
                comparacion.seleccionar(copia, casilla)
            except ErrorAnalisis as error:
                pendientes.append(
                    {'estado': nombre, 'casillas': ruta + (casilla,), 'error': str(error)}
                )
                comparacion.retomar_recursos(base)
                continue
            if not (comparacion.maquina.reg_read(comparacion.r.UC_X86_REG_EFLAGS) & 1):
                cola.append((copia, ruta + (casilla,)))
    return pendientes


# Recorre verbos, inventarios y destinos por las listas reales; las ventanas de plataforma se prueban aparte.
def auditar(raiz, biblioteca):
    comparacion = ComparacionFinalCuevas(raiz, biblioteca, True)
    comparacion.resumen = {}
    comparacion.recursos_menu = True
    carpeta = raiz / 'apps/juego/trabajo/comparacion_final_cuevas'
    instantaneas = sorted(carpeta.glob('*.ds.bin'))
    if not instantaneas:
        raise ErrorAnalisis('Faltan referencias: ejecuta pnpm comparar:final-cuevas --muestra.')
    pendientes = []
    estados = 0
    for fichero in instantaneas:
        datos = bytearray(fichero.read_bytes())
        if palabra(datos, 0x8082) > 14 or not datos[0x3595]:
            continue
        estados += 1
        comparacion.retomar_recursos(datos)
        pendientes.extend(auditar_estado(comparacion, datos, fichero.name))
    informe = {
        'resultado': 'pendientes' if pendientes else 'coinciden',
        'estados': estados,
        'selecciones': comparacion.resumen.get('selecciones', 0),
        'pendientes': pendientes,
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
    parser = argparse.ArgumentParser(description='Audita las opciones reales de quince escenas.')
    parser.add_argument('--biblioteca', type=Path, required=True)
    parser.add_argument('--muestra', action='store_true')
    opciones = parser.parse_args()
    try:
        auditar(Path(__file__).resolve().parents[3], opciones.biblioteca)
    except (OSError, ErrorAnalisis) as error:
        parser.exit(1, f'Error: {error}\n')
