import { readFileSync, writeFileSync } from 'node:fs';
import { dirname, join, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { huella, listarArchivos } from '../distribucion/archivos.mjs';

// Registro explícito de maestros gráficos: actualizar después de revisar un cambio de imagen.
export function actualizarHuellas(raiz) {
  const assets = join(raiz, 'packages/assets');
  const archivos = {};
  for (const grupo of ['lanzador', 'marcos', 'original/decorados'])
    for (const archivo of listarArchivos(join(assets, grupo)).filter((ruta) =>
      ruta.endsWith('.png'),
    ))
      archivos[`${grupo}/${archivo}`] = huella(readFileSync(join(assets, grupo, archivo)));
  writeFileSync(
    join(assets, 'huellas.json'),
    JSON.stringify({ version: 1, imagenes: archivos }, null, 2) + '\n',
  );
}
if (
  typeof process !== 'undefined' &&
  process.argv[1] &&
  resolve(process.argv[1]) === fileURLToPath(import.meta.url)
)
  actualizarHuellas(resolve(dirname(fileURLToPath(import.meta.url)), '../../../..'));
