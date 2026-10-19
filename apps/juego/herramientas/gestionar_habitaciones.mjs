import { rutaAssets } from './rutas_proyecto.mjs';
/** Prepara las habitaciones propias desde sus metadatos, sin incorporar recursos DOS. */
import { readFileSync, writeFileSync, mkdirSync, copyFileSync } from 'node:fs';
import { join } from 'node:path';

// Comprueba cada imagen y cada zona antes de generar el catálogo usado por SDL.
export function prepararHabitaciones(raiz, destino) {
  const origen = rutaAssets(raiz, 'lanzador/habitaciones');
  const carpeta = join(destino, 'assets/lanzador/habitaciones');
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
      const contorno = v.contornoCristal;
      if (
        !Array.isArray(contorno) ||
        contorno.length !== 16 ||
        contorno.some(
          (p) =>
            !Array.isArray(p) ||
            p.length !== 2 ||
            !p.every(Number.isInteger) ||
            p[0] < 0 ||
            p[1] < 0 ||
            p[0] >= ancho ||
            p[1] >= alto,
        )
      )
        throw new Error('Contorno del cristal inválido en ' + v.archivo);
      // La máscara por filas admite contornos convexos; rechazar formas cruzadas o sin área.
      let sentido = 0;
      for (let i = 0; i < contorno.length; ++i) {
        const a = contorno[i],
          b = contorno[(i + 1) % 16],
          c = contorno[(i + 2) % 16];
        const giro = Math.sign((b[0] - a[0]) * (c[1] - b[1]) - (b[1] - a[1]) * (c[0] - b[0]));
        if (giro && sentido && giro !== sentido)
          throw new Error('El cristal debe ser convexo: ' + v.archivo);
        if (giro) sentido = giro;
      }
      if (!sentido) throw new Error('El cristal no tiene superficie: ' + v.archivo);
      variantes.push(
        `{${JSON.stringify(`assets/lanzador/habitaciones/${h.carpeta}/${v.archivo}`)},${ancho},${alto},${zonas.join(',')},${v.botonPCAuxiliar === true},{${contorno.map((p) => `{${p.join(',')}}`).join(',')}}}`,
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
