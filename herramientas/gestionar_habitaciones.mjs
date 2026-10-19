/** Prepara las habitaciones propias desde sus metadatos, sin incorporar recursos DOS. */
import { readFileSync, writeFileSync, mkdirSync, copyFileSync } from 'node:fs';
import { join } from 'node:path';

// Comprueba cada imagen y cada zona antes de generar el catálogo usado por SDL.
export function prepararHabitaciones(raiz, destino) {
  const origen = join(raiz, 'assets/original/habitaciones');
  const carpeta = join(destino, 'assets/original/habitaciones');
  const catalogo = JSON.parse(readFileSync(join(origen, 'habitaciones.json'), 'utf8'));
  const variantes = [],
    habitaciones = [],
    ids = new Set();
  if (catalogo.version !== 1 || !Array.isArray(catalogo.habitaciones))
    throw new Error('Versión o catálogo de habitaciones inválidos.');
  for (const h of catalogo.habitaciones) {
    if (
      !/^[a-z0-9_-]+$/.test(h.id) ||
      !/^[a-z0-9_-]+$/.test(h.carpeta) ||
      ids.has(h.id) ||
      !h.variantes?.length
    )
      throw new Error('Identificador, carpeta o variantes de habitación inválidos.');
    ids.add(h.id);
    const inicio = variantes.length,
      proporciones = new Set();
    for (const v of h.variantes) {
      if (!/^[a-z0-9_-]+\.png$/.test(v.archivo)) throw new Error('Nombre de imagen inválido.');
      const png = readFileSync(join(origen, h.carpeta, v.archivo));
      const [ancho, alto] = v.resolucion;
      if (
        png.subarray(0, 8).toString('hex') !== '89504e470d0a1a0a' ||
        png.readUInt32BE(16) !== ancho ||
        png.readUInt32BE(20) !== alto ||
        proporciones.has(ancho / alto)
      )
        throw new Error('Imagen o proporción duplicada en ' + v.archivo);
      proporciones.add(ancho / alto);
      const zonas = ['pantalla', 'botonPC', 'botonMonitor', 'pilotoPC', 'pilotoMonitor'].map(
        (nombre) => {
          const r = v[nombre];
          if (
            !Array.isArray(r) ||
            r.length !== 4 ||
            !r.every(Number.isInteger) ||
            r[0] < 0 ||
            r[1] < 0 ||
            r[2] <= 0 ||
            r[3] <= 0 ||
            r[0] + r[2] > ancho ||
            r[1] + r[3] > alto
          )
            throw new Error('Zona inválida: ' + v.archivo + '/' + nombre);
          return `{${r.join(',')}}`;
        },
      );
      variantes.push(
        `{${JSON.stringify(`assets/original/habitaciones/${h.carpeta}/${v.archivo}`)},${ancho},${alto},${zonas.join(',')},${v.botonPCAuxiliar === true}}`,
      );
      mkdirSync(join(carpeta, h.carpeta), { recursive: true });
      copyFileSync(join(origen, h.carpeta, v.archivo), join(carpeta, h.carpeta, v.archivo));
    }
    habitaciones.push(
      `{${JSON.stringify(h.id)},${JSON.stringify(h.nombre)},${inicio},${variantes.length - inicio}}`,
    );
  }
  const inicial = catalogo.habitaciones.findIndex((h) => h.id === catalogo.inicial);
  if (inicial < 0) throw new Error('No existe la habitación inicial.');
  mkdirSync(carpeta, { recursive: true });
  copyFileSync(join(origen, 'habitaciones.json'), join(carpeta, 'habitaciones.json'));
  writeFileSync(
    join(destino, 'catalogo_habitaciones.hpp'),
    `// Generado desde habitaciones.json; no editar.\n#pragma once\ninline constexpr VarianteHabitacion variantes_habitaciones[] = {${variantes.join(',\n')}};\ninline constexpr TemaHabitacion temas_habitaciones[] = {${habitaciones.join(',\n')}};\ninline constexpr unsigned int habitacion_inicial = ${inicial};\n`,
  );
}
