"""Contrasta la visita real al probador y variantes aisladas del disfraz frente al EXE."""

import argparse
import ctypes
import json
from pathlib import Path

from comparar_tiendas import ComparacionTiendas, ErrorAnalisis
from comparar_objetos import guardar, palabra


class ComparacionProbador(ComparacionTiendas):
    # Cada instancia conserva su propio CS:1BA2, sin un estado global oculto en la biblioteca.
    def __init__(self, raiz, biblioteca, muestra):
        super().__init__(raiz, biblioteca, muestra)
        self.fase = ctypes.c_ubyte(0)
        self.nativa.avanzar_probador.argtypes = self.nativa.avanzar_jugabilidad.argtypes + [
            ctypes.POINTER(ctypes.c_ubyte),
            ctypes.c_bool,
        ]
        self.nativa.avanzar_probador.restype = ctypes.c_int
        self.nativa.avanzar_jugabilidad = self.avanzar_nativo

    # Conserva la firma del paso compartido y aporta explícitamente el estado externo del probador.
    def avanzar_nativo(self, *argumentos):
        return self.nativa.avanzar_probador(*argumentos, ctypes.byref(self.fase), False)

    # Comprueba también el byte de código original después de cada paso completo.
    def paso(self, datos, semilla, direccion=65535):
        self.maquina.mem_write(0x1BA2, bytes([self.fase.value]))
        self.maquina.ctl_flush_tb()
        salida = super().paso(datos, semilla, direccion)
        if self.maquina.mem_read(0x1BA2, 1)[0] != self.fase.value:
            raise ErrorAnalisis('Fase CS:1BA2 distinta después del paso integrado.')
        if getattr(self, 'rastrear_vestido', False):
            self.fotogramas_vestido.add((datos[0x358C], datos[0x2C2C], self.fase.value))
        return salida

    # Revisa 1A74 en todas las puertas de emisión, con registros libres y agotados.
    def variantes_rutina(self, base):
        for modelo in (255, 19):
            for fotograma in (0, 1, 2, 3, 4):
                for fase in (0, 1):
                    for pose in (0, 12):
                        for lleno in (False, True):
                            d = bytearray(base)
                            d[0x358C], d[0x2C2C], d[0x2C4D] = modelo, fotograma, pose
                            if lleno:
                                for registro in range(0x2C4C, 0x3510, 33):
                                    d[registro] = 1
                            self.fase.value = fase
                            sem = (ctypes.c_ushort * 4)(*self.semilla_menu)
                            self.retomar_recursos(d)
                            self.disponer(d, self.geometria, sem)
                            self.maquina.mem_write(0x1BA2, bytes([fase]))
                            self.maquina.ctl_flush_tb()
                            self.ejecutar_original(0x1A74)
                            esperado = bytes(self.maquina.mem_read(self.segmento, 65536))
                            azar = bytes(self.maquina.mem_read(0x78DE, 8))
                            resultado = self.nativa.avanzar_probador(
                                (ctypes.c_ubyte * 65536).from_buffer(d),
                                sem,
                                self.callback_dimensiones,
                                self.callback_modelo,
                                self.buffer_geometria,
                                len(self.geometria),
                                65535,
                                ctypes.byref(self.fase),
                                True,
                            )
                            if (
                                resultado != 0
                                or d != esperado
                                or bytes(sem) != azar
                                or self.fase.value != self.maquina.mem_read(0x1BA2, 1)[0]
                            ):
                                diferencias = [
                                    f'{i:04X}:{d[i]:02X}/{esperado[i]:02X}'
                                    for i in range(65536)
                                    if d[i] != esperado[i]
                                ][:8]
                                raise ErrorAnalisis(
                                    f'Rutina del probador distinta: {modelo, fotograma, fase, pose, lleno}; {diferencias}; retorno {resultado}.'
                                )
                            self.resumen['variantes_rutina'] = (
                                self.resumen.get('variantes_rutina', 0) + 1
                            )
        self.fase.value = 0
        self.retomar_recursos(base)

    # Revisa la recogida, el inventario lleno y el rechazo de vestir fuera de CHANGE.
    def variantes_vestido(self, tienda, probador):
        for base in (tienda, probador):
            for ocupadas in (0, 9, 10):
                for puntos in (0, 65535):
                    d = bytearray(base)
                    for i in range(10):
                        guardar(d, 0x5225 + i * 2, 79 if i == 0 else 3 if i < ocupadas else 65535)
                    guardar(d, 0x35C7, puntos)
                    guardar(d, 0xB60C, 0x3B53)
                    guardar(d, 0xB620, 79 + 26)
                    guardar(d, 0xB634, 4)
                    guardar(d, 0xB638, 65535)
                    self.retomar_recursos(d)
                    self.seleccionar(d, 0)
        for ocupadas in (0, 9, 10):
            for icono in (0x1F, 0x22, 0):
                for puntos in (0, 65535):
                    d = bytearray(tienda)
                    for i in range(10):
                        guardar(d, 0x5225 + i * 2, 3 if i < ocupadas else 65535)
                    d[0x524E + 79 * 2] = icono
                    guardar(d, 0x35C7, puntos)
                    guardar(d, 0xB60C, 0x563A)
                    guardar(d, 0xB620, 79 + 26)
                    self.retomar_recursos(d)
                    self.seleccionar(d, 0)
        self.retomar_recursos(probador)

    # Variante aislada: comprueba la animación posterior a vestir, sin atribuirla a la visita temprana.
    def secuencia_vestido(self, base):
        semilla = list(self.semilla_menu)
        d = bytearray(base)
        try:
            guardar(d, 0x5225, 79)
            guardar(d, 0xB60C, 0x3B53)
            guardar(d, 0xB620, 79 + 26)
            guardar(d, 0xB634, 4)
            guardar(d, 0xB638, 65535)
            self.fase.value = 0
            self.retomar_recursos(d)
            self.seleccionar(d, 0)
            self.fotogramas_vestido = set()
            self.rastrear_vestido = True
            salida = self.andar(d, self.semilla_menu, 65535, 512)
            if not any(modelo == 19 for modelo, _, _ in self.fotogramas_vestido) or not d[0x3594]:
                raise ErrorAnalisis('El vestido no alcanza CHANGE.BRD y devuelve el control.')
            if salida is not None:
                raise ErrorAnalisis(f'Continuación inesperada del vestido: {salida}.')
            (self.destino / 'variante_vestido_final.ds.bin').write_bytes(d)
            self.resumen['secuencias_vestido'] = 1
            self.salida_vestido = salida
        finally:
            self.rastrear_vestido = False
            self.semilla_menu[:] = semilla
            self.fase.value = 0
            self.retomar_recursos(base)

    # La visita temprana no fabrica el vestido ni los avances del valle necesarios para su puzle.
    def ejecutar(self):
        origen = self.raiz / 'apps/juego/trabajo/comparacion_tiendas/tienda_35'
        if not origen.with_suffix('.ds.bin').exists():
            ComparacionTiendas(self.raiz, self.biblioteca, True).ejecutar()
        tienda = bytearray(origen.with_suffix('.ds.bin').read_bytes())
        d = bytearray(tienda)
        self.semilla_menu[:] = json.loads(origen.with_suffix('.semilla.json').read_text())
        self.destino = self.raiz / 'apps/juego/trabajo/comparacion_probador'
        self.destino.mkdir(parents=True, exist_ok=True)
        self.resumen = {}
        sem = self.semilla_menu
        self.retomar_recursos(d)
        encontrada = None
        for x, y in ((140, 160), (100, 155), (77, 135)):
            encontrada = self.ir(d, sem, x, y)
            if encontrada is not None:
                break
        if encontrada != 36:
            raise ErrorAnalisis(
                f'Entrada del probador: {encontrada}; posición {palabra(d, 0x2C2D)},{palabra(d, 0x2C2F)}.'
            )
        self.cambiar(d, sem, 36)
        self.andar(d, sem, 65535, 128)
        self.hito_tienda(d, 'probador_entrada')
        self.variantes_rutina(d)
        self.variantes_vestido(tienda, d)
        self.secuencia_vestido(d)
        encontrada = None
        for x, y in ((160, 160), (200, 145)):
            encontrada = self.ir(d, sem, x, y)
            if encontrada is not None:
                break
        if encontrada != 27:
            raise ErrorAnalisis(
                f'Regreso del probador: {encontrada}; posición {palabra(d, 0x2C2D)},{palabra(d, 0x2C2F)}.'
            )
        self.cambiar(d, sem, 27)
        self.andar(d, sem, 65535, 128)
        self.hito_tienda(d, 'probador_regreso')
        informe = {
            'resultado': 'coinciden',
            'casos_y_pasos': sum(self.resumen.values()),
            'grupos': self.resumen,
            'estados_auditados': self.estados_auditados,
            'capturas': self.capturas,
            'salida_variante_vestido': self.salida_vestido,
        }
        (self.destino / 'resultado.json').write_text(
            json.dumps(informe, ensure_ascii=False, indent=2) + '\n', encoding='utf-8'
        )
        print(
            f'Probador: {informe["casos_y_pasos"]} casos/pasos y {self.capturas} composiciones coincidentes.'
        )


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Compara el probador y los contratos del vestido.')
    parser.add_argument('--biblioteca', type=Path, required=True)
    parser.add_argument('--muestra', action='store_true')
    opciones = parser.parse_args()
    try:
        ComparacionProbador(
            Path(__file__).resolve().parents[3], opciones.biblioteca, opciones.muestra
        ).ejecutar()
    except (OSError, ErrorAnalisis) as error:
        parser.exit(1, f'Error: {error}\n')
