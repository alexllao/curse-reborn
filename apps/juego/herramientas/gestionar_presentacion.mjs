import { copyFileSync, mkdirSync, readFileSync, writeFileSync } from 'node:fs';
import { join } from 'node:path';
import { rutaAssets } from './rutas_proyecto.mjs';

// Valida los banners propios y genera el catálogo de arranque sin tocar los PNG maestros.
export function prepararPresentacion(raiz, destino) {
  const origen = rutaAssets(raiz, 'lanzador/presentacion');
  const carpeta = join(destino, 'assets/lanzador/presentacion');
  const c = JSON.parse(readFileSync(join(origen, 'presentacion.json'), 'utf8'));
  if (c.version !== 1 || !Array.isArray(c.variantes) || !c.variantes.length)
    throw new Error('Catálogo de presentación inválido.');
  const tiempos = ['entrada_ms', 'visible_ms', 'salida_ms', 'encendido_ms'];
  for (const campo of tiempos)
    if (!Number.isInteger(c[campo]) || c[campo] < 1 || c[campo] > 30000)
      throw new Error('Duración de presentación inválida: ' + campo);
  mkdirSync(carpeta, { recursive: true });
  const variantes = c.variantes.map((v) => {
    if (!/^[a-z0-9_-]+\.png$/.test(v.archivo)) throw new Error('Nombre de banner inválido.');
    const png = readFileSync(join(origen, v.archivo));
    if (
      png.length < 24 ||
      png.toString('hex', 0, 8) !== '89504e470d0a1a0a' ||
      png.readUInt32BE(16) !== v.ancho ||
      png.readUInt32BE(20) !== v.alto ||
      v.ancho <= 0 ||
      v.alto <= 0
    )
      throw new Error('Dimensiones de banner inválidas.');
    copyFileSync(join(origen, v.archivo), join(carpeta, v.archivo));
    return `{${JSON.stringify('assets/lanzador/presentacion/' + v.archivo)},${v.ancho},${v.alto}}`;
  });
  copyFileSync(join(origen, 'presentacion.json'), join(carpeta, 'presentacion.json'));
  writeFileSync(
    join(destino, 'catalogo_presentacion.hpp'),
    `// Generado desde presentacion.json; no editar.\n#pragma once\ninline constexpr VarianteBanner variantes_banner[] = {${variantes.join(',\n')}};\n` +
      tiempos.map((t) => `inline constexpr Uint64 banner_${t} = ${c[t]}ull;\n`).join(''),
  );
}
