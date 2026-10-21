"""Contrasta aproximaciones reales desde cuatro direcciones a los tropiezos9/10."""

import argparse
import json
from pathlib import Path
from auditar_consecuencias import ErrorAnalisis, leer_contexto
from comparar_regreso_remote import ComparacionRegresoRemote
from comparar_calles import ComparacionCalles
from comparar_objetos import palabra
from comparar_tropiezos_cementerio import CANALES

CASOS = (
    (9, 'derecha', 4, ((270, 190),)),
    (9, 'izquierda', 8, ((280, 195), (450, 195), (380, 190))),
    (9, 'arriba', 2, ((280, 195), (300, 198))),
    (9, 'abajo', 1, ((280, 178), (300, 178))),
    (10, 'derecha', 4, ((280, 195), (450, 195), (530, 180), (530, 159))),
    (10, 'izquierda', 8, ((280, 195), (450, 195), (590, 180), (590, 159))),
    (10, 'arriba', 2, ((280, 195), (450, 195), (550, 180), (560, 180))),
    (10, 'abajo', 1, ((280, 195), (450, 195), (530, 180), (530, 150), (565, 150))),
)


class ComparacionAproximaciones(ComparacionRegresoRemote):
    def paso(self, d, sem, direccion=65535):
        salida = super().paso(d, sem, direccion)
        canales = tuple(palabra(d, c) for c in (0x2C3B, 0x2C40, 0x2C45))
        if self.reaccion is None and canales in CANALES:
            self.reaccion = dict(
                canales=canales,
                fotograma=d[0x2C2C],
                intervalos=[d[p] for p in (0x2C3F, 0x2C44, 0x2C49)],
                posicion=[palabra(d, 0x2C2D), palabra(d, 0x2C2F)],
                camara=palabra(d, 0x3591),
            )
            if self.preparando:
                raise ErrorAnalisis(
                    f'La aproximacion {self.caso} tropieza antes de la entrada dirigida: {self.reaccion}.'
                )
        return salida

    def ejecutar(self, casos=None):
        origen = (
            self.raiz
            / 'apps/juego/trabajo/comparacion_tropiezos_cementerio/cementerio_caida_recuperada.ds.bin'
        )
        base = origen.read_bytes()
        semilla = json.loads(
            origen.with_name('cementerio_caida_recuperada.semilla.json').read_text(encoding='utf-8')
        )
        contexto = leer_contexto(origen, 65)
        self.referencia_modelo = contexto['referencia_modelo']
        self.recursos_menu = True
        self.resumen = {}
        self.destino = self.raiz / 'apps/juego/trabajo/comparacion_aproximaciones_cementerio'
        self.destino.mkdir(parents=True, exist_ok=True)
        resultados = []
        for tipo, nombre, direccion, puntos in CASOS:
            self.caso = f'{tipo}_{nombre}'
            if casos and self.caso not in casos:
                continue
            self.reaccion = None
            self.preparando = True
            d = bytearray(base)
            self.semilla_menu[:] = semilla
            self.fase.value = contexto['fase_probador']
            self.retomar_recursos(d)
            inventario = bytes(d[0x5225:0x523D])
            fase_inicial = (d[0x35CA] >> 3) & 7
            if palabra(d, 0x8082) != 65 or fase_inicial != 6 or not d[0x3594] or not d[0x3595]:
                raise ErrorAnalisis(
                    'La referencia no parte del cementerio recuperado con fase6 y control.'
                )
            inicial = self.resumen.get('pasos_jugables', 0)
            for x, y in puntos:
                if ComparacionCalles.ir(self, d, self.semilla_menu, x, y) is not None:
                    raise ErrorAnalisis('Salida durante la aproximacion previa.')
            self.preparando = False
            for paso in range(192):
                salida = self.andar(d, self.semilla_menu, direccion, 1)
                if salida is not None:
                    raise ErrorAnalisis(
                        f'Salida{salida} en {self.caso} antes de detectar el tropiezo;pos{palabra(d, 0x2C2D)}/{palabra(d, 0x2C2F)},cam{palabra(d, 0x3591)}.'
                    )
                if self.reaccion is not None:
                    break
            else:
                raise ErrorAnalisis(
                    f'No se encuentra el tropiezo {self.caso}: posicion{palabra(d, 0x2C2D)}/{palabra(d, 0x2C2F)},camara{palabra(d, 0x3591)}.'
                )
            esperado = 8 if tipo == 9 else 2
            imagen = self.reaccion['fotograma']
            grupo = 0 if imagen <= 8 else 1 if imagen <= 17 else 2 if imagen <= 26 else 3
            if (
                self.reaccion['intervalos'] != [esperado] * 3
                or tuple(self.reaccion['canales']) != CANALES[grupo]
            ):
                raise ErrorAnalisis(f'Reaccion inesperada en {self.caso}: {self.reaccion}.')
            salida = None
            for espera in range(512):
                salida = self.andar(d, self.semilla_menu, 65535, 1)
                if salida is not None:
                    break
                if d[0x3594] and d[0x3595] and palabra(d, 0x2C3B) == 0:
                    break
            else:
                raise ErrorAnalisis('La reaccion no termina dentro del plazo.')
            if salida is not None:
                if salida != 113:
                    raise ErrorAnalisis(f'Salida distinta tras el tropiezo: {salida}.')
                self.cambiar(d, self.semilla_menu, salida)
                for recuperar in range(512):
                    if self.andar(d, self.semilla_menu, 65535, 1) is not None:
                        raise ErrorAnalisis('Otra salida en recuperacion.')
                    if d[0x3594] and d[0x3595]:
                        break
                else:
                    raise ErrorAnalisis('No se recupera el control tras113.')
            self.esperar(d, 128)
            if (
                palabra(d, 0x8082) != 65
                or bytes(d[0x5225:0x523D]) != inventario
                or ((d[0x35CA] >> 3) & 7) != fase_inicial
                or not all(d[0x3594:0x3597])
            ):
                raise ErrorAnalisis('La recuperacion altera escena, inventario o fase.')
            self.conservar(d, 'cementerio_tropiezo_' + self.caso)
            resultado = dict(
                caso=self.caso,
                direccion_solicitada=direccion,
                grupo_observado=grupo,
                reaccion=self.reaccion,
                salida=salida,
                pasos=self.resumen['pasos_jugables'] - inicial,
                fase=fase_inicial,
                control=list(d[0x3594:0x3597]),
            )
            resultados.append(resultado)
            print(resultado, flush=True)
        informe = dict(
            resultado='coinciden',
            casos_contrastados=len(resultados),
            casos=resultados,
            grupos=self.resumen,
            contexto=contexto,
            limite='Aproximaciones por movimiento real y geometria original; no asigna posiciones, imagenes, inventario ni fase. No agota todas las posiciones y estados de actores.',
        )
        (self.destino / ('resultado_parcial.json' if casos else 'resultado.json')).write_text(
            json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8'
        )
        print(json.dumps(informe, ensure_ascii=False), flush=True)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--biblioteca', type=Path, required=True)
    parser.add_argument('--casos', nargs='+', choices=[f'{t}_{n}' for t, n, *_ in CASOS])
    args = parser.parse_args()
    try:
        ComparacionAproximaciones(
            Path(__file__).resolve().parents[4], args.biblioteca, True
        ).ejecutar(args.casos)
    except (OSError, ErrorAnalisis) as error:
        parser.exit(1, f'Error: {error}\n')
