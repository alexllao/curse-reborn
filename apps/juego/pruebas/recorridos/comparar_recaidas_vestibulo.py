"""Contrasta nuevas caidas11 con el actor ya activado por una caida real."""

import argparse
import json
from pathlib import Path

from auditar_consecuencias import ErrorAnalisis, leer_contexto
from comparar_calles import ComparacionCalles
from comparar_objetos import palabra
from comparar_regreso_remote import ComparacionRegresoRemote


class ComparacionRecaidas(ComparacionRegresoRemote):
    # 560B-5638: observa tambien el actor durante la caida, hasta recuperar ambos controles.
    def caer(self, d):
        self.clic(d, 175, 131)
        reaccion = None
        for paso in range(512):
            if self.andar(d, self.semilla_menu, 65535, 1) is not None:
                raise ErrorAnalisis('Salida inesperada durante la caida11.')
            if reaccion is None and palabra(d, 0x2C3B) == 0x9678:
                reaccion = dict(
                    paso=paso + 1,
                    actor=d[0x2C4C],
                    canal=palabra(d, 0x2C5C),
                    contador=d[0x2C5F],
                    intervalo=d[0x2C60],
                )
                if reaccion['canal'] != 0x1A2F or reaccion['intervalo'] != 50:
                    raise ErrorAnalisis('La caida no prepara el canal1A2F/50.')
            if reaccion and d[0x3594] and d[0x3595] and not palabra(d, 0x2C3B):
                return dict(reaccion=reaccion, recuperacion=paso + 1)
        raise ErrorAnalisis('La caida no termina con control en512 pasos.')

    # Cada demora parte de su referencia y semilla reales; no se asignan canales ni contadores.
    def ejecutar(self):
        self.recursos_menu = True
        self.resumen = {}
        self.destino = self.raiz / 'apps/juego/trabajo/comparacion_recaidas_vestibulo'
        self.destino.mkdir(parents=True, exist_ok=True)
        resultados = []
        for nombre, carpeta in (
            ('vestibulo_control', 'comparacion_vestibulo_castillo'),
            ('estudio_vestibulo', 'comparacion_estudio_castillo'),
        ):
            origen = self.raiz / 'apps/juego/trabajo' / carpeta / (nombre + '.ds.bin')
            base = origen.read_bytes()
            contexto = leer_contexto(origen, 69)
            semilla = json.loads(
                origen.with_name(nombre + '.semilla.json').read_text(encoding='utf-8')
            )
            for demora in (0, 32, 96):
                d = bytearray(base)
                self.semilla_menu[:] = semilla
                self.referencia_modelo = contexto['referencia_modelo']
                self.fase.value = contexto['fase_probador']
                self.retomar_recursos(d)
                inventario = bytes(d[0x5225:0x523D])
                inicio = self.resumen.get('pasos_jugables', 0)
                if palabra(d, 0x8082) != 69 or d[0x2C4C] != 3:
                    raise ErrorAnalisis('La referencia no conserva el actor activo del vestibulo.')
                if nombre == 'estudio_vestibulo':
                    self.exigir_objeto(d, 175)
                    # Baja la escalera izquierda y rodea el borde antes de volver al centro.
                    for x, y in ((90, 160), (140, 190), (160, 143)):
                        if ComparacionCalles.ir(self, d, self.semilla_menu, x, y) is not None:
                            raise ErrorAnalisis('Salida durante el regreso al centro.')
                primera = self.caer(d)
                self.esperar(d, demora)
                antes = dict(
                    actor=d[0x2C4C], canal=palabra(d, 0x2C5C), contador=d[0x2C5F], imagen=d[0x2C4D]
                )
                if antes['actor'] != 3 or antes['canal'] != (0 if demora == 96 else 0x1A2F):
                    raise ErrorAnalisis('No se alcanza el estado real de actor esperado.')
                segunda = self.caer(d)
                self.esperar(d, 128)
                if (
                    palabra(d, 0x8082) != 69
                    or bytes(d[0x5225:0x523D]) != inventario
                    or not all(d[0x3594:0x3597])
                ):
                    raise ErrorAnalisis('La recaida altera escena, inventario o control.')
                salida = None
                for x, y in ((90, 160), (48, 126)):
                    salida = ComparacionCalles.ir(self, d, self.semilla_menu, x, y)
                    if salida is not None:
                        break
                if salida != 120:
                    raise ErrorAnalisis('No se puede continuar al estudio tras la recaida.')
                resultado = dict(
                    origen=nombre,
                    demora=demora,
                    actor_antes=antes,
                    primera=primera,
                    segunda=segunda,
                    salida=salida,
                    pasos=self.resumen['pasos_jugables'] - inicio,
                )
                resultados.append(resultado)
                print(json.dumps(resultado), flush=True)
        informe = dict(
            resultado='coinciden',
            casos=resultados,
            grupos=self.resumen,
            limite='Seis recaidas reales desde dos inventarios con tres demoras; no modifica geometria ni estado del actor.',
        )
        (self.destino / 'resultado.json').write_text(
            json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8'
        )


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--biblioteca', type=Path, required=True)
    args = parser.parse_args()
    try:
        ComparacionRecaidas(Path(__file__).resolve().parents[4], args.biblioteca, True).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        parser.exit(1, f'Error: {error}\n')
