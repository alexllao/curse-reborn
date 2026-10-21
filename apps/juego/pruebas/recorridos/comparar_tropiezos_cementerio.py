"""Contrasta contratos de orientacion y una caida real con regreso113."""

import argparse
import json
import struct
import ctypes
from pathlib import Path
from auditar_consecuencias import ComparacionConsecuencias, ErrorAnalisis, leer_contexto
from comparar_objetos import palabra, guardar
from comparar_regreso_loro import ComparacionRegresoLoro

CANALES = (
    (0x9634, 0x963D, 0x9641),
    (0x9656, 0x965F, 0x9663),
    (0x9667, 0x9670, 0x9674),
    (0x9645, 0x964E, 0x9652),
)


class ComparacionTropiezos(ComparacionConsecuencias):
    def cargar(self, nombre):
        origen = self.raiz / 'apps/juego/trabajo/comparacion_cementerio' / (nombre + '.ds.bin')
        d = bytearray(origen.read_bytes())
        self.semilla_menu[:] = json.loads(
            origen.with_name(nombre + '.semilla.json').read_text(encoding='utf-8')
        )
        contexto = leer_contexto(origen, palabra(d, 0x8082))
        self.referencia_modelo = contexto['referencia_modelo']
        self.fase.value = contexto['fase_probador']
        self.retomar_recursos(d)
        return d, contexto

    def contratos(self):
        base, contexto = self.cargar('cementerio_util_167')
        semilla = list(self.semilla_menu)
        grupos = {}
        for tipo in (9, 10, 11):
            for fotograma in range(256):
                d = bytearray(base)
                self.semilla_menu[:] = semilla
                self.fase.value = contexto['fase_probador']
                self.retomar_recursos(d)
                # Fixture aislada: una caja de tropiezo y canales detenidos, no una ruta real.
                self.geometria = struct.pack('<BB4h', 3, tipo, 0, 0, 32767, 32767)
                self.buffer_geometria = (ctypes.c_ubyte * len(self.geometria)).from_buffer_copy(
                    self.geometria
                )
                guardar(d, 0x0DF0, 1)
                for canal in (0x2C3B, 0x2C40, 0x2C45):
                    guardar(d, canal, 0)
                d[0x2C2C] = fotograma
                d[0x3594] = d[0x3595] = d[0x3596] = 1
                salida = self.paso(d, self.semilla_menu)
                grupo = (
                    0 if fotograma <= 8 else 1 if fotograma <= 17 else 2 if fotograma <= 26 else 3
                )
                esperados = (0x9678, 0x968C, 0x9697) if tipo == 11 else CANALES[grupo]
                if (
                    salida is not None
                    or tuple(palabra(d, c) for c in (0x2C3B, 0x2C40, 0x2C45)) != esperados
                ):
                    raise ErrorAnalisis(f'Contrato{tipo}, imagen{fotograma}: canales inesperados.')
                intervalos = tuple(d[c] for c in (0x2C3F, 0x2C44, 0x2C49))
                if intervalos != ((8, 8, 8) if tipo == 9 else (2, 2, 2)):
                    raise ErrorAnalisis(f'Intervalos distintos en contrato{tipo}/{fotograma}.')
                if tipo == 11 and (palabra(d, 0x2C5C) != 0x1A2F or d[0x2C60] != 50):
                    raise ErrorAnalisis('La caida11 no prepara el canal del actor como el EXE.')
                clave = f'{tipo}:{grupo}'
                grupos[clave] = grupos.get(clave, 0) + 1
        return dict(
            casos=768,
            grupos=grupos,
            contexto=contexto,
            limite='Fixture aislada: modifica caja de colision, fotograma y canales; no acredita acceso jugable ni recorre 768 caidas.',
        )

    def caida_real(self):
        d, contexto = self.cargar('cementerio_regreso')
        inventario = bytes(d[0x5225:0x523D])
        encontrados = []
        primera_reaccion = None
        salida = None
        for n in range(768):
            salida = self.andar(d, self.semilla_menu, 8, 1)
            canales = tuple(palabra(d, c) for c in (0x2C3B, 0x2C40, 0x2C45))
            if canales[0] in [c[0] for c in CANALES] + [0x9678] and canales not in encontrados:
                encontrados.append(canales)
                if primera_reaccion is None:
                    primera_reaccion = dict(
                        fotograma=d[0x2C2C], intervalos=[d[p] for p in (0x2C3F, 0x2C44, 0x2C49)]
                    )
            if salida is not None:
                break
        if salida != 113 or not encontrados:
            raise ErrorAnalisis(f'Caida real sin recuperacion113: {salida}, {encontrados}.')
        pasos = n + 1
        self.cambiar(d, self.semilla_menu, salida)
        for espera in range(512):
            if self.andar(d, self.semilla_menu, 65535, 1) is not None:
                raise ErrorAnalisis('Otra salida durante la recuperacion113.')
            if d[0x3594] and d[0x3595]:
                break
        else:
            raise ErrorAnalisis('La recuperacion no devuelve el control.')
        self.esperar(d, 128)
        if palabra(d, 0x8082) != 65 or bytes(d[0x5225:0x523D]) != inventario:
            raise ErrorAnalisis('La recuperacion cambia el destino65 o el inventario.')
        nombre = 'cementerio_caida_recuperada'
        (self.destino / (nombre + '.ds.bin')).write_bytes(d)
        (self.destino / (nombre + '.semilla.json')).write_text(
            json.dumps(list(self.semilla_menu)) + '\n', encoding='utf-8'
        )
        self.conservar_contexto(self.destino, nombre)
        recuperacion = dict(
            posicion=[palabra(d, 0x2C2D), palabra(d, 0x2C2F)],
            camara=palabra(d, 0x3591),
            fase=(d[0x35CA] >> 3) & 7,
        )
        ComparacionRegresoLoro.ruta(self, d, ((280, 195), (450, 195), (550, 178), (606, 195)), 114)
        if palabra(d, 0x8082) != 77 or bytes(d[0x5225:0x523D]) != inventario:
            raise ErrorAnalisis('No se conserva la continuidad114 tras recuperarse.')
        return dict(
            primera_reaccion=primera_reaccion,
            recuperacion=recuperacion,
            continuacion=114,
            escena_final=77,
            pasos_hasta113=pasos,
            espera_control=espera + 1,
            canales=encontrados,
            contexto=contexto,
            posicion=[palabra(d, 0x2C2D), palabra(d, 0x2C2F)],
            camara=palabra(d, 0x3591),
            fase=(d[0x35CA] >> 3) & 7,
        )

    def ejecutar(self):
        self.recursos_menu = True
        self.resumen = {}
        self.destino = self.raiz / 'apps/juego/trabajo/comparacion_tropiezos_cementerio'
        self.destino.mkdir(parents=True, exist_ok=True)
        contratos = self.contratos()
        print('Contratos', contratos, flush=True)
        caida = self.caida_real()
        informe = dict(resultado='coinciden', contratos=contratos, caida=caida, grupos=self.resumen)
        (self.destino / 'resultado.json').write_text(
            json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8'
        )
        print(json.dumps(informe, ensure_ascii=False), flush=True)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--biblioteca', type=Path, required=True)
    args = parser.parse_args()
    try:
        ComparacionTropiezos(Path(__file__).resolve().parents[4], args.biblioteca, True).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        parser.exit(1, f'Error: {error}\n')
