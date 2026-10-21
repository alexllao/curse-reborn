"""Contrasta la suspensión original al comer170 en65, antes de cruzar114."""

import argparse
import json
from pathlib import Path

from auditar_consecuencias import ComparacionConsecuencias, ErrorAnalisis, leer_contexto
from comparar_objetos import palabra


class ComparacionAjoPrimeraZona(ComparacionConsecuencias):
    # 4680 no comprueba escena: acepta fase3 también en65 y fija destino78/177.
    # Conserva la secuencia original; no transforma el bloqueo en una salida o un rechazo.
    def ejecutar(self):
        origen = self.raiz / 'apps/juego/trabajo/comparacion_cementerio/cementerio_piedra.ds.bin'
        datos = bytearray(origen.read_bytes())
        contexto = leer_contexto(origen, 65)
        self.semilla_menu[:] = json.loads(
            origen.with_name('cementerio_piedra.semilla.json').read_text(encoding='utf-8')
        )
        self.referencia_modelo = contexto['referencia_modelo']
        self.fase.value = contexto['fase_probador']
        self.recursos_menu = True
        self.resumen = {}
        self.retomar_recursos(datos)
        if palabra(datos, 0x8082) != 65 or (datos[0x35CA] >> 3) & 7 != 3:
            raise ErrorAnalisis('La referencia no es la fase3 real del primer cementerio.')
        self.exigir_objeto(datos, 170)
        self.elegir_objeto(datos, (2, 3), 170)
        muestras = []
        for paso in range(1, 4097):
            if self.andar(datos, self.semilla_menu, 65535, 1) is not None:
                raise ErrorAnalisis(f'El ajo entrega una salida inesperada en el paso{paso}.')
            if paso not in (512, 1024, 2048, 4096):
                continue
            estado = dict(
                paso=paso,
                posicion=[palabra(datos, 0x2C2D), palabra(datos, 0x2C2F)],
                destino=[palabra(datos, 0x1A38), palabra(datos, 0x1A3A)],
                menu=datos[0x3595],
                control=datos[0x3594],
                fase_vampiro=(datos[0x35CA] >> 3) & 7,
                camino=palabra(datos, 0x359A),
                canal_brad=palabra(datos, 0x2C3B),
            )
            if (
                estado['posicion'] != [64, 130]
                or estado['destino'] != [78, 177]
                or any(estado[k] for k in ('menu', 'control', 'camino', 'canal_brad'))
                or estado['fase_vampiro'] != 4
            ):
                raise ErrorAnalisis(f'La suspensión original cambia: {estado}.')
            muestras.append(estado)
        self.exigir_objeto(datos, 170, False)
        destino = self.raiz / 'apps/juego/trabajo/comparacion_ajo_primera_zona'
        destino.mkdir(parents=True, exist_ok=True)
        informe = dict(
            resultado='coinciden',
            referencia=str(origen.relative_to(self.raiz)),
            contexto=contexto,
            pasos_contrastados=self.resumen.get('pasos_jugables', 0),
            selecciones=self.resumen.get('selecciones', 0),
            muestras=muestras,
            limite='Suspensión observada hasta4096 pasos con DS completo, azar y fase coincidentes. No prueba un ciclo infinito. El recorrido positivo come170 después de114, en77.',
        )
        (destino / 'resultado.json').write_text(
            json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8'
        )
        print(json.dumps(informe, ensure_ascii=False), flush=True)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--biblioteca', type=Path, required=True)
    args = parser.parse_args()
    try:
        ComparacionAjoPrimeraZona(
            Path(__file__).resolve().parents[4], args.biblioteca, True
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        parser.exit(1, f'Error: {error}\n')
