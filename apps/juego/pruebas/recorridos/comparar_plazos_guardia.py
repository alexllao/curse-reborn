"""Contrasta intentos reales de colocar la cerilla antes y despues del plazo."""

import argparse
import json
from pathlib import Path
from auditar_consecuencias import ComparacionConsecuencias, ErrorAnalisis, leer_contexto
from comparar_objetos import palabra


class ComparacionPlazosGuardia(ComparacionConsecuencias):
    # Repite la aproximación desde la misma partida y consume cada demora con pasos originales.
    def ejecutar(self):
        origen = self.raiz / 'apps/juego/trabajo/comparacion_guardia_palacio/guardia_columna.ds.bin'
        base = origen.read_bytes()
        semilla = json.loads(
            origen.with_name('guardia_columna.semilla.json').read_text(encoding='utf-8')
        )
        contexto = leer_contexto(origen, 63)
        self.referencia_modelo = contexto['referencia_modelo']
        self.recursos_menu = True
        self.resumen = {}
        self.destino = self.raiz / 'apps/juego/trabajo/comparacion_plazos_guardia'
        self.destino.mkdir(parents=True, exist_ok=True)
        ramas = []
        for demora in (0, 16, 32, 64):
            d = bytearray(base)
            self.semilla_menu[:] = semilla
            self.fase.value = contexto['fase_probador']
            self.retomar_recursos(d)
            if palabra(d, 0x8082) != 63 or palabra(d, 0x35CE) != 100:
                raise ErrorAnalisis('La referencia no parte de la columna con el plazo real100.')
            self.exigir_objeto(d, 162)
            self.clic(d, 242, 163)
            self.esperar(d, 60 + demora)
            if not d[0x3594] or not d[0x3595]:
                raise ErrorAnalisis('El intento no dispone de control para abrir el menu.')
            antes = palabra(d, 0x35CE)
            self.elegir_objeto(d, (2, 1), 162)
            self.elegir_objeto(d, (), 163)
            self.esperar(d, 256)
            conserva = any(palabra(d, p) == 162 for p in range(0x5225, 0x523D, 2))
            nombre = 'cerilla_demora_' + str(demora)
            (self.destino / (nombre + '.ds.bin')).write_bytes(d)
            (self.destino / (nombre + '.semilla.json')).write_text(
                json.dumps(list(self.semilla_menu)) + '\n', encoding='utf-8'
            )
            self.conservar_contexto(self.destino, nombre)
            ramas.append(
                dict(
                    demora=demora,
                    cuenta_antes=antes,
                    conserva_cerilla=conserva,
                    cuenta_final=palabra(d, 0x35CE),
                    control=list(d[0x3594:0x3597]),
                )
            )
            print(ramas[-1], flush=True)
        if [r['conserva_cerilla'] for r in ramas] != [False, False, True, True]:
            raise ErrorAnalisis('La tanda no alcanza ambos lados del plazo original.')
        informe = dict(
            resultado='coinciden',
            ramas=ramas,
            grupos=self.resumen,
            contexto=contexto,
            limite='Demoras reales desde guardia_columna, sin asignar posiciones, objetos ni contador. Comparacion DS, azar y fase; no agota todos los plazos.',
        )
        (self.destino / 'resultado.json').write_text(
            json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8'
        )
        print(json.dumps(informe, ensure_ascii=False), flush=True)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--biblioteca', type=Path, required=True)
    args = parser.parse_args()
    try:
        ComparacionPlazosGuardia(
            Path(__file__).resolve().parents[4], args.biblioteca, True
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        parser.exit(1, f'Error: {error}\n')
