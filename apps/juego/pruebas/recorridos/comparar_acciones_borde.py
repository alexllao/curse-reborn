"""Contrasta primeros puzles de EDGE y variantes aisladas de sus manejadores."""

import argparse
import json
from pathlib import Path

from comparar_recorrido_borde import ComparacionRecorridoBorde, ErrorAnalisis
from comparar_objetos import guardar, palabra


class ComparacionAccionesBorde(ComparacionRecorridoBorde):
    # Ejecuta cada selección con el menú original y clasifica los contratos comprobados.
    def variante(self, base, grupo, manejador, id, verbo=0, cambios=()):
        d = bytearray(base)
        for posicion, valor, byte in cambios:
            if byte:
                d[posicion] = valor
            else:
                guardar(d, posicion, valor)
        guardar(d, 0xB60C, manejador)
        guardar(d, 0xB620, id + (23 if manejador == 0x32DE else 26))
        guardar(d, 0xB634, verbo)
        guardar(d, 0xB638, 65535)
        self.seleccionar(d, 0)
        self.variantes[grupo] = self.variantes.get(grupo, 0) + 1

    # Contrasta capacidad, selección del modelo y puntos envolventes de las tres recogidas.
    def variantes_recogidas(self, base):
        for id in (86, 87, 95):
            for ocupadas in (0, 9, 10):
                for modelo in (20, 255):
                    for icono in (0, 0x1F, 0x22):
                        for puntos in (0, 65535):
                            cambios = [
                                (0x358C, modelo, True),
                                (0x524E + id * 2, icono, True),
                                (0x35C7, puntos, False),
                            ]
                            cambios += [
                                (0x5225 + i * 2, 3 if i < ocupadas else 65535, False)
                                for i in range(10)
                            ]
                            self.variante(base, 'recogidas', 0x563A, id, cambios=cambios)

    # Comprueba las distancias exactas y las diferencias entre usar guantes y llevarlos.
    def variantes_ropa(self, base):
        for distancia in (-32768, -41, -31, -30, -1, 0, 30, 31, 40, 41, 32767):
            for camara in (0, 640):
                for puntos in (0, 65535):
                    for manejador, verbo in ((0x2FCA, 0), (0x3B53, 4)):
                        cambios = [
                            (0x2C4E, palabra(base, 0x2C2D) + distancia, False),
                            (0x3591, camara, False),
                            (0x35C7, puntos, False),
                            (0x5225, 86, False),
                        ]
                        self.variante(base, 'guantes', manejador, 86, verbo, cambios)
                    cambios = [
                        (0x2DFB, palabra(base, 0x2C2D) + distancia, False),
                        (0x3591, camara, False),
                        (0x35C7, puntos, False),
                        (0x5225, 87, False),
                    ]
                    self.variante(base, 'caramelo', 0x3B53, 87, 4, cambios)

    # Los cuatro botones deben conservar todos los bits ajenos y distinguir su fotograma actual.
    def variantes_botones(self, base):
        for id in range(88, 92):
            entidad = 0x2D33 + (id - 88) * 33
            for bandera in range(256):
                for imagen in (20, 21):
                    self.variante(
                        base,
                        'botones',
                        0x3B53,
                        id,
                        2,
                        ((0x35C9, bandera, True), (entidad + 1, imagen, True)),
                    )
        for altura in (75, 76, 77):
            for camara in (0, 256):
                for puntos in (0, 65535):
                    cambios = (
                        (0x2CB3, altura, False),
                        (0x3591, camara, False),
                        (0x35C7, puntos, False),
                    )
                    self.variante(base, 'roca_empujar', 0x3B53, 92, 2, cambios)
                    self.variante(base, 'roca_saltar', 0x3143, 92, 0, cambios)

    # Contrasta distancias de palabra, creación agotada y temas de la segunda mitad del acantilado.
    def variantes_final(self, base):
        semilla = list(self.semilla_menu)
        for distancia in (-41, -40, -31, -30, -1, 0, 30, 31, 40, 41):
            for camara in (0, 1024):
                for puntos in (0, 65535):
                    for id, origen, manejador, verbo in (
                        (94, 0x551, 0x3B53, 5),
                        (95, 0x61F, 0x3B53, 5),
                        (96, 0x637, 0x3143, 0),
                    ):
                        cambios = (
                            (0x2C2D, origen + distancia - camara, False),
                            (0x3591, camara, False),
                            (0x35C7, puntos, False),
                            (0x5225, id, False),
                        )
                        self.variante(base, f'final_{id}', manejador, id, verbo, cambios)
        for libres in (0, 1, 3, 5):
            for numero in range(4):
                self.semilla_menu[:] = [(n + numero * 7919) & 65535 for n in semilla]
                primera = palabra(base, 0x3588)
                cambios = [(0x2C2D, 1361 - palabra(base, 0x3591), False)]
                cambios += [
                    (0x2C0A + n * 33, 0 if n - primera < libres else 1, True)
                    for n in range(primera, 70)
                ]
                self.variante(base, 'fragmentos_piedra', 0x3B53, 94, 5, cambios)
        for estado in (0, 1, 2, 3):
            for camara in (0, 1792):
                self.variante(
                    base,
                    'inscripcion',
                    0x384A,
                    97,
                    cambios=((0x2F22, estado, True), (0x3591, camara, False)),
                )
                self.variante(base, 'palabras', 0x32DE, 2, cambios=((0x2F22, estado, True),))
        self.semilla_menu[:] = semilla
        self.retomar_recursos(base)

    # Contrasta una caída y su rescate sin modificar la rama principal del recorrido.
    def rescate_persistente(self, base):
        semilla = list(self.semilla_menu)
        d = bytearray(base)
        for direccion, pasos in ((4, 38), (0, 112)):
            if self.andar(d, self.semilla_menu, direccion, pasos) is not None:
                raise ErrorAnalisis('El rescate del pájaro abandona EDGE.')
        if not d[0x3594] or palabra(d, 0x3591) + palabra(d, 0x2C2D) != 175:
            raise ErrorAnalisis('El pájaro no devuelve a Brad al umbral original175.')
        if d[0x2DD8] != 0:
            raise ErrorAnalisis('El obstáculo no desaparece después de provocar la caída.')
        self.hito_borde(d, 'borde_rescate')
        self.semilla_menu[:] = semilla
        self.retomar_recursos(base)

    # Recorre el obstáculo temporizado, recoge los guantes y acciona los cinco botones reales.
    def guantes_y_puente(self, d):
        # En585 se activa la caída sin contacto; esperar permite continuar por el suelo original.
        for direccion, pasos in ((4, 17), (0, 32), (4, 24), (6, 8), (0, 1)):
            if self.andar(d, self.semilla_menu, direccion, pasos) is not None:
                raise ErrorAnalisis('El sendero de los guantes abandona EDGE.')
        if not d[0x3594] or d[0x2DD8]:
            raise ErrorAnalisis('La espera no evita el obstáculo de los guantes.')
        self.auditar(d, 'guantes al alcance')
        self.elegir_objeto(d, (1,), 86)
        if self.andar(d, self.semilla_menu, 65535, 128) is not None:
            raise ErrorAnalisis('Recoger los guantes abandona EDGE.')
        inventario = [palabra(d, 0x5225 + i * 2) for i in range(10)]
        if 86 not in inventario:
            raise ErrorAnalisis('Los guantes no llegan al inventario real.')
        self.hito_borde(d, 'borde_guantes')
        for direccion, pasos in ((4, 8), (0, 1)):
            if self.andar(d, self.semilla_menu, direccion, pasos) is not None:
                raise ErrorAnalisis('La aproximación a los botones abandona EDGE.')
        self.auditar(d, 'botones antes del puente')
        for id, bandera in ((88, 8), (90, 40), (91, 104), (90, 72), (89, 88)):
            self.elegir_objeto(d, (2, 2), id)
            if self.andar(d, self.semilla_menu, 65535, 192) is not None:
                raise ErrorAnalisis('La animación de un botón abandona EDGE.')
            if d[0x35C9] & 0x78 != bandera:
                raise ErrorAnalisis(f'El botón {id} no conserva la combinación original.')
        if d[0x2D12] != 1 or d[0x2D13] != 47:
            raise ErrorAnalisis('Los cinco botones no despliegan el puente original.')
        self.hito_borde(d, 'borde_puente')
        self.auditar(d, 'puente desplegado')

    # Usa el ciclo ADCE real para esperar fuera de la lluvia de rocas, sin escribir su fase.
    def esperar_rocas(self, d):
        for _ in range(182):
            if d[0xADCE] == 180:
                return
            if self.andar(d, self.semilla_menu, 0, 1) is not None:
                raise ErrorAnalisis('La espera del ciclo de rocas abandona EDGE.')
        raise ErrorAnalisis('El temporizador de rocas no inicia un ciclo nuevo.')

    # Conserva entradas, selecciones e imágenes hasta eliminar al personaje eléctrico con los guantes.
    def cuerda_y_electrico(self, d):
        for direccion, pasos in ((4, 100), (0, 1)):
            if self.andar(d, self.semilla_menu, direccion, pasos) is not None:
                raise ErrorAnalisis('El cruce del puente abandona EDGE.')
        self.auditar(d, 'cuerda del acantilado')
        self.elegir_objeto(d, (1,), 95)
        if self.andar(d, self.semilla_menu, 65535, 128) is not None:
            raise ErrorAnalisis('Recoger la cuerda abandona EDGE.')
        self.hito_borde(d, 'borde_cuerda')
        self.esperar_rocas(d)
        for direccion, pasos in ((4, 42), (0, 1)):
            if self.andar(d, self.semilla_menu, direccion, pasos) is not None:
                raise ErrorAnalisis('La aproximación al hombre eléctrico abandona EDGE.')
        self.auditar(d, 'antes del hombre eléctrico')
        self.elegir_objeto(d, (5,), 86)
        if self.andar(d, self.semilla_menu, 65535, 48) is not None:
            raise ErrorAnalisis('Usar los guantes abandona EDGE.')
        if d[0x2C4C] or not d[0x3594]:
            raise ErrorAnalisis(
                'Los guantes no apartan al hombre eléctrico con control restablecido.'
            )
        self.hito_borde(d, 'borde_electrico')

    # Recorre el refugio del caramelo y deja que C093 entregue la piedra durante la animación real.
    def caramelo_y_piedra(self, d):
        if self.andar(d, self.semilla_menu, 8, 8) is not None:
            raise ErrorAnalisis('El regreso al refugio abandona EDGE.')
        self.esperar_rocas(d)
        for direccion, pasos in ((4, 56), (6, 16), (0, 1)):
            if self.andar(d, self.semilla_menu, direccion, pasos) is not None:
                raise ErrorAnalisis('El sendero del caramelo abandona EDGE.')
        self.auditar(d, 'caramelo al alcance')
        self.elegir_objeto(d, (1,), 87)
        if self.andar(d, self.semilla_menu, 65535, 96) is not None:
            raise ErrorAnalisis('Recoger el caramelo abandona EDGE.')
        self.hito_borde(d, 'borde_caramelo')
        self.esperar_rocas(d)
        for direccion, pasos in ((4, 130), (0, 1), (4, 30), (0, 1)):
            if self.andar(d, self.semilla_menu, direccion, pasos) is not None:
                raise ErrorAnalisis('La aproximación a la piedra abandona EDGE.')
        self.auditar(d, 'antes de llevar el caramelo')
        self.elegir_objeto(d, (2, 4), 87)
        if self.andar(d, self.semilla_menu, 65535, 128) is not None:
            raise ErrorAnalisis('Llevar el caramelo abandona EDGE.')
        if 94 not in [palabra(d, 0x5225 + n * 2) for n in range(10)]:
            raise ErrorAnalisis('La animación del caramelo no incorpora la piedra94.')
        self.hito_borde(d, 'borde_caramelo_puesto')
        self.auditar(d, 'piedra pegada al caramelo')
        self.elegir_objeto(d, (2, 5), 94)
        if self.andar(d, self.semilla_menu, 65535, 192) is not None:
            raise ErrorAnalisis('Lanzar la piedra abandona EDGE.')
        self.hito_borde(d, 'borde_roca_grande')

    # Lanza la cuerda, cruza el abismo y abre el paso mediante la inscripción del original.
    def abismo_y_salida(self, d):
        for direccion, pasos in ((4, 100), (0, 1)):
            if self.andar(d, self.semilla_menu, direccion, pasos) is not None:
                raise ErrorAnalisis('La aproximación al abismo abandona EDGE.')
        self.auditar(d, 'antes del abismo')
        self.elegir_objeto(d, (2, 5), 95)
        if self.andar(d, self.semilla_menu, 65535, 128) is not None:
            raise ErrorAnalisis('Lanzar la cuerda abandona EDGE.')
        self.hito_borde(d, 'borde_cuerda_lanzada')
        self.auditar(d, 'cuerda preparada para el salto')
        self.elegir_objeto(d, (6,), 96)
        if self.andar(d, self.semilla_menu, 65535, 192) is not None:
            raise ErrorAnalisis('El salto abandona EDGE antes de llegar al otro lado.')
        self.hito_borde(d, 'borde_abismo_saltado')
        for _ in range(100):
            if palabra(d, 0x3591) + palabra(d, 0x2C2D) >= 1888:
                break
            if self.andar(d, self.semilla_menu, 4, 1) is not None:
                raise ErrorAnalisis('El sendero final abandona EDGE antes del obstáculo.')
        if self.andar(d, self.semilla_menu, 0, 32) is not None or d[0x2EE0] or not d[0x3594]:
            raise ErrorAnalisis('El segundo obstáculo no permite continuar después de esperar.')
        for direccion, pasos in ((4, 100), (0, 1)):
            if self.andar(d, self.semilla_menu, direccion, pasos) is not None:
                raise ErrorAnalisis('La inscripción provoca una salida antes de abrir la puerta.')
        self.auditar(d, 'inscripción al alcance')
        self.elegir_objeto(d, (3,), 97)
        if self.andar(d, self.semilla_menu, 65535, 128) is not None or palabra(d, 0x523D) != 2:
            raise ErrorAnalisis('Examinar la inscripción no habilita el tema2.')
        self.hito_borde(d, 'borde_inscripcion')
        self.auditar(d, 'palabras aprendidas')
        self.menu(d, (4,))
        temas = [palabra(d, 0xB620 + n * 2) for n in range(10)]
        if 25 not in temas:
            raise ErrorAnalisis('Las palabras no figuran en el menú original de temas.')
        self.seleccionar(d, temas.index(25))
        if self.andar(d, self.semilla_menu, 65535, 128) is not None or d[0x2F22] != 1:
            raise ErrorAnalisis('Las palabras no abren la puerta original.')
        self.hito_borde(d, 'borde_puerta_abierta')
        if self.andar(d, self.semilla_menu, 8, 32) is not None:
            raise ErrorAnalisis('La aproximación a la puerta sale antes de subir.')
        salida = self.andar(d, self.semilla_menu, 2, 16)
        if salida != 72:
            raise ErrorAnalisis(f'La puerta solicita {salida}, esperada72.')
        self.hito_borde(d, 'borde_salida')

    # Continúa desde la aproximación real y conserva por separado las variantes de capítulos siguientes.
    def ejecutar(self):
        origen = self.raiz / 'apps/juego/trabajo/comparacion_recorrido_borde/borde_roca'
        if not origen.with_suffix('.ds.bin').exists():
            ComparacionRecorridoBorde(self.raiz, self.biblioteca, True).ejecutar()
        d = bytearray(origen.with_suffix('.ds.bin').read_bytes())
        self.semilla_menu[:] = json.loads(origen.with_suffix('.semilla.json').read_text())
        self.resumen = {}
        self.variantes = {}
        self.destino = self.raiz / 'apps/juego/trabajo/comparacion_acciones_borde'
        self.destino.mkdir(parents=True, exist_ok=True)
        self.retomar_recursos(d)
        self.variantes_recogidas(d)
        self.variantes_ropa(d)
        self.variantes_botones(d)
        self.variantes_final(d)
        self.retomar_recursos(d)
        self.auditar(d, 'primera roca antes de empujar')
        self.elegir_objeto(d, (2, 2), 92)
        if self.andar(d, self.semilla_menu, 65535, 192) is not None:
            raise ErrorAnalisis('Empujar la primera roca abandona EDGE.')
        self.hito_borde(d, 'borde_roca_empujada')
        if palabra(d, 0x2CB3) == 76:
            raise ErrorAnalisis('La animación no desplaza la roca.')
        self.auditar(d, 'primera roca después de empujar')
        self.elegir_objeto(d, (6,), 92)
        if self.andar(d, self.semilla_menu, 65535, 192) is not None:
            raise ErrorAnalisis('El salto de la primera roca abandona EDGE.')
        self.hito_borde(d, 'borde_roca_saltada')
        self.rescate_persistente(d)
        self.guantes_y_puente(d)
        self.cuerda_y_electrico(d)
        self.caramelo_y_piedra(d)
        self.abismo_y_salida(d)
        informe = {
            'resultado': 'coinciden',
            'casos_y_pasos': sum(self.resumen.values()),
            'grupos': self.resumen,
            'variantes_aisladas': self.variantes,
            'capturas': self.capturas,
            'estados_auditados': self.estados_auditados,
            'alcance': 'Recorrido real completo de EDGE hasta solicitar salida72, con rescate alternativo y variantes aisladas. No incluye todavía la habitación del loro ni validación del recorrido SDL.',
        }
        (self.destino / 'resultado.json').write_text(
            json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8'
        )
        print(
            f'Primeras acciones de EDGE: {informe["casos_y_pasos"]} casos/pasos y {self.capturas} composiciones coincidentes.'
        )


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Contrasta los primeros contratos de acciones de EDGE.'
    )
    parser.add_argument('--biblioteca', type=Path, required=True)
    parser.add_argument('--muestra', action='store_true')
    opciones = parser.parse_args()
    try:
        ComparacionAccionesBorde(
            Path(__file__).resolve().parents[4], opciones.biblioteca, opciones.muestra
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        parser.exit(1, f'Error: {error}\n')
