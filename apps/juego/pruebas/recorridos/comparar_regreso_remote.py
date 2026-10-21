"""Contrasta POST-REMOTE-CELESTE-POST con grabacion106 y correo ya enviado."""

import argparse
import json
from pathlib import Path

from auditar_consecuencias import ComparacionConsecuencias, ErrorAnalisis, leer_contexto
from comparar_objetos import palabra
from comparar_regreso_loro import ComparacionRegresoLoro


class ComparacionRegresoRemote(ComparacionConsecuencias):
    # No envía clics durante una reacción: el bucle SDL/DOS los ignora sin control.
    def clic(self, datos, x, y):
        for _ in range(512):
            if datos[0x3594] and datos[0x3595]:
                return super().clic(datos, x, y)
            if self.andar(datos, self.semilla_menu, 65535, 1) is not None:
                raise ErrorAnalisis('Cambio de sala durante la espera del control.')
        raise ErrorAnalisis('No se recupera el control antes del clic.')

    # Conserva las referencias con el contexto que realmente usa el comparador.
    def conservar(self, datos, nombre):
        (self.destino / (nombre + '.ds.bin')).write_bytes(datos)
        (self.destino / (nombre + '.semilla.json')).write_text(
            json.dumps(list(self.semilla_menu)) + '\n', encoding='utf-8'
        )
        self.conservar_contexto(self.destino, nombre)

    # No recupera el mando consumido ni la carta desde otra instantanea.
    def ejecutar(self):
        origen = (
            self.raiz
            / 'apps/juego/trabajo/comparacion_grabacion_completa/grabacion_y_correo_enviado.ds.bin'
        )
        datos = bytearray(origen.read_bytes())
        contexto = leer_contexto(origen, 35)
        self.semilla_menu[:] = json.loads(
            origen.with_name('grabacion_y_correo_enviado.semilla.json').read_text(encoding='utf-8')
        )
        self.referencia_modelo = contexto['referencia_modelo']
        self.fase.value = contexto['fase_probador']
        self.recursos_menu = True
        self.resumen = {}
        self.destino = self.raiz / 'apps/juego/trabajo/comparacion_regreso_remote'
        self.destino.mkdir(parents=True, exist_ok=True)
        self.retomar_recursos(datos)
        if palabra(datos, 0x8082) != 35:
            raise ErrorAnalisis('La referencia no parte de POST tras el correo.')
        self.exigir_objeto(datos, 106)
        for id in (105, 108, 109, 111, 112):
            self.exigir_objeto(datos, id, False)
        inventario = bytes(datos[0x5225:0x523D])
        # Rodea el monton central por debajo antes de caminar al acceso60.
        self.clic(datos, palabra(datos, 0x2C2D) + 16, 185)
        self.esperar(datos, 192)
        estados = []
        for escena, salida, puntos, nombre in (
            (34, 60, ((250, 185), (230, 170), (120, 185), (40, 199)), 'remote_regreso_correo'),
            (33, 56, ((170, 122),), 'celeste_regreso_correo'),
            (35, 61, ((100, 198),), 'post_regreso_correo'),
        ):
            ComparacionRegresoLoro.ruta(self, datos, puntos, salida)
            if palabra(datos, 0x8082) != escena or bytes(datos[0x5225:0x523D]) != inventario:
                raise ErrorAnalisis(
                    f'El regreso{salida} cambia el inventario o el destino{escena}.'
                )
            self.conservar(datos, nombre)
            estados.append(
                dict(
                    referencia=nombre,
                    escena=escena,
                    entrada=salida,
                    actor_2c4c=datos[0x2C4C],
                    actor_2c6d=datos[0x2C6D],
                    flags35c8=datos[0x35C8],
                    flags35c9=datos[0x35C9],
                    posicion=[palabra(datos, 0x2C2D), palabra(datos, 0x2C2F)],
                )
            )
        informe = dict(
            resultado='coinciden',
            contexto=contexto,
            estados=estados,
            grupos=self.resumen,
            pasos_contrastados=self.resumen.get('pasos_jugables', 0),
            limite='Regresos reales60/56/61 con106 y correo enviado, conservando doce palabras de inventario. DS completo, azar y fase contra el EXE; no fuerza entradas ni reconstruye objetos consumidos.',
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
        ComparacionRegresoRemote(
            Path(__file__).resolve().parents[4], args.biblioteca, True
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        parser.exit(1, f'Error: {error}\n')
