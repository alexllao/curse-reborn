"""Contrasta cristal antes de aceite y la oferta del silbato desde la partida real."""

import argparse
import json
from pathlib import Path

from auditar_consecuencias import ComparacionConsecuencias, ErrorAnalisis, leer_contexto
from comparar_calles import ComparacionCalles
from comparar_objetos import palabra


class ComparacionOrdenPoleas(ComparacionConsecuencias):
    # Guarda DS, azar y contexto después de acciones reales, sin fabricar objetos ni banderas.
    def conservar(self, datos, nombre):
        (self.destino / (nombre + '.ds.bin')).write_bytes(datos)
        (self.destino / (nombre + '.semilla.json')).write_text(
            json.dumps(list(self.semilla_menu)) + '\n', encoding='utf-8'
        )
        self.conservar_contexto(self.destino, nombre)

    # 6285 exige los siete útiles; la ultima recogida puede ser144 o145.
    def ejecutar(self):
        origen = (
            self.raiz
            / 'apps/juego/trabajo/comparacion_resolucion_palacio/poleas_ante_utiles.ds.bin'
        )
        datos = bytearray(origen.read_bytes())
        contexto = leer_contexto(origen, 47)
        self.semilla_menu[:] = json.loads(
            origen.with_name('poleas_ante_utiles.semilla.json').read_text(encoding='utf-8')
        )
        self.referencia_modelo = contexto['referencia_modelo']
        self.fase.value = contexto['fase_probador']
        self.recursos_menu = True
        self.resumen = {}
        self.destino = self.raiz / 'apps/juego/trabajo/comparacion_orden_poleas'
        self.destino.mkdir(parents=True, exist_ok=True)
        self.retomar_recursos(datos)
        if palabra(datos, 0x8082) != 47:
            raise ErrorAnalisis('La referencia no está ante los útiles reales de las poleas.')
        for id in (141, 149, 150, 152, 153, 154):
            self.exigir_objeto(datos, id)
        self.exigir_objeto(datos, 144, False)
        self.exigir_objeto(datos, 145, False)
        oferta_inicial = datos[0x4BAE]
        self.elegir_objeto(datos, (1,), 145)
        self.esperar(datos, 192)
        self.exigir_objeto(datos, 145)
        self.exigir_objeto(datos, 141)
        if datos[0x4BAE] != oferta_inicial:
            raise ErrorAnalisis('El cristal ofrece el silbato sin haber recogido el aceite.')
        self.conservar(datos, 'poleas_cristal_primero')
        if ComparacionCalles.ir(self, datos, self.semilla_menu, 238, 150) is not None:
            raise ErrorAnalisis('Salida inesperada al acercarse al aceite.')
        self.elegir_objeto(datos, (1,), 144)
        self.esperar(datos, 192)
        for id in (144, 145):
            self.exigir_objeto(datos, id)
        self.exigir_objeto(datos, 141, False)
        if datos[0x4BAE] != 1:
            raise ErrorAnalisis('Recoger el aceite en segundo lugar no ofrece el silbato.')
        self.conservar(datos, 'poleas_aceite_despues')
        for salida in (86, 88):
            actual = ComparacionCalles.ir(self, datos, self.semilla_menu, 160, 166)
            if actual != salida:
                raise ErrorAnalisis(f'El regreso entrega{actual}, esperado{salida}.')
            self.cambiar(datos, self.semilla_menu, salida)
            self.esperar(datos, 16)
        if ComparacionCalles.ir(self, datos, self.semilla_menu, 261, 188) is not None:
            raise ErrorAnalisis('Salida inesperada ante el silbato.')
        self.elegir_objeto(datos, (1,), 147)
        self.esperar(datos, 192)
        self.exigir_objeto(datos, 147)
        self.conservar(datos, 'ventana_silbato_inverso')
        self.elegir_objeto(datos, (2, 7), 147)
        self.seleccionar(datos, 0)
        self.elegir_objeto(datos, (), 152)
        self.esperar(datos, 192)
        self.exigir_objeto(datos, 148)
        self.elegir_objeto(datos, (5,), 148)
        self.esperar(datos, 512)
        if datos[0x2C4C] != 0:
            raise ErrorAnalisis('El orden inverso deja la ventana cerrada.')
        self.conservar(datos, 'ventana_abierta_inversa')
        self.elegir_objeto(datos, (6,), 146)
        for paso in range(1, 769):
            salida = self.andar(datos, self.semilla_menu, 65535, 1)
            if salida is None:
                continue
            if salida != 90:
                raise ErrorAnalisis(f'La ventana solicita{salida}, esperada90.')
            self.conservar(datos, 'ventana_inversa_solicitud90')
            break
        else:
            raise ErrorAnalisis('El salto no entrega90 tras el orden inverso.')
        informe = dict(
            resultado='coinciden',
            contexto=contexto,
            grupos=self.resumen,
            pasos_contrastados=self.resumen.get('pasos_jugables', 0),
            selecciones=self.resumen.get('selecciones', 0),
            salida=salida,
            pasos_salto=paso,
            limite='Orden145/144 real, oferta147 al reunir los siete útiles, rutas86/88, combinación y ventana hasta pedir90. No carga56 ni cambia inventario o marcas artificialmente.',
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
        ComparacionOrdenPoleas(
            Path(__file__).resolve().parents[4], args.biblioteca, True
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        parser.exit(1, f'Error: {error}\n')
