"""Entrada común de las pruebas agrupadas; mantiene sus importaciones y las órdenes pnpm."""

import importlib
from pathlib import Path
import runpy
import sys


# Comparadores de distintas áreas comparten el mismo puente y los contratos originales.
def ejecutar():
    sys.dont_write_bytecode = True
    carpeta = Path(__file__).resolve().parent
    grupos = [carpeta / nombre for nombre in ('motor', 'video', 'audio', 'recorridos')]
    sys.path[:0] = [str(carpeta.parent / 'herramientas'), *(str(grupo) for grupo in grupos)]
    if len(sys.argv) < 2:
        raise SystemExit('Indica un comparador o --comprobar-importaciones; usa las órdenes pnpm.')
    nombre = sys.argv[1]
    if nombre == '--comprobar-importaciones':
        modulos = sorted(archivo.stem for grupo in grupos for archivo in grupo.glob('*.py'))
        if len(modulos) != len(set(modulos)):
            raise SystemExit('Hay módulos de prueba con nombres duplicados.')
        for modulo in modulos:
            importlib.import_module(modulo)
        print(f'Pruebas: {len(modulos)} módulos de cuatro áreas importados correctamente.')
        return
    candidatos = [grupo / nombre for grupo in grupos if (grupo / nombre).is_file()]
    if Path(nombre).name != nombre or not nombre.endswith('.py') or len(candidatos) != 1:
        raise SystemExit(f'Comparador desconocido o ambiguo: {nombre}.')
    sys.argv = [str(candidatos[0]), *sys.argv[2:]]
    runpy.run_path(str(candidatos[0]), run_name='__main__')


if __name__ == '__main__':
    ejecutar()
