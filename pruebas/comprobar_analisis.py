"""Pruebas con ejecutables sintéticos; no requieren archivos del juego."""

from pathlib import Path
import struct
import sys
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'herramientas'))
from analizar_ejecutable import ErrorAnalisis, leer_mapa, leer_mz, seguir_flujo


def crear_mz(imagen, reubicaciones=()):
    cabecera = max(32, ((28 + len(reubicaciones) * 4 + 15) // 16) * 16)
    tamano = cabecera + len(imagen)
    contenido = bytearray(cabecera) + imagen
    struct.pack_into('<14H', contenido, 0, 0x5A4D, tamano % 512,
                     (tamano + 511) // 512, len(reubicaciones), cabecera // 16,
                     0, 65535, 0, 0, 0, 0, 0, 28, 0)
    for indice, (desplazamiento, segmento) in enumerate(reubicaciones):
        struct.pack_into('<HH', contenido, 28 + indice * 4, desplazamiento, segmento)
    return contenido


class ComprobarMZ(unittest.TestCase):
    def test_separa_cabecera_imagen_y_datos_adicionales(self):
        contenido = crear_mz(b'\xc3')
        cabecera, imagen = leer_mz(contenido + b'fuera')
        self.assertEqual(imagen, b'\xc3')
        self.assertEqual(cabecera['bytes_adicionales'], 5)

    def test_detecta_archivo_truncado(self):
        with self.assertRaises(ErrorAnalisis):
            leer_mz(crear_mz(b'\xc3')[:-1])

    def test_rechaza_reubicacion_fuera_de_imagen(self):
        with self.assertRaises(ErrorAnalisis):
            leer_mz(crear_mz(b'\xc3', [(0, 0)]))

    def test_lee_reubicacion_sin_modificar_original(self):
        contenido = crear_mz(b'\x31\x0f\xc3', [(0, 0)])
        cabecera, imagen = leer_mz(contenido)
        self.assertEqual(cabecera['reubicaciones'][0]['valor_sin_reubicar'], 0xF31)
        self.assertEqual(imagen[:2], b'\x31\x0f')

    def test_rechaza_entrada_fuera_de_imagen(self):
        contenido = crear_mz(b'\xc3')
        struct.pack_into('<H', contenido, 20, 1)
        with self.assertRaises(ErrorAnalisis):
            leer_mz(contenido)


class ComprobarFlujo(unittest.TestCase):
    def test_omite_datos_tras_salto_y_retorno(self):
        flujo = seguir_flujo(b'\xeb\x04\x11\x22\x33\x44\xc3\x90', [(0, 0)])
        self.assertEqual([i['desplazamiento'] for i in flujo['instrucciones']], [0, 6])

    def test_envuelve_destino_relativo_a_16_bits(self):
        imagen = bytearray(65536)
        imagen[:4] = b'\xe8\xed\xff\xc3'
        imagen[0xFFF0] = 0xC3
        flujo = seguir_flujo(imagen, [(0, 0)])
        self.assertEqual(flujo['destinos_llamada'], ['0000:FFF0'])
        self.assertEqual(len(flujo['instrucciones']), 3)

    def test_se_detiene_en_salto_indirecto(self):
        flujo = seguir_flujo(b'\xff\xe0\x90', [(0, 0)])
        self.assertEqual(len(flujo['instrucciones']), 1)
        self.assertIsNone(flujo['enlaces'][0]['destino'])

    def test_sigue_destino_de_bucle_y_su_continuacion(self):
        flujo = seguir_flujo(b'\xe2\x01\xc3\xc3', [(0, 0)])
        self.assertEqual(flujo['enlaces'][0]['destino'], '0000:0003')
        self.assertEqual(len(flujo['instrucciones']), 3)

    def test_llamada_indirecta_conserva_continuacion_provisional(self):
        flujo = seguir_flujo(b'\xff\xd0\xc3', [(0, 0)])
        self.assertEqual(len(flujo['instrucciones']), 2)
        self.assertEqual(len(flujo['incidencias']), 1)

    def test_informa_destino_en_mitad_de_instruccion(self):
        flujo = seguir_flujo(b'\x75\xff\xc3', [(0, 0)])
        self.assertTrue(any('dentro' in i['motivo'] for i in flujo['incidencias']))

    def test_salida_dos_no_continua_hacia_datos(self):
        flujo = seguir_flujo(b'\xb8\x00\x4c\xcd\x21\x90', [(0, 0)])
        self.assertEqual(len(flujo['instrucciones']), 2)

    def test_limite_impide_presentar_analisis_incompleto(self):
        with self.assertRaises(ErrorAnalisis):
            seguir_flujo(b'\x90\xc3', [(0, 0)], limite=1)


class ComprobarMapa(unittest.TestCase):
    def setUp(self):
        # Salto indirecto, dos punteros intercalados y dos destinos con RET.
        self.imagen = bytes.fromhex('ff 26 04 00 08 00 0a 00 c3 90 c3')
        self.cabecera = {'sha256': 'huella-de-prueba'}
        self.mapa = {
            'sha256': 'huella-de-prueba',
            'raices': ['0000:0000'],
            'tablas': [{'nombre': 'Prueba', 'origen': '0000:0000',
                        'ubicacion': '0000:0004', 'cantidad': 2,
                        'segmento_destino': 0, 'omitir_cero': False}],
        }

    def test_resuelve_salto_sin_interpretar_la_tabla_como_codigo(self):
        raices, tablas, resoluciones = leer_mapa(self.mapa, self.cabecera, self.imagen)
        flujo = seguir_flujo(self.imagen, raices, resoluciones=resoluciones)
        self.assertEqual([i['desplazamiento'] for i in flujo['instrucciones']], [0, 8, 10])
        self.assertEqual(flujo['incidencias'], [])
        self.assertEqual(tablas[0]['entradas'][1]['destino'], '0000:000A')

    def test_rechaza_mapa_de_otra_version(self):
        with self.assertRaises(ErrorAnalisis):
            leer_mapa(self.mapa, {'sha256': 'otra-huella'}, self.imagen)

    def test_rechaza_tabla_truncada(self):
        self.mapa['tablas'][0]['cantidad'] = 100
        with self.assertRaises(ErrorAnalisis):
            leer_mapa(self.mapa, self.cabecera, self.imagen)

    def test_rechaza_destino_fuera_de_la_imagen(self):
        imagen = self.imagen[:4] + bytes.fromhex('ff ff') + self.imagen[6:]
        with self.assertRaises(ErrorAnalisis):
            leer_mapa(self.mapa, self.cabecera, imagen)

    def test_cero_omitido_no_se_convierte_en_entrada_de_codigo(self):
        self.mapa['tablas'][0]['omitir_cero'] = True
        imagen = self.imagen[:4] + b'\0\0' + self.imagen[6:]
        _, tablas, resoluciones = leer_mapa(self.mapa, self.cabecera, imagen)
        self.assertIsNone(tablas[0]['entradas'][0]['destino'])
        self.assertIsNone(resoluciones[0, 0][0]['destino'])


if __name__ == '__main__':
    unittest.main()
