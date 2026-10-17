"""Continúa la salida de las cuevas por COUNTRY1 y contrasta el sendero de perspectiva."""

import argparse
import ctypes
import json
from pathlib import Path

from auditar_opciones import auditar_estado
from comparar_final_cuevas import ComparacionFinalCuevas, ErrorAnalisis
from comparar_objetos import palabra


class ComparacionCampo(ComparacionFinalCuevas):
    # Conserva el comparador y los servicios originales del recorrido real anterior.
    def __init__(self, raiz, biblioteca, muestra):
        super().__init__(raiz, biblioteca, muestra)
        self.carpeta_resultados = 'comparacion_campo'
        self.estados_auditados = 0

    # Revisa todas las opciones que ofrecen las listas en esta posición y fase de la pantalla.
    def auditar(self, datos, nombre):
        if not datos[0x3595]:
            return
        semilla = list(self.semilla_menu)
        try:
            pendientes = auditar_estado(self, bytearray(datos), nombre)
        finally:
            self.semilla_menu[:] = semilla
            self.retomar_recursos(datos)
        if pendientes:
            raise ErrorAnalisis(f'Opciones de {nombre} pendientes: {pendientes[:3]}')
        self.estados_auditados += 1
        self.maquina.ctl_flush_tb()

    # Usa una instantánea previamente contrastada o genera la ruta completa si todavía falta.
    def ejecutar(self):
        origen = self.raiz / 'trabajo/comparacion_final_cuevas'
        fichero = origen / 'final_salida_cuevas.ds.bin'
        if not fichero.exists():
            print('Preparando la ruta original hasta salir de las cuevas…', flush=True)
            ComparacionFinalCuevas(self.raiz, self.biblioteca, True).ejecutar()
        datos = bytearray(fichero.read_bytes())
        if len(datos) != 65536 or palabra(datos, 0x8082) != 15 or palabra(datos, 0x3598) != 24:
            raise ErrorAnalisis('La instantánea no corresponde a la salida real de las cuevas.')
        semilla = (ctypes.c_ushort * 4)(
            *json.loads((origen / 'final_salida_cuevas.semilla.json').read_text(encoding='utf-8'))
        )
        self.semilla_menu = semilla
        self.recursos_menu = True
        self.retomar_recursos(datos)
        carpeta = self.raiz / 'trabajo/comparacion_campo'
        carpeta.mkdir(parents=True, exist_ok=True)
        self.resumen = {}
        self.captura(datos, carpeta, 'campo_0')
        anterior = 0
        for paso in (32, 64, 96, 160):
            if self.andar(datos, semilla, 0xFFFF, paso - anterior) is not None:
                raise ErrorAnalisis('La presentación del monstruo abandona COUNTRY1.')
            anterior = paso
            self.captura(datos, carpeta, f'campo_{paso}')
        if not datos[0x3594] or datos[0x358C] != 255:
            raise ErrorAnalisis('El monstruo no devuelve el control al personaje normal.')
        self.auditar(datos, 'máscara caída')
        # Estas posiciones rodean el pozo y el montículo sin alterar BOXDET ni teletransportar.
        for x, y in ((120, 165), (150, 130)):
            self.clic(datos, x, y)
            if self.andar(datos, semilla, 0xFFFF, 96) is not None:
                raise ErrorAnalisis('La aproximación al sendero abandona la pantalla.')
            self.captura(datos, carpeta, f'campo_camino_{x}')
            self.auditar(datos, f'camino {x}')
        self.clic(datos, 110, 119)
        if self.andar(datos, semilla, 0xFFFF, 32) is not None:
            raise ErrorAnalisis('El sendero termina antes de su animación intermedia.')
        self.captura(datos, carpeta, 'campo_sendero')
        for _ in range(512):
            salida = self.andar(datos, semilla, 0xFFFF, 1)
            if salida is not None:
                break
        else:
            raise ErrorAnalisis('El sendero no termina en la salida 25.')
        if salida != 25:
            raise ErrorAnalisis(f'Salida distinta del campo: {salida}.')
        self.cambiar(datos, semilla, salida)
        self.captura(datos, carpeta, 'campo_salida')
        informe = {
            'resultado': 'coinciden',
            'casos_y_pasos': sum(self.resumen.values()),
            'grupos': self.resumen,
            'capturas': 9,
            'estados_auditados': self.estados_auditados,
            'alcance': 'COUNTRY1 desde la salida real de las cuevas hasta la entrada en COUNTRY2; los puzles de COUNTRY2 siguen pendientes.',
        }
        (carpeta / 'resultado.json').write_text(
            json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8'
        )
        print(
            f'Campo: {informe["casos_y_pasos"]} casos/pasos, nueve composiciones y {self.estados_auditados} estados de menú coincidentes.'
        )


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Compara la decimosexta pantalla con DOS.')
    parser.add_argument('--biblioteca', type=Path, required=True)
    parser.add_argument('--muestra', action='store_true')
    opciones = parser.parse_args()
    try:
        ComparacionCampo(
            Path(__file__).resolve().parents[1], opciones.biblioteca, opciones.muestra
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        parser.exit(1, f'Error: {error}\n')
