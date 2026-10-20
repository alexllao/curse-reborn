"""Contrasta los accesos laterales de CELESTE, su azar y los retornos desde DINO/POST."""

import argparse
import json
from pathlib import Path

from comparar_puente_barco import ComparacionPuenteBarco, ErrorAnalisis
from comparar_calles import ComparacionCalles
from comparar_objetos import palabra


class ComparacionEnlacesValle(ComparacionPuenteBarco):
    # Reproduce clics y pasos originales hasta un umbral sin sustituir su decisión aleatoria.
    def alcanzar_salida(self, d, puntos):
        for x, y in puntos:
            salida = ComparacionCalles.ir(self, d, self.semilla_menu, x, y)
            if salida is not None:
                return salida
        raise ErrorAnalisis(f'El camino no alcanza su salida: escena{palabra(d, 0x8082)}.')

    # Retoma una rama real conservando su semilla y sus recursos, sin cambiar inventario ni actores.
    def retomar_rama(self, base, semilla):
        self.semilla_menu[:] = semilla
        d = bytearray(base)
        self.retomar_recursos(d)
        return d

    # Ambas decisiones proceden de la partida real y de esperas en POST, que consume azar.
    def ejecutar(self):
        origen = self.raiz / 'apps/juego/trabajo/comparacion_puente_barco/barco_regreso_celeste'
        if not origen.with_suffix('.ds.bin').exists():
            ComparacionPuenteBarco(self.raiz, self.biblioteca, True).ejecutar()
        base = origen.with_suffix('.ds.bin').read_bytes()
        semilla = json.loads(origen.with_suffix('.semilla.json').read_text(encoding='utf-8'))
        self.destino = self.raiz / 'apps/juego/trabajo/comparacion_enlaces_valle'
        self.destino.mkdir(parents=True, exist_ok=True)
        self.resumen = {}
        d = self.retomar_rama(base, semilla)
        for punto in ((330, 170), (450, 180), (530, 190)):
            if ComparacionCalles.ir(self, d, self.semilla_menu, *punto) is not None:
                raise ErrorAnalisis('El acercamiento abandona CELESTE antes del umbral70.')
        antes = bytes(d)
        semilla_antes = list(self.semilla_menu)
        decisiones = {}
        poste = self.raiz / 'apps/juego/trabajo/comparacion_regreso_loro/valle_post_entrada'
        base_poste = poste.with_suffix('.ds.bin').read_bytes()
        semilla_poste = json.loads(poste.with_suffix('.semilla.json').read_text(encoding='utf-8'))
        for espera in range(16):
            if espera == 0:
                d = self.retomar_rama(antes, semilla_antes)
            else:
                # POST consume azar autónomo: la semilla surge de esperas reales, no de asignaciones.
                d = self.retomar_rama(base_poste, semilla_poste)
                self.esperar(d, espera)
                self.ruta(d, ((750, 145), (750, 110), (720, 100), (700, 95), (685, 90)), 71)
                for punto in ((530, 180), (530, 190)):
                    self.ruta_sin_salida(d, *punto)
            salida = self.alcanzar_salida(d, ((530, 199),))
            if salida not in (52, 70):
                raise ErrorAnalisis(f'El umbral70 decide{salida}, fuera de52/70.')
            if salida in decisiones:
                continue
            decisiones[salida] = espera
            self.cambiar(d, self.semilla_menu, salida)
            self.esperar(d, 128)
            self.hito_borde(d, f'celeste_azar_{salida}')
            self.auditar(d, f'entrada aleatoria{salida}')
            puntos = (
                ((480, 190), (480, 199))
                if salida == 70
                else ((750, 145), (750, 110), (720, 100), (700, 95), (685, 90))
            )
            retorno = self.alcanzar_salida(d, puntos)
            if retorno != 71:
                raise ErrorAnalisis(f'Retorno de{salida}: {retorno}, esperado71.')
            self.cambiar(d, self.semilla_menu, retorno)
            self.esperar(d, 128)
            self.hito_borde(d, f'celeste_regreso_{salida}')
            if len(decisiones) == 2:
                break
        if len(decisiones) != 2:
            raise ErrorAnalisis('Las esperas reales no cubren ambas decisiones del umbral70.')
        d = self.retomar_rama(base, semilla)
        salida = self.alcanzar_salida(d, ((330, 170), (330, 199)))
        if salida != 55:
            raise ErrorAnalisis(f'El acceso inferior a REMOTE devuelve{salida}, esperado55.')
        self.cambiar(d, self.semilla_menu, 55)
        self.esperar(d, 128)
        self.hito_borde(d, 'celeste_remote_55')
        self.auditar(d, 'REMOTE desde55')
        self.ruta(d, ((170, 122),), 56)
        self.hito_borde(d, 'celeste_regreso_56')
        informe = {
            'resultado': 'coinciden',
            'casos_y_pasos': sum(self.resumen.values()),
            'grupos': self.resumen,
            'esperas_por_destino': decisiones,
            'capturas': self.capturas,
            'estados_auditados': self.estados_auditados,
            'alcance': 'Umbral70 de CELESTE con sus dos decisiones originales52/70, retornos71 desde POST y DINO y ruta55/56. No integra SDL ni acredita otros estados argumentales.',
        }
        (self.destino / 'resultado.json').write_text(
            json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8'
        )
        print(
            f'Enlaces del valle: {informe["casos_y_pasos"]} casos/pasos y {self.capturas} composiciones coincidentes.'
        )


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Contrasta los caminos laterales del valle.')
    parser.add_argument('--biblioteca', type=Path, required=True)
    parser.add_argument('--muestra', action='store_true')
    opciones = parser.parse_args()
    try:
        ComparacionEnlacesValle(
            Path(__file__).resolve().parents[4], opciones.biblioteca, opciones.muestra
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        parser.exit(1, f'Error: {error}\n')
