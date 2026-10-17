"""Prolonga la partida real de mazmorra y corredor hasta las primeras acciones de LAKEBED."""

import argparse
import json
from pathlib import Path

from comparar_corredor import ComparacionCorredor, ErrorAnalisis
from comparar_objetos import guardar, palabra


class ComparacionLago(ComparacionCorredor):
    # Mantiene los mismos bancos y contratos que el corredor y separa los resultados del lago.
    def __init__(self, raiz, biblioteca, muestra):
        super().__init__(raiz, biblioteca, muestra)
        self.carpeta_resultados = 'comparacion_lago'

    # Continúa la entrada submarina real, con pecera y agua obtenidas en el corredor.
    def despues_corredor(self, datos, semilla, carpeta):
        (carpeta / 'entrada.bin').write_bytes(bytes(datos))
        (carpeta / 'semilla.json').write_text(json.dumps(list(semilla)), encoding='utf-8')
        for _ in range(128):
            if not palabra(datos, 0x2C3B):
                break
            self.paso(datos, semilla)
        if palabra(datos, 0x2C3B):
            raise ErrorAnalisis('La inmersión no termina su secuencia original.')
        self.interacciones_lago(datos)
        self.menu(datos, (2, 4))
        objetos = [palabra(datos, 0xB620 + i * 2) for i in range(10)]
        if 0x20 not in objetos:
            raise ErrorAnalisis(f'La pecera no figura en ponerse: {objetos}.')
        self.seleccionar(datos, objetos.index(0x20))
        for paso in range(128):
            self.paso(datos, semilla)
            if paso in (7, 23, 63, 127):
                self.captura(datos, carpeta, f'lago_pecera_{paso + 1}')
        inventario = [palabra(datos, 0x5225 + i * 2) for i in range(10)]
        if 6 in inventario or 7 in inventario:
            raise ErrorAnalisis('Ponerse la pecera no consume los objetos originales.')
        (carpeta / 'pecera.bin').write_bytes(bytes(datos))
        (carpeta / 'pecera_semilla.json').write_text(json.dumps(list(semilla)), encoding='utf-8')
        self.resumen['pecera_colocada'] = 1
        self.clic(datos, palabra(datos, 0x2C6F) - 16, palabra(datos, 0x2C71) + 24)
        self.andar(datos, semilla, 0xFFFF, 96)
        self.elegir_objeto(datos, (2, 2), 15)
        self.esperar(datos, semilla, 96)
        self.captura(datos, carpeta, 'lago_pez_liberado')
        self.clic(
            datos, palabra(datos, 0x2D77) + 5, palabra(datos, 0x2D79) + palabra(datos, 0x2D7D) - 2
        )
        self.andar(datos, semilla, 0xFFFF, 96)
        puntos = palabra(datos, 0x35C7)
        self.elegir_objeto(datos, (1,), 26)
        self.esperar(datos, semilla, 96)
        if palabra(datos, 0x35C7) != (puntos + 1) & 0xFFFF or datos[0x2D75]:
            raise ErrorAnalisis('La moneda no suma su punto o continúa activa.')
        self.captura(datos, carpeta, 'lago_moneda')
        self.clic(datos, palabra(datos, 0x2C2D) + 16, 177)
        self.andar(datos, semilla, 0xFFFF, 96)
        self.acercar(datos, semilla, 0x2C8E, 4, 0)
        self.elegir_objeto(datos, (3,), 16)
        self.esperar(datos, semilla, 96)
        self.captura(datos, carpeta, 'lago_gusano_descubierto')
        self.clic(
            datos, palabra(datos, 0x2CB1) + 5, palabra(datos, 0x2CB3) + palabra(datos, 0x2CB7) - 2
        )
        self.andar(datos, semilla, 0xFFFF, 96)
        self.elegir_objeto(datos, (1,), 17)
        self.esperar(datos, semilla, 96)
        if 17 not in [palabra(datos, 0x5225 + i * 2) for i in range(10)]:
            raise ErrorAnalisis('El gusano no se incorpora al inventario.')
        self.captura(datos, carpeta, 'lago_gusano_recogido')
        self.acercar(datos, semilla, 0x2CF1, 12, 52)
        self.elegir_objeto(datos, (2, 6), 17)
        self.elegir_objeto(datos, (), 20)
        self.esperar(datos, semilla, 192)
        if 17 in [palabra(datos, 0x5225 + i * 2) for i in range(10)]:
            raise ErrorAnalisis('Dar el gusano no lo retira del inventario.')
        self.captura(datos, carpeta, 'lago_oxigeno')
        (carpeta / 'oxigeno.bin').write_bytes(bytes(datos))
        (carpeta / 'oxigeno_semilla.json').write_text(json.dumps(list(semilla)), encoding='utf-8')
        self.resumen['intercambio_oxigeno'] = 1

    # Contrasta ramas aisladas sobre copias del estado real: capacidad, puntuación y concha en movimiento.
    def interacciones_lago(self, base):
        for inventario in ((), (6,), (6, 7), tuple(range(10))):
            for puntos in (0, 0xFFFF):
                d = bytearray(base)
                for i in range(10):
                    guardar(d, 0x5225 + i * 2, inventario[i] if i < len(inventario) else 0xFFFF)
                guardar(d, 0x35C7, puntos)
                guardar(d, 0xB60C, 0x3B53)
                guardar(d, 0xB620, 6 + 0x1A)
                guardar(d, 0xB634, 4)
                guardar(d, 0xB638, 0xFFFF)
                self.seleccionar(d, 0)
        for objeto in (17, 18, 23, 26):
            for ocupadas in (0, 9, 10):
                for movimiento in (0, 1):
                    d = bytearray(base)
                    for i in range(10):
                        guardar(d, 0x5225 + i * 2, 3 if i < ocupadas else 0xFFFF)
                    for p in (0x2CE0, 0x2CE5, 0x2CEA):
                        guardar(d, p, movimiento)
                    guardar(d, 0xB60C, 0x563A)
                    guardar(d, 0xB620, objeto + 0x1A)
                    self.seleccionar(d, 0)

    # Busca el identificador que ofrece el menú original, sin depender del orden de las otras casillas.
    def elegir_objeto(self, datos, casillas, identificador):
        if casillas:
            self.menu(datos, casillas)
        objetos = [palabra(datos, 0xB620 + i * 2) for i in range(10)]
        if identificador + 0x1A not in objetos:
            raise ErrorAnalisis(f'Objeto {identificador} ausente del menú: {objetos}.')
        self.seleccionar(datos, objetos.index(identificador + 0x1A))

    # Recorre el fondo y su cámara hasta que el objeto entra en pantalla; después usa el clic original.
    def acercar(self, datos, semilla, entidad, dx, dy):
        for _ in range(400):
            x = palabra(datos, entidad + 2)
            if 40 <= x <= 240:
                break
            if self.andar(datos, semilla, 8 if x >= 32768 or x < 40 else 4, 1) is not None:
                raise ErrorAnalisis('Se abandona el lago antes de alcanzar el objeto.')
        else:
            raise ErrorAnalisis(
                f'El objeto submarino no entra en pantalla: Brad={[palabra(datos, p) for p in (0x2C2D, 0x2C2F, 0x2C3B)]}, objeto={palabra(datos, entidad + 2)}, controles={list(datos[0x3594:0x3598])}.'
            )
        self.clic(datos, palabra(datos, entidad + 2) + dx, palabra(datos, entidad + 4) + dy)
        self.andar(datos, semilla, 0xFFFF, 96)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Compara las primeras acciones del lago con el EXE.'
    )
    parser.add_argument('--biblioteca', type=Path, required=True)
    parser.add_argument('--muestra', action='store_true')
    opciones = parser.parse_args()
    try:
        ComparacionLago(
            Path(__file__).resolve().parents[1], opciones.biblioteca, opciones.muestra
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        parser.exit(1, f'Error: {error}\n')
